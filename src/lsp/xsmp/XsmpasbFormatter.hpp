#pragma once

#include <xsmp/DeploymentFormatter.hpp>

namespace xsmp {

class XsmpasbFormatter final : public DeploymentFormatter {
 public:
  explicit XsmpasbFormatter(const cursiv::CoreServices& services);

 private:
  void formatAssembly(cursiv::FormattingBuilder& builder,
                      const Assembly* node) const;

  void formatAssemblyComponentConfiguration(
      cursiv::FormattingBuilder& builder,
      const AssemblyComponentConfiguration* node) const;

  void formatModelInstance(cursiv::FormattingBuilder& builder,
                           const ModelInstance* node) const;

  void formatAssemblyInstance(cursiv::FormattingBuilder& builder,
                              const AssemblyInstance* node) const;

  void formatSubInstance(cursiv::FormattingBuilder& builder,
                         const SubInstance* node) const;

  void formatGlobalEventHandler(cursiv::FormattingBuilder& builder,
                                const GlobalEventHandler* node) const;

  void formatOperationCall(cursiv::FormattingBuilder& builder,
                           const OperationCall* node) const;

  void formatPropertyValue(cursiv::FormattingBuilder& builder,
                           const PropertyValue* node) const;
};

}  // namespace xsmp
