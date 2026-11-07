"""Process-level CLI and workspace contracts, for native and WASI binaries.

These tests check observable inputs, outputs and files, not the control flow of
the implementation.
"""
import argparse
import json
import os
import pty
from pathlib import Path
import re
import select
import shutil
import subprocess
import sys
import tempfile
import time
import unittest
import uuid

from contribution_process_test import Server


OPTIONS = None
TRANSCRIPTS = []


class Contracts(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory(prefix="xsmp-cli-contracts-")
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)

    def cli(self, *arguments, code=0, cwd=None, timeout=60):
        command = [*OPTIONS.runner, OPTIONS.cli, *map(str, arguments)]
        try:
            result = subprocess.run(command, capture_output=True, text=True,
                                    input="", cwd=cwd or self.root, timeout=timeout)
        except subprocess.TimeoutExpired as error:
            TRANSCRIPTS.append({"test": self.id(), "argv": command, "timeout": timeout,
                "stdout": str(error.stdout), "stderr": str(error.stderr)})
            raise
        record = {"test": self.id(), "argv": command, "cwd": str(cwd or self.root),
                  "code": result.returncode, "stdout": result.stdout, "stderr": result.stderr}
        TRANSCRIPTS.append(record)
        self.assertEqual(result.returncode, code, json.dumps(record, indent=2))
        return result

    def project(self, name="app", *, parent=None, tools=(), dependency=None,
                broken=False, namespace=None):
        directory = (parent or self.root) / name
        (directory / "src").mkdir(parents=True)
        descriptor = f'project "{name}" using "ECSS_SMP_2025"\nsource "src"\n'
        descriptor += "".join(f'tool "{tool}"\n' for tool in tools)
        if dependency:
            descriptor += f'dependency "{dependency}"\n'
        (directory / "xsmp.project").write_text(descriptor)
        namespace = namespace or name.replace("-", "_")
        member = "field MissingType missing" if broken else ""
        (directory / "src" / f"{namespace}.xsmpcat").write_text(
            f"catalogue {namespace}\nnamespace {namespace} {{\n"
            f"/** @uuid {uuid.uuid5(uuid.NAMESPACE_URL, namespace)} */\n"
            f"public struct Value {{ {member} }}\n}}\n")
        return directory

    def native_extension(self):
        extension = self.root / "native-extension"
        extension.mkdir()
        plugin = Path(OPTIONS.plugin)
        handler = extension / ("handler.wasm" if plugin.read_bytes()[:4] == b"\0asm" else "handler")
        shutil.copy2(plugin, handler)
        handler.chmod(0o755)
        entries = []
        for identifier in ("fake-tool", "fake-tool.long"):
            descriptor = extension / f"{identifier}.xsmptool"
            descriptor.write_text(f'tool "{identifier}"\n')
            entries.append({"descriptor": descriptor.name, "handler": handler.name,
                            "apiVersion": "^2.0.0", "aliases": [identifier + "-alias"],
                            "deprecatedAliases": [identifier + "-legacy"]})
        (extension / "package.json").write_text(json.dumps({
            "name": "contract-extension", "publisher": "test",
            "contributes": {"xsmp": entries}}))
        return extension

    def interactive_cli(self, arguments, responses):
        master, slave = pty.openpty()
        command = [*OPTIONS.runner, OPTIONS.cli, *map(str, arguments)]
        process = subprocess.Popen(command, stdin=slave, stdout=slave,
                                   stderr=slave, cwd=self.root)
        os.close(slave)
        transcript = bytearray()
        cursor = 0
        deadline = time.monotonic() + 45

        def read_available():
            if not select.select([master], [], [], 0.1)[0]:
                return False
            try:
                data = os.read(master, 65536)
            except OSError:
                return False
            transcript.extend(data)
            return bool(data)

        try:
            for question, answer in responses:
                needle = question.encode()
                while transcript.find(needle, cursor) < 0:
                    read_available()
                    self.assertIsNone(process.poll(), transcript.decode(errors="replace"))
                    self.assertLess(time.monotonic(), deadline, transcript.decode(errors="replace"))
                cursor = transcript.find(needle, cursor) + len(needle)
                os.write(master, (answer + "\n").encode())
            while process.poll() is None:
                read_available()
                self.assertLess(time.monotonic(), deadline, transcript.decode(errors="replace"))
            while read_available():
                pass
            output = transcript.decode(errors="replace")
            TRANSCRIPTS.append({"test": self.id(), "argv": command,
                "code": process.returncode, "terminalOutput": output})
            self.assertEqual(process.returncode, 0, output)
            return output
        finally:
            if process.poll() is None:
                process.kill()
                process.wait()
            os.close(master)


