"""Load the actual external TAS C++/WASI package in a native or WASI server."""
import argparse
import json
import os
import shutil
from pathlib import Path
import tempfile
from contribution_process_test import Server


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--lsp', required=True)
    parser.add_argument('--package', required=True)
    parser.add_argument('--handler')
    parser.add_argument('--runner', action='append', default=[])
    args = parser.parse_args()
    package_root = Path(args.package).resolve()
    metadata = json.loads((package_root/'package.json').read_text())
    staging = None
    if args.handler:
        staging = tempfile.TemporaryDirectory(prefix='xsmp-tas-package-')
        staged = Path(staging.name)/'extension'
        shutil.copytree(package_root, staged, ignore=shutil.ignore_patterns('server','*.vsix'))
        binary = Path(args.handler).resolve()
        suffix = '.wasm' if binary.read_bytes()[:4] == b'\0asm' else '.exe' if os.name == 'nt' else ''
        relative = 'server/tas-mdk-handler'+suffix
        destination = staged/relative
        destination.parent.mkdir()
        shutil.copy2(binary,destination)
        destination.chmod(0o755)
        for entry in metadata['contributes']['xsmp']: entry['handler'] = relative
        (staged/'package.json').write_text(json.dumps(metadata))
        package_root = staged
    entries = []
    for entry in metadata['contributes']['xsmp']:
        handler = package_root/entry['handler']
        assert handler.is_file(), handler
        if not args.handler: assert handler.read_bytes()[:4] == b'\0asm', handler
        entries.append({**entry, 'extensionId':metadata['publisher']+'.'+metadata['name'],
                        'extensionRoot':str(package_root), 'descriptorPath':str(package_root/entry['descriptor']),
                        'handlerPath':str(handler), 'builtins':[str(package_root/file) for file in entry.get('builtins',[])]})
    os.environ.setdefault('CURSIV_WORKERS','2')
    with tempfile.TemporaryDirectory(prefix='xsmp-external-tas-') as temporary:
        root = Path(temporary)
        foundation = root/'TasMdk'
        (foundation/'smdl').mkdir(parents=True)
        (foundation/'xsmp.project').write_text('project "TasMdk" using "ECSS_SMP_2025"\nsource "smdl"\n')
        (foundation/'smdl/TasMdk.xsmpcat').write_text('''catalogue TasMdk
namespace TasMdk {
 /** Foundation model.
  * @uuid 11111111-1111-1111-1111-111111111111
  */
 public model Model {}
}
''')
        project = root/'Demo'
        (project/'smdl').mkdir(parents=True)
        descriptor = project/'xsmp.project'
        descriptor.write_text('project "Demo" using "ECSS_SMP_2025"\nprofile "tas-mdk"\ndependency "TasMdk"\nsource "smdl"\n')
        catalogue = project/'smdl/Demo.xsmpcat'
        valid = '''catalogue Demo
namespace demo {
 /** Demonstration model.
  * @uuid 01234567-89ab-cdef-0123-456789abcdef
  */
 public model Root extends TasMdk.Model {
   /** Feature description. */
   field Smp.Int32 fea_value = 42
 }
}
'''
        catalogue.write_text(valid)
        server = Server([*args.runner, args.lsp], libraries=[entry['handlerPath'] for entry in entries])
        try:
            server.request('initialize', {'processId':None, 'rootUri':root.as_uri(), 'capabilities':{}})
            server.send('initialized',{})
            before = server.request('xsmp/getContributionSummaries')
            assert not any(item['id'] == 'tas-mdk' for item in before), before
            registered = server.request('xsmp/registerContributions', entries)
            assert not registered['failures'], registered
            after = server.request('xsmp/getContributionSummaries')
            profile = next(item for item in after if item['id'] == 'tas-mdk')
            assert profile['label'] == 'TAS MDK' and profile['hasScaffolder'], profile
            for file, language in ((descriptor,'xsmpproject'),(catalogue,'xsmpcat')):
                server.send('textDocument/didOpen',{'textDocument':{'uri':file.as_uri(),'languageId':language,'version':1,'text':file.read_text()}})
            generated = server.request('xsmp/generateProject', catalogue.as_uri())
            assert generated['generatedProjects'] == ['Demo'] and not generated['failures'], generated
            assert (project/'include-gen/demo/RootGen.h').is_file()
            assert (project/'helpers/builder/demo/Root/generated_info.py').is_file()
            # The worker writes in its own process, and the host removes what is stale.
            header = project/'include-gen/demo/RootGen.h'
            before = header.stat()
            stale = project/'src-gen/demo/old/Stale.cpp'
            stale.parent.mkdir()
            stale.write_text('stale')
            generated = server.request('xsmp/generateProject', catalogue.as_uri())
            assert generated['generatedProjects'] == ['Demo'] and not generated['failures'], generated
            assert not stale.parent.exists()
            after = header.stat()
            assert (after.st_ino, after.st_mtime_ns) == (before.st_ino, before.st_mtime_ns)
            # So does an obstacle give way to a worker.
            header.unlink()
            header.mkdir()
            (header/'old').write_text('in the way')
            generated = server.request('xsmp/generateProject', catalogue.as_uri())
            assert generated['generatedProjects'] == ['Demo'] and not generated['failures'], generated
            assert header.is_file()
            scaffold_dir = root/'Scaffold'
            created = server.request('xsmp/createProject',{'projectName':'Scaffold','projectDir':str(scaffold_dir),
                         'selectedProfileId':'tas-mdk','selectedToolIds':[],'promptValues':{}})
            assert created is None, created
            assert "dependency 'TasMdk'" in (scaffold_dir/'xsmp.project').read_text()
            assert 'CXX_STANDARD=11' in (scaffold_dir/'Makefile').read_text()
            server.send('textDocument/didChange',{'textDocument':{'uri':catalogue.as_uri(),'version':2},
                        'contentChanges':[{'text':valid.replace('fea_value','invalidValue')}]})
            server.request('xsmp/getContributionSummaries')
            publications = [item['params'] for item in server.notifications if item.get('method') == 'textDocument/publishDiagnostics'
                            and item.get('params',{}).get('uri') == catalogue.as_uri()]
            assert publications and any("'fea_'" in item['message'] for item in publications[-1]['diagnostics']), publications
        finally:
            server.close()
    if staging: staging.cleanup()
    print('External TAS C++/WASI package registration, generation, validation and project creation passed.')


if __name__ == '__main__':
    main()
