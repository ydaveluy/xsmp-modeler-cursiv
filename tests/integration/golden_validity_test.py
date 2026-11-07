"""Keep the goldens of tests/fixtures/generator valid.

Each C++ golden must compile against the real headers of its profile and
edition, and each XML golden must follow its schema. The comparison of the
goldens with the generators is the job of `tests/support/GoldenTree.hpp`.

A tree in `expected-2020` or `expected-2025` is for that edition, and a tree in
`expected` is for both.
"""
import argparse
from concurrent.futures import ThreadPoolExecutor
import os
from pathlib import Path
import subprocess
import sys
import tempfile

from smp_schema_test import schema_bundles

# The profile that generates the C++ goldens of each folder of the fixtures. In
# `all-profiles`, the tree has a folder per profile.
PROFILE_OF_FOLDER = {
    'copyright': 'xsmp-sdk',
    'esa-cdk': 'esa-cdk',
    'gap-pattern': 'xsmp-sdk',
    'gap-pattern-forced': 'esa-cdk',
    'tas-mdk': 'tas-mdk',
    'xsmp-sdk': 'xsmp-sdk',
}

# What is known to be invalid, by the end of its path in the fixtures, and why. An
# entry that turns out valid fails the test, so that these lists stay true.
NOT_COMPILABLE = {
    'expected-2025/esa-cdk/src-gen/demo/support/SupportNative.h':
        'The catalogue says that this native type is defined in its own header.',
    'expected-2025/xsmp-sdk/src-gen/demo/support/SupportNative.h':
        'The catalogue says that this native type is defined in its own header.',
}

NOT_VALID_XML = {
    'smp/assemblies/expected-2020/smdl-gen/texts.smpasb':
        'To show the escape, its implementation holds a control character.',
    'smp/assemblies/expected-2025/smdl-gen/texts.smpasb':
        'To show the escape, its implementation holds a control character.',
    'smp/configuration-values/expected-2020/smdl-gen/runtime.smpcfg':
        'An unresolved enumeration literal has no value to write.',
    'smp/configuration-values/expected-2025/smdl-gen/runtime.smpcfg':
        'An unresolved enumeration literal has no value to write.',
}

SCHEMA_OF_SUFFIX = {
    '.smpcat': ('l1', 'Catalogue.xsd'),
    '.smppkg': ('l1', 'Package.xsd'),
    '.smpcfg': ('l1', 'Configuration.xsd'),
    '.smpasb': ('l2', 'Assembly.xsd'),
    '.smplnk': ('l2', 'LinkBase.xsd'),
    '.smpsed': ('l2', 'Schedule.xsd'),
}


def golden_trees(fixtures):
    """Each golden tree, as its case, its edition or None for both, and its root."""
    for case in sorted(path for path in fixtures.glob('*/*') if path.is_dir()):
        for tree in sorted(case.glob('expected*')):
            edition = tree.name[len('expected-'):] or None
            yield f'{case.parent.name}/{case.name}', edition, tree


def cpp_trees(fixtures):
    """Each C++ tree, as its case, its profile, its edition and its root."""
    for name, edition, tree in golden_trees(fixtures):
        folder = name.split('/')[0]
        if folder == 'all-profiles':
            for profile in sorted(path for path in tree.iterdir() if path.is_dir()):
                yield name, profile.name, edition, profile
        elif folder in PROFILE_OF_FOLDER:
            yield name, PROFILE_OF_FOLDER[folder], edition, tree


def compile_unit(compiler, includes, root, source):
    # The headers of each runtime need C++17, whichever the generated code needs.
    # The `stubs` of a case hold the external headers that its catalogue names.
    case = root
    while not case.name.startswith('expected'):
        case = case.parent
    command = [compiler, '-std=c++17', '-fsyntax-only', '-pedantic-errors', '-w',
               '-x', 'c++', *[f'-I{root / folder}' for folder in ('src', 'src-gen')],
               f'-I{case.parent / "stubs"}',
               *[f'-isystem{path}' for path in includes], str(source)]
    done = subprocess.run(command, capture_output=True, text=True)
    return done.returncode, done.stderr


