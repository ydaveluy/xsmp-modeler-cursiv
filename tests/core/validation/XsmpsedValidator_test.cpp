#include <gtest/gtest.h>

#include <algorithm>
#include <xsmp/utils/XsmpUtils.hpp>

#include "core/validation/DeploymentSemanticsFixture.hpp"

using XsmpsedValidatorTest = DeploymentSemantics;

TEST_F(XsmpsedValidatorTest, ScheduleChecksTypedActivityTargetsAndTimes) {
  auto document =
      build("xsmpsed", R"(schedule <Root = "root"> Timed epoch "invalid"
 task Main on demo.Root {
   call writable()
   property child = true
   transfer outValue -> outValue
   trig missing
 }
 event Main mission "not-a-duration" cycle "PT"
 event Main zulu "2026-13-40T25:00:00Z"
)");
  EXPECT_TRUE(
      has(*document,
          "path segment 'writable' shall resolve to a supported member"));
  EXPECT_TRUE(has(*document,
                  "path segment 'child' shall resolve to a supported member"));
  EXPECT_TRUE(has(*document, "Field marked as Input"));
  EXPECT_TRUE(has(*document, "MissionTime shall be a valid Duration"));
  EXPECT_TRUE(has(*document, "CycleTime shall be a valid Duration"));
  EXPECT_TRUE(has(*document, "EpochTime shall be a valid DateTime"));
  EXPECT_TRUE(has(*document, "ZuluTime shall be a valid DateTime"));
}

TEST_F(XsmpsedValidatorTest, TransfersConnectCompatibleFields) {
  add("src/moved.xsmpcat", R"(catalogue Moved namespace moved {
 public model M {
   output field Smp.Int32 oi input field Smp.Int32 ii input field Smp.Float64 if64
 }
})");
  clean(*build("xsmpsed", R"(schedule <Root = "root"> GoodTransfer
 task Main on moved.M { transfer oi -> ii }
)"));
  EXPECT_TRUE(has(*build("xsmpsed", R"(schedule <Root = "root"> BadTransfer
 task Main on moved.M { transfer oi -> if64 }
)"),
                  "same or a semantically equivalent type"));
}

TEST_F(XsmpsedValidatorTest, TaskNamesAreUnique) {
  const auto document = build("xsmpsed", R"(schedule <Root = "root"> TwoTasks
 task T { emit "A" }
 task T { emit "B" }
 task Other { emit "C" }
)");
  EXPECT_EQ(std::ranges::count(document->diagnostics, "Duplicated task name.",
                               &cursiv::Diagnostic::message),
            1);
}

// `Schedule.xsd`: -1 repeats for ever.
TEST_F(XsmpsedValidatorTest, ARepeatCountIsMinusOneOrNotNegative) {
  auto refused = [&, index = 0](std::string_view repeat) mutable {
    const auto document = build(
        "xsmpsed", "schedule <Root = \"root\"> Repeats" + std::to_string(index++)
                       + " task T { emit \"A\" } event T mission \"PT1S\" cycle "
                         "\"PT1S\" repeat "
                       + std::string(repeat) + "\n");
    return std::ranges::any_of(document->diagnostics, [](const auto& d) {
      return d.message.starts_with("RepeatCount shall be");
    });
  };
  EXPECT_FALSE(refused("-1"));
  EXPECT_FALSE(refused("0"));
  EXPECT_TRUE(refused("-2"));
}

// ECSS-E-ST-40-07 scheduler requirements: a repeating event needs a cycle time.
TEST_F(XsmpsedValidatorTest, RepeatingEventsNeedACycleTime) {
  auto schedule = [&, index = 0](std::string_view event) mutable {
    return build("xsmpsed", "schedule <Root = \"root\"> Cycles"
                                + std::to_string(index++)
                                + " task T { emit \"A\" } event T "
                                + std::string(event) + "\n");
  };
  const auto cyclic = "A cyclic Event shall have a positive CycleTime.";
  EXPECT_TRUE(
      has(*schedule(R"(mission "PT1S" cycle "PT0S" repeat 5)"), cyclic));
  EXPECT_TRUE(has(*schedule(R"(mission "PT1S" repeat -1)"), cyclic));
  EXPECT_TRUE(
      has(*schedule(R"(simulation "PT1S" cycle "PT0S" repeat 1)"), cyclic));
  EXPECT_TRUE(has(
      *schedule(
          R"(zulu "2020-01-01T00:00:00Z" cycle "-PT0.000000001S" repeat 1)"),
      cyclic));
  clean(*schedule(R"(mission "PT1S" cycle "PT0S" repeat 0)"));
  clean(*schedule(R"(mission "PT1S" cycle "PT0S")"));
  clean(*schedule(R"(mission "PT1S" cycle "PT1S" repeat 5)"));
  clean(*schedule(R"(simulation "PT1S" cycle "-PT0.000000001S" repeat 1)"));
}

TEST_F(XsmpsedValidatorTest, EventNamesHoldAnyCharacter) {
  clean(*build("xsmpsed", R"(schedule <Root = "root"> Emits
 task Main { emit "\x01" }
)"));
  clean(*build("xsmpsed", R"(schedule <Root = "root"> Starts
 task Main { emit "A" }
 event Main on "\x02"
)"));
  clean(*build("xsmpsed", R"(schedule <Root = "root"> Stops
 task Main { emit "A" }
 event Main on "A" until "\x03"
)"));
}
