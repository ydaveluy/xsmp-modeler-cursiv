#include <xsmp/XsmpprojectFormatter.hpp>

namespace xsmp {

void XsmpprojectFormatter::applyKeyword(cursiv::FormattingRegion keyword) {
  keyword.prepend(newLine({.allowMore = true})).append(oneSpace);
}

void XsmpprojectFormatter::formatProject(cursiv::FormattingBuilder& builder,
                                         const Project* node) const {
  builder.getNodeFormatter(node).keyword("project").prepend(noIndent).append(
      oneSpace);
}

void XsmpprojectFormatter::formatDependency(cursiv::FormattingBuilder& builder,
                                            const Dependency* node) const {
  applyKeyword(builder.getNodeFormatter(node).keyword("dependency"));
}

void XsmpprojectFormatter::formatProfileReference(
    cursiv::FormattingBuilder& builder, const ProfileReference* node) const {
  applyKeyword(builder.getNodeFormatter(node).keyword("profile"));
}

void XsmpprojectFormatter::formatToolReference(
    cursiv::FormattingBuilder& builder, const ToolReference* node) const {
  applyKeyword(builder.getNodeFormatter(node).keyword("tool"));
}

void XsmpprojectFormatter::formatSource(cursiv::FormattingBuilder& builder,
                                        const Source* node) const {
  applyKeyword(builder.getNodeFormatter(node).keyword("source"));
}

XsmpprojectFormatter::XsmpprojectFormatter(const cursiv::CoreServices& services)
    : AbstractFormatter(services) {
  on<Project>(&XsmpprojectFormatter::formatProject);
  on<Dependency>(&XsmpprojectFormatter::formatDependency);
  on<ProfileReference>(&XsmpprojectFormatter::formatProfileReference);
  on<ToolReference>(&XsmpprojectFormatter::formatToolReference);
  on<Source>(&XsmpprojectFormatter::formatSource);
}

}  // namespace xsmp
