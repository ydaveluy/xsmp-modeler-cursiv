#pragma once

#include <cursiv/references/DefaultLinker.hpp>

namespace xsmp {

// The framework's linker, with the XSMP wording of a failed reference.
class XsmpLinker final : public cursiv::DefaultLinker {
 public:
  using DefaultLinker::DefaultLinker;

  [[nodiscard]] cursiv::LinkingError createLinkingError(
      const cursiv::ReferenceInfo& reference, cursiv::LinkingFailure why,
      const cursiv::AstNodeDescription* targetDescription) const override;
};

}  // namespace xsmp
