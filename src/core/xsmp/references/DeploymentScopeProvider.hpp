#pragma once

#include <cursiv/references/DefaultScopeProvider.hpp>
#include <optional>
#include <xsmp/ast.hpp>

namespace xsmp {

// What the deployment languages share: a path resolves segment by segment and a
// local name among the members of its component. Each language adds what only
// it reads.
class DeploymentScopeProvider : public cursiv::DefaultScopeProvider {
 public:
  using DefaultScopeProvider::DefaultScopeProvider;
  const cursiv::AstNodeDescription* getScopeEntry(
      const cursiv::ReferenceInfo& context,
      std::string_view name) const override;

  bool visitScopeEntries(
      const cursiv::ReferenceInfo& context,
      cursiv::function_ref<bool(const cursiv::AstNodeDescription&)> visitor)
      const override;

  bool visitScopeEntries(
      const cursiv::ReferenceInfo& context, std::string_view name,
      cursiv::function_ref<bool(const cursiv::AstNodeDescription&)> visitor)
      const override;

 protected:
  using NodeVisitor = cursiv::function_ref<bool(const cursiv::AstNode*)>;

  // Whether `segment` of `path` may name `target`.
  [[nodiscard]] virtual bool allowsPathTarget(const Path* path,
                                              const cursiv::AstNode* target,
                                              const PathSegment* segment) const;

  // The candidates of a site only this language has, nothing for any other. The
  // answer is whether the walk went to its end.
  [[nodiscard]] virtual std::optional<bool> visitOwnCandidates(
      const cursiv::ReferenceInfo&, NodeVisitor) const {
    return std::nullopt;
  }

 private:
  // Whether the site can hold `description`: its type fits the narrowing of the
  // site and one of the targets of the reference.
  bool accepts(const cursiv::ReferenceInfo& context,
               const cursiv::AstNodeDescription& description) const;

  // Everything the site can name: what only this language has, the members for
  // a path or a local name, otherwise the enclosing scopes and then the
  // documents the project makes visible. Those written as `name`, when it is
  // not empty.
  bool visitCandidates(
      const cursiv::ReferenceInfo& context, std::string_view name,
      cursiv::function_ref<bool(const cursiv::AstNodeDescription&)> visitor)
      const;
};

}  // namespace xsmp
