"""Exercise a native or WASI server command over framed stdio JSON-RPC."""
import json
import os
import select
import subprocess
import sys
import tempfile
import time
from pathlib import Path


def main():
    server = subprocess.Popen([*sys.argv[1:], "--stdio"], stdin=subprocess.PIPE,
                              stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    pending = bytearray()

    def send(message):
        payload = json.dumps(message).encode()
        server.stdin.write(f"Content-Length: {len(payload)}\r\n\r\n".encode() + payload)
        server.stdin.flush()

    def receive(identifier):
        deadline = time.monotonic() + 20
        while time.monotonic() < deadline:
            while b"\r\n\r\n" in pending:
                header, _, body = pending.partition(b"\r\n\r\n")
                length = next(int(line.split(b":", 1)[1]) for line in header.split(b"\r\n")
                              if line.lower().startswith(b"content-length:"))
                if len(body) < length:
                    break
                message = json.loads(body[:length])
                del pending[:len(header) + 4 + length]
                if message.get("id") == identifier:
                    assert "error" not in message, message
                    return message["result"]
            ready, _, _ = select.select([server.stdout], [], [], max(0, deadline - time.monotonic()))
            if not ready:
                break
            data = os.read(server.stdout.fileno(), 65536)
            if not data:
                raise AssertionError("Language server closed stdout early")
            pending.extend(data)
        raise AssertionError(f"No response to request {identifier}")

    def request(identifier, method, params):
        send({"jsonrpc": "2.0", "id": identifier, "method": method, "params": params})
        return receive(identifier)

    try:
        result = request(1, "initialize", {"processId": None, "rootUri": None,
                                          "capabilities": {}, "workspaceFolders": None})
        assert result["capabilities"]["completionProvider"] is not None, result
        send({"jsonrpc": "2.0", "method": "initialized", "params": {}})
        with tempfile.TemporaryDirectory(prefix="xsmp-lsp-") as directory:
            uri = (Path(directory) / "smoke.xsmpcat").as_uri()
            text = "catalogue Smoke\nnamespace demo { struct State { field Smp.Bool enabled } }\n"
            send({"jsonrpc": "2.0", "method": "textDocument/didOpen", "params": {
                "textDocument": {"uri": uri, "languageId": "xsmpcat", "version": 1, "text": text}}})
            symbols = request(2, "textDocument/documentSymbol", {"textDocument": {"uri": uri}})
            assert symbols, symbols
            completion = request(3, "textDocument/completion", {
                "textDocument": {"uri": uri}, "position": {"line": 1, "character": 52}})
            assert completion is not None
        request(4, "shutdown", None)
        send({"jsonrpc": "2.0", "method": "exit", "params": None})
        server.stdin.close()
        assert server.wait(timeout=15) == 0
    finally:
        if server.poll() is None:
            server.kill()
            server.wait()
        server.stdout.close()
        server.stderr.close()


if __name__ == "__main__":
    main()
