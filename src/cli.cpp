#include <algorithm>
#include <iterator>
#include <cctype>
#include <charconv>
#include <cstdlib>
#include <cursiv/utils/UriUtils.hpp>
#include <cursiv/workspace/Document.hpp>
#include <cursiv/workspace/DocumentBuilder.hpp>
#include <cursiv/workspace/DocumentFactory.hpp>
#include <cursiv/workspace/Documents.hpp>
#include <cursiv/workspace/WorkspaceManager.hpp>
#include <filesystem>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <span>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>
#include <xsmp/Extension.hpp>
#include <xsmp/CliWorkspace.hpp>
#include <xsmp/DocumentGenerator.hpp>
#include <xsmp/Json.hpp>
#include <xsmp/ProjectCreation.hpp>
#include <xsmp/tas-mdk/TasMdkContribution.hpp>
#if defined(_WIN32)
#include <io.h>
#else
#include <unistd.h>
#endif
#include <xsmp/smp/SmpService.hpp>
#include <xsmp/utils/TreeUtils.hpp>
#include <xsmp/utils/Utf8.hpp>
#include <xsmp/workspace/SourceUri.hpp>

namespace fs = std::filesystem;

namespace {

struct Options {
  std::string command = "validate", path, workspace, output, profile, name,
              parent = ".";

  std::vector<std::string> tools;
  std::vector<std::string> settings, extensions;
  bool force = false, help = false, interactive = true, yes = false;

  // Given on the command line, which an empty value alone cannot tell.
  bool profileSpecified = false, parentSpecified = false, nameSpecified = false;

  // `implicitHelp`: the command line was incomplete, so the help goes to stderr
  // and the exit code is `kUsage`.
  bool version = false, implicitHelp = false;

  // The command whose help to print, empty for the general help.
  std::string helpContext;
};

// The help of one command, or the general help for an empty `context`.
void usage(std::ostream& stream = std::cout, std::string_view context = {}) {
  if (context.empty())
    stream << R"XSMP_HELP(Usage: xsmp [options] [command]

Create, validate, and generate XSMP projects.

Options:
  -V, --version                output the version number
  -h, --help                   display help for command

Commands:
  validate [options] <path>    validate the target XSMP project and its visible
                               dependency closure
  generate [options] <path>    validate the target XSMP project, then run its
                               active tool/profile generators
  import-smp [options] <path>  import SMP XML back into canonical XSMP source
  new                          create a new XSMP resource
  help [command]               display help for command
)XSMP_HELP";
  else if (context == "validate")
    stream << R"XSMP_HELP(Usage: xsmp validate [options] <path>

validate the target XSMP project and its visible dependency closure

Arguments:
  path                        project directory or xsmp.project file

Options:
  -w, --workspace-root <dir>  workspace root to scan for xsmp.project
                              dependencies
  --extension <dir>           load the contributions of an extension package
                              (repeatable)
  -h, --help                  display help for command
)XSMP_HELP";
  else if (context == "generate")
    stream << R"XSMP_HELP(Usage: xsmp generate [options] <path>

validate the target XSMP project, then run its active tool/profile generators

Arguments:
  path                        project directory or xsmp.project file

Options:
  -w, --workspace-root <dir>  workspace root to scan for xsmp.project
                              dependencies
  -o, --output <dir>          output directory (defaults to the project
                              directory)
  --profile <id>              profile contribution to use in place of the
                              project's
  --tool <id>                 tool contribution to add (repeatable)
  --extension <dir>           load the contributions of an extension package
                              (repeatable)
  -h, --help                  display help for command
)XSMP_HELP";
  else if (context == "import-smp")
    stream << R"XSMP_HELP(Usage: xsmp import-smp [options] <path>

import SMP XML back into canonical XSMP source

Arguments:
  path                 SMP XML file (.smpcat, .smpcfg, .smplnk, .smpasb, or
                       .smpsed)

Options:
  -o, --output <path>  output XSMP source path
  -f, --force          overwrite the output file if it already exists
  -h, --help           display help for command
)XSMP_HELP";
  else if (context == "new")
    stream << R"XSMP_HELP(Usage: xsmp new [options] [command]

create a new XSMP resource

Options:
  -h, --help                            display help for command

Commands:
  project [options] [name] [directory]  create a new XSMP project
  help [command]                        display help for command
)XSMP_HELP";
  else if (context == "new project")
    stream << R"XSMP_HELP(Usage: xsmp new project [options] [name] [directory]

create a new XSMP project

Arguments:
  name               project name
  directory          parent directory (defaults to the current directory)

Options:
  --profile <id>     profile contribution to enable
  --tool <id>        tool contribution to enable (repeatable)
  --set <key=value>  set profile.<id>.<prompt>=value or tool.<id>.<prompt>=value
                     (repeatable)
  --extension <dir>  load the contributions of an extension package
                     (repeatable)
  --no-interactive   do not prompt for missing values
  -y, --yes          create without prompting or confirmation
  -h, --help         display help for command
)XSMP_HELP";
}

// A bad command line. Unlike other usage errors, `main` prints the help after
// it.
class ArgumentError : public cursiv::UsageError {
 public:
  using cursiv::UsageError::UsageError;
};

