#pragma once

#include <xsmp/DeploymentFormatter.hpp>

namespace xsmp {

class XsmplnkFormatter final : public DeploymentFormatter {
 public:
  explicit XsmplnkFormatter(const cursiv::CoreServices& services);

 private:
  void formatLinkBase(cursiv::FormattingBuilder& builder,
                      const LinkBase* node) const;

  void formatComponentLinkBase(cursiv::FormattingBuilder& builder,
                               const ComponentLinkBase* node) const;
};

}  // namespace xsmp
