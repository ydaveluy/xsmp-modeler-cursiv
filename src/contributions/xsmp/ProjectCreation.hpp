#pragma once

#include <xsmp/Contributions.hpp>

namespace xsmp {

// A letter, then only letters, digits, `_`, `.` and `-`.
bool isValidProjectName(std::string_view name);

// `name` as an identifier: any other character becomes `_`, a leading digit
// gets a `_` before it, and a C++ keyword one after it.
std::string projectIdentifier(std::string_view name);

// Reserves the project directory, creating its parents, and removes it again if
// scaffolding fails.
void createProject(const Registry&, ScaffoldContext context);

}  // namespace xsmp