// Reads the command line: `--name=value`, `--name value`, `-n value`,
// `-nvalue`, bundled short flags, and `--` before plain arguments. Throws an
// `ArgumentError`.
Options arguments(int argc, char** argv) {
  Options options;
  std::vector<std::string> positional, unknown, flags;
  auto endOptions = false;
  const auto takesValue = [](std::string_view name) {
    return name == "-w" || name == "--workspace-root" || name == "-o"
           || name == "--output" || name == "--profile" || name == "--tool"
           || name == "--set" || name == "--extension";
  };
  for (auto at = 1; at < argc; ++at) {
    const std::string_view argument = argv[at];
    if (endOptions || argument == "-" || !argument.starts_with('-')) {
      positional.emplace_back(argument);
      continue;
    }
    if (argument == "--") {
      endOptions = true;
      continue;
    }

    // One argument can hold several short flags. A value can be attached to the
    // last.
    std::vector<std::pair<std::string, std::optional<std::string>>> tokens;
    if (argument.starts_with("--")) {
      const auto equals = argument.find('=');
      const auto name = argument.substr(0, equals);
      if (equals != argument.npos && takesValue(name))
        tokens.emplace_back(name, std::string(argument.substr(equals + 1)));
      else
        tokens.emplace_back(argument, std::nullopt);
    } else {
      for (std::size_t index = 1; index < argument.size(); ++index) {
        auto name = "-" + std::string(1, argument[index]);
        if (takesValue(name) && index + 1 < argument.size()) {
          tokens.emplace_back(name, std::string(argument.substr(index + 1)));
          break;
        }
        tokens.emplace_back(std::move(name), std::nullopt);
      }
    }

    for (const auto& [arg, attached] : tokens) {
      flags.push_back(arg);
      const auto value = [&]() -> std::string {
        if (attached) return *attached;
        if (at + 1 >= argc)
          throw ArgumentError("option '" + arg + "' needs a value");
        return argv[++at];
      };
      if (arg == "-V" || arg == "--version") {
        options.version = true;
        return options;
      }
      if (arg == "-h" || arg == "--help")
        options.help = true;
      else if (arg == "-w" || arg == "--workspace-root")
        options.workspace = value();
      else if (arg == "-o" || arg == "--output")
        options.output = value();
      else if (arg == "--profile") {
        if (options.profileSpecified)
          throw ArgumentError("option '--profile' may only be given once");
        options.profileSpecified = true;
        options.profile = value();
      } else if (arg == "--tool")
        options.tools.push_back(value());
      else if (arg == "-f" || arg == "--force")
        options.force = true;
      else if (arg == "--set")
        options.settings.push_back(value());
      else if (arg == "--extension")
        options.extensions.push_back(value());
      else if (arg == "--no-interactive")
        options.interactive = false;
      else if (arg == "-y" || arg == "--yes")
        options.yes = true;
      else
        unknown.push_back(arg);
    }
  }

  if (!positional.empty() && positional.front() == "help") {
    options.help = true;
    positional.erase(positional.begin());
    if (!positional.empty()) {
      const auto& command = positional.front();
      if (command != "validate" && command != "generate"
          && command != "import-smp" && command != "new") {
        options.implicitHelp = true;
        return options;
      }
      options.helpContext = command;
    }
    return options;
  }

  // A first word that names no command is the path of an implicit `validate`.
  if (!positional.empty()) {
    const auto& first = positional.front();
    if (first == "validate" || first == "generate" || first == "import-smp"
        || first == "new") {
      options.command = first;
      options.helpContext = first;
      positional.erase(positional.begin());
    }
  }

  // With `--help`, an unknown option is forgiven and the help is printed.
  if (!options.help && !unknown.empty())
    throw ArgumentError("unknown option '" + unknown.front() + "'");

  if (options.command == "new") {
    if (!positional.empty() && positional.front() == "help") {
      options.help = true;
      positional.erase(positional.begin());
      if (!positional.empty() && positional.front() == "project")
        options.helpContext = "new project";
      return options;
    }
    if (!positional.empty() && positional.front() == "project") {
      positional.erase(positional.begin());
      options.helpContext = "new project";
    } else if (positional.empty()) {
      options.implicitHelp = !options.help;
      options.help = true;
    } else if (!options.help)
      throw ArgumentError("unknown command '" + positional.front() + "'");
  }

  if (options.help) return options;
  if (!unknown.empty())
    throw ArgumentError("unknown option '" + unknown.front() + "'");

  // An option the parser knows is still refused by a command that does not take
  // it.
  for (const auto& flag : flags) {
    const auto newOnly = flag == "--set" || flag == "--no-interactive"
                         || flag == "-y" || flag == "--yes";
    const auto importOnly = flag == "-f" || flag == "--force";
    const auto generationOnly = flag == "--profile" || flag == "--tool";
    const auto output = flag == "-o" || flag == "--output";
    const auto workspace = flag == "-w" || flag == "--workspace-root";
    if ((newOnly && options.command != "new")
        || (importOnly && options.command != "import-smp")
        || (generationOnly && options.command != "new"
            && options.command != "generate")
        || (output && options.command != "import-smp"
            && options.command != "generate")
        || (workspace && options.command != "validate"
            && options.command != "generate"))
      throw ArgumentError("unknown option '" + flag + "'");
  }

  if (argc == 1) {
    options.help = options.implicitHelp = true;
    return options;
  }

  if (positional.empty() && options.command != "new")
    throw ArgumentError("missing required argument 'path'");
  const std::size_t expected = options.command == "new" ? 2 : 1;
  if (positional.size() > expected) {
    std::string supplied;
    for (const auto& argument : positional) {
      if (!supplied.empty()) supplied += ", ";
      supplied += argument;
    }
    throw ArgumentError(
        "too many arguments for '"
        + std::string(options.command == "new" ? "project" : options.command)
        + "'. Expected " + std::to_string(expected)
        + (expected == 1 ? " argument but got " : " arguments but got ")
        + std::to_string(positional.size()) + ": " + supplied + ".");
  }

  if (options.command == "new") {
    if (!positional.empty()) {
      options.name = positional[0];
      options.nameSpecified = true;
    }
    if (positional.size() > 1) {
      options.parent = positional[1];
      options.parentSpecified = true;
    }
  } else {
    options.path = positional[0];
  }
  return options;
}

