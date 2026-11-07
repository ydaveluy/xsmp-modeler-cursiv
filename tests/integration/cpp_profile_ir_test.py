"""Compare the expected and the generated C++ through Clang's optimized IR.

Uses real runtime headers supplied by the caller. The reference catalogue and
every expected C++ file participate: `workspace/xsmp-profile-*/generator-cpp` is
the output of the original generators, kept as this reference, whereas
`generator/all-profiles/reference-catalogue` holds the goldens of this
repository. Compiler attribute numbers
are expanded, metadata annotations are ignored, private string symbols are named
by their complete initializer, and file prefixes are normalized. Functions and
global initializers otherwise compare in full. This is a concrete compiler-output
comparison for the fixture, not a proof for every input model.
"""
import argparse
import difflib
import hashlib
import json
from pathlib import Path
import re
import subprocess
import tempfile

SYMBOL = re.compile(r'@(?:"[^"]+"|[-A-Za-z$._0-9]+)')
QUOTED = re.compile(r'("(?:[^"\\]|\\.)*")')


def ir_model(text):
    attributes = dict(re.findall(r'^attributes #(\d+) = (.*)$', text, re.M))
    globals_ = {match.group(1): match.group(2) for match in re.finditer(
        r'^(@(?:"[^"]+"|[-A-Za-z$._0-9]+)) = (.*)$', text, re.M)}
    string_names = {key: '@STRING_' + hashlib.sha256(value.encode()).hexdigest()
                    for key, value in globals_.items() if key.startswith('@.str')}

    def canonical(value):
        parts = QUOTED.split(value)
        for index in range(0, len(parts), 2):
            part = SYMBOL.sub(lambda match: string_names.get(match.group(), match.group()),
                              parts[index])
            part = re.sub(r' #(\d+)', lambda match:
                          ' ATTRIBUTE' + attributes[match.group(1)], part)
            parts[index] = re.sub(r', ![a-zA-Z][a-zA-Z0-9_.]* ![0-9]+', '', part)
        return ''.join(parts)

    functions = {match.group(2): canonical(match.group()) for match in re.finditer(
        r'^define ([^\n]*? (@[^\s(]+)[^\n]*\{\n[\s\S]*?^\})', text, re.M)}
    globals_ = {string_names.get(key, key): canonical(value)
                for key, value in globals_.items()}
    return functions, globals_


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--cli', type=Path, required=True)
    parser.add_argument('--runner', action='append', default=[])
    parser.add_argument('--compiler', type=Path, required=True)
    parser.add_argument('--fixtures', type=Path, required=True)
    parser.add_argument('--profile', choices=['xsmp-sdk', 'esa-cdk'], required=True)
    parser.add_argument('--include', type=Path, action='append', default=[])
    parser.add_argument('--report', type=Path)
    args = parser.parse_args()
    package = {'xsmp-sdk': 'xsmp-profile-xsmp-sdk',
               'esa-cdk': 'xsmp-profile-esa-cdk'}[args.profile]
    golden = (args.fixtures / 'workspace' / package / 'generator-cpp').resolve()
    results = []
    with tempfile.TemporaryDirectory(prefix='xsmp-cpp-ir-') as directory:
        root = Path(directory)
        (root / 'src').mkdir()
        (root / 'src/catalogue.xsmpcat').write_bytes(
            (args.fixtures / 'generator/all-profiles/reference-catalogue/input/smdl/catalogue.xsmpcat')
            .read_bytes())
        (root / 'xsmp.project').write_text(
            "project 'profile-generators' using 'ECSS_SMP_2025'\nsource 'src'\n")
        generated = root / 'generated'
        run = subprocess.run([*args.runner, str(args.cli), 'generate',
                              str(root / 'src/catalogue.xsmpcat'), '-w', str(root),
                              '-o', str(generated), '--profile', args.profile],
                             capture_output=True, text=True, timeout=90)
        assert run.returncode == 0, (run.stdout, run.stderr)
        expected_files = {str(path.relative_to(golden)) for path in golden.rglob('*')
                          if path.is_file()}
        actual_files = {str(path.relative_to(generated)) for path in generated.rglob('*')
                        if path.is_file()}
        assert expected_files == actual_files, (expected_files - actual_files,
                                               actual_files - expected_files)
        for relative in sorted(Path(path) for path in expected_files if path.endswith('.cpp')):
            data = {}
            unit = {'file': str(relative)}
            for kind, source_root in [('expected', golden), ('generated', generated)]:
                output = root / (kind + '.ll')
                command = [str(args.compiler), '-std=c++17', '-O2', '-DNDEBUG',
                           '-S', '-emit-llvm', '-I' + str(source_root / 'src'),
                           '-I' + str(source_root / 'src-gen'),
                           *['-I' + str(path) for path in args.include],
                           '-ffile-prefix-map=' + str(source_root) + '=.',
                           str(source_root / relative), '-o', str(output)]
                run = subprocess.run(command, capture_output=True, text=True, timeout=90)
                assert run.returncode == 0, (kind, str(relative), run.stdout, run.stderr)
                data[kind] = ir_model(output.read_text())
            for index, name in enumerate(['functions', 'globals']):
                expected, actual = data['expected'][index], data['generated'][index]
                assert expected.keys() == actual.keys(), (str(relative), name,
                    expected.keys() - actual.keys(), actual.keys() - expected.keys())
                differences = [key for key in expected if expected[key] != actual[key]]
                if differences:
                    key = differences[0]
                    diff = ''.join(difflib.unified_diff(expected[key].splitlines(True),
                        actual[key].splitlines(True), fromfile='expected/' + key,
                        tofile='generated/' + key))
                    raise AssertionError((str(relative), name, differences, diff))
                unit[name] = len(expected)
            results.append(unit)
    report = {'profile': args.profile, 'translation_units': len(results),
              'functions_identical': sum(unit['functions'] for unit in results),
              'globals_identical': sum(unit['globals'] for unit in results), 'units': results}
    if args.report:
        args.report.write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps({key: value for key, value in report.items() if key != 'units'}))


if __name__ == '__main__':
    main()
