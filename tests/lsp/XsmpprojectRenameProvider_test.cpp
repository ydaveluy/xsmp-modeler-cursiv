#include <gtest/gtest.h>

#include <cursiv/navigation/RenameProvider.hpp>
#include <string_view>

#include "support/LspTestSupport.hpp"

TEST(XsmpprojectRenameProvider, PreservesQuotedDeclarationAndReferences) {
  auto ws = xsmp::xsmpprojectWorkspace();
  const auto profile =
      xsmp::parse(ws, "xsmpproject", "profile 'custom-profile'\n",
                  "file:///p/custom.xsmpprofile");
  const auto project = xsmp::parse(ws, "xsmpproject",
                                   "project 'Demo'\nprofile 'custom-profile'\n",
                                   "file:///p/xsmp.project");
  const auto position = cursiv::to_lsp(profile->textDocument().positionAt(11));
  const auto& provider = *ws.language("xsmpproject").renameProvider;
  const auto edits = provider.rename(*profile, position, "new-profile");
  ASSERT_TRUE(edits && edits->changes);
  const auto profileEdit =
      edits->changes->find(::lsp::DocumentUri::parse(profile->uri));
  const auto projectEdit =
      edits->changes->find(::lsp::DocumentUri::parse(project->uri));
  ASSERT_NE(profileEdit, edits->changes->end());
  ASSERT_NE(projectEdit, edits->changes->end());
  EXPECT_EQ(xsmp::applyTextEdits(*profile, profileEdit->second),
            "profile 'new-profile'\n");
  EXPECT_EQ(xsmp::applyTextEdits(*project, projectEdit->second),
            "project 'Demo'\nprofile 'new-profile'\n");
}