// A workspace with the languages, the packages of `--extension` and the TAS-MDK
// profile, built in unless a package provides it. A package that fails to load
// is a usage error.
std::unique_ptr<xsmp::XsmpSharedCoreServices> createServices(
    const Options& options) {
  xsmp::loadExtensionLibraries(options.extensions);
  auto shared = xsmp::createXsmpSharedServices();
  xsmp::registerLanguages(*shared);
  auto& registry = xsmp::registry(*shared);
  for (const auto& extension : options.extensions) {
    const auto report = registry.registerPackage(*shared, extension);
    std::string failures;
    for (const auto& item : report.object().get("failures").array()) {
      const auto& failure = item.object();
      const auto name = xsmp::stringAt(failure, "extensionId");
      failures += "\n  " + (name.empty() ? extension : name) + " ("
                  + xsmp::stringAt(failure, "phase")
                  + "): " + xsmp::stringAt(failure, "message");
    }
    if (!failures.empty())
      throw cursiv::UsageError("Cannot load contribution:" + failures);
  }
  if (!registry.resolve("tas-mdk")) xsmp::registerTasMdk(*shared);
  return shared;
}

// Asks on the terminal. An empty answer gives `fallback`, and the end of the
// input cancels with a `UsageError`.
std::string promptLine(std::string_view question,
                       std::string_view fallback = {}) {
  std::cout << question;
  if (!fallback.empty()) std::cout << " [" << fallback << ']';
  std::cout << ": " << std::flush;
  std::string value;
  if (!std::getline(std::cin, value))
    throw cursiv::UsageError("Project creation cancelled.");
  return value.empty() ? std::string(fallback) : value;
}

std::string trim(std::string value) {
  const auto first = value.find_first_not_of(" \t\r\n"),
             last = value.find_last_not_of(" \t\r\n");
  return first == std::string::npos ? ""
                                    : value.substr(first, last - first + 1);
}

// Turns a number from a listing, counted from 1, into its choice. Any other
// text comes back trimmed.
std::string choiceId(std::string value,
                     const std::vector<std::string>& choices) {
  value = trim(std::move(value));
  std::size_t number = 0;
  const auto end = value.data() + value.size();
  const auto parsed = std::from_chars(value.data(), end, number);
  if (parsed.ec == std::errc{} && parsed.ptr == end && number >= 1
      && number <= choices.size())
    return choices[number - 1];
  return value;
}

std::string lower(std::string text) {
  std::ranges::transform(text, text.begin(), [](unsigned char c) {
    return static_cast<char>(std::tolower(c));
  });
  return text;
}

// Asks until the answer is a number or an id from `choices`.
std::string promptSelect(std::string_view question,
                         const std::vector<std::string>& choices,
                         std::string_view fallback = {}) {
  for (;;) {
    auto selected = choiceId(promptLine(question, fallback), choices);
    if (std::ranges::find(choices, selected) != choices.end()) return selected;
    std::cerr << "Please enter one of the listed numbers or identifiers.\n";
  }
}

// Asks until each comma-separated answer is a number or an id from `choices`.
// `none` selects nothing, and repeats are dropped.
std::vector<std::string> promptTools(const std::vector<std::string>& choices,
                                     std::string_view defaults) {
  for (;;) {
    const auto answer = trim(promptLine("Select tools (comma-separated)",
                                        defaults.empty() ? "none" : defaults));
    if (answer.empty() || lower(answer) == "none") return {};
    auto valid = !answer.ends_with(',');
    std::vector<std::string> result;
    std::istringstream selected(answer);
    for (std::string value; std::getline(selected, value, ',');) {
      value = choiceId(std::move(value), choices);
      if (std::ranges::find(choices, value) == choices.end())
        valid = false;
      else if (std::ranges::find(result, value) == result.end())
        result.push_back(std::move(value));
    }
    if (valid) return result;
    std::cerr << "Please enter comma-separated numbers or identifiers from the "
                 "list.\n";
  }
}

// Asks until the answer is yes or no. An empty answer gives `fallback`, if any.
bool promptConfirm(std::string_view question,
                   std::optional<bool> fallback = {}) {
  for (;;) {
    const auto suffix = !fallback ? " [y/n]" : *fallback ? " [Y/n]" : " [y/N]";
    const auto answer = lower(trim(promptLine(std::string(question) + suffix)));
    if (answer.empty() && fallback) return *fallback;
    if (answer == "yes" || answer == "y") return true;
    if (answer == "no" || answer == "n") return false;
    std::cerr << "Please answer yes or no.\n";
  }
}

// Sorted by label, ignoring case.
xsmp::JsonArray sortedSummaries(xsmp::Json summaries) {
  auto result = std::move(summaries.array());
  std::ranges::stable_sort(result, {}, [](const auto& item) {
    return lower(xsmp::stringAt(item.object(), "label"));
  });
  return result;
}

// `path` as absolute, without `.`, `..` or a trailing separator.
fs::path absolutePath(const fs::path& path) {
  const auto result = fs::absolute(path).lexically_normal();
  return result.has_filename() ? result : result.parent_path();
}

// The folders under `root` that hold an `xsmp.project`, skipping the
// directories the workspace scan itself skips.
std::vector<cursiv::WorkspaceFolder> discoverFolders(
    const cursiv::WorkspaceManager& manager, const fs::path& root,
    const fs::path& target, bool projectInput) {
  std::set<fs::path> directories;
  const auto visit = [&](const auto& self, const fs::path& directory) -> void {
    std::error_code error;
    if (fs::is_regular_file(directory / "xsmp.project", error))
      directories.insert(directory);
    fs::directory_iterator entries(
        directory, fs::directory_options::skip_permission_denied, error);
    for (auto end = fs::directory_iterator{}; !error && entries != end;
         entries.increment(error)) {
      const auto& entry = *entries;
      // Links are not followed, so a link cycle cannot trap the walk.
      if (fs::is_directory(entry.symlink_status(error))
          && manager.shouldIncludeEntry(
              {.isFile = false,
               .isDirectory = true,
               .uri = cursiv::path_to_file_uri(entry.path().string())}))
        self(self, entry.path());
      error.clear();
    }
  };

  visit(visit, root);
  // The target counts even when it holds no project or lies outside `root`.
  if (projectInput || directories.empty()) directories.insert(target);

  std::vector<cursiv::WorkspaceFolder> result;
  for (const auto& directory : directories)
    result.push_back({.uri = cursiv::path_to_file_uri(directory.string()),
                      .name = directory.filename().string()});
  return result;
}

