"""SMP mirrors: dependency folders, project sources, XML watches and diagnostics."""
import argparse
from pathlib import Path
import tempfile
import time
from urllib.parse import unquote, urlsplit

from contribution_process_test import Server


def same_uri(first, second):
    first, second = urlsplit(first), urlsplit(second)
    return (first.scheme, first.netloc, unquote(first.path)) == (
        second.scheme, second.netloc, unquote(second.path))


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--lsp', required=True)
    parser.add_argument('--runner', action='append', default=[])
    args = parser.parse_args()
    with tempfile.TemporaryDirectory(prefix='xsmp-smp-workspace-') as temporary:
        root = Path(temporary)
        app, library = root / 'App', root / 'Lib'
        (app / 'src').mkdir(parents=True)
        (library / 'src').mkdir(parents=True)
        (library / 'other').mkdir()
        app_project = app / 'xsmp.project'
        lib_project = library / 'xsmp.project'
        app_project.write_text("project 'App' using 'ECSS_SMP_2025'\nsource 'src'\ndependency 'Lib'\ntool 'smp'\n")
        descriptor = "project 'Lib' using 'ECSS_SMP_2025'\nsource 'src'\n"
        lib_project.write_text(descriptor)
        native_app = app / 'src/app.xsmpcat'
        native_app.write_text('''catalogue App namespace app {
/** @uuid 91a3fc30-33d0-47e8-8b03-71fe958f3abc */
public struct Value { field lib.Shared shared }
}''')

        def xml(name):
            return f'''<Catalogue xmlns="http://www.ecss.nl/smp/2025/Smdl/Catalogue" Name="External" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
<Namespace Name="lib"><Type xsi:type="Types:Structure" Id="opaque-shared-id" Name="{name}" Visibility="public" Uuid="ebd03050-bf02-4199-9cb8-c230d98acc47"/></Namespace></Catalogue>'''

        source = library / 'src/model.smpcat'
        source.write_text(xml('Shared'))
        outside = library / 'ignored.smpcat'
        outside.write_text(xml('OutsideSource'))
        warning_source = library / 'src/plan.smpsed'
        warning_source.write_text('''<Schedule xmlns="http://www.ecss.nl/smp/2025/Smdl/Schedule" Name="Warnings" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:Assembly="http://www.ecss.nl/smp/2025/Smdl/Assembly" xmlns:Schedule="http://www.ecss.nl/smp/2025/Smdl/Schedule"><Parameter Name="Root" xsi:type="Assembly:StringArgument" Value="Root"/><Task Name="Main"><Activity xsi:type="Schedule:Unsupported"/></Task></Schedule>''')
        mirror = source.as_uri().replace('file:', 'xsmp-smp:', 1)
        warning_mirror = warning_source.as_uri().replace('file:', 'xsmp-smp:', 1)
        server = Server([*args.runner, args.lsp, '--stdio'])

        def symbols(name):
            return server.request('workspace/symbol', {'query': name})

        def latest_diagnostics(uri):
            result = None
            for notification in server.notifications:
                params = notification.get('params', {})
                if notification.get('method') == 'textDocument/publishDiagnostics' and same_uri(params.get('uri', ''), uri):
                    result = params.get('diagnostics', [])
            return result

        def errors(uri):
            diagnostics = latest_diagnostics(uri)
            return None if diagnostics is None else [item for item in diagnostics if item.get('severity') == 1]

        def until(predicate, description):
            deadline = time.monotonic() + 45
            while time.monotonic() < deadline:
                symbols('lib.Shared')  # request barrier also collects notifications
                if predicate():
                    return
                time.sleep(0.03)
            raise AssertionError((description, server.notifications[-30:]))

        def content(uri):
            return server.request('workspace/textDocumentContent', {'uri': uri})['text']

        def refreshed(uri):
            return any(value.get('method') == 'workspace/textDocumentContent/refresh' and
                       same_uri(value.get('params', {}).get('uri', ''), uri)
                       for value in server.notifications)

        def watch(file, kind=2):
            server.send('workspace/didChangeWatchedFiles', {'changes': [{'uri': file.as_uri(), 'type': kind}]})

        def folders(added=(), removed=()):
            server.send('workspace/didChangeWorkspaceFolders', {'event': {
                'added': [{'uri': path.as_uri(), 'name': path.name} for path in added],
                'removed': [{'uri': path.as_uri(), 'name': path.name} for path in removed]}})

        try:
            server.request('initialize', {'processId': None, 'rootUri': app.as_uri(),
                'capabilities': {'workspace': {'textDocumentContent': {'dynamicRegistration': True}}},
                'workspaceFolders': [{'uri': app.as_uri(), 'name': 'App'}]})
            server.send('initialized', {})
            until(lambda: bool(errors(native_app.as_uri())), 'App initially misses Lib')
            assert not symbols('lib.Shared')
            folders(added=[library])
            until(lambda: len(symbols('lib.Shared')) == 1 and errors(native_app.as_uri()) == [],
                  'folder add resolves the XML dependency')
            assert not symbols('lib.OutsideSource')
            watch(outside)
            assert not symbols('lib.OutsideSource')
            assert latest_diagnostics(outside.as_uri()) in (None, [])
            assert refreshed(mirror), server.notifications
            assert 'Shared' in content(mirror)
            assert any(item.get('severity') == 2 for item in latest_diagnostics(warning_source.as_uri()) or [])
            outside_mirror = outside.as_uri().replace('file:', 'xsmp-smp:', 1)
            assert content(outside_mirror) == ''
            # Resolve a filename/id from another workspace folder, not from the
            # import's own directory. The opaque ID cannot serve as a QName.
            incoming = app/'incoming/source.smpcat'
            incoming.parent.mkdir()
            incoming.write_text('''<Catalogue xmlns="http://www.ecss.nl/smp/2025/Smdl/Catalogue" Name="Imported"
xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:xlink="http://www.w3.org/1999/xlink">
<Namespace Name="local"><Type xsi:type="Types:Structure" Name="Container" Id="container">
<Field Name="shared"><Type xlink:href="model.smpcat#opaque-shared-id"/></Field>
</Type></Namespace></Catalogue>''')
            imported = server.request('xsmp/importSmpFile', {'uri': incoming.as_uri()})
            assert 'field lib.Shared shared' in Path(imported['outputPath']).read_text(), imported
            assert not imported['warnings'], imported

            lib_project.write_text(descriptor.replace("source 'src'", "source 'other'"))
            watch(lib_project)
            until(lambda: not symbols('lib.Shared') and bool(errors(native_app.as_uri())),
                  'removing Source removes its mirror')
            assert content(mirror) == ''
            lib_project.write_text(descriptor)
            watch(lib_project)
            until(lambda: len(symbols('lib.Shared')) == 1 and errors(native_app.as_uri()) == [],
                  'restoring Source recreates the mirror')

            source.write_text(xml('Renamed'))
            watch(source)
            until(lambda: len(symbols('lib.Renamed')) == 1 and not symbols('lib.Shared')
                  and bool(errors(native_app.as_uri())), 'XML rename relinks consumers')
            source.write_text(xml('Shared'))
            watch(source)
            until(lambda: len(symbols('lib.Shared')) == 1 and errors(native_app.as_uri()) == [],
                  'restoring XML heals consumers')

            sidecar = source.with_suffix('.xsmpcat')
            sidecar.write_text('''catalogue Native namespace lib {
/** @uuid ebd03050-bf02-4199-9cb8-c230d98acc47 */ public struct Shared {}
}''')
            watch(sidecar, 1)
            until(lambda: len(symbols('lib.Shared')) == 1 and
                  same_uri(symbols('lib.Shared')[0]['location']['uri'], sidecar.as_uri()),
                  'native sidecar has priority')
            sidecar.unlink()
            watch(sidecar, 3)
            until(lambda: len(symbols('lib.Shared')) == 1 and
                  same_uri(symbols('lib.Shared')[0]['location']['uri'], mirror),
                  'deleting native sidecar restores XML')

            source.write_text('<Catalogue xmlns="http://www.ecss.nl/smp/2025/Smdl/Catalogue">')
            watch(source)
            until(lambda: bool(errors(source.as_uri())) and not symbols('lib.Shared'),
                  'malformed existing XML removes stale symbols and diagnoses physical source')
            source.write_text(xml('Shared'))
            watch(source)
            until(lambda: errors(source.as_uri()) == [] and len(symbols('lib.Shared')) == 1
                  and errors(native_app.as_uri()) == [], 'healing XML restores symbols and clears errors')
            generated = server.request('xsmp/generateProject', app_project.as_uri())
            latest = {}
            for notification in server.notifications:
                if notification.get('method') == 'textDocument/publishDiagnostics':
                    params = notification['params']
                    latest[params['uri']] = params.get('diagnostics', [])
            assert 'App' in generated['generatedProjects'], {
                'report': generated, 'errors': {uri: findings for uri, findings in latest.items()
                    if any(item.get('severity') == 1 for item in findings)}}
            broken = library / 'src/broken.smpcfg'
            broken.write_text('<Configuration xmlns="http://www.ecss.nl/smp/2025/Smdl/Configuration">')
            watch(broken, 1)
            until(lambda: bool(errors(broken.as_uri())), 'new invalid XML without mirror publishes diagnostics')
            assert errors(native_app.as_uri()) == []
            blocked = server.request('xsmp/generateProject', app_project.as_uri())
            assert not blocked['generatedProjects'], blocked

            folders(removed=[library])
            until(lambda: not symbols('lib.Shared') and latest_diagnostics(broken.as_uri()) == []
                  and latest_diagnostics(warning_source.as_uri()) == [],
                  'folder removal clears mirrors and physical error/warning diagnostics')
            assert content(warning_mirror) == ''
            server.close()
        finally:
            if server.process.poll() is None:
                server.process.kill()
                server.process.wait()
    print('SMP source/folder/watch/sidecar/diagnostic lifecycle passed.')


if __name__ == '__main__':
    main()
