#pragma once

#include <cursiv/formatting/AbstractFormatter.hpp>
#include <xsmp/ast.hpp>

namespace xsmp {

/// Formatter for `xsmp.project`. Only the Project and its four element kinds
/// are formatted; a standalone Profile or Tool declaration has no rule.
class XsmpprojectFormatter : public cursiv::AbstractFormatter {
 public:
  explicit XsmpprojectFormatter(const cursiv::CoreServices& services);

 protected:
  virtual void formatProject(cursiv::FormattingBuilder& builder,
                             const Project* node) const;

  virtual void formatDependency(cursiv::FormattingBuilder& builder,
                                const Dependency* node) const;

  virtual void formatProfileReference(cursiv::FormattingBuilder& builder,
                                      const ProfileReference* node) const;

  virtual void formatToolReference(cursiv::FormattingBuilder& builder,
                                   const ToolReference* node) const;

  virtual void formatSource(cursiv::FormattingBuilder& builder,
                            const Source* node) const;

 private:
  // `<keyword>` on its own line (blank lines allowed) + one trailing space.
  static void applyKeyword(cursiv::FormattingRegion keyword);
};

}  // namespace xsmp
