#include <gtest/gtest.h>

#include <string>
#include <xsmp/ast.hpp>

#include "support/DeploymentWorld.hpp"

TEST(XsmpsedGrammar, SchedulePreservesEveryActivityAndEventKind) {
  xsmp::DeploymentWorld world;
  constexpr std::string_view source = R"(
schedule <Root = "platform", Cycles = 3> Plan mission "2025-01-01T00:00:00Z" epoch "2025-01-01T00:00:00Z"
task Main on demo.Platform {
  call child.reset(value = 5)
  property mode = demo.Mode.On
  transfer child.output -> other.input
  trig child.step
  execute Plan.Work<Root = "child"> at child
  async emit "Ready"
}
task Work { emit "Done" }
event Main mission "PT1S" cycle "PT2S" repeat 3
event Main epoch "2025-01-01T00:00:01Z"
event Work simulation "PT5S"
event Work zulu "2025-01-01T00:00:02Z"
event Main on "Ready" until "Done" using mission delay "PT3S" cycle "PT4S" repeat 2
)";
  auto result = world.parse("xsmpsed", source);
  xsmp::expectClean(result);
  auto* schedule = cursiv::ast_cast<xsmp::Schedule>(result.value);
  ASSERT_NE(schedule, nullptr);
  ASSERT_EQ(schedule->elements.size(), 7u);
  auto* main = cursiv::ast_cast<xsmp::Task>(schedule->elements[0]);
  ASSERT_NE(main, nullptr);
  EXPECT_EQ(main->elements.size(), 6u);
  auto* emitted = cursiv::ast_cast<xsmp::EmitGlobalEvent>(main->elements[5]);
  ASSERT_NE(emitted, nullptr);
  EXPECT_TRUE(emitted->asynchronous);
  auto* event =
      cursiv::ast_cast<xsmp::GlobalEventTriggeredEvent>(schedule->elements[6]);
  ASSERT_NE(event, nullptr);
  EXPECT_EQ(event->timeKind, "mission");
  EXPECT_EQ(event->repeatCount, 2);
}

TEST(XsmpsedGrammar, UnsafeIsANameWhereANameGoesOn) {
  xsmp::DeploymentWorld world;
  xsmp::expectClean(world.parse("xsmpsed", R"(schedule S
task T on demo.Root {
  trig unsafe.step
  trig unsafe other.step
  property unsafe = 1i32
}
)"));
}
