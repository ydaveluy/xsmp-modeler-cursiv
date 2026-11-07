#pragma once

#include <xsmp/DeploymentFormatter.hpp>

namespace xsmp {

class XsmpcfgFormatter final : public DeploymentFormatter {
 public:
  explicit XsmpcfgFormatter(const cursiv::CoreServices& services);

 private:
  void formatConfiguration(cursiv::FormattingBuilder& builder,
                           const Configuration* node) const;

  void formatComponentConfiguration(cursiv::FormattingBuilder& builder,
                                    const ComponentConfiguration* node) const;

  void formatConfigurationUsage(cursiv::FormattingBuilder& builder,
                                const ConfigurationUsage* node) const;

  void formatCfgStructureFieldValue(cursiv::FormattingBuilder& builder,
                                    const CfgStructureFieldValue* node) const;
};

}  // namespace xsmp
