"""Exercise indexing and repeated invalid/valid edits in a real language server.

Timings/RSS are evidence for this workload, not a universal performance SLA.
"""
import argparse
import json
import os
from pathlib import Path
import tempfile
import time
import uuid

from contribution_process_test import Server


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--lsp', required=True)
    parser.add_argument('--runner', action='append', default=[])
    parser.add_argument('--documents', type=int, default=200)
    parser.add_argument('--edits', type=int, default=6)
    parser.add_argument('--report', type=Path)
    args = parser.parse_args()
    assert args.documents > 0 and args.edits > 0
    began = time.monotonic()
    with tempfile.TemporaryDirectory(prefix='xsmp-production #?% é-') as temporary:
        root = Path(temporary)
        (root/'smdl').mkdir()
        (root/'xsmp.project').write_text("project 'Load' using 'ECSS_SMP_2025' source 'smdl'\n")
        for index in range(args.documents):
            identifier = uuid.uuid5(uuid.NAMESPACE_URL, 'xsmp-production-load:'+str(index))
            (root/'smdl'/f'model-{index}.xsmpcat').write_text(
                f'catalogue C{index} namespace n{index} {{ /** @uuid {identifier} */ '
                f'public struct T{index} {{ field Smp.Int32 value }} }}\n')
        server = Server([*args.runner, args.lsp, '--stdio'])
        try:
            server.request('initialize', {'processId': None, 'rootUri': root.as_uri(), 'capabilities': {}})
            server.send('initialized', {})
            deadline = time.monotonic()+90
            while True:
                symbols = server.request('workspace/symbol', {'query': f'n{args.documents-1}.T{args.documents-1}'})
                if symbols:
                    break
                assert time.monotonic() < deadline, 'Workspace never finished indexing'
            indexed = time.monotonic()
            uri = (root/'smdl'/'edited.xsmpcat').as_uri()
            text = 'catalogue Edited namespace edit { struct Live { field Smp.Int32 value } }'
            broken = text.replace('Smp.Int32', 'MissingType')
            server.send('textDocument/didOpen', {'textDocument': {
                'uri': uri, 'languageId': 'xsmpcat', 'version': 1, 'text': text}})
            latencies = []
            for index in range(args.edits):
                value = broken if index % 2 == 0 else text
                offset = value.index('MissingType' if index % 2 == 0 else 'Smp.Int32')+2
                start = time.monotonic()
                server.send('textDocument/didChange', {'textDocument': {'uri': uri, 'version': index+2},
                    'contentChanges': [{'text': value}]})
                result = server.request('textDocument/definition', {'textDocument': {'uri': uri},
                    'position': {'line': 0, 'character': offset}})
                assert bool(result) == (index % 2 == 1), (index, result)
                latencies.append(time.monotonic()-start)
            symbols = server.request('workspace/symbol', {'query': 'n'})
            count = sum(item['name'].rsplit('.',1)[-1].startswith('T') and
                        item['location']['uri'].startswith(root.as_uri()) for item in symbols)
            assert count == args.documents, (count, args.documents)
            rss = None
            status = Path(f'/proc/{server.process.pid}/status')
            if status.exists():
                for line in status.read_text().splitlines():
                    if line.startswith('VmHWM:'):
                        rss = int(line.split()[1])
            server.close()
            report = {'documents': args.documents, 'alternatingInvalidValidEdits': args.edits,
                'indexedTypes': count, 'indexSeconds': indexed-began,
                'maximumEditSeconds': max(latencies), 'editSeconds': latencies,
                'processPeakRssKiB': rss, 'gracefulShutdown': True,
                'command': [*args.runner, args.lsp, '--stdio']}
            if args.report:
                args.report.write_text(json.dumps(report, indent=2)+'\n')
            print(json.dumps(report))
        finally:
            if server.process.poll() is None:
                server.process.kill()
                server.process.wait()


if __name__ == '__main__':
    main()
