#pragma once

#include <gtest/gtest.h>

#include <algorithm>
#include <atomic>
#include <cursiv/ExpectDiagnosticOptions.hpp>
#include <cursiv/ExpectFormatting.hpp>
#include <cursiv/ExpectedBase.hpp>
#include <cursiv/ExpectedCompletion.hpp>
#include <cursiv/ExpectedFindReferences.hpp>
#include <cursiv/ExpectedFoldings.hpp>
#include <cursiv/ExpectedGoToDefinition.hpp>
#include <cursiv/ExpectedHighlight.hpp>
#include <cursiv/ExpectedHover.hpp>
#include <cursiv/ExpectedSymbols.hpp>
#include <cursiv/ParseHelper.hpp>
#include <cursiv/ReplacedIndices.hpp>
#include <cursiv/TextPredicate.hpp>
#include <cursiv/ValidationHelper.hpp>
#include <cursiv/completion/CompletionProvider.hpp>
#include <cursiv/formatting/Formatter.hpp>
#include <cursiv/hover/HoverProvider.hpp>
#include <cursiv/ranges/FoldingRangeProvider.hpp>
#include <cursiv/runtime/LanguageServer.hpp>
#include <cursiv/services/ServiceRegistry.hpp>
#include <cursiv/symbols/DocumentSymbolProvider.hpp>
#include <cursiv/text/LspPosition.hpp>
#include <cursiv/workspace/DocumentBuilder.hpp>
#include <cursiv/workspace/DocumentFactory.hpp>
#include <cursiv/workspace/Documents.hpp>
#include <cursiv/workspace/LocalFileSystemProvider.hpp>
#include <functional>
#include <memory>
#include <span>
#include <string>
#include <variant>
#include <vector>
#include <xsmp/LspModule.hpp>
#include <xsmp/XsmpNodeKindProvider.hpp>
#include <xsmp/XsmpprojectLspModule.hpp>
#include <xsmp/ast.hpp>
#include <xsmp/workspace/ProjectManager.hpp>

namespace xsmp {

class TestWorkspace {
  std::unique_ptr<xsmp::XsmpSharedServices> _shared;

 public:
  TestWorkspace() = default;

  template <class Shared>
  static TestWorkspace withShared() {
    static_assert(std::is_same_v<Shared, xsmp::XsmpSharedServices>);
    TestWorkspace world;
    world._shared = std::make_unique<Shared>();
    world._shared->fileSystemProvider =
        std::make_unique<cursiv::LocalFileSystemProvider>();
    cursiv::install_default_shared_lsp_services(*world._shared);
    world._shared->nodeKindProvider = std::make_unique<XsmpNodeKindProvider>();
    return world;
  }

  template <class Shared = xsmp::XsmpSharedServices>
  Shared& shared() {
    return *_shared;
  }

  void registerLanguage(std::unique_ptr<cursiv::CoreServices> language) {
    _shared->serviceRegistry->registerServices(std::move(language));
  }