class CliContracts(Contracts):
    def test_parser_responses(self):
        fixture = json.loads((Path(__file__).parents[1] / 'fixtures/integration/cli-parser-contracts.json').read_text())
        for case in fixture['cases']:
            with self.subTest(argv=case['argv']):
                result = self.cli(*case['argv'], code=case['code'])
                self.assertEqual(result.stdout, case['stdout'])
                self.assertEqual(result.stderr, case['stderr'])

    def test_help_context_streams_and_versions(self):
        for args, first_line in (((), "Usage: xsmp [options] [command]"),
                (("new",), "Usage: xsmp new [options] [command]")):
            with self.subTest(args=args):
                result = self.cli(*args, code=2)
                self.assertEqual(result.stdout, "")
                self.assertTrue(result.stderr.startswith(first_line), result.stderr)
        for args, context in ((("help",), "xsmp [options] [command]"),
                (("validate", "--help"), "xsmp validate [options] <path>"),
                (("help", "generate"), "xsmp generate [options] <path>"),
                (("new", "--help"), "xsmp new [options] [command]"),
                (("new", "help", "project"), "xsmp new project [options] [name] [directory]"),
                (("--bogus", "--help"), "xsmp [options] [command]")):
            with self.subTest(args=args):
                result = self.cli(*args)
                self.assertTrue(result.stdout.startswith("Usage: " + context), result.stdout)
                self.assertEqual(result.stderr, "")
        for args in (("validate",), ("generate", "missing", "--bogus"),
                     ("new", "--bogus"), ("validate", "missing", "-w")):
            with self.subTest(args=args):
                result = self.cli(*args, code=2)
                self.assertEqual(result.stdout, "")
                self.assertTrue(result.stderr.startswith("error:"), result.stderr)
                self.assertIn("Usage: xsmp ", result.stderr)
        # A folder that holds no project is a mistake in the path, not a clean run.
        empty = self.root / "no-project"
        empty.mkdir()
        (empty / "stray.xsmpcat").write_text("catalogue stray\n")
        for command in ("validate", "generate"):
            with self.subTest(command=command):
                result = self.cli(command, empty, code=2)
                self.assertEqual(result.stdout, "")
                self.assertIn("does not contain an 'xsmp.project' file", result.stderr)
        for args in (("--version",), ("-V",), ("validate", "missing", "--version"),
                     ("new", "project", "--version"), ("--version", "ignored")):
            with self.subTest(args=args):
                result = self.cli(*args)
                self.assertEqual(result.stdout, OPTIONS.project_version + "\n")
                self.assertEqual(result.stderr, "")

    def test_long_equals_attached_values_bundles_and_option_terminator(self):
        project = self.project()
        for workspace in ("--workspace-root=" + str(self.root), "-w" + str(self.root)):
            self.cli("validate", project, workspace)
        for index, flags in enumerate((("--profile=xsmp-sdk", "--tool=smp", "--yes"),
                ("--tool=python", "--no-interactive"), ("-y", "--", "Third", str(self.root)))):
            if "--" in flags:
                self.cli("new", "project", *flags)
                self.assertTrue((self.root / "Third/xsmp.project").is_file())
            else:
                self.cli("new", "project", f"Equals{index}", self.root, *flags)
                self.assertTrue((self.root / f"Equals{index}/xsmp.project").is_file())
        fixture = Path(__file__).parents[1] / "fixtures/workspace/smp/test.smpcat"
        source = self.root / "source.smpcat"
        shutil.copy2(fixture, source)
        for index, flag in enumerate(("--output=", "-o", "-fo")):
            target = self.root / f"output{index}.xsmpcat"
            if flag == "-fo":
                target.write_text("old output")
            self.cli("import-smp", source, flag + str(target))
            self.assertIn("catalogue", target.read_text())
        self.cli("import-smp", source, "--force=true", code=2)
        self.cli("new", "project", "BadBoolean", self.root, "--no-interactive=false", code=2)
        dashed = self.project("Dash").rename(self.root / "-dash")
        self.cli("validate", "--", dashed.name)

    def test_removed_aliases_are_unknown_options(self):
        for arguments, option in ((("validate", "x", "--workspace", self.root), "--workspace"),
                                  (("validate", "x", "--root", self.root), "--root"),
                                  (("generate", "x", "-d", self.root), "-d"),
                                  (("generate", "x", "--destination", self.root), "--destination")):
            with self.subTest(option=option):
                result = self.cli(*arguments, code=2)
                self.assertTrue(result.stderr.startswith(f"error: unknown option '{option}'\n"), result.stderr)

    def test_an_option_without_its_value_names_the_option_as_written(self):
        for option in ("-w", "--workspace-root", "-o"):
            with self.subTest(option=option):
                command = ("generate" if option == "-o" else "validate", "x", option)
                result = self.cli(*command, code=2)
                self.assertTrue(result.stderr.startswith(f"error: option '{option}' needs a value\n"), result.stderr)

    def test_explicit_empty_profile_is_an_error_instead_of_no_selection(self):
        for flags in (("--profile", ""), ("--profile=",)):
            result = self.cli("new", "project", "EmptyProfile", self.root, *flags, "--yes", code=2)
            self.assertIn("profile", result.stderr.lower())
            self.assertFalse((self.root / "EmptyProfile").exists())

    def test_terminal_explicit_dot_parent_does_not_prompt_again(self):
        output = self.interactive_cli(["new", "project", "DotParent", ".",
            "--profile", "xsmp-sdk", "--tool", "smp"], [("Create this project?", "y")])
        self.assertNotIn("Parent directory:", output)
        self.assertTrue((self.root / "DotParent/xsmp.project").is_file())

    def test_terminal_selections_retry_invalid_answers_and_none_is_only_a_tool_token(self):
        output = self.interactive_cli(["new", "project", "Retried", self.root],
            [("Select a profile", "unknown"), ("Select a profile", "NONE"),
             ("Select a profile", "__no_profile__"), ("Select tools", "bad"),
             ("Select tools", "1,,2"), ("Select tools", "1,"),
             ("Select tools", "NONE"), ("Create this project?", "true"),
             ("Create this project?", "1"), ("Create this project?", " YES ")])
        self.assertEqual(output.count("Please enter one of the listed numbers or identifiers."), 2)
        self.assertEqual(output.count("Please enter comma-separated numbers or identifiers from the list."), 3)
        self.assertEqual(output.count("Please answer yes or no."), 2)
        text = (self.root / "Retried/xsmp.project").read_text()
        self.assertFalse(re.search(r"^\s*(?:profile|tool)\s+['\"]", text, re.M))

    def test_terminal_numeric_choices_take_decimal_digits_only(self):
        for index, selection in enumerate(("+1", "1.0", "0x1", "0b1", "0o1")):
            output = self.interactive_cli(["new", "project", f"Numeric{index}", self.root],
                [("Select a profile", "__no_profile__"), ("Select tools", selection),
                 ("Select tools", "1"), ("Create this project?", "y")])
            self.assertEqual(output.count("Please enter comma-separated numbers or identifiers from the list."), 1,
                             selection)
            text = (self.root / f"Numeric{index}/xsmp.project").read_text()
            self.assertIn("tool 'adoc'", text)

    def test_terminal_prompts_retry_until_the_answer_is_valid(self):
        extension = self.native_extension()
        output = self.interactive_cli(["new", "project", "Prompts", self.root,
            "--extension", extension, "--tool", "fake-tool"],
            [("Select a profile", "__no_profile__"), ("fake-tool: Module name", "custom"),
             ("fake-tool: Enabled", "maybe"), ("fake-tool: Enabled", "YES"),
             ("fake-tool: Flavor", "unknown"), ("fake-tool: Flavor", "+2"),
             ("fake-tool: Flavor", "2"), ("Create this project?", "y")])
        self.assertIn("Please answer yes or no.", output)
        self.assertEqual(output.count("Please enter one of the listed numbers or identifiers."), 2)
        self.assertEqual((self.root / "Prompts/enabled.txt").read_text(), "true")
        self.assertEqual((self.root / "Prompts/flavor.txt").read_text(), "second")

    def test_unknown_generator_preflight_preserves_existing_outputs(self):
        project = self.project(tools=("smp",))
        target = project / "smdl-gen"
        target.mkdir()
        (target / "keep.txt").write_text("must survive failed generator selection")
        result = self.cli("generate", project, "--tool", "unknown-tool", code=2)
        self.assertIn("unknown-tool", result.stderr)
        self.assertEqual((target / "keep.txt").read_text(), "must survive failed generator selection")
        self.cli("generate", project, "--profile=", code=2)
        self.assertEqual((target / "keep.txt").read_text(), "must survive failed generator selection")

    def test_invalid_project_name_does_not_load_an_external_handler(self):
        extension = self.root / "marker-extension"
        extension.mkdir()
        # A file no server can load: only a name checked first gives this answer.
        (extension / "handler").write_text("not a library")
        (extension / "marker.xsmptool").write_text('tool "marker"\n')
        (extension / "package.json").write_text(json.dumps({
            "name": "marker", "publisher": "test", "contributes": {"xsmp": [{
                "descriptor": "marker.xsmptool", "handler": "handler", "apiVersion": "^2.0.0"}]}}))
        result = self.cli("new", "project", "1invalid", self.root,
            "--extension", extension, "--yes", code=2)
        self.assertIn("Project name must follow", result.stderr)

    def test_help_exposes_commands_and_scriptable_options(self):
        result = self.cli("--help")
        for word in ("validate", "generate", "import-smp", "new"):
            self.assertIn(word, result.stdout)
        result = self.cli("new", "project", "--help")
        for spelling in ("[name]", "[directory]", "--profile <id>", "--tool <id>",
                         "--set <key=value>", "--no-interactive", "--yes",
                         "profile.<id>.<prompt>", "tool.<id>.<prompt>"):
            self.assertIn(spelling, result.stdout)
        self.assertNotIn("--no-profile", result.stdout)
        self.assertNotIn("--no-tools", result.stdout)
        self.assertIn("--extension <dir>", result.stdout)
        result = self.cli("generate", "--help")
        for spelling in ("--output <dir>", "--profile <id>", "--tool <id>", "--extension <dir>"):
            self.assertIn(spelling, result.stdout)
        self.assertIn("--extension <dir>", self.cli("validate", "--help").stdout)

    def test_omitted_profile_and_tools_stay_disabled_in_each_scriptable_mode(self):
        for index, flags in enumerate(((), ("--yes",), ("--no-interactive",))):
            name = f"Minimal{index}"
            with self.subTest(flags=flags):
                self.cli("new", "project", name, self.root, *flags)
                project = self.root / name
                text = (project / "xsmp.project").read_text()
                self.assertFalse(re.search(r"^\s*(?:profile|tool)\s+['\"]", text, re.M))
                self.assertTrue((project / "smdl" / f"{name}.xsmpcat").is_file())
                self.assertFalse((project / "CMakeLists.txt").exists())

    def test_complete_selected_project_and_tool_deduplication_keep_order(self):
        self.cli("new", "project", "Mission", self.root, "--profile", "xsmp-sdk",
                 "--tool", "smp", "--tool", "python", "--tool", "smp", "--no-interactive")
        project = self.root / "Mission"
        text = (project / "xsmp.project").read_text()
        self.assertIn("profile 'xsmp-sdk'", text)
        self.assertEqual(text.count("tool 'smp'"), 1)
        self.assertEqual(text.count("tool 'python'"), 1)
        self.assertLess(text.index("tool 'smp'"), text.index("tool 'python'"))
        for relative in ("CMakeLists.txt", "README.md", "pytest.ini", "python/Mission/test_Mission.py"):
            self.assertTrue((project / relative).is_file(), relative)

    def test_noninteractive_missing_name_returns_two_and_creates_nothing(self):
        result = self.cli("new", "project", code=2)
        self.assertRegex(result.stderr.lower(), r"project name.*required")
        self.assertEqual(list(self.root.iterdir()), [])

    def test_terminal_empty_name_reprompts_and_empty_parent_uses_default(self):
        output = self.interactive_cli(
            ["new", "project", "--profile", "xsmp-sdk", "--tool", "smp"],
            [("Project name", ""), ("Project name", "Valid-App"),
             ("Parent directory", ""), ("Create this project?", "yes")])
        self.assertIn("Project name must follow", output)
        self.assertTrue((self.root / "Valid-App/xsmp.project").is_file())

    def test_terminal_can_explicitly_select_no_profile_and_no_tools(self):
        self.interactive_cli(["new", "project", "Minimal", self.root],
            [("Select a profile", "__no_profile__"), ("Select tools", "none"),
             ("Create this project?", "yes")])
        text = (self.root / "Minimal/xsmp.project").read_text()
        self.assertFalse(re.search(r"^\s*(?:profile|tool)\s+['\"]", text, re.M))

    def test_terminal_confirmation_is_case_insensitive(self):
        self.interactive_cli(["new", "project", "Accepted", self.root,
            "--profile", "xsmp-sdk", "--tool", "smp"], [("Create this project?", "YES")])
        self.assertTrue((self.root / "Accepted/xsmp.project").is_file())

    def test_terminal_cancellation_writes_nothing(self):
        output = self.interactive_cli(["new", "project", "Cancelled", self.root,
            "--profile", "xsmp-sdk", "--tool", "smp"], [("Create this project?", "no")])
        self.assertIn("cancel", output.lower())
        self.assertFalse((self.root / "Cancelled").exists())

    def test_redirected_stdout_disables_prompts_even_with_terminal_stdin(self):
        master, slave = pty.openpty()
        process = subprocess.Popen([*OPTIONS.runner, OPTIONS.cli, "new", "project",
            "Redirected", str(self.root)], stdin=slave, stdout=subprocess.PIPE,
            stderr=subprocess.PIPE, cwd=self.root)
        os.close(slave)
        try:
            stdout, stderr = process.communicate(timeout=15)
            self.assertEqual(process.returncode, 0, (stdout, stderr))
            text = (self.root / "Redirected/xsmp.project").read_text()
            self.assertFalse(re.search(r"^\s*(?:profile|tool)\s+['\"]", text, re.M))
        finally:
            if process.poll() is None:
                process.kill()
                process.communicate()
            os.close(master)

    def test_project_names_follow_the_pattern_and_normalize_catalogue_identifier(self):
        for name in ("1bad", "_bad", ".hidden", "bad name", "../escape", "bad/name", "bad'name"):
            with self.subTest(name=name):
                result = self.cli("new", "project", name, self.root, "--yes", code=2)
                self.assertIn("Project name must follow", result.stderr)
        self.cli("new", "project", "My.lib-core", self.root, "--yes")
        text = (self.root / "My.lib-core/smdl/My_lib_core.xsmpcat").read_text()
        self.assertIn("catalogue My_lib_core", text)
        self.assertIn("namespace My_lib_core", text)

    def test_a_new_project_with_a_dash_in_its_name_satisfies_the_tas_mdk_checks(self):
        self.cli("new", "project", "my-proj", self.root, "--profile", "tas-mdk", "--yes")
        result = self.cli("validate", self.root / "my-proj", code=1)
        self.assertNotIn("must match the file name", result.stderr)

    def test_a_new_project_creates_its_missing_parents(self):
        parent = self.root / "not/yet/there"
        self.cli("new", "project", "Mission", parent, "--yes")
        self.assertTrue((parent / "Mission/xsmp.project").is_file())

    def test_wrong_kind_and_unknown_contributions_fail_before_creating_target(self):
        for index, (flag, identifier, words) in enumerate((
                ("--profile", "missing-profile", ("profile", "missing-profile")),
                ("--tool", "missing-tool", ("tool", "missing-tool")),
                ("--profile", "python", ("profile", "python", "tool")),
                ("--tool", "xsmp-sdk", ("tool", "xsmp-sdk", "profile")))):
            with self.subTest(flag=flag, identifier=identifier):
                name = f"Invalid{index}"
                result = self.cli("new", "project", name, self.root, flag, identifier, "--yes", code=2)
                for word in words:
                    self.assertIn(word, result.stderr.lower())
                self.assertFalse((self.root / name).exists())

    def test_repeated_profile_and_removed_options_return_two(self):
        self.cli("new", "project", "Repeated", self.root, "--profile", "xsmp-sdk",
                 "--profile", "xsmp-sdk", "--yes", code=2)
        for flag in ("--no-profile", "--no-tools"):
            with self.subTest(flag=flag):
                result = self.cli("new", "project", "Removed", self.root, flag, "--yes", code=2)
                self.assertIn(f"unknown option '{flag}'", result.stderr.lower())
        self.assertEqual(list(self.root.iterdir()), [])

    def test_existing_target_is_byte_for_byte_untouched(self):
        target = self.root / "Existing"
        target.mkdir()
        (target / "keep.bin").write_bytes(b"keep\x00this\xff")
        result = self.cli("new", "project", "Existing", self.root, "--yes", code=2)
        self.assertIn("already exists", result.stderr)
        self.assertEqual([p.name for p in target.iterdir()], ["keep.bin"])
        self.assertEqual((target / "keep.bin").read_bytes(), b"keep\x00this\xff")

    def test_settings_use_aliases_longest_id_and_preserve_typed_values(self):
        extension = self.native_extension()
        self.cli("new", "project", "Settings", self.root, "--extension", extension,
                 "--tool", "fake-tool.long-alias",
                 "--set", "tool.fake-tool.long-legacy.moduleName=Chosen=Value",
                 "--set", "tool.fake-tool.long.enabled=FALSE",
                 "--set", "tool.fake-tool.long.flavor=second", "--yes")
        project = self.root / "Settings"
        self.assertEqual((project / "generated.txt").read_text(), "Chosen=Value")
        self.assertEqual((project / "enabled.txt").read_text(), "false")
        self.assertEqual((project / "flavor.txt").read_text(), "second")
        self.assertIn("tool 'fake-tool.long'", (project / "xsmp.project").read_text())

    def test_invalid_and_omitted_settings_fail_without_partial_projects(self):
        extension = self.native_extension()
        for index, setting in enumerate(("malformed", "=empty", "   =value", "wrong.fake.x=y",
                "tool.fake-tool.unknown=value", "tool.fake-tool.enabled=maybe",
                "tool.fake-tool.flavor=third", "tool.fake-tool.long.moduleName=unselected")):
            with self.subTest(setting=setting):
                name = f"BadSettings{index}"
                self.cli("new", "project", name, self.root, "--extension", extension,
                         "--tool", "fake-tool", "--set", setting, "--yes", code=2)
                self.assertFalse((self.root / name).exists())
        for setting in ("profile.xsmp-sdk.unselected=value", "tool.python.unselected=value"):
            result = self.cli("new", "project", "Omitted", self.root, "--set", setting, "--yes", code=2)
            self.assertIn("not selected", result.stderr.lower())

    def test_settings_duplicate_after_canonicalizing_aliases_is_rejected(self):
        extension = self.native_extension()
        for index, second_key in enumerate(("tool.fake-tool.moduleName", "tool.fake-tool-alias.moduleName")):
            with self.subTest(second_key=second_key):
                name = f"DuplicateSettings{index}"
                result = self.cli("new", "project", name, self.root,
                    "--extension", extension, "--tool", "fake-tool",
                    "--set", "tool.fake-tool.moduleName=first", "--set", second_key + "=second", "--yes", code=2)
                self.assertIn("more than once", result.stderr)
                self.assertFalse((self.root / name).exists())

    def test_sibling_dependencies_are_discovered_by_default(self):
        self.project("foundation")
        project = self.project(dependency="foundation")
        result = self.cli("validate", project)
        self.assertIn("0 errors, 0 warnings", result.stdout)

    def test_a_trailing_separator_or_dot_names_the_same_project(self):
        self.project("foundation")
        project = self.project(dependency="foundation")
        for path, cwd in ((str(project) + os.sep, None), (".", project), (project.name + os.sep, self.root)):
            with self.subTest(path=path):
                result = self.cli("validate", path, cwd=cwd)
                self.assertIn("0 errors, 0 warnings", result.stdout)
                self.cli("generate", path, cwd=cwd)
        # Also for the workspace root.
        self.assertIn("0 errors", self.cli("validate", project, "-w", str(self.root) + os.sep).stdout)

    def test_source_warnings_of_an_unrelated_project_are_not_reported(self):
        project = self.project()
        other = self.project("other")
        unsupported = ('<Schedule xmlns="http://www.ecss.nl/smp/2025/Smdl/Schedule" Name="Plan" '
                       'xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"><Task Name="Main">'
                       '<Activity xsi:type="Schedule:Unknown"/></Task></Schedule>')
        (other / "src/plan.smpsed").write_text(unsupported)
        self.assertIn("0 errors, 0 warnings", self.cli("validate", project).stdout)
        result = self.cli("validate", other, code=1)
        self.assertIn("1 warnings", result.stdout)
        self.assertIn("plan.smpsed", result.stderr)

    def test_an_extension_can_choose_its_handler_for_each_kind_of_server(self):
        plugin = Path(OPTIONS.plugin)
        if plugin.read_bytes()[:4] == b"\0asm":
            self.skipTest("the native handler of a native server")
        extension = self.native_extension()
        package = extension / "package.json"
        metadata = json.loads(package.read_text())
        for entry in metadata["contributes"]["xsmp"]:
            entry["nativeHandler"] = entry.pop("handler")
        package.write_text(json.dumps(metadata))
        self.cli("new", "project", "Native", self.root, "--extension", extension, "--tool", "fake-tool", "--yes")
        for entry in metadata["contributes"]["xsmp"]:
            del entry["nativeHandler"]
        package.write_text(json.dumps(metadata))
        result = self.cli("new", "project", "Unhandled", self.root, "--extension", extension, "--yes", code=2)
        self.assertIn("no handler for this platform", result.stderr)

    def test_an_extension_can_provide_the_tas_mdk_profile(self):
        extension = self.root / "tas-extension"
        extension.mkdir()
        plugin = Path(OPTIONS.plugin)
        handler = extension / ("handler.wasm" if plugin.read_bytes()[:4] == b"\0asm" else "handler")
        shutil.copy2(plugin, handler)
        handler.chmod(0o755)
        (extension / "tas-mdk.xsmpprofile").write_text("profile 'tas-mdk'\n")
        (extension / "package.json").write_text(json.dumps({
            "name": "tas", "publisher": "test", "contributes": {"xsmp": [{
                "descriptor": "tas-mdk.xsmpprofile", "handler": handler.name, "apiVersion": "^2.0.0"}]}}))
        self.cli("new", "project", "Tas", self.root, "--extension", extension, "--profile", "tas-mdk", "--yes",
                 "--set", "profile.tas-mdk.moduleName=mine")
        # What the handler of this extension writes: the built-in profile has no such prompt.
        self.assertEqual((self.root / "Tas/generated.txt").read_text(), "mine")

    def test_only_the_dependency_closure_is_loaded(self):
        self.project("foundation")
        self.project("unrelated", dependency="missing")
        project = self.project(dependency="foundation")
        self.assertIn("0 errors, 0 warnings", self.cli("validate", project).stdout)
        # A project named instead of a path selects the same.
        self.assertIn("0 errors, 0 warnings", self.cli("validate", "app", "-w", self.root).stdout)

    def test_projects_under_lib_and_coverage_are_found(self):
        for directory in ("lib", "coverage"):
            with self.subTest(directory=directory):
                library = f"{directory}_foundation"
                self.project(library, parent=self.root / directory)
                project = self.project(f"{directory}_app", dependency=library)
                result = self.cli("validate", project, "--workspace-root", self.root)
                self.assertIn("0 errors, 0 warnings", result.stdout)

    def test_explicit_workspace_also_scans_target_outside_that_root(self):
        external = self.root / "external"
        self.project("foundation", parent=external)
        project = self.project(parent=self.root / "targets", dependency="foundation")
        result = self.cli("validate", project, "--workspace-root", external)
        self.assertIn("0 errors, 0 warnings", result.stdout)

    def test_directory_descriptor_and_project_name_select_the_same_project(self):
        project = self.project()
        self.project("unrelated", broken=True)
        for selection in (project, project / "xsmp.project", "app"):
            with self.subTest(selection=str(selection)):
                result = self.cli("validate", selection, "--workspace-root", self.root)
                self.assertNotIn("unrelated.xsmpcat", result.stderr)
                self.assertIn("0 errors, 0 warnings", result.stdout)

    def test_missing_dependency_has_diagnostic_and_summary(self):
        project = self.project(dependency="foundation")
        result = self.cli("validate", project, "--workspace-root", self.root, code=1)
        self.assertIn('Missing dependency project "foundation"', result.stderr)
        self.assertIn("1 errors, 0 warnings", result.stdout)

    def test_invalid_dependency_closure_blocks_generation_and_keeps_the_output(self):
        self.project("foundation", broken=True)
        project = self.project(tools=("smp",), dependency="foundation")
        output = project / "smdl-gen"
        output.mkdir()
        (output / "keep.txt").write_text("kept when validation fails")
        result = self.cli("generate", project, "--workspace-root", self.root, code=1)
        self.assertIn("1 errors, 0 warnings", result.stdout)
        self.assertEqual((output / "keep.txt").read_text(), "kept when validation fails")
        self.assertFalse((output / "app.smpcat").exists())

    def test_project_without_contributions_generates_no_outputs_and_succeeds(self):
        project = self.project()
        before = {p.relative_to(project) for p in project.rglob("*")}
        result = self.cli("generate", project, "--workspace-root", self.root)
        self.assertIn('Nothing to generate for project "app"', result.stdout)
        self.assertNotIn("Generated", result.stdout)
        self.assertEqual({p.relative_to(project) for p in project.rglob("*")}, before)
        catalogue = project / "src/app.xsmpcat"
        self.assertIn("Nothing to generate:", self.cli("generate", catalogue.rename(self.root / "alone.xsmpcat")).stdout)

    def test_generation_removes_stale_tool_outputs_and_preserves_python_user_files(self):
        project = self.project(tools=("python", "smp"))
        self.cli("generate", project, "--workspace-root", self.root)
        target = project / "python/app/__init__.py"
        self.assertTrue(target.is_file())
        (project / "python/stale.txt").write_text("stale")
        (project / "smdl-gen/stale.txt").write_text("stale generated output")
        self.cli("generate", project, "--workspace-root", self.root)
        self.assertEqual((project / "python/stale.txt").read_text(), "stale")
        self.assertFalse((project / "smdl-gen/stale.txt").exists())

    def test_output_write_failure_returns_two_instead_of_silently_succeeding(self):
        project = self.project(tools=("python",))
        obstruction = project / "python/app/__init__.py"
        obstruction.mkdir(parents=True)
        result = self.cli("generate", project, "--workspace-root", self.root, code=2)
        self.assertIn("Error generating file", result.stderr)
        self.assertIn("python/app/__init__.py", result.stderr)

    def test_scaffolder_failure_rolls_back_its_partial_output(self):
        result = self.cli("new", "project", "FailingScaffold", self.root, "--tool", "fake-tool",
                          "--extension", self.native_extension(), "--yes", code=2)
        self.assertIn("scaffold failure", result.stderr)
        self.assertNotIn('"error"', result.stderr)
        self.assertFalse((self.root / "FailingScaffold").exists())

    def test_a_stale_file_that_cannot_be_removed_fails_generation(self):
        # A stale source left behind can break a build.
        project = self.project(tools=("fake-tool",))
        locked = project / "custom-gen/locked"
        locked.mkdir(parents=True)
        (locked / "stale.txt").write_text("stale")
        locked.chmod(0o500)
        self.addCleanup(locked.chmod, 0o700)
        if os.access(locked, os.W_OK):
            self.skipTest("this user can write everywhere")
        result = self.cli("generate", project, "-w", self.root, "--extension", self.native_extension(),
                          code=2)
        self.assertIn("Cannot remove generated path", result.stderr)
        self.assertTrue(list((project / "custom-gen").rglob("*.txt")))

    def test_a_failed_generation_keeps_the_previous_files(self):
        project = self.project(tools=("fake-tool",))
        extension = self.native_extension()
        self.cli("generate", project, "-w", self.root, "--extension", extension)
        output = project / "custom-gen/fake-tool/app.txt"
        self.assertTrue(output.is_file())
        stale = project / "custom-gen/stale.txt"
        stale.write_text("stale")
        (project / "fail-generation").write_text("")
        result = self.cli("generate", project, "-w", self.root, "--extension", extension, code=2)
        self.assertIn("generation failure", result.stderr)
        self.assertTrue(output.is_file())
        self.assertTrue(stale.is_file())

    def test_generation_removes_the_stale_files_and_keeps_the_unchanged_ones(self):
        project = self.project(tools=("fake-tool",))
        extension = self.native_extension()
        self.cli("generate", project, "-w", self.root, "--extension", extension)
        output = project / "custom-gen/fake-tool/app.txt"
        before = output.stat()
        (project / "custom-gen/old").mkdir()
        (project / "custom-gen/old/stale.txt").write_text("stale")
        self.cli("generate", project, "-w", self.root, "--extension", extension)
        self.assertFalse((project / "custom-gen/old").exists())
        after = output.stat()
        self.assertEqual((after.st_ino, after.st_mtime_ns), (before.st_ino, before.st_mtime_ns))

    def sdk_project(self):
        return self.sdk_project_in("app")

    def sdk_project_in(self, name):
        project = self.project(name, namespace="app")
        (project / "xsmp.project").write_text(
            f'project "{name}" using "ECSS_SMP_2025"\nsource "src"\nprofile "xsmp-sdk"\n')
        source = project / "src/app.xsmpcat"
        source.write_text(f"""catalogue app
namespace app {{
/** @uuid {uuid.uuid5(uuid.NAMESPACE_URL, 'Kept')} */
public struct Kept {{ field Smp.Int32 a }}
}}
namespace old {{
namespace deep {{
/** @uuid {uuid.uuid5(uuid.NAMESPACE_URL, 'Removed')} */
public struct Removed {{ field Smp.Int32 a }}
}}
}}
""")
        return project, source

    def test_a_type_removed_from_the_catalogue_loses_its_files_and_folders(self):
        project, source = self.sdk_project()
        self.cli("generate", project, "-w", self.root)
        self.assertTrue((project / "src-gen/old/deep/Removed.h").is_file())
        self.assertTrue((project / "src-gen/old/deep/Removed.cpp").is_file())
        kept = project / "src-gen/app/Kept.h"
        before = kept.stat()
        text = source.read_text()
        source.write_text(text[:text.index("namespace old")])
        self.cli("generate", project, "-w", self.root)
        self.assertFalse((project / "src-gen/old").exists())
        after = kept.stat()
        self.assertEqual((after.st_ino, after.st_mtime_ns), (before.st_ino, before.st_mtime_ns))

    def test_an_unchanged_file_keeps_its_time_and_inode(self):
        project, source = self.sdk_project()
        self.cli("generate", project, "-w", self.root)
        files = sorted(path for path in (project / "src-gen").rglob("*") if path.is_file())
        self.assertGreater(len(files), 3)
        before = [(path.stat().st_ino, path.stat().st_mtime_ns) for path in files]
        self.cli("generate", project, "-w", self.root)
        self.assertEqual([(path.stat().st_ino, path.stat().st_mtime_ns) for path in files], before)
        # One that changes does change.
        source.write_text(source.read_text().replace("field Smp.Int32 a }\n}\nnamespace old",
                                                      "field Smp.Int32 b }\n}\nnamespace old"))
        self.cli("generate", project, "-w", self.root)
        self.assertIn(" b;", (project / "src-gen/app/Kept.h").read_text())

    def test_generation_never_writes_through_a_link(self):
        project = self.project(tools=("smp",))
        external = self.root / "external"
        external.mkdir()
        (external / "app.smpcat").write_text("user file")
        (project / "smdl-gen").symlink_to(external)
        self.cli("generate", project, "-w", self.root)
        self.assertFalse((project / "smdl-gen").is_symlink())
        self.assertEqual((external / "app.smpcat").read_text(), "user file")
        self.assertTrue((project / "smdl-gen/app.smpcat").is_file())

        (project / "smdl-gen/nested").mkdir()
        (project / "smdl-gen/nested/link").symlink_to(external)
        (project / "smdl-gen/app.smpcat").unlink()
        (project / "smdl-gen/app.smpcat").symlink_to(external / "app.smpcat")
        self.cli("generate", project, "-w", self.root)
        self.assertEqual((external / "app.smpcat").read_text(), "user file")
        self.assertFalse((project / "smdl-gen/app.smpcat").is_symlink())
        self.assertFalse((project / "smdl-gen/nested").exists())

    def test_an_obstacle_in_a_generated_folder_does_not_fail_generation(self):
        project = self.project(tools=("smp",))
        (project / "smdl-gen/app.smpcat").mkdir(parents=True)
        (project / "smdl-gen/app.smpcat/old.txt").write_text("in the way")
        self.cli("generate", project, "-w", self.root)
        self.assertTrue((project / "smdl-gen/app.smpcat").is_file())

        shutil.rmtree(project / "smdl-gen")
        (project / "smdl-gen").write_text("a file where a folder goes")
        self.cli("generate", project, "-w", self.root)
        self.assertTrue((project / "smdl-gen/app.smpcat").is_file())

        sdk, _ = self.sdk_project_in("other")
        self.cli("generate", sdk, "-w", self.root)
        (sdk / "src-gen/app/Kept.h").unlink()
        shutil.rmtree(sdk / "src-gen/old")
        (sdk / "src-gen/old").write_text("a file where a folder goes")
        self.cli("generate", sdk, "-w", self.root)
        self.assertTrue((sdk / "src-gen/app/Kept.h").is_file())
        self.assertTrue((sdk / "src-gen/old/deep/Removed.h").is_file())

    def test_an_output_folder_loses_what_is_stale_and_keeps_what_is_unchanged(self):
        project = self.project("two", tools=("smp", "adoc"))
        output = self.root / "shared-output"
        (output / "smdl-gen").mkdir(parents=True)
        (output / "smdl-gen/stale.txt").write_text("stale")
        self.cli("generate", project, "--output", output)
        self.assertTrue((output / "smdl-gen/two.smpcat").is_file())
        self.assertTrue((output / "adoc-gen/two-gen.adoc").is_file())
        self.assertFalse((output / "smdl-gen/stale.txt").exists())
        before = (output / "smdl-gen/two.smpcat").stat()
        self.cli("generate", project, "--output", output)
        after = (output / "smdl-gen/two.smpcat").stat()
        self.assertEqual((after.st_ino, after.st_mtime_ns), (before.st_ino, before.st_mtime_ns))

    def test_all_five_smp_import_commands_report_the_document_kind(self):
        fixtures = Path(__file__).parents[1] / "fixtures/workspace/smp"
        for extension, native, kind in (("smpcat", "xsmpcat", "catalogue"),
                ("smpcfg", "xsmpcfg", "configuration"), ("smplnk", "xsmplnk", "linkbase"),
                ("smpasb", "xsmpasb", "assembly"), ("smpsed", "xsmpsed", "schedule")):
            with self.subTest(extension=extension):
                source = self.root / f"demo.{extension}"
                shutil.copy2(fixtures / f"test.{extension}", source)
                result = self.cli("import-smp", source)
                self.assertTrue(source.with_suffix("." + native).is_file())
                self.assertIn("Imported " + kind, result.stdout)
        source = self.root / "demo.smpcfg"
        target = self.root / "generated/explicit.xsmpcfg"
        self.cli("import-smp", source, "--output", target)
        self.assertTrue(target.is_file())

    def test_smp_overwrite_refusal_is_clean_and_force_replaces_output(self):
        fixtures = Path(__file__).parents[1] / "fixtures/workspace/smp"
        source = self.root / "demo.smpcat"
        shutil.copy2(fixtures / "test.smpcat", source)
        target = source.with_suffix(".xsmpcat")
        target.write_text("existing")
        result = self.cli("import-smp", source, code=2)
        self.assertIn("Refusing to overwrite existing file", result.stderr)
        self.assertNotRegex(result.stderr, r"\n\s+at ")
        self.assertEqual(target.read_text(), "existing")
        self.cli("import-smp", source, "--force")
        self.assertNotEqual(target.read_text(), "existing")
        package = self.root / "demo.smppkg"
        shutil.copy2(fixtures / "test.smppkg", package)
        result = self.cli("import-smp", package, code=2)
        self.assertIn("Unsupported SMP XML root", result.stderr)


