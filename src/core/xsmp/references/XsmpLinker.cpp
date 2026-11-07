#include <xsmp/references/XsmpLinker.hpp>

namespace xsmp {

cursiv::LinkingError XsmpLinker::createLinkingError(
    const cursiv::ReferenceInfo& reference, cursiv::RefState why,
    const cursiv::AstNodeDescription* description) const {
  // A name found nowhere is reported without the kind its site accepts.
  auto plain = reference;
  plain.narrow = cursiv::kNoNarrow;
  return DefaultLinker::createLinkingError(plain, why, description);
}

}  // namespace xsmp