  const cursiv::LspServices& language(std::string_view id) const {
    return dynamic_cast<const cursiv::LspServices&>(
        _shared->serviceRegistry->getServices({}, id));
  }
};

inline std::shared_ptr<cursiv::Document> parse(TestWorkspace& world,
                                               std::string_view id,
                                               std::string text,
                                               std::string uri = {}) {
  static std::atomic<unsigned> sequence{0};
  if (uri.empty() && id == "xsmpproject")
    uri = "file:///test/" + std::to_string(sequence.fetch_add(1))
          + "/xsmp.project";
  if (!uri.empty()) {
    auto& shared = world.shared();
    if (auto document = shared.documents->getDocument(uri)) {
      document->isOpen = true;
      shared.documentFactory->attachTextDocument(
          *document,
          cursiv::TextDocument::create(uri, std::string(id),
                                       document->textDocument().version() + 1,
                                       std::move(text)));
      const std::string_view changed[] = {uri};
      shared.documentBuilder->update(changed, {});
      return document;
    }
  }
  cursiv::test::ParseHelperOptions options;
  options.documentUri = std::move(uri);
  options.validation = false;
  return cursiv::test::parseHelper(world.language(id), std::move(text),
                                   options);
}

// What a method the protocol does not define answers.
inline ::lsp::LSPAny customRequest(const cursiv::LanguageServer& server,
                                   std::string_view method,
                                   const ::lsp::LSPAny& param) {
  for (const auto& [name, handler] : server.customHandlers().requests)
    if (name == method) return handler(param, {});
  throw std::invalid_argument("No request " + std::string(method));
}

inline std::string applyTextEdits(const cursiv::Document& document,
                                  const std::vector<::lsp::TextEdit>& edits) {
  std::string text(document.textDocument().getText());

  struct Edit {
    cursiv::TextOffset begin;
    cursiv::TextOffset end;
    std::string text;
  };

  std::vector<Edit> sorted;
  for (const auto& edit : edits)
    sorted.push_back({.begin = document.textDocument().offsetAt(
                          cursiv::from_lsp(edit.range.start)),
                      .end = document.textDocument().offsetAt(
                          cursiv::from_lsp(edit.range.end)),
                      .text = edit.newText});
  std::ranges::sort(
      sorted, [](const auto& a, const auto& b) { return a.begin > b.begin; });
  for (const auto& edit : sorted)
    text.replace(edit.begin, edit.end - edit.begin, edit.text);
  return text;
}

inline std::string completionText(const ::lsp::CompletionItem& item) {
  // LSP applies textEdit in preference to insertText when both are present.
  if (item.textEdit)
    return std::visit([](const auto& edit) { return edit.newText; },
                      *item.textEdit);
  return item.insertText.value_or(item.label);
}

inline std::string hoverText(const ::lsp::Hover& hover) {
  if (const auto* markup = std::get_if<::lsp::MarkupContent>(&hover.contents))
    return markup->value;
  return {};
}

struct MarkedDocument {
  std::shared_ptr<cursiv::Document> document;
  ::lsp::Position position;
};

// The options of a test document that has this URI.
inline cursiv::test::ParseHelperOptions documentAt(std::string uri) {
  cursiv::test::ParseHelperOptions options;
  options.documentUri = std::move(uri);
  return options;
}

// A document of `text`, and the position of its caret marker `<|>`, for a
// check that `cursiv::test` does not make. Without a caret, the end.
inline MarkedDocument marked(TestWorkspace& world, std::string_view id,
                             std::string text, std::string uri = {}) {
  auto markers = cursiv::test::replaceIndices({.text = std::move(text)});
  const auto at = markers.indices.empty()
                      ? static_cast<cursiv::TextOffset>(markers.output.size())
                      : markers.indices.front();
  auto document = parse(world, id, std::move(markers.output), std::move(uri));
  const auto position = cursiv::to_lsp(document->textDocument().positionAt(at));
  return {.document = std::move(document), .position = position};
}

// A workspace with a project manager and the catalogue language.
inline TestWorkspace xsmpcatWorkspace() {
  auto world = TestWorkspace::withShared<XsmpSharedServices>();
  auto& shared = world.shared();
  shared.projectManager = std::make_unique<ProjectManager>(shared);
  world.registerLanguage(createXsmpcatServices(shared));
  return world;
}

// A workspace with a project manager and the project language.
inline TestWorkspace xsmpprojectWorkspace() {
  auto world = TestWorkspace::withShared<XsmpSharedServices>();
  auto& shared = world.shared();
  shared.projectManager = std::make_unique<ProjectManager>(shared);
  world.registerLanguage(createXsmpprojectServices(shared));
  return world;
}

// A workspace with a project manager and the four deployment languages.
inline TestWorkspace deploymentWorkspace() {
  auto world = TestWorkspace::withShared<XsmpSharedServices>();
  auto& shared = world.shared();
  shared.projectManager = std::make_unique<ProjectManager>(shared);
  registerXsmpasbServices(shared);
  registerXsmpcfgServices(shared);
  registerXsmplnkServices(shared);
  registerXsmpsedServices(shared);
  return world;
}

template <typename T>
const T* findNamed(std::span<NamedElement* const> elements,
                   std::string_view name) {
  for (auto* element : elements) {
    if (auto* typed = cursiv::ast_cast<const T>(element);
        typed != nullptr && typed->name == name) {
      return typed;
    }
  }
  return nullptr;
}

inline bool hasLabel(const ::lsp::CompletionList& completion,
                     std::string_view label) {
  return std::ranges::any_of(
      completion.items, [&](const auto& item) { return item.label == label; });
}

}  // namespace xsmp
