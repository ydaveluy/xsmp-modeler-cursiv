#!/usr/bin/env python3
"""Validate the SMP schedules of the examples and of the starter files.

The bundled schemas are the official ones, unchanged. The temporary bundle
applies the local import adjustments needed by xmllint. The goldens are checked
by golden_validity_test.py with the same bundle.
"""
import argparse
import json
from pathlib import Path
import re
import shutil
import subprocess
import tempfile

XML_STUB = '''<?xml version="1.0"?>
<xs:schema xmlns:xs="http://www.w3.org/2001/XMLSchema"
 targetNamespace="http://www.w3.org/XML/1998/namespace"
 xmlns:xml="http://www.w3.org/XML/1998/namespace"
 elementFormDefault="qualified" attributeFormDefault="qualified">
 <xs:attribute name="lang" type="xs:language"/>
</xs:schema>'''

ROOT = Path(__file__).resolve().parents[2]


def run(command):
    completed = subprocess.run([str(item) for item in command], text=True, capture_output=True)
    if completed.returncode:
        raise RuntimeError(' '.join(map(str, command)) + '\n' + completed.stdout + completed.stderr)
    return completed.stdout


def schema_bundles(source, destination):
    shutil.copytree(source, destination)
    l1_2020 = destination / 'l1-2020'
    (l1_2020 / 'xml.xsd').write_text(XML_STUB)
    link = l1_2020 / 'xlink.xsd'
    link.write_text(link.read_text().replace('http://www.w3.org/2001/xml.xsd', '../xml.xsd'))
    # xlink.xsd sits directly in l1-2020, so its local XML import is beside it.
    link.write_text(link.read_text().replace('schemaLocation="../xml.xsd"', 'schemaLocation="xml.xsd"'))
    l2 = destination / 'l2'
    (l2 / 'Core').mkdir(exist_ok=True)
    for name in ('Elements.xsd', 'Types.xsd'):
        shutil.copyfile(l1_2020 / 'Core' / name, l2 / 'Core' / name)
    shutil.copyfile(l1_2020 / 'xlink.xsd', l2 / 'xlink.xsd')
    (l2 / 'xml.xsd').write_text(XML_STUB)
    for path in (l2 / 'Smdl').glob('*.xsd'):
        contents = path.read_text()
        for original, replacement in (
            ('schemaLocation="xlink.xsd"', 'schemaLocation="../xlink.xsd"'),
            ('schemaLocation="Elements.xsd"', 'schemaLocation="../Core/Elements.xsd"'),
            ('schemaLocation="Types.xsd"', 'schemaLocation="../Core/Types.xsd"'),
            ('Elements:DayTimeDuration', 'xsd:duration'),
        ):
            contents = contents.replace(original, replacement)
        path.write_text(contents)
    return destination


def check_example_schedules(args, temporary, schema):
    examples = temporary / 'examples'
    shutil.copytree(ROOT / 'example', examples)
    for project, schedule in (('04-orbital-segment', 'orbital_schedule'),
                              ('05-payload-segment', 'payload_segment_schedule'),
                              ('06-mission-system', 'mission_schedule')):
        run([*args.runner, args.cli, 'generate', examples / project,
             '--workspace-root', examples, '--tool', 'smp'])
        run([args.xmllint, '--nonet', '--noout', '--schema', schema,
             examples / project / 'smdl-gen' / (schedule + '.smpsed')])


def check_starter_files(args, temporary, schema):
    """The files of the wizard have no diagnostic, and their schedule is valid."""
    golden = json.loads((ROOT / 'vscode/test/fixtures/starter-files.json').read_text())
    for key, starter in golden.items():
        stem = key.split('/')[0]
        project = temporary / ('starter-' + stem)
        (project / 'smdl').mkdir(parents=True, exist_ok=True)
        (project / 'xsmp.project').write_text(
            "project 'Starter' using 'ECSS_SMP_2025'\nsource 'smdl'\ntool 'smp'\n")
        # The wizard writes a new UUID in place of its placeholder.
        (project / 'smdl' / starter['fileName']).write_text(re.sub(
            r'@uuid \S+', '@uuid 11111111-1111-4111-8111-111111111111', starter['content']))
    for project in temporary.glob('starter-*'):
        report = run([*args.runner, args.cli, 'validate', project, '--workspace-root', project])
        if '0 errors, 0 warnings' not in report:
            raise RuntimeError(f'{project.name}: {report}')
        run([*args.runner, args.cli, 'generate', project, '--workspace-root', project])
        for schedule in (project / 'smdl-gen').glob('*.smpsed'):
            run([args.xmllint, '--nonet', '--noout', '--schema', schema, schedule])


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--cli', type=Path, required=True)
    parser.add_argument('--runner', action='append', default=[])
    parser.add_argument('--xmllint', type=Path, required=True)
    parser.add_argument('--xsd', type=Path, required=True)
    args = parser.parse_args()
    with tempfile.TemporaryDirectory(prefix='xsmp-schema-') as temporary:
        temporary = Path(temporary)
        schemas = schema_bundles(args.xsd, temporary / 'schemas')
        schedule_schema = schemas / 'l2/Smdl/Schedule.xsd'
        check_example_schedules(args, temporary, schedule_schema)
        check_starter_files(args, temporary, schedule_schema)
    print('SMP schema conformance: the example schedules and the starter files passed.')


if __name__ == '__main__':
    main()