// What the closure of a project needs of its descriptor.
struct Descriptor {
  std::string name;
  std::vector<std::string> dependencies;
};

// The `xsmp.project` of `folder`. Nothing when it cannot be read or holds no
// project.
std::optional<Descriptor> readDescriptor(
    const xsmp::XsmpSharedCoreServices& shared,
    const cursiv::WorkspaceFolder& folder) {
  const auto uri = folder.uri + "/xsmp.project";
  const auto path = cursiv::file_uri_to_path(uri);
  if (!path) return std::nullopt;
  std::string text;
  try {
    text = xsmp::readText(*path);
  } catch (const std::runtime_error&) {
    return std::nullopt;
  }
  const auto document = shared.documentFactory->fromString(text, uri);
  const auto* project =
      cursiv::ast_cast<xsmp::Project>(document->parseResult.value);
  if (!project) return std::nullopt;
  Descriptor descriptor{.name = std::string(project->name), .dependencies = {}};
  for (const auto* element : project->elements)
    if (const auto* dependency = cursiv::ast_cast<xsmp::Dependency>(element))
      descriptor.dependencies.emplace_back(dependency->project.refText);
  return descriptor;
}

// The folders of the projects that `isTarget` selects and of their
// dependencies, found by reading the descriptors alone, so that the other
// projects of a large workspace are not loaded. Without a target, there is
// nothing to load.
template <class IsTarget>
std::vector<cursiv::WorkspaceFolder> closureFolders(
    const xsmp::XsmpSharedCoreServices& shared,
    const std::vector<cursiv::WorkspaceFolder>& folders,
    const IsTarget& isTarget) {
  struct Selectable : Descriptor {
    bool selected = false;
  };

  std::vector<Selectable> descriptors;
  std::vector<std::size_t> pending;
  for (const auto& folder : folders) {
    auto& entry = descriptors.emplace_back();
    auto descriptor = readDescriptor(shared, folder);
    if (!descriptor) continue;
    static_cast<Descriptor&>(entry) = std::move(*descriptor);
    if (isTarget(folder, entry.name)) {
      entry.selected = true;
      pending.push_back(descriptors.size() - 1);
    }
  }

  // A name held by several projects selects them all, for the validation to
  // report it.
  while (!pending.empty()) {
    const auto dependencies = descriptors[pending.back()].dependencies;
    pending.pop_back();
    for (const auto& name : dependencies)
      for (std::size_t index = 0; index < descriptors.size(); ++index)
        if (descriptors[index].name == name && !descriptors[index].selected) {
          descriptors[index].selected = true;
          pending.push_back(index);
        }
  }

  std::vector<cursiv::WorkspaceFolder> result;
  for (std::size_t index = 0; index < descriptors.size(); ++index)
    if (descriptors[index].selected) result.push_back(folders[index]);
  return result;
}

