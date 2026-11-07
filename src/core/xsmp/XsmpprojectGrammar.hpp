#pragma once

#include <xsmp/CommonGrammar.hpp>
#include <xsmp/LiteralConversions.hpp>
#include <xsmp/ast.hpp>

namespace xsmp {

// A project string is the text between its quotes: a backslash is an ordinary
// character, as in a Windows path.
inline constexpr auto project_quoted = [](std::string_view text) {
  return text.substr(1, text.size() - 2);
};

// The language of `xsmp.project`, `.xsmpprofile` and `.xsmptool` files.
struct Xsmpproject : CommonRules {
  struct STRING
      : terminal<as<project_quoted>((lit<"\""> + many(cr<R"(^")">) + lit<"\"">)
                                    | (lit<"'"> + many(cr<"^'">) + lit<"'">))> {
  };

  struct ProfileReference : rule<kw<"profile">.doc<R"(`profile '<profile-name>'`

Activates an XSMP profile contribution for the current project.
Profiles usually group reusable conventions, generators or validation rules that should apply to every source of the project.
Use content assist to pick one of the installed profile identifiers.

**Example**
```xsmp
profile 'xsmp-sdk'
```)">() + assign<&xsmp::ProfileReference::profile>(ref<STRING>)> {};

  struct ToolReference : rule<kw<"tool">.doc<R"(`tool '<tool-name>'`

Enables an XSMP tool contribution for the current project.
Tools typically provide code generation, export pipelines or extra validation passes.
Use content assist to pick one of the installed tool identifiers.

**Example**
```xsmp
tool 'smp'
```)">() + assign<&xsmp::ToolReference::tool>(ref<STRING>)> {};

  struct Dependency : rule<kw<"dependency">.doc<R"(`dependency '<project-name>'`

Declares that the current project depends on another XSMP project.
Dependencies make the dependent project's visible modeling documents available for linking and validation,
such as catalogues, configurations, assemblies, link bases and schedules.
Profile/tool declarations and contribution payload documents are not inherited through dependencies.

**Example**
```xsmp
dependency '01-foundation'
```)">() + assign<&xsmp::Dependency::project>(ref<STRING>)> {};

  struct Source : rule<kw<"source">.doc<R"(`source '<path>'`

Adds a modeling source root to the project.
The path is relative to the project directory and can target a folder or a single file.

**Example**
```xsmp
source 'smdl'
```)">() + assign<&xsmp::Source::path>(ref<STRING>)> {};

  struct Project
      : rule<
            create<xsmp::Project>
            + kw<"project">.doc<R"(`project '<project-name>' [using '<standard>']`

Declares an XSMP project root.
Use a project to select the SMP standard revision, list modeling sources, activate installed profiles and tools, and declare dependencies on other XSMP projects.

**Example**
```xsmp
project 'MissionDemo' using 'ECSS_SMP_2025'
source 'smdl'
dependency 'foundation'
profile 'xsmp-sdk'
tool 'smp'
```)">() + assign<&xsmp::Project::name>(ref<STRING>)
            + option(
                kw<"using">.doc<"Specify which version of the standard to use.">()
                + assign<&xsmp::Project::standard>(ref<STRING>))
            + many(append<&xsmp::Project::elements>(
                ref<ProfileReference> | ref<ToolReference> | ref<Dependency>
                | ref<Source>))> {};

  struct Profile : rule<create<xsmp::Profile>
                        + kw<"profile">.doc<R"(`profile '<profile-name>'`

Declares an XSMP profile contribution identifier that can be referenced from a project.
In practice, profile descriptors are usually provided by installed XSMP extensions.

**Example**
```xsmp
profile 'xsmp-sdk'
```)">() + assign<&xsmp::Profile::name>(ref<STRING>)> {};

  struct Tool : rule<create<xsmp::Tool> + kw<"tool">.doc<R"(`tool '<tool-name>'`

Declares an XSMP tool contribution identifier that can be referenced from a project.
In practice, tool descriptors are usually provided by installed XSMP extensions.

**Example**
```xsmp
tool 'smp'
```)">() + assign<&xsmp::Tool::name>(ref<STRING>)> {};

  struct ProjectRoot : rule<ref<Project> | ref<Profile> | ref<Tool>> {};

  using entry = ProjectRoot;
};

}  // namespace xsmp
