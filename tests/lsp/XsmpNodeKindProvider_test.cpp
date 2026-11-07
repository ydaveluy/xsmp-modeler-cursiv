#include <algorithm>
#include <array>
#include <cstdint>
#include <cursiv/hover/HoverProvider.hpp>
#include <cursiv/navigation/DefinitionProvider.hpp>
#include <cursiv/navigation/ReferencesProvider.hpp>
#include <cursiv/navigation/RenameProvider.hpp>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <variant>
#include <xsmp/Builtins.hpp>
#include <xsmp/Json.hpp>
#include <xsmp/LspModule.hpp>
#include <xsmp/XsmpNodeKindProvider.hpp>
#include <xsmp/ast.hpp>

#include "support/LspTestSupport.hpp"

TEST(XsmpNodeKindProvider, NodeKinds) {
  xsmp::XsmpNodeKindProvider provider;
  xsmp::Task task;
  task.type = &cursiv::ast_type_tag<xsmp::Task>;
  EXPECT_EQ(provider.getSymbolKind(task), ::lsp::SymbolKind::Method);
  xsmp::Assembly assembly;
  assembly.type = &cursiv::ast_type_tag<xsmp::Assembly>;
  EXPECT_EQ(provider.getCompletionItemKind(assembly),
            ::lsp::CompletionItemKind::File);
}
