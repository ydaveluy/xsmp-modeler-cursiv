#include <xsmp/XsmpcfgCompletionProvider.hpp>

namespace xsmp {

namespace {

const auto* const kNested = &cursiv::ast_type_tag<ComponentConfiguration>;

}  // namespace

bool XsmpcfgCompletionProvider::atStatement(const Context& context) const {
  return writes(context, kNested, "elements")
         || writes(context, &cursiv::ast_type_tag<Configuration>, "elements");
}

void XsmpcfgCompletionProvider::completeStatement(const Context& context,
                                                  const cursiv::AstNode* block,
                                                  Accept accept) const {
  const bool nested = cursiv::ast_cast<ComponentConfiguration>(block);
  offerSnippet(
      context, accept,
      nested ? "Component Configuration" : "Root Component Configuration",
      std::string(nested ? "${1:child}" : "/${1:root}") + ": "
          + choice(2, names(context, kNested, "context"), "demo.Component")
          + "\n{\n\t$0\n}",
      nested ? "Nested Component Configuration"
             : "Root Component Configuration");
  if (nested)
    offerFieldAssignments(context, accept, componentOf(block),
                          "Configurable field of ");
}

void XsmpcfgCompletionProvider::completeKeyword(const Context& context,
                                                const cursiv::AstNode* block,
                                                std::string_view keyword,
                                                Accept accept) const {
  if (keyword == "configuration")
    offerSnippet(context, accept, "Configuration",
                 "configuration ${1:Name}\n$0", "Configuration Definition");
  if (keyword == "include")
    offerSnippet(
        context, accept, "Include Configuration",
        "include "
            + choice(1,
                     names(context, &cursiv::ast_type_tag<ConfigurationUsage>,
                           "configuration"),
                     "Configuration")
            + (cursiv::ast_cast<ComponentConfiguration>(block)
                   ? " at ${2:path}"
                   : "${2: at path}"),
        "Configuration Include");
}

}  // namespace xsmp
