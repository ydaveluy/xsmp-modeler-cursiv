#pragma once

#include <cursiv/navigation/DefaultDefinitionProvider.hpp>
#include <cursiv/navigation/TypeDefinitionProvider.hpp>
#include <cursiv/references/DefaultReferences.hpp>

namespace xsmp {

// What a caret denotes in the deployment languages, beyond the default's
// resolved references: on a `{name}` placeholder, its template parameter; on a
// pattern path segment, the member it resolves to. The references of a template
// parameter hold its placeholders too. Go to definition, references, highlight
// and rename all read this.
class XsmpDeploymentReferences final : public cursiv::DefaultReferences {
 public:
  using DefaultReferences::DefaultReferences;
  cursiv::Declaration findDeclaration(
      const cursiv::Document&, cursiv::TextOffset,
      const cursiv::CancellationToken& = {}) const override;

  std::vector<cursiv::ReferenceDescription> findReferences(
      const cursiv::AstNode&, const cursiv::FindReferencesOptions& = {},
      const cursiv::CancellationToken& = {}) const override;
};

// Go to definition in the deployment languages. On top of the default: each
// `{name}` placeholder inside a token and pattern path segments lead somewhere,
// and an `unsafe` name, which is not checked, leads nowhere.
class XsmpDefinitionProvider final : public cursiv::DefaultDefinitionProvider {
 public:
  using DefaultDefinitionProvider::DefaultDefinitionProvider;

  std::optional<std::vector<::lsp::LocationLink>> getDefinition(
      const cursiv::Document&, ::lsp::Position,
      const cursiv::CancellationToken& = {}) const override;
};

// Go to type definition in catalogues: where the caret stands on a type, its
// declaration; on a member, the declaration of its type. Anything else gives
// nothing.
class XsmpcatTypeDefinitionProvider final
    : public cursiv::TypeDefinitionProvider {
 public:
  explicit XsmpcatTypeDefinitionProvider(const cursiv::CoreServices& services)
      : services(services) {}

  std::optional<std::vector<::lsp::LocationLink>> getTypeDefinition(
      const cursiv::Document&, ::lsp::Position,
      const cursiv::CancellationToken& = {}) const override;

 private:
  const cursiv::CoreServices& services;
};

}  // namespace xsmp