def check_cpp(args, fixtures):
    """Compiles the units of each tree, and returns the failures."""
    includes = {('xsmp-sdk', '2020'): args.sdk_2020, ('xsmp-sdk', '2025'): args.sdk_2025}
    jobs, missing, counts = [], {}, {}
    for name, profile, edition, root in cpp_trees(fixtures):
        units = sorted(root.rglob('*.cpp')) + sorted(root.rglob('*.h'))
        # The ESA CDK has a single edition, that of its own headers.
        editions = ['own'] if profile == 'esa-cdk' else [edition] if edition else ['2020', '2025']
        for wanted in editions:
            paths = args.esa_cdk if profile == 'esa-cdk' else includes.get((profile, wanted))
            if not paths:
                reason = (f'no headers for {profile} exist' if profile == 'tas-mdk'
                          else f'no headers given for {profile} {wanted}')
                missing.setdefault(reason, set()).add(name)
                continue
            counts.setdefault((profile, wanted), [0, 0])
            for unit in units:
                jobs.append((name, profile, wanted, root, unit, paths))

    def run(job):
        name, profile, wanted, root, unit, paths = job
        code, errors = compile_unit(args.compiler, paths, root, unit)
        return job, code, errors

    failures = []
    invalid = set()
    with ThreadPoolExecutor(args.jobs) as pool:
        for (name, profile, wanted, root, unit, paths), code, errors in pool.map(run, jobs):
            relative = unit.relative_to(fixtures).as_posix()
            known = next((end for end in NOT_COMPILABLE if relative.endswith(end)), None)
            counts[(profile, wanted)][0 if code == 0 else 1] += 1
            if code and known:
                invalid.add(known)
            elif code:
                lines = [line for line in errors.splitlines() if 'error' in line][:3]
                failures.append(f'{relative} ({profile} {wanted}):\n  ' + '\n  '.join(lines))
            elif known:
                failures.append(f'{relative} compiles: remove it from NOT_COMPILABLE')
    for (profile, wanted), (good, bad) in sorted(counts.items()):
        print(f'{profile} {wanted}: {good} units compile, {bad} do not')
    for reason, names in sorted(missing.items()):
        print(f'Not compiled, {reason}: {", ".join(sorted(names))}')
    for end in sorted(invalid):
        print(f'Does not compile: {end}: {NOT_COMPILABLE[end]}')
    return failures


def check_xml(args, fixtures, temporary):
    """Validates the XML of each tree, and returns the failures."""
    schemas = schema_bundles(args.xsd, temporary / 'schemas')
    jobs = []
    for name, edition, tree in golden_trees(fixtures):
        for document in sorted(tree.rglob('*')):
            if document.suffix not in SCHEMA_OF_SUFFIX:
                continue
            level, schema = SCHEMA_OF_SUFFIX[document.suffix]
            for wanted in ([edition] if edition else ['2020', '2025']):
                if level == 'l2':
                    path = schemas / 'l2/Smdl' / schema
                elif wanted == '2020':
                    path = schemas / 'l1-2020/Smdl' / schema
                else:
                    path = schemas / 'l1-2025' / schema
                jobs.append((name, document, path))

    def run(job):
        name, document, schema = job
        done = subprocess.run([args.xmllint, '--nonet', '--noout', '--schema', str(schema),
                               str(document)], capture_output=True, text=True)
        return job, done

    failures = []
    invalid = 0
    with ThreadPoolExecutor(args.jobs) as pool:
        for (name, document, schema), done in pool.map(run, jobs):
            relative = document.relative_to(fixtures).as_posix()
            if done.returncode and relative in NOT_VALID_XML:
                invalid += 1
                print(f'Does not follow {schema.name}: {relative}: {NOT_VALID_XML[relative]}')
            elif done.returncode:
                failures.append(f'{relative} against {schema.name}:\n  '
                                + done.stderr[:600].replace(str(fixtures), ''))
            elif relative in NOT_VALID_XML:
                failures.append(f'{relative} is valid: remove it from NOT_VALID_XML')
    print(f'XML: {len(jobs) - len(failures) - invalid} of {len(jobs)} documents follow their schema')
    return failures


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--fixtures', type=Path, required=True)
    parser.add_argument('--compiler', default=None)
    parser.add_argument('--sdk-2020', type=Path, action='append', default=[])
    parser.add_argument('--sdk-2025', type=Path, action='append', default=[])
    parser.add_argument('--esa-cdk', type=Path, action='append', default=[])
    parser.add_argument('--xmllint', type=Path)
    parser.add_argument('--xsd', type=Path)
    parser.add_argument('--jobs', type=int, default=min(8, os.cpu_count() or 1))
    args = parser.parse_args()
    failures = []
    if args.compiler:
        failures += check_cpp(args, args.fixtures)
    if args.xmllint:
        with tempfile.TemporaryDirectory(prefix='xsmp-golden-xml-') as temporary:
            failures += check_xml(args, args.fixtures, Path(temporary))
    for failure in failures:
        print(failure, file=sys.stderr)
    sys.exit(1 if failures else 0)


if __name__ == '__main__':
    main()