// `xsmp new project`: takes the name, parent, profile, tools and settings from
// the options, and asks on the terminal for what is missing, unless told not
// to. Then creates the project.
int newProject(const Options& options) {
  using namespace xsmp;
#if defined(_WIN32)
  const bool terminal = _isatty(0) != 0 && _isatty(1) != 0;
#else
  const auto terminal = isatty(0) != 0 && isatty(1) != 0;
#endif
  const auto interactive = terminal && options.interactive && !options.yes;

  auto name = options.name;
  if (options.nameSpecified && !isValidProjectName(name))
    throw cursiv::UsageError(
        "Project name must follow the format [a-zA-Z][a-zA-Z0-9_.-]*.");
  if (name.empty() && !interactive)
    throw cursiv::UsageError(
        "Project name is required. Pass it as an argument or run in an "
        "interactive terminal.");
  auto promptedName = false;
  while (name.empty() || !isValidProjectName(name)) {
    if (promptedName)
      std::cerr
          << "Project name must follow the format [a-zA-Z][a-zA-Z0-9_.-]*.\n";
    name = trim(promptLine("Project name"));
    promptedName = true;
  }

  auto parent = options.parent;
  if (interactive && !options.parentSpecified) {
    parent = trim(promptLine("Parent directory", fs::current_path().string()));
    if (parent.empty()) parent = fs::current_path().string();
  }

  auto shared = createServices(options);
  auto& registry = xsmp::registry(*shared);
  // The canonical id of `id`, which must be of `kind`. Warns about a deprecated
  // alias.
  const auto resolve = [&](std::string_view kind, std::string_view id) {
    const auto* entry = registry.resolve(id);
    if (!entry)
      throw cursiv::UsageError("Unknown " + std::string(kind) + ": "
                               + std::string(id));
    if (entry->kind != kind)
      throw cursiv::UsageError("Contribution '" + std::string(id) + "' is a "
                               + entry->kind + ", not a " + std::string(kind)
                               + ".");
    if (id != entry->id
        && std::ranges::find(entry->aliases, id) == entry->aliases.end()
        && std::ranges::find(entry->deprecatedAliases, id)
               != entry->deprecatedAliases.end())
      std::cerr << "warning " << kind << " '" << id << "' is deprecated; use '"
                << entry->id << "'.\n";
    return entry->id;
  };

  auto profile = options.profile;
  auto tools = options.tools;
  if (interactive && !options.profileSpecified) {
    std::string preferred;
    std::vector<std::string> choices;
    std::cout << "Profiles:\n";
    const auto profiles = sortedSummaries(registry.summaries("profile"));
    for (const auto& value : profiles) {
      const auto& item = value.object();
      choices.push_back(stringAt(item, "id"));
      std::cout << "  " << choices.size() << ". " << choices.back() << " - "
                << stringAt(item, "label") << '\n';
      if (preferred.empty() && boolAt(item, "defaultSelected"))
        preferred = stringAt(item, "id");
    }
    choices.emplace_back("__no_profile__");
    std::cout << "  " << choices.size() << ". __no_profile__ - No profile\n";
    profile = promptSelect("Select a profile", choices,
                           preferred.empty() ? "__no_profile__" : preferred);
    if (profile == "__no_profile__") profile.clear();
  }
  if (options.profileSpecified || !profile.empty())
    profile = resolve("profile", profile);

  if (interactive && tools.empty()) {
    std::string defaults;
    std::vector<std::string> choices;
    std::cout << "Tools:\n";
    const auto summaries = sortedSummaries(registry.summaries("tool"));
    for (const auto& value : summaries) {
      const auto& item = value.object();
      choices.push_back(stringAt(item, "id"));
      std::cout << "  " << choices.size() << ". " << choices.back() << " - "
                << stringAt(item, "label") << '\n';
      if (boolAt(item, "defaultSelected")) {
        if (!defaults.empty()) defaults += ',';
        defaults += stringAt(item, "id");
      }
    }
    tools = promptTools(choices, defaults);
  }

  std::vector<std::string> canonicalTools;
  for (const auto& tool : tools) {
    const auto canonical = resolve("tool", tool);
    if (std::ranges::find(canonicalTools, canonical) == canonicalTools.end())
      canonicalTools.push_back(canonical);
  }

  // The `--set` values, by the `key` of the prompt each one answers.
  const auto prompts = registry.wizardPrompts(profile, canonicalTools);
  std::map<std::string, std::string> provided;
  for (const auto& setting : options.settings) {
    const auto equals = setting.find('=');
    if (equals == std::string::npos || equals == 0)
      throw cursiv::UsageError("Invalid setting '" + setting +
                               "'. Expected --set profile.<id>.<prompt>=value "
                               "or --set tool.<id>.<prompt>=value.");
    auto key = setting.substr(0, equals);
    const auto first = key.find_first_not_of(" \t\r\n"),
               last = key.find_last_not_of(" \t\r\n");
    key = first == std::string::npos ? "" : key.substr(first, last - first + 1);

    // An id or alias can hold dots, so the longest matching prefix wins.
    std::string canonicalKey;
    const Contribution* matched = nullptr;
    std::string matchedAlias;
    std::size_t longest = 0;
    for (const auto& [id, entry] : registry.all()) {
      auto names = entry.aliases;
      names.insert(names.end(), entry.deprecatedAliases.begin(),
                   entry.deprecatedAliases.end());
      names.push_back(id);
      for (const auto& alias : names) {
        const auto prefix = entry.kind + "." + alias + ".";
        if (!key.starts_with(prefix) || prefix.size() <= longest) continue;
        canonicalKey = id + "." + key.substr(prefix.size());
        longest = prefix.size();
        matched = &entry;
        matchedAlias = alias;
      }
    }

    if (matched) {
      if ((matched->kind == "profile" && profile != matched->id)
          || (matched->kind == "tool"
              && std::ranges::find(canonicalTools, matched->id)
                     == canonicalTools.end()))
        throw cursiv::UsageError("Contribution '" + matched->id
                                 + "' is not selected.");
      // Only for its deprecation warning.
      (void)resolve(matched->kind, matchedAlias);
    }

    auto known = false;
    for (const auto& prompt : prompts.array())
      if (stringAt(prompt.object(), "key") == canonicalKey) known = true;
    if (!known)
      throw cursiv::UsageError("Unknown project setting '" + key + "'.");
    if (!provided.emplace(canonicalKey, setting.substr(equals + 1)).second)
      throw cursiv::UsageError("Setting '" + key
                               + "' was provided more than once.");
  }

  // A value for every prompt: from `--set`, else the terminal, else its
  // default.
  JsonObject values;
  for (const auto& prompt : prompts.array()) {
    const auto& definition = prompt.object();
    const auto key = stringAt(definition, "key"),
               type = stringAt(definition, "type");
    Json value;
    auto fromDefault = false;
    if (const auto found = provided.find(key); found != provided.end())
      value = found->second;
    else if (type == "choice"
             && (!definition.find("choices")
                 || definition.get("choices").array().empty())) {
      if (!definition.find("defaultValue"))
        throw cursiv::UsageError("Missing required setting '" + key
                                 + "'. Pass --set or use interactive mode.");
      value = definition.get("defaultValue");
      fromDefault = true;
    } else if (interactive) {
      std::string fallback;
      if (const auto* item = definition.find("defaultValue"))
        fallback = item->isString()  ? item->string()
                   : item->boolean() ? "true"
                                     : "false";
      std::vector<std::string> choices;
      if (type == "choice")
        if (const auto* options = definition.find("choices"))
          for (const auto& choice : options->array()) {
            choices.push_back(stringAt(choice.object(), "value"));
            std::cout << "  " << choices.size() << ". "
                      << stringAt(choice.object(), "label", choices.back())
                      << " (" << choices.back() << ")\n";
          }
      const auto question = stringAt(definition, "contributionId") + ": "
                            + stringAt(definition, "label");
      if (type == "boolean") {
        std::optional<bool> defaultValue;
        if (const auto* item = definition.find("defaultValue");
            item && item->isBoolean())
          defaultValue = item->boolean();
        value = promptConfirm(question, defaultValue);
      } else if (type == "choice")
        value = promptSelect(question, choices, fallback);
      else
        value = promptLine(question, fallback);
    } else if (const auto* fallback = definition.find("defaultValue")) {
      value = *fallback;
      fromDefault = true;
    } else
      throw cursiv::UsageError("Missing required setting '" + key
                               + "'. Pass --set or use interactive mode.");

    // Check what the user gave. A default is taken as it is.
    if (type == "boolean" && value.isString() && !fromDefault) {
      auto input = value.string();
      std::ranges::transform(input, input.begin(),
                             [](unsigned char c) { return std::tolower(c); });
      if (input != "true" && input != "false")
        throw cursiv::UsageError("Setting '" + key
                                 + "' must be true or false.");
      value = input == "true";
    }
    if (type == "choice" && !fromDefault) {
      auto found = false;
      if (const auto* choices = definition.find("choices"))
        for (const auto& choice : choices->array())
          if (choice.object().get("value") == value) found = true;
      if (!found)
        throw cursiv::UsageError("Invalid choice for setting '" + key + "'.");
    }
    values.insert(key, std::move(value));
  }

  ScaffoldContext context;
  context.projectName = name;
  context.projectDir = fs::absolute(fs::path(parent) / name).lexically_normal();
  context.selectedProfileId = profile;
  context.selectedToolIds = canonicalTools;
  context.promptValues = std::move(values);

  if (interactive) {
    std::cout << "\nProject summary:\n  Name: " << name
              << "\n  Directory: " << context.projectDir.string()
              << "\n  Profile: "
              << (profile.empty()
                      ? "none"
                      : registry.get(profile).label + " (" + profile + ")")
              << "\n  Tools: ";
    if (canonicalTools.empty()) std::cout << "none";
    for (std::size_t index = 0; index < canonicalTools.size(); ++index) {
      const auto& tool = canonicalTools[index];
      if (index) std::cout << ", ";
      std::cout << registry.get(tool).label << " (" << tool << ')';
    }
    std::cout << "\n\n";
    if (!promptConfirm("Create this project?", true)) {
      std::cout << "Project creation cancelled.\n";
      return 0;
    }
  }

  createProject(registry, std::move(context));
  std::cout << "Created XSMP project \"" << name << "\" in "
            << fs::absolute(fs::path(parent) / name).lexically_normal().string()
            << '\n';
  return cursiv::kClean;
}

