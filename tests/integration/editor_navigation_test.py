"""Replay real JSON-RPC navigation requests and compare them with the fixture.

Each case is opened in a server, then definition, type definition, hover,
prepare rename, references and rename are requested at its marked position.
"""
import argparse
import json
from pathlib import Path
import tempfile
from urllib.parse import unquote, urlsplit

from contribution_process_test import Server


def differences(expected, actual, path=""):
    if type(expected) is not type(actual):
        return [path + ": different JSON types"]
    if isinstance(expected, dict):
        result = []
        for key in sorted(expected.keys() | actual.keys()):
            if key not in expected or key not in actual:
                result.append(path + "/" + key + ": missing or extra field")
            else:
                result.extend(differences(expected[key], actual[key], path + "/" + key))
        return result
    if isinstance(expected, list):
        if len(expected) != len(actual):
            return [f"{path}: {len(expected)} expected items, {len(actual)} actual items"]
        result = []
        for index, (left, right) in enumerate(zip(expected, actual)):
            result.extend(differences(left, right, path + "/" + str(index)))
        return result
    return [] if expected == actual else [path + ": different value"]


def normalize(method, value):
    if method == "textDocument/hover" and isinstance(value, dict):
        # The range of the hovered word is not part of the contract.
        value = {key: item for key, item in value.items() if key != "range"}
    if method in ["textDocument/definition", "textDocument/typeDefinition", "textDocument/references"]:
        if value is None:
            return []
        if method == "textDocument/references":
            return sorted(value, key=lambda item: json.dumps(item, sort_keys=True))
    if method == "textDocument/rename" and isinstance(value, dict):
        value = json.loads(json.dumps(value))
        for uri, edits in value.get("changes", {}).items():
            value["changes"][uri] = sorted(edits, key=lambda item: json.dumps(item, sort_keys=True))
    return value


def relocate_locations(value, placeholder, target):
    if isinstance(value, list):
        return [relocate_locations(item, placeholder, target) for item in value]
    if isinstance(value, dict):
        result = {}
        for key, item in value.items():
            if key in ["uri", "targetUri"] and isinstance(item, str):
                result[key] = item.replace(placeholder, target)
            elif key == "changes":
                result[key] = {uri.replace(placeholder, target): edits for uri, edits in item.items()}
            else:
                result[key] = relocate_locations(item, placeholder, target)
        return result
    return value


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--report", type=Path)
    parser.add_argument("server", nargs=argparse.REMAINDER)
    args = parser.parse_args()
    command = args.server[1:] if args.server[:1] == ["--"] else args.server
    assert command, "provide a native server or WASI runner command"
    fixture = json.loads((Path(__file__).parents[1] / "fixtures/lsp/deployment-navigation-services.json").read_text())
    records = []
    for index, case in enumerate(fixture["cases"]):
        placeholder = f"file:///navigation/case-{index}"
        with tempfile.TemporaryDirectory(prefix="xsmp-editor-navigation-") as temporary:
            root = Path(temporary)
            documents = [*case["documents"], {"uri": case["uri"], "text": case["text"]}]
            before = case["text"][:case["text"].index("<|>")]
            position = {"line": before.count("\n"),
                        "character": len(before.rsplit("\n", 1)[-1].encode("utf-16-le")) // 2}
            for document in documents:
                relative = document["uri"].removeprefix(placeholder).lstrip("/")
                path = root / unquote(urlsplit(relative).path)
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_text(document["text"].replace("<|>", ""), encoding="utf-8")
            uri = case["uri"].replace(placeholder, root.as_uri())
            server = Server([*command, "--stdio"])
            actual = {}
            errors = []
            try:
                server.request("initialize", {
                    "processId": None, "rootUri": root.as_uri(),
                    "workspaceFolders": [{"uri": root.as_uri(), "name": "navigation"}],
                    "capabilities": {"workspace": {"workspaceFolders": True}, "textDocument": {
                        "definition": {"linkSupport": True}, "typeDefinition": {"linkSupport": True},
                        "hover": {"contentFormat": ["markdown", "plaintext"]}}}})
                server.send("initialized", {})
                # The server answers once the workspace is scanned.
                server.request("workspace/textDocumentContent", {"uri": uri})
                for document in documents:
                    extension = Path(urlsplit(document["uri"]).path).suffix.lstrip(".")
                    language = "xsmpproject" if extension in ["project", "xsmpprofile", "xsmptool"] else extension
                    server.send("textDocument/didOpen", {"textDocument": {
                        "uri": document["uri"].replace(placeholder, root.as_uri()),
                        "languageId": language, "version": 1, "text": document["text"].replace("<|>", "")}})
                expected = relocate_locations(case["expected"], placeholder, root.as_uri())
                # Rename is last.
                methods = ["textDocument/definition", "textDocument/typeDefinition", "textDocument/hover",
                           "textDocument/prepareRename", "textDocument/references", "textDocument/rename"]
                for method in methods:
                    parameters = {"textDocument": {"uri": uri}, "position": position}
                    if method.endswith("references"):
                        parameters["context"] = {"includeDeclaration": True}
                    if method.endswith("rename"):
                        parameters["newName"] = "Renamed"
                    actual[method] = server.request(method, parameters)
                    errors.extend(differences(normalize(method, expected[method]), normalize(method, actual[method]), method))
                print(f"{case['name']}: {'PASS' if not errors else '; '.join(errors[:3])}", flush=True)
                records.append({"name": case["name"], "errors": errors, "expected": expected, "actual": actual})
            finally:
                server.close()
    if args.report:
        args.report.write_text(json.dumps({"cases": records}, ensure_ascii=False, indent=2) + "\n")
    failed = sum(bool(record["errors"]) for record in records)
    print(f"{len(records) - failed}/{len(records)} navigation cases passed")
    assert not failed, f"{failed} navigation cases differ from the expected responses"


if __name__ == "__main__":
    main()
