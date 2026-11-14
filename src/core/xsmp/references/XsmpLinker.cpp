#include <xsmp/references/XsmpLinker.hpp>

namespace xsmp {

cursiv::LinkingError XsmpLinker::createLinkingError(
    const cursiv::ReferenceInfo& reference, cursiv::LinkingFailure why,
    const cursiv::AstNodeDescription* targetDescription) const {
  // A name found nowhere is reported without the kind its site accepts.
  auto plain = reference;
  plain.narrow = cursiv::kNoNarrow;
  return DefaultLinker::createLinkingError(plain, why, targetDescription);
}

}  // namespace xsmp
