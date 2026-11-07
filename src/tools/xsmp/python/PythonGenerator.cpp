#include "PythonGenerator.hpp"

#include <algorithm>
#include <array>
#include <cursiv/text/TextBuilder.hpp>
#include <vector>
#include <xsmp/CopyrightNoticeProvider.hpp>
#include <xsmp/utils/FileWriter.hpp>

namespace xsmp {

namespace {

// `name` as a Python name: a keyword of Python takes a `_` before it, as it
// would not import. An XSMP name starts with a letter, so that cannot collide.
std::string pythonName(std::string_view name) {
  static constexpr std::array<std::string_view, 35> keywords{
      "False",  "None",   "True",    "and",      "as",       "assert", "async",
      "await",  "break",  "class",   "continue", "def",      "del",    "elif",
      "else",   "except", "finally", "for",      "from",     "global", "if",
      "import", "in",     "is",      "lambda",   "nonlocal", "not",    "or",
      "pass",   "raise",  "return",  "try",      "while",    "with",   "yield"};
  return (std::ranges::binary_search(keywords, name) ? "_" : "")
         + std::string(name);
}

// The folders from the catalogue down to `element`.
std::filesystem::path qualifiedPath(const NamedElement& element) {
  std::filesystem::path path(pythonName(element.name));
  for (auto* parent = element.parent; parent; parent = parent->parent) {
    const auto* named = cursiv::ast_cast<const NamedElement>(parent);
    if (!named) break;
    path = std::filesystem::path(pythonName(named->name)) / path;
  }
  return path;
}

// What each generated file starts with.
void writeHeader(cursiv::TextBuilder& out, const std::filesystem::path& file,
                 const std::optional<std::string>& notice) {
  constexpr std::string_view separator =
      "# ---------------------------------------------------------------------"
      "--------";
  if (notice) out.write(*notice).line();
  out.line();
  out.line(separator);
  out.line("# File Name    : ", file.generic_string());
  out.line(separator);
  out.line(
      "# This file is auto-generated. Do not edit otherwise your changes will "
      "be lost.");
  out.line();
}

}  // namespace

void PythonGenerator::generate(const Catalogue& catalogue,
                               const std::filesystem::path& destination) const {
  const auto root = destination / "python";
  const auto file = qualifiedPath(catalogue) / "__init__.py";
  const auto notice =
      getCopyrightNotice(*services.commentProvider, catalogue, "# ");
  cursiv::TextBuilder out;
  writeHeader(out, file, notice);
  for (const auto* ns : catalogue.elements)
    out.line("from . import ", pythonName(ns->name));
  if (catalogue.elements.empty()) out.line();
  writeFileIfChanged(root / file, out.text());
  for (const auto* ns : catalogue.elements)
    generateNamespace(*ns, root, notice);
}

void PythonGenerator::generateNamespace(
    const Namespace& ns, const std::filesystem::path& root,
    const std::optional<std::string>& notice) const {
  const auto file = qualifiedPath(ns) / "__init__.py";
  cursiv::TextBuilder out;
  writeHeader(out, file, notice);
  for (const auto* element : ns.elements)
    if (const auto* child = cursiv::ast_cast<const Namespace>(element)) {
      out.line("from . import ", pythonName(child->name));
      generateNamespace(*child, root, notice);
    }
  auto imported = false;
  for (const auto* element : ns.elements)
    if (const auto* component = cursiv::ast_cast<const Component>(element)) {
      if (!imported) out.line("import ecss_smp").line();
      imported = true;
      out.line("class ", pythonName(component->name), ":");
      out.line("    uuid: ecss_smp.Smp.Uuid = ecss_smp.Smp.Uuid(\"",
               docTag(*component, "uuid").value_or(""), "\")");
    }
  if (imported) out.line();
  writeFileIfChanged(root / file, out.text());
}

}  // namespace xsmp
