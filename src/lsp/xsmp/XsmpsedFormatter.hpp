#pragma once

#include <xsmp/DeploymentFormatter.hpp>

namespace xsmp {

class XsmpsedFormatter final : public DeploymentFormatter {
 public:
  explicit XsmpsedFormatter(const cursiv::CoreServices& services);

 private:
  template <class T>
  void formatEvent(cursiv::FormattingBuilder& builder, const T* node) const;
  void formatSchedule(cursiv::FormattingBuilder& builder,
                      const Schedule* node) const;

  void formatTask(cursiv::FormattingBuilder& builder, const Task* node) const;
  void formatTransfer(cursiv::FormattingBuilder& builder,
                      const Transfer* node) const;

  void formatTrigger(cursiv::FormattingBuilder& builder,
                     const Trigger* node) const;

  void formatExecuteTask(cursiv::FormattingBuilder& builder,
                         const ExecuteTask* node) const;

  void formatEmitGlobalEvent(cursiv::FormattingBuilder& builder,
                             const EmitGlobalEvent* node) const;

  void formatCallOperation(cursiv::FormattingBuilder& builder,
                           const CallOperation* node) const;

  void formatSetProperty(cursiv::FormattingBuilder& builder,
                         const SetProperty* node) const;
};

}  // namespace xsmp
