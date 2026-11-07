#include <algorithm>
#include <cursiv/hierarchy/HierarchyProviderSupport.hpp>
#include <cursiv/references/NameProvider.hpp>
#include <cursiv/references/References.hpp>
#include <cursiv/services/SharedCoreServices.hpp>
#include <cursiv/syntax-tree/AstNode.hpp>
#include <cursiv/syntax-tree/CstUtils.hpp>
#include <cursiv/text/LspPosition.hpp>
#include <cursiv/workspace/Documents.hpp>
#include <optional>
#include <utility>
#include <vector>
#include <xsmp/XsmpDocumentSymbolProvider.hpp>
#include <xsmp/XsmpNodeKindProvider.hpp>
#include <xsmp/XsmpcatTypeHierarchyProvider.hpp>
#include <xsmp/ast.hpp>
#include <xsmp/utils/XsmpDocumentation.hpp>

namespace xsmp {

namespace {

// The declared direct supertypes of `type`: a Class/Exception `extends` base,
// an Interface `extends` list, a Model/Service `extends` base plus its
// `implements` interfaces, an Integer/Float `extends` primitive type, the item
// type of an Array or what a ValueReference refers to. Unresolved references
// are skipped.
std::vector<const Type*> directSupertypes(const Type& type) {
  std::vector<const Type*> result;
  const auto add = [&](const Type* candidate) {
    if (candidate != nullptr) {
      result.push_back(candidate);
    }
  };
  if (const auto* clazz = cursiv::ast_cast<const Class>(&type)) {
    if (clazz->base.has_value()) {
      add(clazz->base->get());
    }
  } else if (const auto* interface = cursiv::ast_cast<const Interface>(&type)) {
    for (const auto& base : interface->base) {
      add(base.get());
    }
  } else if (const auto* component = cursiv::ast_cast<const Component>(&type)) {
    if (component->base.has_value()) {
      add(component->base->get());
    }
    for (const auto& interface : component->interface_) {
      add(interface.get());
    }
  } else if (const auto* integer = cursiv::ast_cast<Integer>(&type)) {
    if (integer->primitiveType) add(integer->primitiveType->get());
  } else if (const auto* floating = cursiv::ast_cast<Float>(&type)) {
    if (floating->primitiveType) add(floating->primitiveType->get());
  } else if (const auto* array = cursiv::ast_cast<Array>(&type)) {
    add(array->itemType.get());
  } else if (const auto* reference = cursiv::ast_cast<ValueReference>(&type)) {
    add(reference->type.get());
  }
  return result;
}

// The view's item for a type, selecting its name. Nothing for a type with no
// name in a parsed document.
std::optional<::lsp::TypeHierarchyItem> makeItem(
    const cursiv::CoreServices& core, const Type& type) {
  const auto* document = type.getDocument();
  if (!document || type.cstNodeId == cursiv::kNoNode) return std::nullopt;
  const auto name = core.nameProvider->getNameNode(type, document);
  if (!name) return std::nullopt;
  const auto& record = document->parseResult.cst()[type.cstNodeId];

  ::lsp::TypeHierarchyItem item;
  item.name = std::string(type.name);
  item.kind = XsmpNodeKindProvider{}.getSymbolKind(type);
  item.detail = getXsmpNodeDetails(type);
  if (docTag(type, "deprecated")) {
    item.tags.emplace();
    item.tags->emplace_back(::lsp::SymbolTag::Deprecated);
  }

  item.uri = ::lsp::DocumentUri::parse(document->uri);
  item.range = cursiv::to_lsp(
      document->textDocument().rangeAt(record.begin, record.end));
  item.selectionRange = cursiv::to_lsp(
      document->textDocument().rangeAt(name->begin(), name->end()));
  return item;
}

}  // namespace

std::vector<::lsp::TypeHierarchyItem>
XsmpcatTypeHierarchyProvider::prepareTypeHierarchy(
    const cursiv::Document& document, ::lsp::Position position,
    const cursiv::CancellationToken& token) const {
  std::vector<::lsp::TypeHierarchyItem> result;
  for (const auto* node :
       cursiv::hierarchy_nodes_at(*_core.references, document, position, token))
    if (const auto* type = cursiv::ast_cast<Type>(node))
      if (auto item = makeItem(_core, *type))
        result.push_back(std::move(*item));
  return result;
}

std::vector<::lsp::TypeHierarchyItem> XsmpcatTypeHierarchyProvider::supertypes(
    const ::lsp::TypeHierarchyItem& item,
    const cursiv::CancellationToken& token) const {
  const auto* node = cursiv::hierarchy_node_of_item(
      *_core.shared().documents, *_core.references, item.uri,
      item.selectionRange.start, token);
  const auto* type = cursiv::ast_cast<Type>(node);
  std::vector<::lsp::TypeHierarchyItem> result;
  if (!type) return result;
  for (const auto* super : directSupertypes(*type)) {
    cursiv::throw_if_cancelled(token);
    if (auto row = makeItem(_core, *super)) result.push_back(std::move(*row));
  }
  return result;
}

std::vector<::lsp::TypeHierarchyItem> XsmpcatTypeHierarchyProvider::subtypes(
    const ::lsp::TypeHierarchyItem& item,
    const cursiv::CancellationToken& token) const {
  const auto* node = cursiv::hierarchy_node_of_item(
      *_core.shared().documents, *_core.references, item.uri,
      item.selectionRange.start, token);
  const auto* type = cursiv::ast_cast<Type>(node);
  std::vector<::lsp::TypeHierarchyItem> result;
  if (!cursiv::ast_cast<LanguageType>(type)) return result;
  // Any LanguageType that names this type is a subtype, arrays and value
  // references included, in reference-index order.
  for (const auto& reference :
       _core.references->findReferences(*type, {}, token)) {
    cursiv::throw_if_cancelled(token);
    const auto document =
        _core.shared().documents->getDocument(reference.sourceDocumentId);
    if (!document) continue;
    const auto* candidate = cursiv::ast_cast<LanguageType>(
        cursiv::owner_at(document->parseResult.cst(), reference.sourceBegin));
    if (candidate)
      if (auto row = makeItem(_core, *candidate))
        result.push_back(std::move(*row));
  }
  return result;
}

}  // namespace xsmp
