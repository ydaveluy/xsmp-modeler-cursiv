"""End-to-end CLI scaffolding, project routing, generators and SMP import."""
import py_compile
import subprocess
import sys
import tempfile
from pathlib import Path


def main():
    command = sys.argv[1:]

    def run(*args, success=True, cwd=None):
        result = subprocess.run([*command, *map(str, args)], capture_output=True,
                                text=True, timeout=30, cwd=cwd)
        if success:
            assert result.returncode == 0, (args, result.stdout, result.stderr)
        else:
            assert result.returncode != 0, args
        return result

    with tempfile.TemporaryDirectory(prefix='xsmp-cli-test-') as directory:
        root = Path(directory)
        project = root / 'Demo'
        run('new', 'project', 'Demo', root, '--profile', 'xsmp-sdk',
            '--tool', 'smp', '--tool', 'python', '--tool', 'adoc', '--no-interactive')
        assert (project / 'CMakeLists.txt').is_file()
        assert (project / 'python/Demo/test_Demo.py').is_file()
        assert (project / 'doc/Demo.adoc').is_file()
        run('validate', project)
        # Only UTF-8 is read: a Latin-1 file stops the command before anything.
        latin1 = root / 'Latin1'
        (latin1 / 'smdl').mkdir(parents=True)
        (latin1 / 'xsmp.project').write_text("project 'Latin1' source 'smdl' tool 'smp'\n")
        (latin1 / 'smdl/Latin1.xsmpcat').write_bytes(b'// caf\xe9\ncatalogue Latin1 namespace l {}\n')
        refused = run('generate', latin1, success=False)
        assert refused.returncode == 2, (refused.stdout, refused.stderr)
        assert refused.stderr.strip() == (
            f"File '{latin1 / 'smdl/Latin1.xsmpcat'}' is not valid UTF-8. "
            'Save it as UTF-8 before running the CLI.'), refused.stderr
        assert not (latin1 / 'smdl-gen').exists()
        run('validate', 'Demo', cwd=root)
        run('validate', 'Demo/xsmp.project', '--workspace-root', '.', cwd=root)
        run('validate', project / 'xsmp.project', '--workspace-root', root)
        run('generate', project)
        assert (project / 'src-gen/Demo.cpp').is_file()
        assert (project / 'smdl-gen/Demo.smpcat').is_file()
        assert (project / 'smdl-gen/Demo.smppkg').is_file()
        assert (project / 'adoc-gen/Demo-gen.adoc').is_file()
        for file in (project / 'python').rglob('*.py'):
            py_compile.compile(str(file), doraise=True)
        imported = root / 'Imported.xsmpcat'
        xml = project / 'smdl-gen/Demo.smpcat'
        run('import-smp', xml, '-o', imported)
        first = imported.read_text()
        run('import-smp', xml, '-o', imported, success=False)
        assert imported.read_text() == first
        run('import-smp', xml, '-o', imported, '--force')
        run('validate', xml)
        # A native sidecar has priority over an XML-derived read-only mirror.
        sidecar = xml.with_suffix('.xsmpcat')
        sidecar.write_text('catalogue Override\nnamespace demo {}\n')
        run('validate', xml)
        run('new', 'project', 'Demo', root, success=False)
        alias = run('new', 'project', 'TasAlias', root, '--profile', 'org.eclipse.xsmp.profile.tas-mdk')
        descriptor = (root / 'TasAlias/xsmp.project').read_text()
        assert "profile 'tas-mdk'" in descriptor
        assert "dependency 'TasMdk'" in descriptor
        assert 'deprecated' in alias.stderr


if __name__ == '__main__':
    main()
