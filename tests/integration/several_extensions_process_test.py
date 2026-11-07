"""Several separate extensions load together at startup, and a bad one stays alone."""
import argparse, json, os, pathlib, shutil, subprocess, tempfile

from contribution_process_test import Server

parser = argparse.ArgumentParser()
parser.add_argument('--cli', required=True)
parser.add_argument('--lsp', required=True)
parser.add_argument('--plugin', required=True)
parser.add_argument('--tas-package', required=True)
parser.add_argument('--tas-handler', required=True)
parser.add_argument('--runner', action='append', default=[])
args = parser.parse_args()


def stage(binary, directory):
    source = pathlib.Path(binary)
    name = 'handler.wasm' if source.read_bytes()[:4] == b'\0asm' else 'handler'
    shutil.copy2(source, directory/name)
    (directory/name).chmod(0o755)
    return name


def extension(root, name, descriptor, api='^2.0.0'):
    directory = root/name
    directory.mkdir()
    (directory/f'{name}.xsmptool').write_text(descriptor)
    entry = {'descriptor': f'{name}.xsmptool', 'handler': stage(args.plugin, directory), 'apiVersion': api}
    (directory/'package.json').write_text(json.dumps({
        'name': name, 'publisher': 'test', 'contributes': {'xsmp': [entry]}}))
    return directory


def resolved(directory):
    metadata = json.loads((directory/'package.json').read_text())
    return [{**entry, 'extensionId': f"{metadata['publisher']}.{metadata['name']}",
             'extensionRoot': str(directory),
             'descriptorPath': str(directory/entry['descriptor']),
             'handlerPath': str(directory/entry['handler']),
             'builtins': [str(directory/item) for item in entry.get('builtins', [])]}
            for entry in metadata['contributes']['xsmp']]


def cli(*arguments, code=0):
    run = subprocess.run([*args.runner, args.cli, *map(str, arguments)], capture_output=True, text=True, timeout=120)
    assert run.returncode == code, (arguments, run.returncode, run.stdout, run.stderr)
    return run


with tempfile.TemporaryDirectory(prefix='xsmp-several-') as temporary:
    root = pathlib.Path(temporary)
    first = extension(root, 'first', "tool 'first-tool'\n")
    second = extension(root, 'second', "tool 'second-tool'\n")
    broken = extension(root, 'broken', "tool\n")
    future = extension(root, 'future', "tool 'future-tool'\n", api='^3.0.0')

    # Command line: `--extension` repeated.
    cli('new', 'project', 'Both', root, '--extension', first, '--extension', second,
        '--tool', 'first-tool', '--tool', 'second-tool', '--no-interactive')
    descriptor = (root/'Both/xsmp.project').read_text()
    assert "tool 'first-tool'" in descriptor and "tool 'second-tool'" in descriptor, descriptor

    # On the command line an extension is asked for by name: failing to load it is an error.
    run = cli('new', 'project', 'Partial', root, '--extension', broken, '--extension', second,
              '--tool', 'second-tool', '--no-interactive', code=2)
    assert 'test.broken' in run.stderr and not (root/'Partial').exists(), run.stderr

    # Language server: everything in one request, as the client does at startup.
    tas_metadata = json.loads((pathlib.Path(args.tas_package)/'package.json').read_text())
    tas = root/'tas-mdk'
    shutil.copytree(args.tas_package, tas, ignore=shutil.ignore_patterns('node_modules', 'server', 'out', '*.vsix'))
    handler = stage(args.tas_handler, tas)
    for entry in tas_metadata['contributes']['xsmp']: entry['handler'] = handler
    (tas/'package.json').write_text(json.dumps(tas_metadata))

    foundation = root/'TasMdk'
    (foundation/'smdl').mkdir(parents=True)
    (foundation/'xsmp.project').write_text("project 'TasMdk' using 'ECSS_SMP_2025'\nsource 'smdl'\n")
    (foundation/'smdl/TasMdk.xsmpcat').write_text('''catalogue TasMdk
namespace TasMdk {
 /** Foundation model.
  * @uuid 11111111-1111-1111-1111-111111111111
  */
 public model Model {}
}
''')
    project = root/'Mission'
    (project/'smdl').mkdir(parents=True)
    catalogue = project/'smdl/Demo.xsmpcat'
    catalogue.write_text('''catalogue Demo
namespace demo {
 /** Demonstration model.
  * @uuid 01234567-89ab-cdef-0123-456789abcdef
  */
 public model Root extends TasMdk.Model {
   /** Feature description. */
   field Smp.Int32 fea_value = 42
 }
}
''')
    manifest = project/'xsmp.project'
    manifest.write_text("project 'Mission' using 'ECSS_SMP_2025'\nprofile 'tas-mdk'\ntool 'first-tool'\ntool 'second-tool'\ndependency 'TasMdk'\nsource 'smdl'\n")

    entries = [*resolved(first), *resolved(broken), *resolved(future), *resolved(second), *resolved(tas)]
    server = Server([*args.runner, args.lsp], libraries=[entry['handlerPath'] for entry in entries])
    try:
        server.request('initialize', {'processId': None, 'rootUri': root.as_uri(), 'capabilities': {}})
        server.send('initialized', {})
        report = server.request('xsmp/registerContributions', entries)
        assert sorted(item['id'] for item in report['registered']) == ['first-tool', 'second-tool', 'tas-mdk'], report
        assert {item['extensionId']: item['phase'] for item in report['failures']} == {
            'test.broken': 'descriptor', 'test.future': 'apiVersion'}, report
        summaries = {item['id']: item['extensionId'] for item in server.request('xsmp/getContributionSummaries')}
        assert summaries['first-tool'] == 'test.first' and summaries['second-tool'] == 'test.second', summaries
        for file, language in ((manifest, 'xsmpproject'), (catalogue, 'xsmpcat')):
            server.send('textDocument/didOpen', {'textDocument': {'uri': file.as_uri(), 'languageId': language,
                        'version': 1, 'text': file.read_text()}})
        generated = server.request('xsmp/generateProject', catalogue.as_uri())
        assert generated['generatedProjects'] == ['Mission'] and not generated['failures'], generated
        assert (project/'custom-gen/first-tool/Demo.txt').is_file(), 'the first tool did not generate'
        assert (project/'custom-gen/second-tool/Demo.txt').is_file(), 'the second tool did not generate'
        assert (project/'include-gen/demo/RootGen.h').is_file(), 'TAS-MDK did not generate'
    finally:
        server.close()
print('Several extensions passed.')
