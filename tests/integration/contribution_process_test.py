"""External native/WASI contribution: metadata, prompts and callbacks."""
import argparse
import json
import os
from pathlib import Path
import re
import select
import shutil
import subprocess
import tempfile
import time


class Server:
    def __init__(self, command, libraries=()):
        # As the editor client does: a WASI server loads its libraries as it starts.
        environment = {**os.environ, 'XSMP_EXTENSION_LIBRARIES': '\n'.join(map(str, libraries))}
        self.process = subprocess.Popen(command, stdin=subprocess.PIPE, env=environment,
                                        stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        self.pending = bytearray()
        self.notifications = []
        self.identifier = 0

    def send(self, method, params, identifier=None):
        # A client wraps a value that is no object, as JSON-RPC requires.
        if params is not None and not isinstance(params, (dict, list)):
            params = [params]
        value = {'jsonrpc': '2.0', 'method': method, 'params': params}
        if identifier is not None:
            value['id'] = identifier
        payload = json.dumps(value).encode()
        self.process.stdin.write(f'Content-Length: {len(payload)}\r\n\r\n'.encode() + payload)
        self.process.stdin.flush()

    def request(self, method, params=None):
        self.identifier += 1
        identifier = self.identifier
        self.send(method, params, identifier)
        deadline = time.monotonic() + 45
        while time.monotonic() < deadline:
            value = self.receive(deadline)
            if value.get('id') == identifier:
                assert 'error' not in value, value
                return value.get('result')
            self.notifications.append(value)
        raise AssertionError(f'Timeout: {method}')

    def request_error(self, method, params=None):
        """The error a request is answered with."""
        self.identifier += 1
        identifier = self.identifier
        self.send(method, params, identifier)
        deadline = time.monotonic() + 45
        while time.monotonic() < deadline:
            value = self.receive(deadline)
            if value.get('id') == identifier:
                assert 'error' in value, value
                return value['error']
            self.notifications.append(value)
        raise AssertionError(f'Timeout: {method}')

    def receive(self, deadline):
        while time.monotonic() < deadline:
            while b'\r\n\r\n' in self.pending:
                header, _, body = self.pending.partition(b'\r\n\r\n')
                length = int(next(line.split(b':', 1)[1] for line in header.split(b'\r\n')
                                  if line.lower().startswith(b'content-length:')))
                if len(body) < length:
                    break
                value = json.loads(body[:length])
                del self.pending[:len(header) + 4 + length]
                return value
            if select.select([self.process.stdout], [], [], max(0, deadline-time.monotonic()))[0]:
                data = os.read(self.process.stdout.fileno(), 65536)
                if not data:
                    raise AssertionError(self.process.stderr.read().decode())
                self.pending.extend(data)
        raise AssertionError('Timeout receiving JSON-RPC message')

    def wait_notifications(self, predicate):
        deadline = time.monotonic() + 45
        while not predicate(self.notifications):
            self.notifications.append(self.receive(deadline))

    def close(self):
        self.request('shutdown')
        self.send('exit', None)
        self.process.stdin.close()
        assert self.process.wait(timeout=20) == 0


def saving_again_starts_no_formatter(lsp, root):
    """A server does not format again what a save generates again."""
    tools = root/'formatter'
    tools.mkdir()
    (tools/'fake-clang-format').symlink_to(
        Path(__file__).resolve().parents[1]/'fixtures/clang-format/fake-clang-format')
    project = root/'formatted'
    (project/'src').mkdir(parents=True)
    (project/'xsmp.project').write_text("project 'Formatted' using 'ECSS_SMP_2025'\nprofile 'xsmp-sdk'\nsource 'src'\n")
    (project/'src/model.xsmpcat').write_text("""catalogue Model
namespace demo {
  /** @uuid 1fbd9991-0c38-5c97-803f-bdc038b9f0fb */
  public struct Point { field Smp.Int32 x }
}
""")
    environment = dict(os.environ)
    try:
        os.environ['XSMP_CLANG_FORMAT'] = str(tools/'fake-clang-format')
        server = Server([*lsp, '--stdio'])
    finally:
        os.environ.clear()
        os.environ.update(environment)

    # The formatting runs: the version is asked once per generation.
    def runs():
        return sum(line != '--version' for line in (tools/'runs').read_text().splitlines())

    def generate():
        result = server.request('xsmp/generateProject', (project/'xsmp.project').as_uri())
        assert result['generatedProjects'] == ['Formatted'], result

    generated = project/'src-gen/demo/Point.h'
    server.request('initialize', {'processId': None, 'rootUri': project.as_uri(), 'capabilities': {}})
    server.send('initialized', {})
    generate()
    assert generated.read_text().startswith('// formatted\n')
    first = runs()
    assert first > 1, first
    written = generated.stat()

    generate()
    assert runs() == first
    assert generated.read_text().startswith('// formatted\n')
    again = generated.stat()
    assert (again.st_ino, again.st_mtime_ns) == (written.st_ino, written.st_mtime_ns)

    generated.write_text('edited by hand\n')
    generate()
    assert runs() == first + 1
    assert generated.read_text().startswith('// formatted\n')
    server.close()


def a_generation_goes_to_its_end(lsp, root):
    """Neither an edit nor a cancellation leaves generated files half done."""
    tools = root/'slow-formatter'
    tools.mkdir()
    (tools/'fake-clang-format').symlink_to(
        Path(__file__).resolve().parents[1]/'fixtures/clang-format/fake-clang-format')
    (tools/'slow').touch()
    project = root/'Interrupted'
    (project/'src').mkdir(parents=True)
    (project/'xsmp.project').write_text("project 'Interrupted' using 'ECSS_SMP_2025'\nprofile 'xsmp-sdk'\nsource 'src'\n")
    names = [f'S{index}' for index in range(12)]

    def text(suffix=''):
        return 'catalogue Model\nnamespace demo {\n' + ''.join(
            f'  /** @uuid 00000000-0000-4000-8000-{index:012d} */\n  public struct {name}{suffix} {{ field Smp.Int32 x }}\n'
            for index, name in enumerate(names)) + '}\n'
    source = project/'src/model.xsmpcat'
    source.write_text(text())
    environment = dict(os.environ)
    try:
        os.environ['XSMP_CLANG_FORMAT'] = str(tools/'fake-clang-format')
        server = Server([*lsp, '--stdio'])
    finally:
        os.environ.clear()
        os.environ.update(environment)
    server.request('initialize', {'processId': None, 'rootUri': project.as_uri(), 'capabilities': {}})
    server.send('initialized', {})
    server.send('textDocument/didOpen', {'textDocument': {
        'uri': source.as_uri(), 'languageId': 'xsmpcat', 'version': 1, 'text': text()}})
    server.request('workspace/symbol', {'query': 'S0'})

    def formats():
        runs = tools/'runs'
        return sum(1 for line in runs.read_text().splitlines() if '--version' not in line) if runs.exists() else 0

    def generated():
        return {path.name for path in (project/'src-gen/demo').iterdir()} if (project/'src-gen/demo').is_dir() else set()

    for interruption in ('edit', 'cancel'):
        shutil.rmtree(project/'src-gen', ignore_errors=True)
        before = formats()
        server.identifier += 1
        identifier = server.identifier
        server.send('xsmp/generateProject', source.as_uri(), identifier)
        deadline = time.monotonic() + 30
        while formats() < before + 2 and time.monotonic() < deadline:
            time.sleep(0.005)
        assert formats() >= before + 2, 'the generation did not start'
        if interruption == 'edit':
            server.send('textDocument/didChange', {'textDocument': {'uri': source.as_uri(), 'version': 2},
                                                   'contentChanges': [{'text': text('Edited')}]})
        else:
            server.send('$/cancelRequest', {'id': identifier})
        reply_deadline = time.monotonic() + 60
        while True:
            value = server.receive(reply_deadline)
            if value.get('id') == identifier and 'method' not in value:
                break
            server.notifications.append(value)
        # Every type has its files, each one formatted, and none of the edit.
        files = generated()
        for name in names:
            for file in (f'{name}.h', f'{name}.cpp'):
                assert file in files, (interruption, file, sorted(files))
                assert (project/'src-gen/demo'/file).read_text().startswith('// formatted\n'), (interruption, file)
        assert not any('Edited' in file for file in files), (interruption, sorted(files))
        if interruption == 'edit':
            assert value['result']['generatedProjects'] == ['Interrupted'], value
            # The edit waited for the generation, and it is not lost.
            def edited():
                return 'S0Edited' in str(server.request('textDocument/documentSymbol', {'textDocument': {'uri': source.as_uri()}}))

            edit_deadline = time.monotonic() + 5
            while not edited():
                assert time.monotonic() < edit_deadline, 'the edit was lost'
                time.sleep(0.01)
            server.send('textDocument/didChange', {'textDocument': {'uri': source.as_uri(), 'version': 3},
                                                   'contentChanges': [{'text': text()}]})
            while edited():
                assert time.monotonic() < edit_deadline + 5, 'the second edit was lost'
                time.sleep(0.01)
    server.close()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--cli', required=True)
    parser.add_argument('--lsp', required=True)
    parser.add_argument('--plugin', required=True)
    parser.add_argument('--runner', action='append', default=[])
    args = parser.parse_args()
    with tempfile.TemporaryDirectory(prefix='xsmp-contribution-') as temporary:
        root = Path(temporary)
        extension = root/'extension #?% é'
        (extension/'builtins').mkdir(parents=True)
        plugin = Path(args.plugin)
        handler = extension/('handler.wasm' if plugin.read_bytes()[:4] == b'\0asm' else 'handler')
        shutil.copy2(plugin, handler)
        handler.chmod(0o755)
        (extension/'fake.xsmptool').write_text('tool "fake-tool"\n')
        (extension/'builtins/external.xsmpcat').write_text('catalogue External namespace ext { public struct ExternalType {} }')
        special_builtin = extension/'builtins/a #?% é.xsmpcat'
        special_builtin.write_text('catalogue Extra namespace extra { public struct ExtraType {} }')
        entry = {'descriptor': './fake.xsmptool', 'handler': './'+handler.name,
                 'apiVersion': '^2.0.0', 'aliases': ['fake-tool-alias'],
                 'deprecatedAliases': ['fake-tool-legacy'],
                 'builtins': ['./builtins', './builtins/'+special_builtin.name]}
        (extension/'package.json').write_text(json.dumps({'name': 'fake', 'publisher': 'test',
                                                         'contributes': {'xsmp': [entry]}}))
        foundation = root/'Foundation'
        (foundation/'src').mkdir(parents=True)
        (foundation/'xsmp.project').write_text("project 'Foundation' source 'src'\n")

        def cli(*arguments, success=True):
            result = subprocess.run([*args.runner, args.cli, *map(str, arguments)], capture_output=True,
                                    text=True, timeout=90)
            assert (result.returncode == 0) == success, (arguments, result.stdout, result.stderr)
            return result

        metadata = json.loads((extension/'package.json').read_text())
        metadata['contributes']['xsmp'] = [None,entry]
        (extension/'package.json').write_text(json.dumps(metadata))
        malformed = cli('new','project','MalformedPackage',root,'--extension',extension,'--yes',success=False)
        assert f'{extension} (descriptor): ' in malformed.stderr, malformed.stderr
        assert '{' not in malformed.stderr, malformed.stderr
        assert not (root/'MalformedPackage').exists()
        metadata['contributes']['xsmp'] = [entry]
        (extension/'package.json').write_text(json.dumps(metadata))

        cli('new', 'project', 'Demo', root, '--extension', extension, '--tool', 'fake-tool-alias',
            '--set', 'tool.fake-tool-legacy.moduleName=chosen', '--set', 'tool.fake-tool.enabled=false',
            '--set', 'tool.fake-tool.flavor=second', '--no-interactive')
        project = root/'Demo'
        assert (project/'generated.txt').read_text() == 'chosen'
        assert (project/'enabled.txt').read_text() == 'false'
        assert (project/'flavor.txt').read_text() == 'second'
        descriptor = (project/'xsmp.project').read_text()
        assert "tool 'fake-tool'" in descriptor and "dependency 'Foundation'" in descriptor
        assert 'profile ' not in descriptor
        cli('generate', project, '-w', root, '--extension', extension)
        assert (project/'custom-gen/fake-tool/Demo.txt').is_file()
        if os.name == 'posix':
            for closed in ((0,), (1,), (0, 1), (0, 1, 2)):
                (project/'custom-gen/fake-tool/Demo.txt').unlink()
                def close_streams():
                    for descriptor in closed:
                        try:
                            os.close(descriptor)
                        except OSError:
                            pass
                result = subprocess.run([*args.runner, args.cli, 'generate', str(project),
                                         '-w', str(root), '--extension', str(extension)],
                                        capture_output=True, text=True, timeout=90,
                                        preexec_fn=close_streams)
                assert result.returncode == 0, (closed, result.stdout, result.stderr)
                assert (project/'custom-gen/fake-tool/Demo.txt').is_file(), closed
        (project/'custom-gen/stale.txt').write_text('stale')
        cli('generate', project, '-w', root, '--extension', extension)
        assert not (project/'custom-gen/stale.txt').exists()
        cli('new','project','Invalid',root,'--extension',extension,'--tool','fake-tool',
            '--set','tool.fake-tool.enabled=invalid','--yes',success=False)
        assert not (root/'Invalid').exists()
        cli_validation = root/'Several'
        (cli_validation/'smdl').mkdir(parents=True)
        (cli_validation/'xsmp.project').write_text("project 'Several' source 'smdl' tool 'fake-tool'\n")
        (cli_validation/'smdl/First.xsmpcat').write_text('catalogue First namespace first {}\n')
        (cli_validation/'smdl/Second.xsmpcat').write_text(
            'catalogue Second namespace second { public struct S { field missing.Type value } }\n')
        result = cli('validate', cli_validation, '--extension', extension, success=False)
        # The checks of the contribution reach each document, and those of the languages
        # stay single.
        assert result.stderr.count('Fake contribution validation.') == 2, result.stderr
        assert result.stderr.count('Could not resolve') == 1, result.stderr
        saving_again_starts_no_formatter([*args.runner, args.lsp], root)
        if os.name == 'posix':
            a_generation_goes_to_its_end([*args.runner, args.lsp], root)
        server = Server([*args.runner, args.lsp, '--stdio'], libraries=[handler])
        try:
            initialize = server.request('initialize', {'processId': None, 'rootUri': root.as_uri(), 'capabilities': {}})
            assert initialize['capabilities']['workspace']['workspaceFolders']['changeNotifications'] is True
            server.send('initialized', {})
            initial = server.request('xsmp/getContributionSummaries', 'profile')
            assert all(item['id'] != 'tas-mdk' for item in initial)
            resolved = {**entry, 'extensionId': 'test.fake', 'extensionRoot': str(extension),
                        'descriptorPath': str(extension/'fake.xsmptool'), 'handlerPath': str(handler),
                        'builtins': [str(extension/'builtins'),str(special_builtin)]}
            # An editor reopened before the extension is registered holds placeholders
            # for its documents: the text of the extension replaces them.
            for uri, text in (('xsmp:/extensions/test.fake/builtins/external.xsmpcat', ''),
                              ('xsmp:/extensions/test.fake/fake.xsmptool', "tool 'obsolete'\n")):
                server.send('textDocument/didOpen', {'textDocument': {
                    'uri': uri, 'languageId': 'xsmpcat' if uri.endswith('.xsmpcat') else 'xsmpproject',
                    'version': 1, 'text': text}})
            report = server.request('xsmp/registerContributions', [resolved])
            assert report['registered'][0]['id'] == 'fake-tool' and not report['failures'], report
            restored = server.request('workspace/textDocumentContent',
                                      {'uri': 'xsmp:/extensions/test.fake/fake.xsmptool'})['text']
            assert 'obsolete' not in restored, restored
            repeated = server.request('xsmp/registerContributions', [resolved])
            assert not repeated['failures'] and len(repeated['registered']) == 1, repeated
            summaries = server.request('xsmp/getContributionSummaries', 'tool')
            item = next(item for item in summaries if item['id'] == 'fake-tool')
            assert item['hasScaffolder'] and item['defaultSelected'] and item['extensionId'] == 'test.fake', item
            prompts = server.request('xsmp/getContributionWizardPrompts', {'selectedToolIds': ['fake-tool']})
            assert [item['type'] for item in prompts] == ['string', 'boolean', 'choice'], prompts
            assert prompts[0]['key'] == 'fake-tool.moduleName'
            # A check reads the buffers of the editor, routed by language ID even
            # when their URI has no language extension.
            source = re.search(r"source\s+['\"]([^'\"]+)['\"]", descriptor).group(1)
            unsaved = (project/source/'Unsaved').as_uri()
            server.send('textDocument/didOpen', {'textDocument': {
                'uri': unsaved, 'languageId': 'xsmpcat', 'version': 42,
                'text': 'catalogue SnapshotMetadata namespace snapshot {}'}})
            server.request('workspace/symbol', {'query': 'SnapshotMetadata'})
            server.wait_notifications(lambda notes: any('Snapshot metadata: xsmpcat:42:open' in str(value)
                                                        for value in notes))
            assert any('Snapshot metadata: xsmpcat:42:open' in str(value)
                       for value in server.notifications), server.notifications
            # IDs and prompt values are arbitrary strings, not URI fields.
            for identifier in ('builtin:request-id', 'xsmp:request-id'):
                server.send('workspace/symbol', {'query': 'NoMatchingSymbol'}, identifier)
            server.request('workspace/symbol', {'query': 'Barrier'})
            server.wait_notifications(lambda notes: all(any(item.get('id') == identifier for item in notes)
                for identifier in ('builtin:request-id', 'xsmp:request-id')))
            responses = [item.get('id') for item in server.notifications if 'id' in item]
            assert responses.count('builtin:request-id') == 1 and responses.count('xsmp:request-id') == 1, responses
            literal = root/'Literal'
            created = server.request('xsmp/createProject', {
                'projectName': 'Literal', 'projectDir': str(literal), 'selectedToolIds': ['fake-tool'],
                'promptValues': {'fake-tool.moduleName': 'xsmp:literal', 'fake-tool.enabled': True,
                                 'fake-tool.flavor': 'second'}})
            assert created is None, created
            assert (literal/'generated.txt').read_text() == 'xsmp:literal'
            assert "tool 'fake-tool'" in (literal/'xsmp.project').read_text()
            # A project whose creation fails is removed with everything written so far.
            failing = root/'FailingScaffold'
            error = server.request_error('xsmp/createProject', {
                'projectName': 'FailingScaffold', 'projectDir': str(failing),
                'selectedToolIds': ['fake-tool'], 'promptValues': {
                    'fake-tool.moduleName': 'x', 'fake-tool.enabled': True, 'fake-tool.flavor': 'first'}})
            assert 'scaffold failure' in error['message'] and '{' not in error['message'], error
            assert not failing.exists()
            # The text of a builtin is read through the standard request, in any spelling of its URI.
            def content(uri):
                return server.request('workspace/textDocumentContent', {'uri': uri})['text']
            contents = content('xsmp:///extensions/test.fake/builtins/external.xsmpcat')
            assert 'ExternalType' in contents
            special_uri = special_builtin.as_uri().split('/builtins/',1)[1]
            assert 'ExtraType' in content('xsmp:///extensions/test.fake/builtins/'+special_uri)
            assert content('xsmp:/extensions/test.fake/builtins/external.xsmpcat') == contents
            assert content('xsmp:/extensions/test.fake/builtins/missing.xsmpcat') == ''
            # Sharing a library must preserve visibility for projects selecting
            # the first contribution, even after the second one is registered.
            second_descriptor = extension/'shared.xsmptool'
            second_descriptor.write_text("tool 'fake-shared'\n")
            shared_entry = {**resolved,'descriptorPath':str(second_descriptor),
                            'aliases':[],'deprecatedAliases':[]}
            shared_report = server.request('xsmp/registerContributions',[shared_entry])
            assert not shared_report['failures'], shared_report
            assert shared_report['registered'][0]['id'] == 'fake-shared', shared_report
            consumer = (project/source/'Consumer.xsmpcat').as_uri()
            consumer_text = 'catalogue Consumer namespace client { public struct Uses { field ext.ExternalType shared } }'
            server.send('textDocument/didOpen',{'textDocument':{'uri':consumer,'languageId':'xsmpcat',
                'version':1,'text':consumer_text}})
            definition = server.request('textDocument/definition',{'textDocument':{'uri':consumer},
                'position':{'line':0,'character':consumer_text.index('ext.ExternalType')+5}})
            assert definition and 'external.xsmpcat' in str(definition), definition
            server.send('textDocument/didClose',{'textDocument':{'uri':consumer}})
            result = server.request('xsmp/generateProject', (project/'xsmp.project').as_uri())
            assert result['generatedProjects'] == ['Demo'] and not result['failures'], result
            assert any('Fake contribution validation.' in str(value) for value in server.notifications)
            server.close()
        finally:
            if server.process.poll() is None:
                server.process.kill()
                server.process.wait()
    print('External contribution lifecycle passed.')


if __name__ == '__main__':
    main()
