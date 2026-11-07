#pragma once

#include <xsmp/Contributions.hpp>
#include <xsmp/SharedCoreServices.hpp>
#include <xsmp/ast.hpp>

namespace xsmp {

// The documents of `project`, and with `dependencies` also those of the
// projects it depends on. Its own `xsmp.project` counts. Sorted by URI.
std::vector<cursiv::Document*> projectDocuments(XsmpSharedCoreServices&,
                                                const Project&,
                                                bool dependencies);

// The SMP files of the source folders of `project` and of its dependencies that
// could not be imported, as URI and message.
std::vector<std::pair<std::string, std::string>> projectSourceErrors(
    const XsmpSharedCoreServices&, const Project&);

// The same for the SMP files imported with a warning.
std::vector<std::pair<std::string, std::string>> projectSourceWarnings(
    const XsmpSharedCoreServices&, const Project&);

// The caller holds the workspace write lock. Validates the dependency closure,
// cleans each selected contribution once, and generates only the target
// project.
//
// An empty `output` means the folder of the `xsmp.project`. Returns
// `generatedProjects`, `skippedProjects` (errors were found, nothing was
// cleaned) and `failures`. A failure while generating is reported; a
// cancellation is thrown.
Json generateProject(XsmpSharedCoreServices&, const Project&,
                     const std::filesystem::path& output = {},
                     const cursiv::CancellationToken& = {});

}  // namespace xsmp
