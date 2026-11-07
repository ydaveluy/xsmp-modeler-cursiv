#include "ProjectCreation.hpp"

#include <algorithm>
#include <chrono>
#include <exception>
#include <iomanip>
#include <sstream>
#include <xsmp/utils/FileWriter.hpp>
#include <xsmp/utils/GenerationStamp.hpp>
#include <xsmp/utils/XsmpUtils.hpp>

#include "Json.hpp"

namespace xsmp {

bool isValidProjectName(std::string_view name) {
  const auto letter = [](char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
  };
  return !name.empty() && letter(name.front())
         && std::ranges::all_of(name, [&](char c) {
              return letter(c) || (c >= '0' && c <= '9') || c == '_' || c == '.'
                     || c == '-';
            });
}

std::string projectIdentifier(std::string_view name) {
  std::string identifier(name);
  for (auto& character : identifier)
    if (!std::isalnum(static_cast<unsigned char>(character))
        && character != '_')
      character = '_';
  if (!identifier.empty()
      && std::isdigit(static_cast<unsigned char>(identifier.front())))
    identifier.insert(identifier.begin(), '_');
  // The catalogue and its namespace are named so: a C++ keyword would not do.
  if (isCppKeyword(identifier)) identifier += '_';
  return identifier;
}

void createProject(const Registry& registry, ScaffoldContext context) {
  if (!isValidProjectName(context.projectName))
    throw std::invalid_argument(
        "Project name must follow the format [a-zA-Z][a-zA-Z0-9_.-]*.");
  context.projectDir =
      std::filesystem::absolute(context.projectDir).lexically_normal();
  std::filesystem::create_directories(context.projectDir.parent_path());
  if (!std::filesystem::create_directory(context.projectDir))
    throw std::invalid_argument("Project folder already exists: "
                                + context.projectDir.string());
  try {
    const auto pinned = reproducibleTime();
    const auto now = pinned ? std::chrono::system_clock::from_time_t(*pinned)
                            : std::chrono::system_clock::now();
    const auto date = utcCalendar(std::chrono::system_clock::to_time_t(now));
    std::ostringstream stamp;
    stamp << std::put_time(&date, "%Y-%m-%dT%H:%M:%S") << '.'
          << std::setfill('0') << std::setw(3)
          << (std::chrono::duration_cast<std::chrono::milliseconds>(
                  now.time_since_epoch())
                  .count()
              % 1000)
          << 'Z';

    const auto identifier = projectIdentifier(context.projectName);
    // The `${...}` of the notice stay as they are: the generators expand them.
    writeFileIfChanged(
        context.projectDir / "smdl" / (identifier + ".xsmpcat"),
        "// Copyright (C) ${year} ${user}. All rights reserved.\n//\n// "
        "Generation date:  ${date} ${time}\n\n/**\n * Catalogue "
            + context.projectName + "\n *\n * @creator " + generationUser()
            + "\n * @date " + stamp.str() + "\n */\ncatalogue " + identifier
            + "\n\nnamespace " + identifier + "\n{\n\n} // namespace "
            + identifier + "\n\n");

    const auto result = registry.scaffoldProject(context);
    if (!result.object().get("failures").array().empty()) {
      std::string message = "Project scaffold failed: ";
      for (const auto& failure : result.object().get("failures").array())
        message += stringAt(failure.object(), "contributionId") + ": "
                   + stringAt(failure.object(), "message") + "; ";
      throw std::runtime_error(message);
    }

    // Written last, as it lists the dependencies the scaffolders asked for.
    auto descriptor = "\n/**\n * XSMP Project configuration for " +
                             context.projectName + "\n */\nproject '" +
                             context.projectName +
                             "'\n\n// project relative path(s) containing "
                             "modeling file(s)\nsource 'smdl'\n\n";
    if (!context.selectedProfileId.empty()) {
      const auto& entry = registry.get(context.selectedProfileId);
      descriptor +=
          "\n// use " + entry.label + "\nprofile '" + entry.id + "'\n\n";
    }
    for (const auto& id : context.selectedToolIds) {
      const auto& entry = registry.get(id);
      descriptor += "\n// use " + entry.label + "\ntool '" + entry.id + "'\n\n";
    }
    for (const auto& dependency : result.object().get("dependencies").array())
      descriptor += "\ndependency '" + dependency.string() + "'\n\n";
    descriptor +=
        "\n// If your project needs types from outside sources,\n// you can "
        "include them by adding project dependencies.\n"
        "// For example: dependency 'otherProject'\n//              dependency "
        "'otherProject2'\n\n";
    writeFileIfChanged(context.projectDir / "xsmp.project", descriptor);
  } catch (...) {
    const auto error = std::current_exception();
    std::error_code cleanup;
    std::filesystem::remove_all(context.projectDir, cleanup);
    if (cleanup)
      throw std::runtime_error(
          "Incomplete project folder could not be removed: "
          + cleanup.message());
    std::rethrow_exception(error);
  }
}

}  // namespace xsmp
