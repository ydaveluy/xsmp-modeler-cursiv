"""Replay the completion cases against a real server.

Each case of the completion contracts is opened in a server, then the
completion list is checked against the assertions of the case: the same ones
the unit tests check through the providers.
"""
import argparse
import json
import os
from pathlib import Path
import tempfile
from urllib.parse import unquote, urlsplit

from contribution_process_test import Server


def insertion(item):
    # A client applies textEdit in preference to insertText.
    if "textEdit" in item:
        return item["textEdit"]["newText"]
    return item.get("insertText", item["label"])


def offset(text, position):
    lines = text.split("\n")
    before = "\n".join(lines[:position["line"]])
    line = lines[position["line"]].encode("utf-16-le")[:2 * position["character"]].decode("utf-16-le")
    return len(before) + (1 if position["line"] else 0) + len(line)


def applied(text, edit):
    span = edit.get("range") or edit["replace"]
    start, end = offset(text, span["start"]), offset(text, span["end"])
    return text[:start] + edit["newText"] + text[end:]


def failures(assertions, items, text):
    """The assertions of a case that the items break."""
    result = []
    labels = [item["label"] for item in items]

    def find(label):
        return next((item for item in items if item["label"] == label), None)

    for assertion in assertions:
        kind, label, value = assertion["kind"], assertion.get("label", ""), assertion.get("value")
        item = find(label)
        if kind == "present":
            ok = item is not None
        elif kind == "absent":
            ok = item is None
        elif kind == "count":
            ok = labels.count(label) == value
        elif kind == "before":
            ok = label in labels and value in labels and labels.index(label) < labels.index(value)
        elif kind in ("labelAt", "preselectAt", "insertStartsWith"):
            index = assertion["index"]
            ok = index < len(items) and {
                "labelAt": lambda at: at["label"] == label,
                "preselectAt": lambda at: at.get("preselect", False) == value,
                "insertStartsWith": lambda at: insertion(at).startswith(value),
            }[kind](items[index])
        elif item is None:
            ok = False
        elif kind == "editEquals":
            ok = insertion(item) == value
        elif kind == "insertContains":
            ok = value in insertion(item)
        elif kind == "insertNotContains":
            ok = value not in insertion(item)
        elif kind == "detailContains":
            ok = value in item.get("detail", "")
        elif kind == "applyContains":
            ok = "textEdit" in item and value in applied(text, item["textEdit"])
        else:
            raise AssertionError(f"unknown completion assertion: {kind}")
        if not ok:
            result.append(f"{kind}: {label} {value if value is not None else ''}".strip())
    return result


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--report", type=Path)
    parser.add_argument("--case-name", action="append", help="run matching case names only")
    parser.add_argument("--family", choices=["catalogue", "deployment", "project"], action="append")
    parser.add_argument("server", nargs=argparse.REMAINDER)
    args = parser.parse_args()
    command = args.server
    if command and command[0] == "--":
        command = command[1:]
    assert command, "provide a native server or a WASI runner command"
    fixtures = Path(__file__).parents[1] / "fixtures"
    results = []
    for family in args.family or ["catalogue", "deployment", "project"]:
        data = json.loads((fixtures / "lsp" / f"{family}-completion-contracts.json").read_text())
        for case in data["cases"]:
            if args.case_name and not any(name in case["name"] for name in args.case_name):
                continue
            marked = case["text"]
            before = marked[:marked.index("<|>")]
            position = {"line": before.count("\n"),
                        "character": len(before.rsplit("\n", 1)[-1].encode("utf-16-le")) // 2}
            text = marked.replace("<|>", "")
            documents = [*case.get("documents", []), {"uri": case["uri"], "text": text}]
            common = os.path.commonpath([str(Path(unquote(urlsplit(d["uri"]).path)).parent)
                                         for d in documents])
            with tempfile.TemporaryDirectory(prefix="xsmp-editor-completion-") as temporary:
                root = Path(temporary)
                mapped = {d["uri"]: root / Path(unquote(urlsplit(d["uri"]).path)).relative_to(common)
                          for d in documents}
                for document in documents:
                    path = mapped[document["uri"]]
                    path.parent.mkdir(parents=True, exist_ok=True)
                    path.write_text(document["text"], encoding="utf-8")
                uri = mapped[case["uri"]].as_uri()
                server = Server([*command, "--stdio"])
                try:
                    server.request("initialize", {
                        "processId": None, "rootUri": root.as_uri(),
                        "workspaceFolders": [{"uri": root.as_uri(), "name": "completion"}],
                        "capabilities": {
                            "workspace": {"workspaceFolders": True},
                            "textDocument": {"completion": {"completionItem": {
                                "snippetSupport": True, "documentationFormat": ["markdown", "plaintext"]}}}}})
                    server.send("initialized", {})
                    # The server answers once the workspace is scanned.
                    server.request("workspace/textDocumentContent", {"uri": uri})
                    for document in documents:
                        extension = mapped[document["uri"]].suffix.lstrip(".")
                        language = "xsmpproject" if extension in ["project", "xsmpprofile", "xsmptool"] else extension
                        server.send("textDocument/didOpen", {"textDocument": {
                            "uri": mapped[document["uri"]].as_uri(), "languageId": language,
                            "version": 1, "text": document["text"]}})
                    actual = server.request("textDocument/completion", {
                        "textDocument": {"uri": uri}, "position": position, "context": {"triggerKind": 1}})
                    errors = failures(case["assertions"], actual["items"], text)
                    results.append({"family": family, "name": case["name"], "errors": errors, "actual": actual})
                    print(f"{family}: {case['name']}: {'PASS' if not errors else '; '.join(errors[:3])}", flush=True)
                finally:
                    server.close()
    if args.report:
        args.report.write_text(json.dumps({"cases": results}, ensure_ascii=False, indent=2) + "\n")
    failed = sum(bool(case["errors"]) for case in results)
    print(f"{len(results) - failed}/{len(results)} editor cases passed")
    assert not failed, f"{failed} editor cases break their assertions"


if __name__ == "__main__":
    main()
