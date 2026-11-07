#include <xsmp/DeploymentSemantics.hpp>
#include <xsmp/references/XsmplnkScopeProvider.hpp>

namespace xsmp {

bool XsmplnkScopeProvider::allowsPathTarget(const Path* path,
                                            const AstNode* target,
                                            const PathSegment* segment) const {
  // The source path of an interface link in a link base anchored on an assembly
  // is not linked: the link validation resolves it against the assembly.
  if (path)
    if (const auto* link = cursiv::ast_cast<InterfaceLink>(path->parent);
        link && link->sourcePath == path)
      if (const auto* base = cursiv::container_of_type<LinkBase>(path);
          base && !base->assembly.refText.empty())
        return false;
  return DeploymentScopeProvider::allowsPathTarget(path, target, segment);
}

}  // namespace xsmp