class WorkspaceContracts(Contracts):
    def setUp(self):
        super().setUp()
        if not OPTIONS.lsp:
            self.skipTest("--lsp required for workspace protocol contracts")

    def server(self, folder):
        server = Server([*OPTIONS.runner, OPTIONS.lsp, "--stdio"])
        self.addCleanup(self.stop, server)
        server.request("initialize", {"processId": None, "rootUri": None,
            "workspaceFolders": [{"name": folder.name, "uri": folder.as_uri()}], "capabilities": {}})
        server.send("initialized", {})
        server.request("workspace/symbol", {"query": "Barrier"})
        return server

    @staticmethod
    def stop(server):
        if server.process.poll() is None:
            try:
                server.close()
            except (AssertionError, BrokenPipeError):
                pass
            finally:
                if server.process.poll() is None:
                    server.process.kill()
                    server.process.wait()
        for pipe in (server.process.stdin, server.process.stdout, server.process.stderr):
            if pipe and not pipe.closed:
                pipe.close()

    def test_added_folders_are_indexed_and_removed_folders_disappear(self):
        first = self.project("first", namespace="ContractFirst")
        second = self.project("second", namespace="ContractSecond")
        server = self.server(first)
        self.assertFalse(server.request("workspace/symbol", {"query": "ContractSecond.Value"}))
        folder = {"name": "second", "uri": second.as_uri()}
        server.send("workspace/didChangeWorkspaceFolders", {"event": {"added": [folder], "removed": []}})
        self.assertTrue(server.request("workspace/symbol", {"query": "ContractSecond.Value"}))
        server.send("workspace/didChangeWorkspaceFolders", {"event": {"added": [], "removed": [folder]}})
        self.assertFalse(server.request("workspace/symbol", {"query": "ContractSecond.Value"}))

    def test_preloaded_open_document_relinks_on_add_and_survives_folder_removal(self):
        first = self.project("first", namespace="ContractFirst")
        second = self.project("second", tools=("adoc",), namespace="ContractSecond")
        source = second / "src/ContractSecond.xsmpcat"
        server = self.server(first)
        server.send("textDocument/didOpen", {"textDocument": {"uri": source.as_uri(),
            "languageId": "xsmpcat", "version": 1, "text": source.read_text()}})
        self.assertTrue(server.request("workspace/symbol", {"query": "ContractSecond.Value"}))
        folder = {"name": "second", "uri": second.as_uri()}
        server.send("workspace/didChangeWorkspaceFolders", {"event": {"added": [folder], "removed": []}})
        generated = server.request("xsmp/generateProject", source.as_uri())
        self.assertEqual(generated["generatedProjects"], ["second"], generated)
        server.send("workspace/didChangeWorkspaceFolders", {"event": {"added": [], "removed": [folder]}})
        # Querying the global index does not recreate an evicted document, so
        # this proves the open document was retained during removal.
        self.assertTrue(server.request("workspace/symbol", {"query": "ContractSecond.Value"}))

    def test_initial_mirror_rebuild_notifies_readonly_clients(self):
        project = self.project("mirror")
        xml = project / "src/external.smpcat"
        xml.write_text('''<?xml version="1.0"?><Catalogue:Catalogue
 xmlns:Catalogue="http://www.ecss.nl/smp/2025/Smdl/Catalogue"
 xmlns:Types="http://www.ecss.nl/smp/2025/Core/Types"
 xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" Id="external" Name="external"/>''')
        server = self.server(project)
        mirror = "xsmp-smp:" + xml.as_uri()[7:]
        # A client that announces the content refresh gets it for the mirror document.
        modern = Server([*OPTIONS.runner, OPTIONS.lsp, "--stdio"])
        self.addCleanup(self.stop, modern)
        modern.request("initialize", {"processId": None, "rootUri": None,
            "workspaceFolders": [{"name": project.name, "uri": project.as_uri()}],
            "capabilities": {"workspace": {"textDocumentContent": {"dynamicRegistration": True}}}})
        modern.send("initialized", {})
        modern.request("workspace/symbol", {"query": "Barrier"})
        self.assertTrue(any(notification.get("method") == "workspace/textDocumentContent/refresh"
            and notification.get("params", {}).get("uri") == mirror
            for notification in modern.notifications), modern.notifications)
        text = modern.request("workspace/textDocumentContent", {"uri": mirror})["text"]
        self.assertIn("catalogue external", text)
        self.assertEqual(text, server.request("workspace/textDocumentContent", {"uri": mirror})["text"])


def main():
    global OPTIONS
    parser = argparse.ArgumentParser()
    parser.add_argument("--cli", required=True)
    parser.add_argument("--plugin", required=True)
    parser.add_argument("--project-version", required=True)
    parser.add_argument("--lsp")
    parser.add_argument("--runner", action="append", default=[])
    parser.add_argument("--report")
    OPTIONS, remaining = parser.parse_known_args()
    OPTIONS.cli = str(Path(OPTIONS.cli).resolve())
    OPTIONS.plugin = str(Path(OPTIONS.plugin).resolve())
    if OPTIONS.lsp:
        OPTIONS.lsp = str(Path(OPTIONS.lsp).resolve())
    program = unittest.main(argv=[sys.argv[0], *remaining], exit=False, verbosity=2)
    if OPTIONS.report:
        Path(OPTIONS.report).write_text(json.dumps(TRANSCRIPTS, indent=2))
    raise SystemExit(not program.result.wasSuccessful())


if __name__ == "__main__":
    main()