// Runs a command. `validate` and `generate` share the first steps: find the
// workspace root and the documents, then print the diagnostics.
int run(const Options& options) {
  if (options.version) {
    std::cout << XSMP_VERSION << '\n';
    return cursiv::kClean;
  }

  if (options.help) {
    usage(options.implicitHelp ? std::cerr : std::cout, options.helpContext);
    return options.implicitHelp ? cursiv::kUsage : cursiv::kClean;
  }

  if (options.command == "new") return newProject(options);
  if (options.command == "import-smp") {
    const auto imported =
        xsmp::importFile(options.path, options.output, options.force, {});
    for (const auto& warning : imported.warnings)
      std::cerr << "warning: " << warning << '\n';
    std::cout << "Imported " << imported.kind << " to "
              << imported.outputPath.string() << '\n';
    return cursiv::kClean;
  }

  auto shared = createServices(options);
  const auto input = absolutePath(options.path);
  // With `--workspace-root`, a path that does not exist can be a project name.
  if (!fs::exists(input) && options.workspace.empty())
    throw cursiv::UsageError("Path does not exist: " + input.string());
  const auto directory = fs::is_directory(input);
  if (directory && !fs::is_regular_file(input / "xsmp.project"))
    throw cursiv::UsageError("Directory '" + options.path
                             + "' does not contain an 'xsmp.project' file.");
  const auto inputExtension = input.extension().string();
  // Without a workspace root, an SMP file does not look for a project above it.
  const auto standaloneXml =
      !directory && options.workspace.empty()
      && (inputExtension == ".smpcat" || inputExtension == ".smpcfg"
          || inputExtension == ".smpasb" || inputExtension == ".smplnk"
          || inputExtension == ".smpsed");

  auto projectDirectory = directory ? input : input.parent_path();
  if (!directory && !standaloneXml && input.filename() != "xsmp.project") {
    for (auto ancestor = projectDirectory; !ancestor.empty();
         ancestor = ancestor.parent_path()) {
      if (fs::is_regular_file(ancestor / "xsmp.project")) {
        projectDirectory = ancestor;
        break;
      }
      if (ancestor == ancestor.parent_path()) break;
    }
  }

  const auto hasProject =
      fs::is_regular_file(projectDirectory / "xsmp.project");
  // By default the parent of the project, so that sibling projects are found.
  const auto root =
      options.workspace.empty()
          ? (hasProject && !standaloneXml ? projectDirectory.parent_path()
                                          : input.parent_path())
          : absolutePath(options.workspace);
  if (!fs::is_directory(root))
    throw cursiv::UsageError("Workspace root '" + root.string()
                             + "' does not exist or is not a directory.");

  // A project that depends on nothing needs no walk of the folders around it,
  // which can be large.
  auto folders = [&]() -> std::vector<cursiv::WorkspaceFolder> {
    if (hasProject && !standaloneXml) {
      const cursiv::WorkspaceFolder own{
          .uri = cursiv::path_to_file_uri(projectDirectory.string()),
          .name = projectDirectory.filename().string()};
      if (const auto descriptor = readDescriptor(*shared, own);
          descriptor && descriptor->dependencies.empty())
        return {own};
    }
    return discoverFolders(*shared->workspaceManager, root,
                           hasProject ? projectDirectory : root, hasProject);
  }();
  // Only the closure of a project is reported, so only that is loaded. A path
  // that names no project, such as a file out of any project, needs them all.
  if (hasProject && !standaloneXml) {
    const auto target = cursiv::path_to_file_uri(projectDirectory.string());
    folders = closureFolders(*shared, folders, [&](const auto& folder, auto&&) {
      return folder.uri == target;
    });
  } else if (!options.workspace.empty() && !fs::exists(input))
    folders = closureFolders(*shared, folders, [&](auto&&, const auto& name) {
      return name == options.path;
    });
  shared->workspaceManager->initializeWorkspace(folders);

  // The documents the path selects: a folder, a file, or a project name.
  std::vector<cursiv::Document*> documents;
  if (directory) {
    const auto prefix = cursiv::path_to_file_uri(input.string()) + "/";
    for (const auto& document : shared->documents->all())
      if (xsmp::sourceDocumentUri(document->uri).starts_with(prefix))
        documents.push_back(document.get());
  } else {
    const xsmp::Project* project = nullptr;
    if (fs::exists(input)) {
      cursiv::Document* document = nullptr;
      const auto extension = input.extension().string();
      // An SMP file stands for the native file beside it or, without one, for
      // its mirror document.
      if (extension == ".smpcat" || extension == ".smpcfg"
          || extension == ".smpasb" || extension == ".smplnk"
          || extension == ".smpsed") {
        auto sidecar = input;
        sidecar.replace_extension(".x" + extension.substr(1));
        if (fs::is_regular_file(sidecar))
          document =
              &cursiv::build_document_from_path(sidecar.string(), *shared);
        else {
          const auto mirror =
              xsmp::smpMirrorUri(cursiv::path_to_file_uri(input.string()));
          auto native = shared->documents->getOrCreateDocument(mirror);
          document = native.get();
          shared->documentBuilder->build(std::span(&document, 1), {});
        }
      } else
        document = &cursiv::build_document_from_path(input.string(), *shared);
      project =
          cursiv::ast_cast<const xsmp::Project>(document->parseResult.value);
      if (!project) documents.push_back(document);
    } else
      // The path that does not exist names a project.
      project = shared->projectManager->getProjectByName(options.path);
    if (project) {
      const auto* descriptor = project->getDocument();
      for (const auto& document : shared->documents->all())
        if (document.get() == descriptor
            || shared->projectManager->getProject(*document) == project)
          documents.push_back(document.get());
    }
  }

  // The folder of a project selects exactly the documents of that project.
  const xsmp::Project* directoryProject = nullptr;
  if (directory) {
    const auto descriptorUri =
        cursiv::path_to_file_uri((input / "xsmp.project").string());
    if (const auto descriptor = shared->documents->getDocument(descriptorUri))
      directoryProject =
          cursiv::ast_cast<xsmp::Project>(descriptor->parseResult.value);
  }
  if (directoryProject)
    documents = xsmp::projectDocuments(*shared, *directoryProject, false);

  // Only UTF-8 is read, as by the other XSMP tools: a byte of another encoding
  // would be lost on the way to the outputs.
  for (const auto& document : shared->documents->all())
    if (document->uri.starts_with("file:"))
      if (const auto text = document->textDocument().getText();
          xsmp::utf8Length(text) != text.size())
        throw cursiv::UsageError(
            "File '"
            + cursiv::file_uri_to_path(document->uri).value_or(document->uri)
            + "' is not valid UTF-8. Save it as UTF-8 before running the CLI.");

  std::set<const xsmp::Project*> projects;
  for (const auto* document : documents) {
    const auto* owner =
        cursiv::ast_cast<xsmp::Project>(document->parseResult.value);
    if (!owner) owner = shared->projectManager->getProject(*document);
    if (owner) projects.insert(owner);
  }

  // A file inside a project selects that project's whole generation, as in the
  // language server.
  if (!directory && projects.size() == 1)
    documents = xsmp::projectDocuments(*shared, **projects.begin(), false);

  // Generation covers the selection. Validation also covers the dependencies.
  auto generationDocuments = documents;
  std::ranges::sort(generationDocuments, {}, &cursiv::Document::uri);
  for (const auto* project : projects) {
    const auto visible = xsmp::projectDocuments(*shared, *project, true);
    documents.insert(documents.end(), visible.begin(), visible.end());
  }
  std::ranges::sort(documents, {}, &cursiv::Document::uri);
  documents.erase(std::ranges::unique(documents).begin(), documents.end());

  // Print each diagnostic once, in a stable order.
  using Diagnostic =
      std::tuple<std::string, unsigned, unsigned, int, std::string>;
  std::set<Diagnostic> diagnostics;
  const auto addDiagnostic = [&](std::string uri, unsigned line,
                                 unsigned column, int severity,
                                 std::string message) {
    const auto source = xsmp::sourceDocumentUri(uri);
    const auto path = cursiv::file_uri_to_path(source);
    diagnostics.emplace(
        path ? fs::path(*path).lexically_relative(root).generic_string()
             : source,
        line, column, severity, std::move(message));
  };

  // These have no position in a document, so they go on the first line. A
  // selection without a project reports them all.
  const auto addSourceDiagnostics = [&](const auto& found, int severity) {
    for (const auto& [uri, message] : found)
      addDiagnostic(uri, 1, 1, severity, message);
  };
  for (const auto* project : projects) {
    addSourceDiagnostics(xsmp::projectSourceErrors(*shared, *project), 1);
    addSourceDiagnostics(xsmp::projectSourceWarnings(*shared, *project), 2);
  }
  if (projects.empty()) {
    addSourceDiagnostics(xsmp::sourceErrors(*shared), 1);
    addSourceDiagnostics(xsmp::sourceWarnings(*shared), 2);
  }

  for (const auto* document : documents) {
    // A dependency missing from the workspace gets this clearer message, in
    // place of the link error on the same range.
    std::vector<std::pair<cursiv::TextRange, std::string>> missing;
    if (const auto* project =
            cursiv::ast_cast<xsmp::Project>(document->parseResult.value))
      for (const auto* element : project->elements)
        if (const auto* dependency =
                cursiv::ast_cast<xsmp::Dependency>(element);
            dependency && !dependency->project.get()
            && !shared->projectManager->getProjectByName(
                dependency->project.refText))
          missing.emplace_back(xsmp::rangeForFeature(*dependency, "project"),
                               "Missing dependency project \""
                                   + std::string(dependency->project.refText)
                                   + "\" in the scanned workspace.");

    for (const auto& diagnostic : document->diagnostics) {
      const auto position =
          document->textDocument().positionAt(diagnostic.range.begin);
      auto replaced = false;
      for (const auto& [range, message] : missing)
        if (diagnostic.severity == cursiv::DiagnosticSeverity::Error
            && diagnostic.range.begin >= range.begin
            && diagnostic.range.begin <= range.end
            && diagnostic.message.find("resolve") != std::string::npos) {
          replaced = true;
          break;
        }
      if (!replaced)
        addDiagnostic(document->uri, position.line + 1, position.character + 1,
                      static_cast<int>(diagnostic.severity),
                      diagnostic.message);
    }

    for (const auto& [range, message] : missing) {
      const auto position = document->textDocument().positionAt(range.begin);
      addDiagnostic(document->uri, position.line + 1, position.character + 1, 1,
                    message);
    }
  }

  std::size_t errors = 0, warnings = 0;
  for (const auto& [path, line, column, severity, message] : diagnostics) {
    errors += severity == 1;
    warnings += severity == 2;
    std::cerr << path << ':' << line << ':' << column << ' '
              << (severity == 1   ? "error"
                  : severity == 2 ? "warning"
                  : severity == 3 ? "information"
                                  : "hint")
              << ' ' << message << '\n';
  }
  std::cout << errors << " errors, " << warnings << " warnings\n";

  if (errors) return cursiv::kDiagnostics;
  if (documents.empty()) throw cursiv::UsageError("No XSMP document found");
  if (options.command == "validate") {
    return cursiv::kClean;
  }

  // `generate`: group the documents by output folder and by selected
  // contributions.
  const auto destination = options.output.empty()
                               ? (directory ? input : input.parent_path())
                               : fs::absolute(options.output);
  auto& registry = xsmp::registry(*shared);
  using Group = std::pair<fs::path, std::vector<std::string>>;
  std::map<Group, std::vector<const cursiv::AstNode*>> batches;
  // The projects with something selected, null for documents without a project.
  std::set<const xsmp::Project*> selecting;
  for (const auto* document : generationDocuments) {
    const auto* node = document->parseResult.value;
    if (!node || !cursiv::ast_cast<const xsmp::Document>(node)) continue;
    std::vector<std::string> identifiers;
    if (options.profileSpecified) identifiers.push_back(options.profile);
    auto output = destination;
    const auto* project = shared->projectManager->getProject(*document);
    if (project) {
      for (const auto* element : project->elements) {
        if (const auto* p =
                cursiv::ast_cast<const xsmp::ProfileReference>(element);
            p && !options.profileSpecified)
          identifiers.emplace_back(p->profile.refText);
        if (const auto* t =
                cursiv::ast_cast<const xsmp::ToolReference>(element))
          identifiers.emplace_back(t->tool.refText);
      }
      if (options.output.empty())
        if (const auto projectPath =
                cursiv::file_uri_to_path(project->getDocument()->uri))
          output = fs::path(*projectPath).parent_path();
    }
    identifiers.insert(identifiers.end(), options.tools.begin(),
                       options.tools.end());
    // Resolve every selection before anything is generated. A bad contribution
    // in a later project must leave earlier outputs intact.
    for (const auto& id : identifiers) (void)registry.get(id);
    if (!identifiers.empty()) selecting.insert(project);
    batches[{output, identifiers}].push_back(node);
  }

  std::vector<xsmp::GenerationGroup> groups;
  for (const auto& [group, nodes] : batches)
    groups.push_back({group.first, group.second, nodes});
  registry.generate(groups);

  for (const auto* project : projects)
    if (selecting.contains(project))
      std::cout << "Generated outputs for project \"" << project->name
                << "\".\n";
    else
      std::cout << "Nothing to generate for project \"" << project->name
                << "\": no profile or tool selected.\n";
  if (projects.empty()) {
    if (selecting.empty())
      std::cout << "Nothing to generate: no profile or tool selected.\n";
    else
      std::cout << "Generated into " << destination.string() << '\n';
  }
  return cursiv::kClean;
}

}  // namespace

int main(int argc, char** argv) {
#if defined(__wasi__)
  // WASI starts with a virtual cwd of '/'; the runner supplies the host cwd.
  if (const auto* directory = std::getenv("PWD")) {
    std::error_code error;
    fs::current_path(directory, error);
    if (error) {
      std::cerr << "Cannot set working directory: " << error.message() << '\n';
      return cursiv::kFatal;
    }
  }
#endif
  try {
    return run(arguments(argc, argv));
  } catch (const ArgumentError& error) {
    std::cerr << "error: " << error.what() << "\n\n";
    // The options were not read, so the command comes from `argv` again.
    std::string context;
    if (argc > 1) {
      const std::string_view command = argv[1];
      if (command == "validate" || command == "generate"
          || command == "import-smp" || command == "new")
        context = command;
      if (command == "new" && argc > 2
          && std::string_view(argv[2]) == "project")
        context = "new project";
    }
    usage(std::cerr, context);
    return cursiv::kUsage;
  } catch (const cursiv::UsageError& error) {
    std::cerr << error.what() << '\n';
    return cursiv::kUsage;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return cursiv::kUsage;
  }
}
