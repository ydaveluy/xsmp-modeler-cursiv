#include "xsmp/references/XsmpcatScopeComputation.hpp"

#include <utility>
#include <xsmp/utils/XsmpUtils.hpp>
#include <xsmp/workspace/SourceUri.hpp>

#include "cursiv/syntax-tree/AstNode.hpp"
#include "cursiv/workspace/AstNodeDescriptionProvider.hpp"
#include "cursiv/workspace/Document.hpp"

namespace xsmp {

namespace {

// `ast_cast` for a node the caller may change.
template <class T>
T* as(cursiv::AstNode* node) {
  return cursiv::ast_cast<T>(node) != nullptr ? static_cast<T*>(node) : nullptr;
}

// The member list a container publishes into its own name, whichever of the
// three shapes it is.
const std::vector<NamedElement*>* members_of(cursiv::AstNode& node) {
  if (const auto* const structure = cursiv::ast_cast<Structure>(&node))
    return &structure->elements;
  if (const auto* const iface = cursiv::ast_cast<Interface>(&node))
    return &iface->elements;
  if (const auto* const component = cursiv::ast_cast<Component>(&node))
    return &component->elements;
  return nullptr;
}

// What a type publishes under its own name: an enumeration's literals, and the
// constants a container exposes. Native local aliases include all constants;
// workspace exports apply visibility.
template <class F>
void each_named_value(NamedElement& type, F&& yield, bool local = false) {
  if (auto* const enumeration = as<Enumeration>(&type)) {
    for (EnumerationLiteral* const literal : enumeration->literal)
      if (literal != nullptr && !literal->name.empty())
        yield(static_cast<cursiv::AstNode&>(*literal), literal->name);
    return;
  }

  const std::vector<NamedElement*>* const members = members_of(type);
  if (members == nullptr) return;
  for (NamedElement* const member : *members) {
    if (member == nullptr || member->name.empty()) continue;
    // Only constants travel outside their container: a field is read through
    // the instance that holds it, never by name from another document.
    if (cursiv::ast_cast<Constant>(member) == nullptr) continue;
    if (!local && !isEffectivelyPublic(*member)) continue;
    yield(static_cast<cursiv::AstNode&>(*member), member->name);
  }
}

}  // namespace

void XsmpcatScopeComputation::exportType(
    NamedElement& type, std::string_view name,
    std::vector<cursiv::AstNodeDescription>& exported,
    const cursiv::Document& document) const {
  if (auto description = services.astNodeDescriptionProvider->createDescription(
          type, name, &document))
    exported.push_back(*std::move(description));

  each_named_value(type, [&](cursiv::AstNode& member, std::string_view leaf) {
    if (auto description =
            services.astNodeDescriptionProvider->createDescription(
                member, document.keep_text(name, ".", leaf), &document))
      exported.push_back(*std::move(description));
  });
}

void XsmpcatScopeComputation::exportNamespace(
    Namespace& ns, std::string_view name,
    std::vector<cursiv::AstNodeDescription>& exported,
    const cursiv::Document& document,
    const cursiv::CancellationToken& cancelToken) const {
  if (auto description = services.astNodeDescriptionProvider->createDescription(
          ns, name, &document))
    exported.push_back(*std::move(description));

  for (NamedElement* const element : ns.elements) {
    if (element == nullptr || element->name.empty()) continue;
    cursiv::throw_if_cancelled(cancelToken);
    const auto dotted = document.keep_text(name, ".", element->name);
    if (auto* const nested = as<Namespace>(element))
      exportNamespace(*nested, dotted, exported, document, cancelToken);
    else
      exportType(*element, dotted, exported, document);
  }
}

std::vector<cursiv::AstNodeDescription>
XsmpcatScopeComputation::collectExportedSymbols(
    const cursiv::Document& document,
    const cursiv::CancellationToken& cancelToken) const {
  if (hasNativeSidecar(document.uri, services.shared())) return {};
  auto* const catalogue = as<Catalogue>(document.parseResult.value);
  if (catalogue == nullptr) return {};

  std::vector<cursiv::AstNodeDescription> exported;
  if (auto description = services.astNodeDescriptionProvider->createDescription(
          *catalogue, {}, &document))
    exported.push_back(*std::move(description));

  for (Namespace* const ns : catalogue->elements) {
    if (ns == nullptr || ns->name.empty()) continue;
    cursiv::throw_if_cancelled(cancelToken);
    exportNamespace(*ns, ns->name, exported, document, cancelToken);
  }
  return exported;
}

std::vector<cursiv::AstNodeDescription> XsmpcatScopeComputation::qualifyInto(
    Namespace& ns, cursiv::LocalSymbols& symbols,
    const cursiv::Document& document,
    const cursiv::CancellationToken& cancelToken) const {
  std::vector<cursiv::AstNodeDescription> contributed;
  for (NamedElement* const element : ns.elements) {
    if (element == nullptr || element->name.empty()) continue;
    cursiv::throw_if_cancelled(cancelToken);
    if (auto* const nested = as<Namespace>(element)) {
      for (const auto& inner :
           qualifyInto(*nested, symbols, document, cancelToken)) {
        auto dotted = inner;
        dotted.name = document.keep_text(nested->name, ".", inner.name);
        // A sibling of the nested namespace names it dotted from here.
        symbols.add(&ns, dotted);
        contributed.push_back(std::move(dotted));
      }
    } else {
      // ...and a sibling of a type names its members the same way, `Type.Leaf`
      // from here and dotted again by every namespace above.
      each_named_value(
          *element,
          [&](cursiv::AstNode& member, std::string_view leaf) {
            auto description =
                services.astNodeDescriptionProvider->createDescription(
                    member, document.keep_text(element->name, ".", leaf), &document);
            if (!description) return;
            symbols.add(&ns, *description);
            contributed.push_back(*std::move(description));
          },
          true);
    }
    if (auto description =
            services.astNodeDescriptionProvider->createDescription(*element, {},
                                                                   &document))
      contributed.push_back(*std::move(description));
  }
  return contributed;
}

cursiv::LocalSymbols XsmpcatScopeComputation::collectLocalSymbols(
    const cursiv::Document& document,
    const cursiv::CancellationToken& cancelToken) const {
  cursiv::LocalSymbols symbols;
  symbols.reserve(document.parseResult.nodes().size() / 4);

  // Every named node in its own container, which is the default's answer and
  // stays true here: the dotted spellings are ADDED to it, never instead of it.
  for (cursiv::AstNode* const node : document.parseResult.nodes()) {
    cursiv::throw_if_cancelled(cancelToken);
    addLocalSymbol(*node, document, symbols);
  }

  if (auto* const catalogue = as<Catalogue>(document.parseResult.value)) {
    for (Namespace* const ns : catalogue->elements) {
      if (ns == nullptr || ns->name.empty()) continue;
      for (auto inner : qualifyInto(*ns, symbols, document, cancelToken)) {
        inner.name = document.keep_text(ns->name, ".", inner.name);
        symbols.add(catalogue, std::move(inner));
      }
    }
  }
  symbols.build_index();
  return symbols;
}

}  // namespace xsmp
