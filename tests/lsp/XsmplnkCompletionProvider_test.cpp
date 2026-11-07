#include <xsmp/Builtins.hpp>

#include "support/LspTestSupport.hpp"

TEST(XsmplnkCompletionProvider, StatementsFollowAWholeLink) {
  auto world = xsmp::deploymentWorkspace();
  auto& shared = world.shared();
  world.registerLanguage(xsmp::createXsmpcatServices(shared));
  world.registerLanguage(xsmp::createXsmpprojectServices(shared));
  xsmp::loadXsmpcatBuiltins(shared);
  (void)xsmp::parse(world, "xsmpproject",
                    "project 'P' using 'ECSS_SMP_2025'\nsource 'smdl'\n",
                    "file:///links/xsmp.project");
  (void)xsmp::parse(world, "xsmpcat",
                    "catalogue C\nnamespace n\n{\n    public model Kid\n    {\n"
                    "        input field Smp.Int32 i\n"
                    "        output field Smp.Int32 o\n    }\n"
                    "    public model Root { container Kid kid = n.Kid }\n}\n",
                    "file:///links/smdl/C.xsmpcat");
  (void)xsmp::parse(world, "xsmpasb",
                    "assembly Asm\nRoot: n.Root\n{ kid += Kid: n.Kid }\n",
                    "file:///links/smdl/Asm.xsmpasb");
  (void)cursiv::test::expectCompletion(
      world.language("xsmplnk"),
      cursiv::test::ExpectedCompletionCallback{
          .text = "link L for Asm\n/\n{\n    field link Kid.o -> Kid.i\n    "
                  "<|>\n}\n",
          .assertion =
              [](const ::lsp::CompletionList& list) {
                EXPECT_TRUE(xsmp::hasLabel(list, "Component Link Base"));
                EXPECT_TRUE(xsmp::hasLabel(list, "Field Link"));
                EXPECT_TRUE(xsmp::hasLabel(list, "Kid"));
                // Not a member of `Kid`, read as if the path went on.
                EXPECT_FALSE(xsmp::hasLabel(list, "i"));
              },
          .parseOptions = xsmp::documentAt("file:///links/smdl/L.xsmplnk"),
          .disposeAfterCheck = true});
}
