#include <gtest/gtest.h>

#include <string>
#include <xsmp/ast.hpp>

#include "support/DeploymentWorld.hpp"

TEST(XsmpsedScopeProvider, ScheduleTaskReferencesResolveLocallyAndQualified) {
  xsmp::DeploymentWorld world;
  auto doc = world.build("xsmpsed", R"(schedule <Root = "platform"> Plan
    task Main { execute Plan.Work emit "Ready" }
    task Work { emit "Done" }
    event Main simulation "PT1S"
  )");
  auto* schedule = cursiv::ast_cast<xsmp::Schedule>(doc->parseResult.value);
  ASSERT_NE(schedule, nullptr);
  auto* main = cursiv::ast_cast<xsmp::Task>(schedule->elements[0]);
  ASSERT_NE(main, nullptr);
  auto* call = cursiv::ast_cast<xsmp::ExecuteTask>(main->elements[0]);
  ASSERT_NE(call, nullptr);
  EXPECT_EQ(call->task.get(), schedule->elements[1]);
  auto* event = cursiv::ast_cast<xsmp::SimulationEvent>(schedule->elements[2]);
  ASSERT_NE(event, nullptr);
  EXPECT_EQ(event->task.get(), main);
  for (const auto& diagnostic : doc->diagnostics)
    ADD_FAILURE() << diagnostic.message;
  EXPECT_TRUE(doc->diagnostics.empty());
}
