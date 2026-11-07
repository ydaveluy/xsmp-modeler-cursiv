#pragma once
#include <string>
#include <utility>
#include <vector>

namespace xsmp {
struct Assertion {
  std::string mode;
  std::vector<std::string> values;
  bool diagnostics = false;
  std::vector<std::string> filterValues;
  bool filterContains = false;
  std::size_t count = 0;
};
struct Scenario {
  std::string lang, name;
  std::vector<std::pair<std::string, std::string>> docs;
  std::string text;
  std::vector<Assertion> assertions;
};
// Deployment scenarios: the documents to build, the text under test and the
// diagnostics it must (or must not) produce.
inline const std::vector<Scenario>& deploymentCases() {
  static const std::vector<Scenario> cases = {
      {R"dsl(xsmpasb)dsl",
       R"dsl(validates typed assembly configuration and link paths and honors unsafe)dsl",
       {{R"dsl(xsmp.project)dsl",
         R"dsl(project 'Demo' using 'ECSS_SMP_2025'
source 'src'
)dsl"},
        {R"dsl(src/types.xsmpcat)dsl", R"dsl(catalogue DemoTypes

namespace demo
{
    public event FlagEvent extends Smp.Bool
    @SimpleArray
    public array SimpleIntQuad = Smp.Int32[4]

    public struct Counters
    {
        field Smp.Int32 count
        field Smp.Bool enabled
        field SimpleIntQuad values
    }

    public model Child
    {
        field Smp.Int32 count
        field Smp.Float64 ratio
        input field Smp.Int32 inValue
        output field Smp.Int32 outValue
        reference Smp.IComponent backLogger
        reference Smp.IService backService

        entrypoint step
        {
            in inValue
            out outValue
        }

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }

    public model Root
    {
        field Smp.Bool enabledState
        field Smp.Int32 countState
        field Counters state
        field Smp.Char8 grade
        field Smp.String8 label
        output field Smp.Int32 outValue
        input field Smp.Int32 inValue
        container Child child = demo.Child
        reference Smp.IComponent logger
        reference Smp.IService serviceRef

        public property Smp.Bool enabled -> enabledState
        readOnly property Smp.Bool status -> enabledState
        public property Smp.Int32 count -> countState
        public def void apply(in Smp.Int32 nextCount, in Smp.Float64 nextRatio)

        entrypoint step
        {
            in inValue
            out outValue
        }

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }

    public model RootWithoutDefault
    {
        container Child child
    }

    public model RootWithTwoDefaults
    {
        container Child first = demo.Child
        container Child second = demo.Child
    }

    public model System
    {
        container Root bus = demo.Root
    }
}
)dsl"}},
       R"dsl(assembly Demo

configure Child
{
    count = true
    missing = 1i32
    unsafe missing = 1i32
}

configure enabled
{
}

Root: demo.Root
{
    child += Child: demo.Child
    field link outValue -> Child.outValue
    field link unsafe outValue -> unsafe Child.outValue
    event link outbound -> Child.inbound
    event link outbound -> Child.missing
}
)dsl",
       {{R"dsl(contains)dsl",
         {R"dsl(The value shall be compatible with type Smp.Int32.)dsl",
          R"dsl(The path segment 'missing' shall resolve to a Field of the configured Component.)dsl",
          R"dsl(The path segment 'enabled' shall resolve to a child Model Instance or Assembly Instance.)dsl",
          R"dsl(The path segment 'outValue' shall resolve to a Field marked as Input of the current Component.)dsl",
          R"dsl(The path segment 'missing' shall resolve to a supported member of the current Component.)dsl"},
         false,
         {},
         false,
         0},
        {R"dsl(none)dsl",
         {},
         false,
         {R"dsl(unsafe)dsl"},
         true,
         0}}},
      {R"dsl(xsmpasb)dsl",
       R"dsl(matches templated child instance names from assembly parameters)dsl",
       {{R"dsl(xsmp.project)dsl",
         R"dsl(project 'Demo' using 'ECSS_SMP_2025'
source 'src'
)dsl"},
        {R"dsl(src/types.xsmpcat)dsl", R"dsl(catalogue DemoTypes

namespace demo
{
    public event FlagEvent extends Smp.Bool
    @SimpleArray
    public array SimpleIntQuad = Smp.Int32[4]

    public struct Counters
    {
        field Smp.Int32 count
        field Smp.Bool enabled
        field SimpleIntQuad values
    }

    public model Child
    {
        field Smp.Int32 count
        field Smp.Float64 ratio
        input field Smp.Int32 inValue
        output field Smp.Int32 outValue
        reference Smp.IComponent backLogger
        reference Smp.IService backService

        entrypoint step
        {
            in inValue
            out outValue
        }

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }

    public model Root
    {
        field Smp.Bool enabledState
        field Smp.Int32 countState
        field Counters state
        field Smp.Char8 grade
        field Smp.String8 label
        output field Smp.Int32 outValue
        input field Smp.Int32 inValue
        container Child child = demo.Child
        reference Smp.IComponent logger
        reference Smp.IService serviceRef

        public property Smp.Bool enabled -> enabledState
        readOnly property Smp.Bool status -> enabledState
        public property Smp.Int32 count -> countState
        public def void apply(in Smp.Int32 nextCount, in Smp.Float64 nextRatio)

        entrypoint step
        {
            in inValue
            out outValue
        }

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }

    public model RootWithoutDefault
    {
        container Child child
    }

    public model RootWithTwoDefaults
    {
        container Child first = demo.Child
        container Child second = demo.Child
    }

    public model System
    {
        container Root bus = demo.Root
    }
}
)dsl"}},
       R"dsl(assembly <Side = "Left"> Demo

configure {Side}Receiver
{
    count = 1i32
}

Root: demo.Root
{
    child += {Side}Receiver: demo.Child
}
)dsl",
       {{R"dsl(exact)dsl", {}, false, {}, false, 0}}},
      {R"dsl(xsmpasb)dsl",
       R"dsl(allows sub-instance model implementations to be inferred from container defaults)dsl",
       {{R"dsl(xsmp.project)dsl",
         R"dsl(project 'Demo' using 'ECSS_SMP_2025'
source 'src'
)dsl"},
        {R"dsl(src/types.xsmpcat)dsl", R"dsl(catalogue DemoTypes

namespace demo
{
    public event FlagEvent extends Smp.Bool
    @SimpleArray
    public array SimpleIntQuad = Smp.Int32[4]

    public struct Counters
    {
        field Smp.Int32 count
        field Smp.Bool enabled
        field SimpleIntQuad values
    }

    public model Child
    {
        field Smp.Int32 count
        field Smp.Float64 ratio
        input field Smp.Int32 inValue
        output field Smp.Int32 outValue
        reference Smp.IComponent backLogger
        reference Smp.IService backService

        entrypoint step
        {
            in inValue
            out outValue
        }

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }

    public model Root
    {
        field Smp.Bool enabledState
        field Smp.Int32 countState
        field Counters state
        field Smp.Char8 grade
        field Smp.String8 label
        output field Smp.Int32 outValue
        input field Smp.Int32 inValue
        container Child child = demo.Child
        reference Smp.IComponent logger
        reference Smp.IService serviceRef

        public property Smp.Bool enabled -> enabledState
        readOnly property Smp.Bool status -> enabledState
        public property Smp.Int32 count -> countState
        public def void apply(in Smp.Int32 nextCount, in Smp.Float64 nextRatio)

        entrypoint step
        {
            in inValue
            out outValue
        }

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }

    public model RootWithoutDefault
    {
        container Child child
    }

    public model RootWithTwoDefaults
    {
        container Child first = demo.Child
        container Child second = demo.Child
    }

    public model System
    {
        container Root bus = demo.Root
    }
}
)dsl"}},
       R"dsl(assembly Demo

configure Child
{
    count = 1i32
}

Root: demo.Root
{
    child += Child
}
)dsl",
       {{R"dsl(exact)dsl", {}, false, {}, false, 0}}},
      {R"dsl(xsmpasb)dsl",
       R"dsl(allows sub-instance model implementations to be inferred from container defaults)dsl",
       {{R"dsl(xsmp.project)dsl",
         R"dsl(project 'Demo' using 'ECSS_SMP_2025'
source 'src'
)dsl"},
        {R"dsl(src/types.xsmpcat)dsl", R"dsl(catalogue DemoTypes

namespace demo
{
    public event FlagEvent extends Smp.Bool
    @SimpleArray
    public array SimpleIntQuad = Smp.Int32[4]

    public struct Counters
    {
        field Smp.Int32 count
        field Smp.Bool enabled
        field SimpleIntQuad values
    }

    public model Child
    {
        field Smp.Int32 count
        field Smp.Float64 ratio
        input field Smp.Int32 inValue
        output field Smp.Int32 outValue
        reference Smp.IComponent backLogger
        reference Smp.IService backService

        entrypoint step
        {
            in inValue
            out outValue
        }

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }

    public model Root
    {
        field Smp.Bool enabledState
        field Smp.Int32 countState
        field Counters state
        field Smp.Char8 grade
        field Smp.String8 label
        output field Smp.Int32 outValue
        input field Smp.Int32 inValue
        container Child child = demo.Child
        reference Smp.IComponent logger
        reference Smp.IService serviceRef

        public property Smp.Bool enabled -> enabledState
        readOnly property Smp.Bool status -> enabledState
        public property Smp.Int32 count -> countState
        public def void apply(in Smp.Int32 nextCount, in Smp.Float64 nextRatio)

        entrypoint step
        {
            in inValue
            out outValue
        }

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }

    public model RootWithoutDefault
    {
        container Child child
    }

    public model RootWithTwoDefaults
    {
        container Child first = demo.Child
        container Child second = demo.Child
    }

    public model System
    {
        container Root bus = demo.Root
    }
}
)dsl"}},
       R"dsl(assembly Adjacent

Root: demo.RootWithTwoDefaults
{
    first += First
    second += Second
}
)dsl",
       {{R"dsl(exact)dsl", {}, false, {}, false, 0}}},
      {R"dsl(xsmpasb)dsl",
       R"dsl(requires a root model implementation and default-backed shorthand sub-instances)dsl",
       {{R"dsl(xsmp.project)dsl",
         R"dsl(project 'Demo' using 'ECSS_SMP_2025'
source 'src'
)dsl"},
        {R"dsl(src/types.xsmpcat)dsl", R"dsl(catalogue DemoTypes

namespace demo
{
    public event FlagEvent extends Smp.Bool
    @SimpleArray
    public array SimpleIntQuad = Smp.Int32[4]

    public struct Counters
    {
        field Smp.Int32 count
        field Smp.Bool enabled
        field SimpleIntQuad values
    }

    public model Child
    {
        field Smp.Int32 count
        field Smp.Float64 ratio
        input field Smp.Int32 inValue
        output field Smp.Int32 outValue
        reference Smp.IComponent backLogger
        reference Smp.IService backService

        entrypoint step
        {
            in inValue
            out outValue
        }

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }

    public model Root
    {
        field Smp.Bool enabledState
        field Smp.Int32 countState
        field Counters state
        field Smp.Char8 grade
        field Smp.String8 label
        output field Smp.Int32 outValue
        input field Smp.Int32 inValue
        container Child child = demo.Child
        reference Smp.IComponent logger
        reference Smp.IService serviceRef

        public property Smp.Bool enabled -> enabledState
        readOnly property Smp.Bool status -> enabledState
        public property Smp.Int32 count -> countState
        public def void apply(in Smp.Int32 nextCount, in Smp.Float64 nextRatio)

        entrypoint step
        {
            in inValue
            out outValue
        }

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }

    public model RootWithoutDefault
    {
        container Child child
    }

    public model RootWithTwoDefaults
    {
        container Child first = demo.Child
        container Child second = demo.Child
    }

    public model System
    {
        container Root bus = demo.Root
    }
}
)dsl"}},
       R"dsl(assembly Demo

Root
{
}
)dsl",
       {{R"dsl(exact)dsl",
         {R"dsl(A root Model Instance shall declare an implementation.)dsl"},
         false,
         {},
         false,
         0}}},
      {R"dsl(xsmpasb)dsl",
       R"dsl(requires a root model implementation and default-backed shorthand sub-instances)dsl",
       {{R"dsl(xsmp.project)dsl",
         R"dsl(project 'Demo' using 'ECSS_SMP_2025'
source 'src'
)dsl"},
        {R"dsl(src/types.xsmpcat)dsl", R"dsl(catalogue DemoTypes

namespace demo
{
    public event FlagEvent extends Smp.Bool
    @SimpleArray
    public array SimpleIntQuad = Smp.Int32[4]

    public struct Counters
    {
        field Smp.Int32 count
        field Smp.Bool enabled
        field SimpleIntQuad values
    }

    public model Child
    {
        field Smp.Int32 count
        field Smp.Float64 ratio
        input field Smp.Int32 inValue
        output field Smp.Int32 outValue
        reference Smp.IComponent backLogger
        reference Smp.IService backService

        entrypoint step
        {
            in inValue
            out outValue
        }

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }

    public model Root
    {
        field Smp.Bool enabledState
        field Smp.Int32 countState
        field Counters state
        field Smp.Char8 grade
        field Smp.String8 label
        output field Smp.Int32 outValue
        input field Smp.Int32 inValue
        container Child child = demo.Child
        reference Smp.IComponent logger
        reference Smp.IService serviceRef

        public property Smp.Bool enabled -> enabledState
        readOnly property Smp.Bool status -> enabledState
        public property Smp.Int32 count -> countState
        public def void apply(in Smp.Int32 nextCount, in Smp.Float64 nextRatio)

        entrypoint step
        {
            in inValue
            out outValue
        }

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }

    public model RootWithoutDefault
    {
        container Child child
    }

    public model RootWithTwoDefaults
    {
        container Child first = demo.Child
        container Child second = demo.Child
    }

    public model System
    {
        container Root bus = demo.Root
    }
}
)dsl"}},
       R"dsl(assembly DemoNoDefault

Root: demo.RootWithoutDefault
{
    child += Child
}
)dsl",
       {{R"dsl(exact)dsl",
         {R"dsl(A sub-instance without an explicit implementation shall target a Container with a default component.)dsl"},
         false,
         {},
         false,
         0}}},
      {R"dsl(xsmpasb)dsl",
       R"dsl(allows unsuffixed numeric values when the safe resolved target type is known)dsl",
       {{R"dsl(xsmp.project)dsl",
         R"dsl(project 'Demo' using 'ECSS_SMP_2025'
source 'src'
)dsl"},
        {R"dsl(src/types.xsmpcat)dsl", R"dsl(catalogue DemoTypes

namespace demo
{
    public event FlagEvent extends Smp.Bool
    @SimpleArray
    public array SimpleIntQuad = Smp.Int32[4]

    public struct Counters
    {
        field Smp.Int32 count
        field Smp.Bool enabled
        field SimpleIntQuad values
    }

    public model Child
    {
        field Smp.Int32 count
        field Smp.Float64 ratio
        input field Smp.Int32 inValue
        output field Smp.Int32 outValue
        reference Smp.IComponent backLogger
        reference Smp.IService backService

        entrypoint step
        {
            in inValue
            out outValue
        }

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }

    public model Root
    {
        field Smp.Bool enabledState
        field Smp.Int32 countState
        field Counters state
        field Smp.Char8 grade
        field Smp.String8 label
        output field Smp.Int32 outValue
        input field Smp.Int32 inValue
        container Child child = demo.Child
        reference Smp.IComponent logger
        reference Smp.IService serviceRef

        public property Smp.Bool enabled -> enabledState
        readOnly property Smp.Bool status -> enabledState
        public property Smp.Int32 count -> countState
        public def void apply(in Smp.Int32 nextCount, in Smp.Float64 nextRatio)

        entrypoint step
        {
            in inValue
            out outValue
        }

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }

    public model RootWithoutDefault
    {
        container Child child
    }

    public model RootWithTwoDefaults
    {
        container Child first = demo.Child
        container Child second = demo.Child
    }

    public model System
    {
        container Root bus = demo.Root
    }
}
)dsl"}},
       R"dsl(assembly Demo

configure Child
{
    count = 2
    ratio = 1.5
}

Root: demo.Root
{
    property count = 3
    call apply(nextCount = 4, nextRatio = 2.5)
    child += Child: demo.Child
}
)dsl",
       {{R"dsl(exact)dsl", {}, false, {}, false, 0}}},
      {R"dsl(xsmpasb)dsl",
       R"dsl(rejects duplicate document names across document kinds)dsl",
       {{R"dsl(xsmp.project)dsl",
         R"dsl(project 'Demo' using 'ECSS_SMP_2025'
source 'src'
)dsl"},
        {R"dsl(src/types.xsmpcat)dsl", R"dsl(catalogue DemoTypes

namespace demo
{
    public event FlagEvent extends Smp.Bool
    @SimpleArray
    public array SimpleIntQuad = Smp.Int32[4]

    public struct Counters
    {
        field Smp.Int32 count
        field Smp.Bool enabled
        field SimpleIntQuad values
    }

    public model Child
    {
        field Smp.Int32 count
        field Smp.Float64 ratio
        input field Smp.Int32 inValue
        output field Smp.Int32 outValue
        reference Smp.IComponent backLogger
        reference Smp.IService backService

        entrypoint step
        {
            in inValue
            out outValue
        }

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }

    public model Root
    {
        field Smp.Bool enabledState
        field Smp.Int32 countState
        field Counters state
        field Smp.Char8 grade
        field Smp.String8 label
        output field Smp.Int32 outValue
        input field Smp.Int32 inValue
        container Child child = demo.Child
        reference Smp.IComponent logger
        reference Smp.IService serviceRef

        public property Smp.Bool enabled -> enabledState
        readOnly property Smp.Bool status -> enabledState
        public property Smp.Int32 count -> countState
        public def void apply(in Smp.Int32 nextCount, in Smp.Float64 nextRatio)

        entrypoint step
        {
            in inValue
            out outValue
        }

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }

    public model RootWithoutDefault
    {
        container Child child
    }

    public model RootWithTwoDefaults
    {
        container Child first = demo.Child
        container Child second = demo.Child
    }

    public model System
    {
        container Root bus = demo.Root
    }
}
)dsl"}},
       R"dsl(assembly DemoTypes

Root: demo.Root
{
}
)dsl",
       {{R"dsl(contains)dsl",
         {R"dsl(Duplicated Document name.)dsl"},
         false,
         {},
         false,
         0}}},
      {R"dsl(xsmpasb)dsl",
       R"dsl(concretizes nested assembly instance names from template arguments)dsl",
       {{R"dsl(xsmp.project)dsl",
         R"dsl(project 'Demo' using 'ECSS_SMP_2025'
source 'src'
)dsl"},
        {R"dsl(src/types.xsmpcat)dsl", R"dsl(catalogue DemoTypes

namespace demo
{
    public event FlagEvent extends Smp.Bool
    @SimpleArray
    public array SimpleIntQuad = Smp.Int32[4]

    public struct Counters
    {
        field Smp.Int32 count
        field Smp.Bool enabled
        field SimpleIntQuad values
    }

    public model Child
    {
        field Smp.Int32 count
        field Smp.Float64 ratio
        input field Smp.Int32 inValue
        output field Smp.Int32 outValue
        reference Smp.IComponent backLogger
        reference Smp.IService backService

        entrypoint step
        {
            in inValue
            out outValue
        }

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }

    public model Root
    {
        field Smp.Bool enabledState
        field Smp.Int32 countState
        field Counters state
        field Smp.Char8 grade
        field Smp.String8 label
        output field Smp.Int32 outValue
        input field Smp.Int32 inValue
        container Child child = demo.Child
        reference Smp.IComponent logger
        reference Smp.IService serviceRef

        public property Smp.Bool enabled -> enabledState
        readOnly property Smp.Bool status -> enabledState
        public property Smp.Int32 count -> countState
        public def void apply(in Smp.Int32 nextCount, in Smp.Float64 nextRatio)

        entrypoint step
        {
            in inValue
            out outValue
        }

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }

    public model RootWithoutDefault
    {
        container Child child
    }

    public model RootWithTwoDefaults
    {
        container Child first = demo.Child
        container Child second = demo.Child
    }

    public model System
    {
        container Root bus = demo.Root
    }
}
)dsl"},
        {R"dsl(src/nested.xsmpasb)dsl",
         R"dsl(assembly <Target = "child"> NestedAsm

NestedRoot: demo.Root
{
    child += {Target}: demo.Child
}
)dsl"}},
       R"dsl(assembly Demo

configure Bus.leaf
{
    count = 1i32
}

Root: demo.System
{
    bus += Bus: NestedAsm<Target = "leaf">
}
)dsl",
       {{R"dsl(exact)dsl", {}, false, {}, false, 0}}},
      {R"dsl(xsmpasb)dsl",
       R"dsl(reports unknown placeholders in assembly paths)dsl",
       {{R"dsl(xsmp.project)dsl",
         R"dsl(project 'Demo' using 'ECSS_SMP_2025'
source 'src'
)dsl"},
        {R"dsl(src/types.xsmpcat)dsl", R"dsl(catalogue DemoTypes

namespace demo
{
    public event FlagEvent extends Smp.Bool
    @SimpleArray
    public array SimpleIntQuad = Smp.Int32[4]

    public struct Counters
    {
        field Smp.Int32 count
        field Smp.Bool enabled
        field SimpleIntQuad values
    }

    public model Child
    {
        field Smp.Int32 count
        field Smp.Float64 ratio
        input field Smp.Int32 inValue
        output field Smp.Int32 outValue
        reference Smp.IComponent backLogger
        reference Smp.IService backService

        entrypoint step
        {
            in inValue
            out outValue
        }

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }

    public model Root
    {
        field Smp.Bool enabledState
        field Smp.Int32 countState
        field Counters state
        field Smp.Char8 grade
        field Smp.String8 label
        output field Smp.Int32 outValue
        input field Smp.Int32 inValue
        container Child child = demo.Child
        reference Smp.IComponent logger
        reference Smp.IService serviceRef

        public property Smp.Bool enabled -> enabledState
        readOnly property Smp.Bool status -> enabledState
        public property Smp.Int32 count -> countState
        public def void apply(in Smp.Int32 nextCount, in Smp.Float64 nextRatio)

        entrypoint step
        {
            in inValue
            out outValue
        }

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }

    public model RootWithoutDefault
    {
        container Child child
    }

    public model RootWithTwoDefaults
    {
        container Child first = demo.Child
        container Child second = demo.Child
    }

    public model System
    {
        container Root bus = demo.Root
    }
}
)dsl"}},
       R"dsl(assembly Demo

configure {Missing}Receiver
{
    count = 1i32
}

Root: demo.Root
{
    child += LeftReceiver: demo.Child
}
)dsl",
       {{R"dsl(exact)dsl",
         {R"dsl(The placeholder '{Missing}' shall resolve to a Template Argument of the enclosing Assembly.)dsl"},
         false,
         {},
         false,
         0}}},
      {R"dsl(xsmpasb)dsl",
       R"dsl(supports template placeholders followed by suffix text in instance names)dsl",
       {{R"dsl(xsmp.project)dsl",
         R"dsl(project 'Demo' using 'ECSS_SMP_2025'
source 'src'
)dsl"},
        {R"dsl(src/types.xsmpcat)dsl", R"dsl(catalogue DemoTypes

namespace demo
{
    public event FlagEvent extends Smp.Bool
    @SimpleArray
    public array SimpleIntQuad = Smp.Int32[4]

    public struct Counters
    {
        field Smp.Int32 count
        field Smp.Bool enabled
        field SimpleIntQuad values
    }

    public model Child
    {
        field Smp.Int32 count
        field Smp.Float64 ratio
        input field Smp.Int32 inValue
        output field Smp.Int32 outValue
        reference Smp.IComponent backLogger
        reference Smp.IService backService

        entrypoint step
        {
            in inValue
            out outValue
        }

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }

    public model Root
    {
        field Smp.Bool enabledState
        field Smp.Int32 countState
        field Counters state
        field Smp.Char8 grade
        field Smp.String8 label
        output field Smp.Int32 outValue
        input field Smp.Int32 inValue
        container Child child = demo.Child
        reference Smp.IComponent logger
        reference Smp.IService serviceRef

        public property Smp.Bool enabled -> enabledState
        readOnly property Smp.Bool status -> enabledState
        public property Smp.Int32 count -> countState
        public def void apply(in Smp.Int32 nextCount, in Smp.Float64 nextRatio)

        entrypoint step
        {
            in inValue
            out outValue
        }

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }

    public model RootWithoutDefault
    {
        container Child child
    }

    public model RootWithTwoDefaults
    {
        container Child first = demo.Child
        container Child second = demo.Child
    }

    public model System
    {
        container Root bus = demo.Root
    }
}
)dsl"}},
       R"dsl(assembly <Index = 1, Suffix = "Tail"> Demo

configure Unit{Index}_{Suffix}
{
    count = 1i32
}

Root: demo.Root
{
    child += Unit{Index}_{Suffix}: demo.Child
}
)dsl",
       {{R"dsl(exact)dsl", {}, false, {}, false, 0}}},
      {R"dsl(xsmpasb)dsl",
       R"dsl(validates that a sub-instance type matches the selected container)dsl",
       {{R"dsl(xsmp.project)dsl",
         R"dsl(project 'Demo' using 'ECSS_SMP_2025'
source 'src'
)dsl"},
        {R"dsl(src/types.xsmpcat)dsl", R"dsl(catalogue DemoTypes

namespace demo
{
    public event FlagEvent extends Smp.Bool
    @SimpleArray
    public array SimpleIntQuad = Smp.Int32[4]

    public struct Counters
    {
        field Smp.Int32 count
        field Smp.Bool enabled
        field SimpleIntQuad values
    }

    public model Child
    {
        field Smp.Int32 count
        field Smp.Float64 ratio
        input field Smp.Int32 inValue
        output field Smp.Int32 outValue
        reference Smp.IComponent backLogger
        reference Smp.IService backService

        entrypoint step
        {
            in inValue
            out outValue
        }

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }

    public model Root
    {
        field Smp.Bool enabledState
        field Smp.Int32 countState
        field Counters state
        field Smp.Char8 grade
        field Smp.String8 label
        output field Smp.Int32 outValue
        input field Smp.Int32 inValue
        container Child child = demo.Child
        reference Smp.IComponent logger
        reference Smp.IService serviceRef

        public property Smp.Bool enabled -> enabledState
        readOnly property Smp.Bool status -> enabledState
        public property Smp.Int32 count -> countState
        public def void apply(in Smp.Int32 nextCount, in Smp.Float64 nextRatio)

        entrypoint step
        {
            in inValue
            out outValue
        }

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }

    public model RootWithoutDefault
    {
        container Child child
    }

    public model RootWithTwoDefaults
    {
        container Child first = demo.Child
        container Child second = demo.Child
    }

    public model System
    {
        container Root bus = demo.Root
    }
}
)dsl"},
        {R"dsl(src/nested.xsmpasb)dsl", R"dsl(assembly NestedAsm

NestedRoot: demo.System
{
}
)dsl"}},
       R"dsl(assembly Demo

Root: demo.Root
{
    child += Nested: NestedAsm
}
)dsl",
       {{R"dsl(exact)dsl",
         {R"dsl(The type of the sub-instance shall be compatible with the selected Container.)dsl"},
         false,
         {},
         false,
         0}}},
      {R"dsl(xsmpasb)dsl",
       R"dsl(reports when a container upper bound is exceeded)dsl",
       {{R"dsl(xsmp.project)dsl",
         R"dsl(project 'Demo' using 'ECSS_SMP_2025'
source 'src'
)dsl"},
        {R"dsl(src/types.xsmpcat)dsl", R"dsl(catalogue DemoTypes

namespace demo
{
    public event FlagEvent extends Smp.Bool
    @SimpleArray
    public array SimpleIntQuad = Smp.Int32[4]

    public struct Counters
    {
        field Smp.Int32 count
        field Smp.Bool enabled
        field SimpleIntQuad values
    }

    public model Child
    {
        field Smp.Int32 count
        field Smp.Float64 ratio
        input field Smp.Int32 inValue
        output field Smp.Int32 outValue
        reference Smp.IComponent backLogger
        reference Smp.IService backService

        entrypoint step
        {
            in inValue
            out outValue
        }

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }

    public model Root
    {
        field Smp.Bool enabledState
        field Smp.Int32 countState
        field Counters state
        field Smp.Char8 grade
        field Smp.String8 label
        output field Smp.Int32 outValue
        input field Smp.Int32 inValue
        container Child child = demo.Child
        reference Smp.IComponent logger
        reference Smp.IService serviceRef

        public property Smp.Bool enabled -> enabledState
        readOnly property Smp.Bool status -> enabledState
        public property Smp.Int32 count -> countState
        public def void apply(in Smp.Int32 nextCount, in Smp.Float64 nextRatio)

        entrypoint step
        {
            in inValue
            out outValue
        }

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }

    public model RootWithoutDefault
    {
        container Child child
    }

    public model RootWithTwoDefaults
    {
        container Child first = demo.Child
        container Child second = demo.Child
    }

    public model System
    {
        container Root bus = demo.Root
    }
}
)dsl"}},
       R"dsl(assembly Demo

Root: demo.Root
{
    child += Left: demo.Child
    child += Right: demo.Child
}
)dsl",
       {{R"dsl(exact)dsl",
         {R"dsl(The Container 'child' shall not contain more than 1 sub-instance(s).)dsl"},
         false,
         {},
         false,
         0}}},
      {R"dsl(xsmpasb)dsl",
       R"dsl(reports when a reference upper bound is exceeded across nested assembly occurrences)dsl",
       {{R"dsl(xsmp.project)dsl",
         R"dsl(project 'Demo' using 'ECSS_SMP_2025'
source 'src'
)dsl"},
        {R"dsl(src/types.xsmpcat)dsl", R"dsl(catalogue DemoTypes

namespace demo
{
    public event FlagEvent extends Smp.Bool
    @SimpleArray
    public array SimpleIntQuad = Smp.Int32[4]

    public struct Counters
    {
        field Smp.Int32 count
        field Smp.Bool enabled
        field SimpleIntQuad values
    }

    public model Child
    {
        field Smp.Int32 count
        field Smp.Float64 ratio
        input field Smp.Int32 inValue
        output field Smp.Int32 outValue
        reference Smp.IComponent backLogger
        reference Smp.IService backService

        entrypoint step
        {
            in inValue
            out outValue
        }

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }

    public model Root
    {
        field Smp.Bool enabledState
        field Smp.Int32 countState
        field Counters state
        field Smp.Char8 grade
        field Smp.String8 label
        output field Smp.Int32 outValue
        input field Smp.Int32 inValue
        container Child child = demo.Child
        reference Smp.IComponent logger
        reference Smp.IService serviceRef

        public property Smp.Bool enabled -> enabledState
        readOnly property Smp.Bool status -> enabledState
        public property Smp.Int32 count -> countState
        public def void apply(in Smp.Int32 nextCount, in Smp.Float64 nextRatio)

        entrypoint step
        {
            in inValue
            out outValue
        }

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }

    public model RootWithoutDefault
    {
        container Child child
    }

    public model RootWithTwoDefaults
    {
        container Child first = demo.Child
        container Child second = demo.Child
    }

    public model System
    {
        container Root bus = demo.Root
    }
}
)dsl"},
        {R"dsl(src/nested.xsmpasb)dsl", R"dsl(assembly NestedAsm

NestedRoot: demo.Root
{
    child += Child: demo.Child
    interface link logger -> Child
}
)dsl"}},
       R"dsl(assembly Demo

Root: demo.System
{
    bus += Bus: NestedAsm
    interface link Bus.logger -> Bus.Child
}
)dsl",
       {{R"dsl(exact)dsl",
         {R"dsl(The Reference 'logger' of instance '/Bus' shall not be connected more than 1 time(s).)dsl"},
         false,
         {},
         false,
         0}}},
      {R"dsl(xsmpasb)dsl",
       R"dsl(warns when an assembly template parameter is not used)dsl",
       {{R"dsl(xsmp.project)dsl",
         R"dsl(project 'Demo' using 'ECSS_SMP_2025'
source 'src'
)dsl"},
        {R"dsl(src/types.xsmpcat)dsl", R"dsl(catalogue DemoTypes

namespace demo
{
    public event FlagEvent extends Smp.Bool
    @SimpleArray
    public array SimpleIntQuad = Smp.Int32[4]

    public struct Counters
    {
        field Smp.Int32 count
        field Smp.Bool enabled
        field SimpleIntQuad values
    }

    public model Child
    {
        field Smp.Int32 count
        field Smp.Float64 ratio
        input field Smp.Int32 inValue
        output field Smp.Int32 outValue
        reference Smp.IComponent backLogger
        reference Smp.IService backService

        entrypoint step
        {
            in inValue
            out outValue
        }

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }

    public model Root
    {
        field Smp.Bool enabledState
        field Smp.Int32 countState
        field Counters state
        field Smp.Char8 grade
        field Smp.String8 label
        output field Smp.Int32 outValue
        input field Smp.Int32 inValue
        container Child child = demo.Child
        reference Smp.IComponent logger
        reference Smp.IService serviceRef

        public property Smp.Bool enabled -> enabledState
        readOnly property Smp.Bool status -> enabledState
        public property Smp.Int32 count -> countState
        public def void apply(in Smp.Int32 nextCount, in Smp.Float64 nextRatio)

        entrypoint step
        {
            in inValue
            out outValue
        }

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }

    public model RootWithoutDefault
    {
        container Child child
    }

    public model RootWithTwoDefaults
    {
        container Child first = demo.Child
        container Child second = demo.Child
    }

    public model System
    {
        container Root bus = demo.Root
    }
}
)dsl"}},
       R"dsl(assembly <Used = "Left", Unused = "Right"> Demo

configure {Used}Receiver
{
    count = 1i32
}

Root: demo.Root
{
    child += {Used}Receiver: demo.Child
}
)dsl",
       {{R"dsl(exact)dsl",
         {R"dsl(The Template Parameter 'Unused' is not used.)dsl"},
         false,
         {},
         false,
         0}}},
      {R"dsl(xsmpasb)dsl",
       R"dsl(requires explicit template values and validates expanded instance names)dsl",
       {{R"dsl(xsmp.project)dsl",
         R"dsl(project 'Demo' using 'ECSS_SMP_2025'
source 'src'
)dsl"},
        {R"dsl(src/types.xsmpcat)dsl", R"dsl(catalogue DemoTypes

namespace demo
{
    public event FlagEvent extends Smp.Bool
    @SimpleArray
    public array SimpleIntQuad = Smp.Int32[4]

    public struct Counters
    {
        field Smp.Int32 count
        field Smp.Bool enabled
        field SimpleIntQuad values
    }

    public model Child
    {
        field Smp.Int32 count
        field Smp.Float64 ratio
        input field Smp.Int32 inValue
        output field Smp.Int32 outValue
        reference Smp.IComponent backLogger
        reference Smp.IService backService

        entrypoint step
        {
            in inValue
            out outValue
        }

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }

    public model Root
    {
        field Smp.Bool enabledState
        field Smp.Int32 countState
        field Counters state
        field Smp.Char8 grade
        field Smp.String8 label
        output field Smp.Int32 outValue
        input field Smp.Int32 inValue
        container Child child = demo.Child
        reference Smp.IComponent logger
        reference Smp.IService serviceRef

        public property Smp.Bool enabled -> enabledState
        readOnly property Smp.Bool status -> enabledState
        public property Smp.Int32 count -> countState
        public def void apply(in Smp.Int32 nextCount, in Smp.Float64 nextRatio)

        entrypoint step
        {
            in inValue
            out outValue
        }

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }

    public model RootWithoutDefault
    {
        container Child child
    }

    public model RootWithTwoDefaults
    {
        container Child first = demo.Child
        container Child second = demo.Child
    }

    public model System
    {
        container Root bus = demo.Root
    }
}
)dsl"}},
       R"dsl(assembly <Lane: string, Index: int32, Bad = "1bad"> Demo

Root: demo.Root
{
    child += {Bad}: demo.Child
}
)dsl",
       {{R"dsl(contains)dsl",
         {R"dsl(A template parameter needs a value.)dsl",
          R"dsl(The expanded name '1bad' is not valid for SMP Level 2.)dsl"},
         false,
         {},
         false,
         0},
        {R"dsl(count)dsl",
         {},
         false,
         {R"dsl(A template parameter needs a value.)dsl"},
         false,
         2}}},
      {R"dsl(xsmpasb)dsl",
       R"dsl(validates subscriptions, operation parameters, and writable properties)dsl",
       {{R"dsl(xsmp.project)dsl",
         R"dsl(project 'Demo' using 'ECSS_SMP_2025'
source 'src'
)dsl"},
        {R"dsl(src/types.xsmpcat)dsl", R"dsl(catalogue DemoTypes

namespace demo
{
    public event FlagEvent extends Smp.Bool
    @SimpleArray
    public array SimpleIntQuad = Smp.Int32[4]

    public struct Counters
    {
        field Smp.Int32 count
        field Smp.Bool enabled
        field SimpleIntQuad values
    }

    public model Child
    {
        field Smp.Int32 count
        field Smp.Float64 ratio
        input field Smp.Int32 inValue
        output field Smp.Int32 outValue
        reference Smp.IComponent backLogger
        reference Smp.IService backService

        entrypoint step
        {
            in inValue
            out outValue
        }

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }

    public model Root
    {
        field Smp.Bool enabledState
        field Smp.Int32 countState
        field Counters state
        field Smp.Char8 grade
        field Smp.String8 label
        output field Smp.Int32 outValue
        input field Smp.Int32 inValue
        container Child child = demo.Child
        reference Smp.IComponent logger
        reference Smp.IService serviceRef

        public property Smp.Bool enabled -> enabledState
        readOnly property Smp.Bool status -> enabledState
        public property Smp.Int32 count -> countState
        public def void apply(in Smp.Int32 nextCount, in Smp.Float64 nextRatio)

        entrypoint step
        {
            in inValue
            out outValue
        }

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }

    public model RootWithoutDefault
    {
        container Child child
    }

    public model RootWithTwoDefaults
    {
        container Child first = demo.Child
        container Child second = demo.Child
    }

    public model System
    {
        container Root bus = demo.Root
    }
}
)dsl"}},
       R"dsl(assembly Demo

configure child
{
    subscribe count -> "Tick"
}

Root: demo.Root
{
    property status = true
    call enabled(nextCount = 1i32)
    call apply(nextCount = 1i32, nextCount = 2i32, missing = 3i32, nextRatio = true)
    child += Child: demo.Child
}
)dsl",
       {{R"dsl(contains)dsl",
         {R"dsl(The selected entry point shall resolve to an EntryPoint of the current Component.)dsl",
          R"dsl(A PropertyValue shall target a writable Property.)dsl",
          R"dsl(The selected operation shall resolve to an Operation of the current Component.)dsl",
          R"dsl(Duplicated parameter name.)dsl",
          R"dsl(The parameter 'missing' shall resolve to a Parameter of operation apply.)dsl",
          R"dsl(The value shall be compatible with type Smp.Float64.)dsl"},
         false,
         {},
         false,
         0}}},
      {R"dsl(xsmpasb)dsl",
       R"dsl(validates assembly structure and array values)dsl",
       {{R"dsl(xsmp.project)dsl",
         R"dsl(project 'Demo' using 'ECSS_SMP_2025'
source 'src'
)dsl"},
        {R"dsl(src/types.xsmpcat)dsl", R"dsl(catalogue DemoTypes

namespace demo
{
    public event FlagEvent extends Smp.Bool
    @SimpleArray
    public array SimpleIntQuad = Smp.Int32[4]

    public struct Counters
    {
        field Smp.Int32 count
        field Smp.Bool enabled
        field SimpleIntQuad values
    }

    public model Child
    {
        field Smp.Int32 count
        field Smp.Float64 ratio
        input field Smp.Int32 inValue
        output field Smp.Int32 outValue
        reference Smp.IComponent backLogger
        reference Smp.IService backService

        entrypoint step
        {
            in inValue
            out outValue
        }

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }

    public model Root
    {
        field Smp.Bool enabledState
        field Smp.Int32 countState
        field Counters state
        field Smp.Char8 grade
        field Smp.String8 label
        output field Smp.Int32 outValue
        input field Smp.Int32 inValue
        container Child child = demo.Child
        reference Smp.IComponent logger
        reference Smp.IService serviceRef

        public property Smp.Bool enabled -> enabledState
        readOnly property Smp.Bool status -> enabledState
        public property Smp.Int32 count -> countState
        public def void apply(in Smp.Int32 nextCount, in Smp.Float64 nextRatio)

        entrypoint step
        {
            in inValue
            out outValue
        }

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }

    public model RootWithoutDefault
    {
        container Child child
    }

    public model RootWithTwoDefaults
    {
        container Child first = demo.Child
        container Child second = demo.Child
    }

    public model System
    {
        container Root bus = demo.Root
    }
}
)dsl"}},
       R"dsl(assembly Demo

Root: demo.Root
{
    state = {
        count = true,
        missing = 1i32,
        values = [2: 7, 8, 9],
        false,
        true
    }
    grade = "A"
    label = 'A'
}
)dsl",
       {{R"dsl(contains)dsl",
         {R"dsl(The value shall be compatible with type Smp.Int32.)dsl",
          R"dsl(The structure field path 'missing' does not exist on type demo.Counters.)dsl",
          R"dsl(The array value shall not exceed 4 item(s) when StartIndex is applied.)dsl",
          R"dsl(The structure value shall not contain more values than the fields of demo.Counters.)dsl",
          R"dsl(The value shall be compatible with type Smp.Char8.)dsl",
          R"dsl(The value shall be compatible with type Smp.String8.)dsl"},
         false,
         {},
         false,
         0}}},
      {R"dsl(xsmpasb)dsl",
       R"dsl(validates interface links against endpoint types and path constraints)dsl",
       {{R"dsl(xsmp.project)dsl",
         R"dsl(project 'Demo' using 'ECSS_SMP_2025'
source 'src'
)dsl"},
        {R"dsl(src/types.xsmpcat)dsl", R"dsl(catalogue DemoTypes

namespace demo
{
    public event FlagEvent extends Smp.Bool
    @SimpleArray
    public array SimpleIntQuad = Smp.Int32[4]

    public struct Counters
    {
        field Smp.Int32 count
        field Smp.Bool enabled
        field SimpleIntQuad values
    }

    public model Child
    {
        field Smp.Int32 count
        field Smp.Float64 ratio
        input field Smp.Int32 inValue
        output field Smp.Int32 outValue
        reference Smp.IComponent backLogger
        reference Smp.IService backService

        entrypoint step
        {
            in inValue
            out outValue
        }

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }

    public model Root
    {
        field Smp.Bool enabledState
        field Smp.Int32 countState
        field Counters state
        field Smp.Char8 grade
        field Smp.String8 label
        output field Smp.Int32 outValue
        input field Smp.Int32 inValue
        container Child child = demo.Child
        reference Smp.IComponent logger
        reference Smp.IService serviceRef

        public property Smp.Bool enabled -> enabledState
        readOnly property Smp.Bool status -> enabledState
        public property Smp.Int32 count -> countState
        public def void apply(in Smp.Int32 nextCount, in Smp.Float64 nextRatio)

        entrypoint step
        {
            in inValue
            out outValue
        }

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }

    public model RootWithoutDefault
    {
        container Child child
    }

    public model RootWithTwoDefaults
    {
        container Child first = demo.Child
        container Child second = demo.Child
    }

    public model System
    {
        container Root bus = demo.Root
    }
}
)dsl"}},
       R"dsl(assembly Demo

Root: demo.Root
{
    child += Child: demo.Child
    field link ../outValue -> /Child.inValue
    interface link serviceRef -> Child
    interface link ../logger -> Child
    interface link logger -> Child:outValue
    interface link logger -> Child:backService
}
)dsl",
       {{R"dsl(contains)dsl",
         {R"dsl(Paths shall not contain '..'.)dsl",
          R"dsl(The Client Path shall refer to the current Model Instance or one of its children.)dsl",
          R"dsl(The selected source reference shall be compatible with the Client Component.)dsl",
          R"dsl(The selected reference shall resolve to a Reference of the Client Component.)dsl",
          R"dsl(The selected reference shall be compatible with the Owner Component.)dsl"},
         false,
         {},
         false,
         0}}},
      {R"dsl(xsmpasb)dsl",
       R"dsl(reports recursive assembly instantiation without crashing)dsl",
       {{R"dsl(xsmp.project)dsl",
         R"dsl(project 'Demo' using 'ECSS_SMP_2025'
source 'src'
)dsl"},
        {R"dsl(src/types.xsmpcat)dsl", R"dsl(catalogue DemoTypes

namespace demo
{
    public event FlagEvent extends Smp.Bool
    @SimpleArray
    public array SimpleIntQuad = Smp.Int32[4]

    public struct Counters
    {
        field Smp.Int32 count
        field Smp.Bool enabled
        field SimpleIntQuad values
    }

    public model Child
    {
        field Smp.Int32 count
        field Smp.Float64 ratio
        input field Smp.Int32 inValue
        output field Smp.Int32 outValue
        reference Smp.IComponent backLogger
        reference Smp.IService backService

        entrypoint step
        {
            in inValue
            out outValue
        }

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }

    public model Root
    {
        field Smp.Bool enabledState
        field Smp.Int32 countState
        field Counters state
        field Smp.Char8 grade
        field Smp.String8 label
        output field Smp.Int32 outValue
        input field Smp.Int32 inValue
        container Child child = demo.Child
        reference Smp.IComponent logger
        reference Smp.IService serviceRef

        public property Smp.Bool enabled -> enabledState
        readOnly property Smp.Bool status -> enabledState
        public property Smp.Int32 count -> countState
        public def void apply(in Smp.Int32 nextCount, in Smp.Float64 nextRatio)

        entrypoint step
        {
            in inValue
            out outValue
        }

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }

    public model RootWithoutDefault
    {
        container Child child
    }

    public model RootWithTwoDefaults
    {
        container Child first = demo.Child
        container Child second = demo.Child
    }

    public model System
    {
        container Root bus = demo.Root
    }
}
)dsl"}},
       R"dsl(assembly Demo

Root: demo.System
{
    bus += Inner: Demo
}
)dsl",
       {{R"dsl(contains)dsl",
         {R"dsl(Recursive Assembly instantiation is not allowed.)dsl"},
         false,
         {},
         false,
         0}}},
      {R"dsl(xsmpasb)dsl",
       R"dsl(navigates to templated child instances from a concrete configuration path)dsl",
       {{R"dsl(xsmp.project)dsl",
         R"dsl(project 'Demo' using 'ECSS_SMP_2025'
source 'src'
)dsl"},
        {R"dsl(src/types.xsmpcat)dsl", R"dsl(catalogue DemoTypes

namespace demo
{
    public event FlagEvent extends Smp.Bool
    @SimpleArray
    public array SimpleIntQuad = Smp.Int32[4]

    public struct Counters
    {
        field Smp.Int32 count
        field Smp.Bool enabled
        field SimpleIntQuad values
    }

    public model Child
    {
        field Smp.Int32 count
        field Smp.Float64 ratio
        input field Smp.Int32 inValue
        output field Smp.Int32 outValue
        reference Smp.IComponent backLogger
        reference Smp.IService backService

        entrypoint step
        {
            in inValue
            out outValue
        }

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }

    public model Root
    {
        field Smp.Bool enabledState
        field Smp.Int32 countState
        field Counters state
        field Smp.Char8 grade
        field Smp.String8 label
        output field Smp.Int32 outValue
        input field Smp.Int32 inValue
        container Child child = demo.Child
        reference Smp.IComponent logger
        reference Smp.IService serviceRef

        public property Smp.Bool enabled -> enabledState
        readOnly property Smp.Bool status -> enabledState
        public property Smp.Int32 count -> countState
        public def void apply(in Smp.Int32 nextCount, in Smp.Float64 nextRatio)

        entrypoint step
        {
            in inValue
            out outValue
        }

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }

    public model RootWithoutDefault
    {
        container Child child
    }

    public model RootWithTwoDefaults
    {
        container Child first = demo.Child
        container Child second = demo.Child
    }

    public model System
    {
        container Root bus = demo.Root
    }
}
)dsl"}},
       R"dsl(assembly <Lane = "A"> Demo

Root: demo.System
{
    bus += Unit{Lane}: demo.Root
}
configure UnitA
{
}
)dsl",
       {{R"dsl(exact)dsl", {}, false, {}, false, 0}}},
      {R"dsl(xsmpasb)dsl",
       R"dsl(rejects an out-of-bounds array index in an assembly field path)dsl",
       {{R"dsl(xsmp.project)dsl",
         R"dsl(project 'Demo' using 'ECSS_SMP_2025'
source 'src'
)dsl"},
        {R"dsl(src/types.xsmpcat)dsl", R"dsl(catalogue DemoTypes

namespace demo
{
    public event FlagEvent extends Smp.Bool
    @SimpleArray
    public array SimpleIntQuad = Smp.Int32[4]

    public struct Counters
    {
        field Smp.Int32 count
        field Smp.Bool enabled
        field SimpleIntQuad values
    }

    public model Child
    {
        field Smp.Int32 count
        field Smp.Float64 ratio
        input field Smp.Int32 inValue
        output field Smp.Int32 outValue
        reference Smp.IComponent backLogger
        reference Smp.IService backService

        entrypoint step
        {
            in inValue
            out outValue
        }

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }

    public model Root
    {
        field Smp.Bool enabledState
        field Smp.Int32 countState
        field Counters state
        field Smp.Char8 grade
        field Smp.String8 label
        output field Smp.Int32 outValue
        input field Smp.Int32 inValue
        container Child child = demo.Child
        reference Smp.IComponent logger
        reference Smp.IService serviceRef

        public property Smp.Bool enabled -> enabledState
        readOnly property Smp.Bool status -> enabledState
        public property Smp.Int32 count -> countState
        public def void apply(in Smp.Int32 nextCount, in Smp.Float64 nextRatio)

        entrypoint step
        {
            in inValue
            out outValue
        }

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }

    public model RootWithoutDefault
    {
        container Child child
    }

    public model RootWithTwoDefaults
    {
        container Child first = demo.Child
        container Child second = demo.Child
    }

    public model System
    {
        container Root bus = demo.Root
    }
}
)dsl"}},
       R"dsl(assembly Demo

Root: demo.Root
{
    state.values[5] = 0i32
}
)dsl",
       {{R"dsl(contains)dsl",
         {R"dsl(Array index 5 is out of bounds for the array of size 4.)dsl"},
         false,
         {},
         false,
         0}}},
      {R"dsl(xsmpasb)dsl",
       R"dsl(rejects a negative array index in an assembly field path)dsl",
       {{R"dsl(xsmp.project)dsl",
         R"dsl(project 'Demo' using 'ECSS_SMP_2025'
source 'src'
)dsl"},
        {R"dsl(src/types.xsmpcat)dsl", R"dsl(catalogue DemoTypes

namespace demo
{
    public event FlagEvent extends Smp.Bool
    @SimpleArray
    public array SimpleIntQuad = Smp.Int32[4]

    public struct Counters
    {
        field Smp.Int32 count
        field Smp.Bool enabled
        field SimpleIntQuad values
    }

    public model Child
    {
        field Smp.Int32 count
        field Smp.Float64 ratio
        input field Smp.Int32 inValue
        output field Smp.Int32 outValue
        reference Smp.IComponent backLogger
        reference Smp.IService backService

        entrypoint step
        {
            in inValue
            out outValue
        }

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }

    public model Root
    {
        field Smp.Bool enabledState
        field Smp.Int32 countState
        field Counters state
        field Smp.Char8 grade
        field Smp.String8 label
        output field Smp.Int32 outValue
        input field Smp.Int32 inValue
        container Child child = demo.Child
        reference Smp.IComponent logger
        reference Smp.IService serviceRef

        public property Smp.Bool enabled -> enabledState
        readOnly property Smp.Bool status -> enabledState
        public property Smp.Int32 count -> countState
        public def void apply(in Smp.Int32 nextCount, in Smp.Float64 nextRatio)

        entrypoint step
        {
            in inValue
            out outValue
        }

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }

    public model RootWithoutDefault
    {
        container Child child
    }

    public model RootWithTwoDefaults
    {
        container Child first = demo.Child
        container Child second = demo.Child
    }

    public model System
    {
        container Root bus = demo.Root
    }
}
)dsl"}},
       R"dsl(assembly Demo

Root: demo.Root
{
    state.values[-1] = 0i32
}
)dsl",
       {{R"dsl(contains)dsl",
         {R"dsl(Array index shall not be negative.)dsl"},
         false,
         {},
         false,
         0}}},
      {R"dsl(xsmpcfg)dsl",
       R"dsl(keeps legacy text mode when no component is defined)dsl",
       {{R"dsl(xsmp.project)dsl",
         R"dsl(project 'Demo' using 'ECSS_SMP_2025'
source 'src'
)dsl"},
        {R"dsl(src/types.xsmpcat)dsl", R"dsl(catalogue DemoTypes

namespace demo
{
    public array IntPair = Smp.Int32[2]
    @SimpleArray
    public array SimpleIntQuad = Smp.Int32[4]
    public integer TinyCounter extends Smp.UInt8 in 1 ... 3
    public float SmallRatio extends Smp.Float32 in 0.0 ..< 2.0

    public struct Counters
    {
        field Smp.Int32 count
        field Smp.Bool enabled
        field IntPair values
    }

    public model Child
    {
        field Smp.Int32 value
    }

    public model Root
    {
        field Smp.UInt8 byteValue
        field TinyCounter tiny
        field SmallRatio smallRatio
        field SimpleIntQuad simpleValues
        field Counters state
        field Smp.Bool flag
        field Smp.Float64 ratio
        container Child child
    }
}
)dsl"},
        {R"dsl(src/assembly.xsmpasb)dsl",
         R"dsl(assembly <Lane = "Ops"> DemoAsm
Root: demo.Root
{
    child += Child{Lane}: demo.Child
}
)dsl"},
        {R"dsl(src/other.xsmpcfg)dsl",
         R"dsl(configuration Other /Other {})dsl"}},
       R"dsl(configuration Legacy
/Root
{
    flag = 1i32
    child
    {
        value = true
    }
}
)dsl",
       {{R"dsl(exact)dsl", {}, false, {}, false, 0}}},
      {R"dsl(xsmpcfg)dsl",
       R"dsl(allows unsuffixed numeric values when the safe resolved target type is known)dsl",
       {{R"dsl(xsmp.project)dsl",
         R"dsl(project 'Demo' using 'ECSS_SMP_2025'
source 'src'
)dsl"},
        {R"dsl(src/types.xsmpcat)dsl", R"dsl(catalogue DemoTypes

namespace demo
{
    public array IntPair = Smp.Int32[2]
    @SimpleArray
    public array SimpleIntQuad = Smp.Int32[4]
    public integer TinyCounter extends Smp.UInt8 in 1 ... 3
    public float SmallRatio extends Smp.Float32 in 0.0 ..< 2.0

    public struct Counters
    {
        field Smp.Int32 count
        field Smp.Bool enabled
        field IntPair values
    }

    public model Child
    {
        field Smp.Int32 value
    }

    public model Root
    {
        field Smp.UInt8 byteValue
        field TinyCounter tiny
        field SmallRatio smallRatio
        field SimpleIntQuad simpleValues
        field Counters state
        field Smp.Bool flag
        field Smp.Float64 ratio
        container Child child
    }
}
)dsl"},
        {R"dsl(src/assembly.xsmpasb)dsl",
         R"dsl(assembly <Lane = "Ops"> DemoAsm
Root: demo.Root
{
    child += Child{Lane}: demo.Child
}
)dsl"},
        {R"dsl(src/other.xsmpcfg)dsl",
         R"dsl(configuration Other /Other {})dsl"}},
       R"dsl(configuration Demo
/Root: demo.Root
{
    byteValue = 255
    tiny = 2
    smallRatio = 1.5f32
    ratio = 1.5
    state = {
        count = 1,
        values = [1, 2]
    }
}
)dsl",
       {{R"dsl(exact)dsl", {}, false, {}, false, 0}}},
      {R"dsl(xsmpcfg)dsl",
       R"dsl(rejects duplicate document names across document kinds)dsl",
       {{R"dsl(xsmp.project)dsl",
         R"dsl(project 'Demo' using 'ECSS_SMP_2025'
source 'src'
)dsl"},
        {R"dsl(src/types.xsmpcat)dsl", R"dsl(catalogue DemoTypes

namespace demo
{
    public array IntPair = Smp.Int32[2]
    @SimpleArray
    public array SimpleIntQuad = Smp.Int32[4]
    public integer TinyCounter extends Smp.UInt8 in 1 ... 3
    public float SmallRatio extends Smp.Float32 in 0.0 ..< 2.0

    public struct Counters
    {
        field Smp.Int32 count
        field Smp.Bool enabled
        field IntPair values
    }

    public model Child
    {
        field Smp.Int32 value
    }

    public model Root
    {
        field Smp.UInt8 byteValue
        field TinyCounter tiny
        field SmallRatio smallRatio
        field SimpleIntQuad simpleValues
        field Counters state
        field Smp.Bool flag
        field Smp.Float64 ratio
        container Child child
    }
}
)dsl"},
        {R"dsl(src/assembly.xsmpasb)dsl",
         R"dsl(assembly <Lane = "Ops"> DemoAsm
Root: demo.Root
{
    child += Child{Lane}: demo.Child
}
)dsl"},
        {R"dsl(src/other.xsmpcfg)dsl",
         R"dsl(configuration Other /Other {})dsl"}},
       R"dsl(configuration DemoAsm
/Root: demo.Root
{
}
)dsl",
       {{R"dsl(contains)dsl",
         {R"dsl(Duplicated Document name.)dsl"},
         false,
         {},
         false,
         0}}},
      {R"dsl(xsmpcfg)dsl",
       R"dsl(validates typed field paths and values and honors unsafe paths)dsl",
       {{R"dsl(xsmp.project)dsl",
         R"dsl(project 'Demo' using 'ECSS_SMP_2025'
source 'src'
)dsl"},
        {R"dsl(src/types.xsmpcat)dsl", R"dsl(catalogue DemoTypes

namespace demo
{
    public array IntPair = Smp.Int32[2]
    @SimpleArray
    public array SimpleIntQuad = Smp.Int32[4]
    public integer TinyCounter extends Smp.UInt8 in 1 ... 3
    public float SmallRatio extends Smp.Float32 in 0.0 ..< 2.0

    public struct Counters
    {
        field Smp.Int32 count
        field Smp.Bool enabled
        field IntPair values
    }

    public model Child
    {
        field Smp.Int32 value
    }

    public model Root
    {
        field Smp.UInt8 byteValue
        field TinyCounter tiny
        field SmallRatio smallRatio
        field SimpleIntQuad simpleValues
        field Counters state
        field Smp.Bool flag
        field Smp.Float64 ratio
        container Child child
    }
}
)dsl"},
        {R"dsl(src/assembly.xsmpasb)dsl",
         R"dsl(assembly <Lane = "Ops"> DemoAsm
Root: demo.Root
{
    child += Child{Lane}: demo.Child
}
)dsl"},
        {R"dsl(src/other.xsmpcfg)dsl",
         R"dsl(configuration Other /Other {})dsl"}},
       R"dsl(configuration Demo
/Root: demo.Root
{
    flag = 1i32
    state.unknown = 1i32
    state = {
        count = true,
        missing = 1i32,
        unsafe skipped = 1i32,
        values = [1i32, true]
    }
    flag
    {
        value = 1i32
    }
    include Other at flag
    include Other at unsafe flag
}
)dsl",
       {{R"dsl(contains)dsl",
         {R"dsl(The value shall be compatible with type Smp.Bool.)dsl",
          R"dsl(The path segment 'unknown' shall resolve to a Field of the current Structure value.)dsl",
          R"dsl(The structure field 'missing' does not exist on type demo.Counters.)dsl",
          R"dsl(The value shall be compatible with type Smp.Int32.)dsl",
          R"dsl(A safe Component Configuration inside a Component-backed context shall declare an explicit context.)dsl"},
         false,
         {},
         false,
         0},
        {R"dsl(none)dsl",
         {},
         false,
         {R"dsl(skipped)dsl"},
         true,
         0}}},
      {R"dsl(xsmpcfg)dsl",
       R"dsl(rejects unsuffixed numeric values outside safe resolved typing)dsl",
       {{R"dsl(xsmp.project)dsl",
         R"dsl(project 'Demo' using 'ECSS_SMP_2025'
source 'src'
)dsl"},
        {R"dsl(src/types.xsmpcat)dsl", R"dsl(catalogue DemoTypes

namespace demo
{
    public array IntPair = Smp.Int32[2]
    @SimpleArray
    public array SimpleIntQuad = Smp.Int32[4]
    public integer TinyCounter extends Smp.UInt8 in 1 ... 3
    public float SmallRatio extends Smp.Float32 in 0.0 ..< 2.0

    public struct Counters
    {
        field Smp.Int32 count
        field Smp.Bool enabled
        field IntPair values
    }

    public model Child
    {
        field Smp.Int32 value
    }

    public model Root
    {
        field Smp.UInt8 byteValue
        field TinyCounter tiny
        field SmallRatio smallRatio
        field SimpleIntQuad simpleValues
        field Counters state
        field Smp.Bool flag
        field Smp.Float64 ratio
        container Child child
    }
}
)dsl"},
        {R"dsl(src/assembly.xsmpasb)dsl",
         R"dsl(assembly <Lane = "Ops"> DemoAsm
Root: demo.Root
{
    child += Child{Lane}: demo.Child
}
)dsl"},
        {R"dsl(src/other.xsmpcfg)dsl",
         R"dsl(configuration Other /Other {})dsl"}},
       R"dsl(configuration Demo
/Root
{
    flag = 1
}
/Typed: demo.Root
{
    unsafe ratio = 1.5
    state = {
        unsafe count = 1
    }
}
)dsl",
       {{R"dsl(contains)dsl",
         {R"dsl(This value has no known type: write a suffix, as in 1i32.)dsl",
          R"dsl(This value has no known type: write a suffix, as in 1.0f64.)dsl"},
         false,
         {},
         false,
         0}}},
      {R"dsl(xsmpcfg)dsl",
       R"dsl(validates primitive and derived numeric ranges)dsl",
       {{R"dsl(xsmp.project)dsl",
         R"dsl(project 'Demo' using 'ECSS_SMP_2025'
source 'src'
)dsl"},
        {R"dsl(src/types.xsmpcat)dsl", R"dsl(catalogue DemoTypes

namespace demo
{
    public array IntPair = Smp.Int32[2]
    @SimpleArray
    public array SimpleIntQuad = Smp.Int32[4]
    public integer TinyCounter extends Smp.UInt8 in 1 ... 3
    public float SmallRatio extends Smp.Float32 in 0.0 ..< 2.0

    public struct Counters
    {
        field Smp.Int32 count
        field Smp.Bool enabled
        field IntPair values
    }

    public model Child
    {
        field Smp.Int32 value
    }

    public model Root
    {
        field Smp.UInt8 byteValue
        field TinyCounter tiny
        field SmallRatio smallRatio
        field SimpleIntQuad simpleValues
        field Counters state
        field Smp.Bool flag
        field Smp.Float64 ratio
        container Child child
    }
}
)dsl"},
        {R"dsl(src/assembly.xsmpasb)dsl",
         R"dsl(assembly <Lane = "Ops"> DemoAsm
Root: demo.Root
{
    child += Child{Lane}: demo.Child
}
)dsl"},
        {R"dsl(src/other.xsmpcfg)dsl",
         R"dsl(configuration Other /Other {})dsl"}},
       R"dsl(configuration Demo
/Root: demo.Root
{
    byteValue = 256
    tiny = 0
    smallRatio = 2.0f32
}
)dsl",
       {{R"dsl(contains)dsl",
         {R"dsl(Conversion overflow for type UInt8.)dsl",
          R"dsl(Integral value shall be greater than or equal to 1.)dsl",
          R"dsl(Float value shall be less than 2.)dsl"},
         false,
         {},
         false,
         0}}},
      {R"dsl(xsmpcfg)dsl",
       R"dsl(inherits the effective component in nested component configurations with an explicit context)dsl",
       {{R"dsl(xsmp.project)dsl",
         R"dsl(project 'Demo' using 'ECSS_SMP_2025'
source 'src'
)dsl"},
        {R"dsl(src/types.xsmpcat)dsl", R"dsl(catalogue DemoTypes

namespace demo
{
    public array IntPair = Smp.Int32[2]
    @SimpleArray
    public array SimpleIntQuad = Smp.Int32[4]
    public integer TinyCounter extends Smp.UInt8 in 1 ... 3
    public float SmallRatio extends Smp.Float32 in 0.0 ..< 2.0

    public struct Counters
    {
        field Smp.Int32 count
        field Smp.Bool enabled
        field IntPair values
    }

    public model Child
    {
        field Smp.Int32 value
    }

    public model Root
    {
        field Smp.UInt8 byteValue
        field TinyCounter tiny
        field SmallRatio smallRatio
        field SimpleIntQuad simpleValues
        field Counters state
        field Smp.Bool flag
        field Smp.Float64 ratio
        container Child child
    }
}
)dsl"},
        {R"dsl(src/assembly.xsmpasb)dsl",
         R"dsl(assembly <Lane = "Ops"> DemoAsm
Root: demo.Root
{
    child += Child{Lane}: demo.Child
}
)dsl"},
        {R"dsl(src/other.xsmpcfg)dsl",
         R"dsl(configuration Other /Other {})dsl"}},
       R"dsl(configuration Demo
/Root: demo.Root
{
    ShadowChild: demo.Child
    {
        value = 2
    }
}
)dsl",
       {{R"dsl(exact)dsl", {}, false, {}, false, 0}}},
      {R"dsl(xsmpcfg)dsl",
       R"dsl(allows explicit nested component contexts inside a component-backed configuration without resolving a container)dsl",
       {{R"dsl(xsmp.project)dsl",
         R"dsl(project 'Demo' using 'ECSS_SMP_2025'
source 'src'
)dsl"},
        {R"dsl(src/types.xsmpcat)dsl", R"dsl(catalogue DemoTypes

namespace demo
{
    public array IntPair = Smp.Int32[2]
    @SimpleArray
    public array SimpleIntQuad = Smp.Int32[4]
    public integer TinyCounter extends Smp.UInt8 in 1 ... 3
    public float SmallRatio extends Smp.Float32 in 0.0 ..< 2.0

    public struct Counters
    {
        field Smp.Int32 count
        field Smp.Bool enabled
        field IntPair values
    }

    public model Child
    {
        field Smp.Int32 value
    }

    public model Root
    {
        field Smp.UInt8 byteValue
        field TinyCounter tiny
        field SmallRatio smallRatio
        field SimpleIntQuad simpleValues
        field Counters state
        field Smp.Bool flag
        field Smp.Float64 ratio
        container Child child
    }
}
)dsl"},
        {R"dsl(src/assembly.xsmpasb)dsl",
         R"dsl(assembly <Lane = "Ops"> DemoAsm
Root: demo.Root
{
    child += Child{Lane}: demo.Child
}
)dsl"},
        {R"dsl(src/other.xsmpcfg)dsl",
         R"dsl(configuration Other /Other {})dsl"}},
       R"dsl(configuration Demo
/Root: demo.Root
{
    ShadowChild: demo.Child
    {
        value = 2i32
    }
}
)dsl",
       {{R"dsl(exact)dsl", {}, false, {}, false, 0}}},
      {R"dsl(xsmpcfg)dsl",
       R"dsl(rejects safe nested component configurations without an explicit context inside a component-backed configuration)dsl",
       {{R"dsl(xsmp.project)dsl",
         R"dsl(project 'Demo' using 'ECSS_SMP_2025'
source 'src'
)dsl"},
        {R"dsl(src/types.xsmpcat)dsl", R"dsl(catalogue DemoTypes

namespace demo
{
    public array IntPair = Smp.Int32[2]
    @SimpleArray
    public array SimpleIntQuad = Smp.Int32[4]
    public integer TinyCounter extends Smp.UInt8 in 1 ... 3
    public float SmallRatio extends Smp.Float32 in 0.0 ..< 2.0

    public struct Counters
    {
        field Smp.Int32 count
        field Smp.Bool enabled
        field IntPair values
    }

    public model Child
    {
        field Smp.Int32 value
    }

    public model Root
    {
        field Smp.UInt8 byteValue
        field TinyCounter tiny
        field SmallRatio smallRatio
        field SimpleIntQuad simpleValues
        field Counters state
        field Smp.Bool flag
        field Smp.Float64 ratio
        container Child child
    }
}
)dsl"},
        {R"dsl(src/assembly.xsmpasb)dsl",
         R"dsl(assembly <Lane = "Ops"> DemoAsm
Root: demo.Root
{
    child += Child{Lane}: demo.Child
}
)dsl"},
        {R"dsl(src/other.xsmpcfg)dsl",
         R"dsl(configuration Other /Other {})dsl"}},
       R"dsl(configuration Demo
/Root: demo.Root
{
    child
    {
        value = 2i32
    }
}
)dsl",
       {{R"dsl(exact)dsl",
         {R"dsl(A safe Component Configuration inside a Component-backed context shall declare an explicit context.)dsl"},
         false,
         {},
         false,
         0}}},
      {R"dsl(xsmpcfg)dsl",
       R"dsl(allows safe configuration usage paths inside a component-backed configuration)dsl",
       {{R"dsl(xsmp.project)dsl",
         R"dsl(project 'Demo' using 'ECSS_SMP_2025'
source 'src'
)dsl"},
        {R"dsl(src/types.xsmpcat)dsl", R"dsl(catalogue DemoTypes

namespace demo
{
    public array IntPair = Smp.Int32[2]
    @SimpleArray
    public array SimpleIntQuad = Smp.Int32[4]
    public integer TinyCounter extends Smp.UInt8 in 1 ... 3
    public float SmallRatio extends Smp.Float32 in 0.0 ..< 2.0

    public struct Counters
    {
        field Smp.Int32 count
        field Smp.Bool enabled
        field IntPair values
    }

    public model Child
    {
        field Smp.Int32 value
    }

    public model Root
    {
        field Smp.UInt8 byteValue
        field TinyCounter tiny
        field SmallRatio smallRatio
        field SimpleIntQuad simpleValues
        field Counters state
        field Smp.Bool flag
        field Smp.Float64 ratio
        container Child child
    }
}
)dsl"},
        {R"dsl(src/assembly.xsmpasb)dsl",
         R"dsl(assembly <Lane = "Ops"> DemoAsm
Root: demo.Root
{
    child += Child{Lane}: demo.Child
}
)dsl"},
        {R"dsl(src/other.xsmpcfg)dsl",
         R"dsl(configuration Other /Other {})dsl"}},
       R"dsl(configuration Demo
/Root: demo.Root
{
    include Other at child
}
)dsl",
       {{R"dsl(exact)dsl", {}, false, {}, false, 0}}},
      {R"dsl(xsmpcfg)dsl",
       R"dsl(accepts an assembly context and resolves inherited template parameters in paths)dsl",
       {{R"dsl(xsmp.project)dsl",
         R"dsl(project 'Demo' using 'ECSS_SMP_2025'
source 'src'
)dsl"},
        {R"dsl(src/types.xsmpcat)dsl", R"dsl(catalogue DemoTypes

namespace demo
{
    public array IntPair = Smp.Int32[2]
    @SimpleArray
    public array SimpleIntQuad = Smp.Int32[4]
    public integer TinyCounter extends Smp.UInt8 in 1 ... 3
    public float SmallRatio extends Smp.Float32 in 0.0 ..< 2.0

    public struct Counters
    {
        field Smp.Int32 count
        field Smp.Bool enabled
        field IntPair values
    }

    public model Child
    {
        field Smp.Int32 value
    }

    public model Root
    {
        field Smp.UInt8 byteValue
        field TinyCounter tiny
        field SmallRatio smallRatio
        field SimpleIntQuad simpleValues
        field Counters state
        field Smp.Bool flag
        field Smp.Float64 ratio
        container Child child
    }
}
)dsl"},
        {R"dsl(src/assembly.xsmpasb)dsl",
         R"dsl(assembly <Lane = "Ops"> DemoAsm
Root: demo.Root
{
    child += Child{Lane}: demo.Child
}
)dsl"},
        {R"dsl(src/other.xsmpcfg)dsl",
         R"dsl(configuration Other /Other {})dsl"}},
       R"dsl(configuration Demo
/Root: DemoAsm
{
    flag = true
    Child{Lane}
    {
        value = 2i32
    }
}
)dsl",
       {{R"dsl(exact)dsl", {}, false, {}, false, 0}}},
      {R"dsl(xsmpcfg)dsl",
       R"dsl(rejects explicit nested contexts inside an assembly-backed configuration when the path is safe)dsl",
       {{R"dsl(xsmp.project)dsl",
         R"dsl(project 'Demo' using 'ECSS_SMP_2025'
source 'src'
)dsl"},
        {R"dsl(src/types.xsmpcat)dsl", R"dsl(catalogue DemoTypes

namespace demo
{
    public array IntPair = Smp.Int32[2]
    @SimpleArray
    public array SimpleIntQuad = Smp.Int32[4]
    public integer TinyCounter extends Smp.UInt8 in 1 ... 3
    public float SmallRatio extends Smp.Float32 in 0.0 ..< 2.0

    public struct Counters
    {
        field Smp.Int32 count
        field Smp.Bool enabled
        field IntPair values
    }

    public model Child
    {
        field Smp.Int32 value
    }

    public model Root
    {
        field Smp.UInt8 byteValue
        field TinyCounter tiny
        field SmallRatio smallRatio
        field SimpleIntQuad simpleValues
        field Counters state
        field Smp.Bool flag
        field Smp.Float64 ratio
        container Child child
    }
}
)dsl"},
        {R"dsl(src/assembly.xsmpasb)dsl",
         R"dsl(assembly <Lane = "Ops"> DemoAsm
Root: demo.Root
{
    child += Child{Lane}: demo.Child
}
)dsl"},
        {R"dsl(src/other.xsmpcfg)dsl",
         R"dsl(configuration Other /Other {})dsl"}},
       R"dsl(configuration Demo
/Root: DemoAsm
{
    Child{Lane}: demo.Child
    {
        value = 2i32
    }
}
)dsl",
       {{R"dsl(exact)dsl",
         {R"dsl(A safe Component Configuration inside an Assembly-backed context shall not declare an explicit context.)dsl"},
         false,
         {},
         false,
         0}}},
      {R"dsl(xsmpcfg)dsl",
       R"dsl(allows explicit nested contexts inside an assembly-backed configuration when the path is unsafe)dsl",
       {{R"dsl(xsmp.project)dsl",
         R"dsl(project 'Demo' using 'ECSS_SMP_2025'
source 'src'
)dsl"},
        {R"dsl(src/types.xsmpcat)dsl", R"dsl(catalogue DemoTypes

namespace demo
{
    public array IntPair = Smp.Int32[2]
    @SimpleArray
    public array SimpleIntQuad = Smp.Int32[4]
    public integer TinyCounter extends Smp.UInt8 in 1 ... 3
    public float SmallRatio extends Smp.Float32 in 0.0 ..< 2.0

    public struct Counters
    {
        field Smp.Int32 count
        field Smp.Bool enabled
        field IntPair values
    }

    public model Child
    {
        field Smp.Int32 value
    }

    public model Root
    {
        field Smp.UInt8 byteValue
        field TinyCounter tiny
        field SmallRatio smallRatio
        field SimpleIntQuad simpleValues
        field Counters state
        field Smp.Bool flag
        field Smp.Float64 ratio
        container Child child
    }
}
)dsl"},
        {R"dsl(src/assembly.xsmpasb)dsl",
         R"dsl(assembly <Lane = "Ops"> DemoAsm
Root: demo.Root
{
    child += Child{Lane}: demo.Child
}
)dsl"},
        {R"dsl(src/other.xsmpcfg)dsl",
         R"dsl(configuration Other /Other {})dsl"}},
       R"dsl(configuration Demo
/Root: DemoAsm
{
    unsafe ShadowChild: demo.Child
    {
        value = 2i32
    }
}
)dsl",
       {{R"dsl(exact)dsl", {}, false, {}, false, 0}}},
      {R"dsl(xsmpcfg)dsl",
       R"dsl(warns when unsafe is useless on a nested component configuration with an explicit component context)dsl",
       {{R"dsl(xsmp.project)dsl",
         R"dsl(project 'Demo' using 'ECSS_SMP_2025'
source 'src'
)dsl"},
        {R"dsl(src/types.xsmpcat)dsl", R"dsl(catalogue DemoTypes

namespace demo
{
    public array IntPair = Smp.Int32[2]
    @SimpleArray
    public array SimpleIntQuad = Smp.Int32[4]
    public integer TinyCounter extends Smp.UInt8 in 1 ... 3
    public float SmallRatio extends Smp.Float32 in 0.0 ..< 2.0

    public struct Counters
    {
        field Smp.Int32 count
        field Smp.Bool enabled
        field IntPair values
    }

    public model Child
    {
        field Smp.Int32 value
    }

    public model Root
    {
        field Smp.UInt8 byteValue
        field TinyCounter tiny
        field SmallRatio smallRatio
        field SimpleIntQuad simpleValues
        field Counters state
        field Smp.Bool flag
        field Smp.Float64 ratio
        container Child child
    }
}
)dsl"},
        {R"dsl(src/assembly.xsmpasb)dsl",
         R"dsl(assembly <Lane = "Ops"> DemoAsm
Root: demo.Root
{
    child += Child{Lane}: demo.Child
}
)dsl"},
        {R"dsl(src/other.xsmpcfg)dsl",
         R"dsl(configuration Other /Other {})dsl"}},
       R"dsl(configuration Demo
/Root: demo.Root
{
    unsafe ShadowChild: demo.Child
    {
        value = 2i32
    }
}
)dsl",
       {{R"dsl(exact)dsl",
         {R"dsl(2:The `unsafe` modifier is unnecessary when a nested Component Configuration already declares an explicit Component context.)dsl"},
         true,
         {},
         false,
         0}}},
      {R"dsl(xsmpcfg)dsl",
       R"dsl(warns when unsafe is useless on a configuration usage inside a component-backed context)dsl",
       {{R"dsl(xsmp.project)dsl",
         R"dsl(project 'Demo' using 'ECSS_SMP_2025'
source 'src'
)dsl"},
        {R"dsl(src/types.xsmpcat)dsl", R"dsl(catalogue DemoTypes

namespace demo
{
    public array IntPair = Smp.Int32[2]
    @SimpleArray
    public array SimpleIntQuad = Smp.Int32[4]
    public integer TinyCounter extends Smp.UInt8 in 1 ... 3
    public float SmallRatio extends Smp.Float32 in 0.0 ..< 2.0

    public struct Counters
    {
        field Smp.Int32 count
        field Smp.Bool enabled
        field IntPair values
    }

    public model Child
    {
        field Smp.Int32 value
    }

    public model Root
    {
        field Smp.UInt8 byteValue
        field TinyCounter tiny
        field SmallRatio smallRatio
        field SimpleIntQuad simpleValues
        field Counters state
        field Smp.Bool flag
        field Smp.Float64 ratio
        container Child child
    }
}
)dsl"},
        {R"dsl(src/assembly.xsmpasb)dsl",
         R"dsl(assembly <Lane = "Ops"> DemoAsm
Root: demo.Root
{
    child += Child{Lane}: demo.Child
}
)dsl"},
        {R"dsl(src/other.xsmpcfg)dsl",
         R"dsl(configuration Other /Other {})dsl"}},
       R"dsl(configuration Demo
/Root: demo.Root
{
    include Other at unsafe child
}
)dsl",
       {{R"dsl(exact)dsl",
         {R"dsl(2:The `unsafe` modifier is unnecessary for a Configuration Usage inside a Component-backed context.)dsl"},
         true,
         {},
         false,
         0}}},
      {R"dsl(xsmpcfg)dsl",
       R"dsl(accepts StartIndex for simple arrays in ECSS_SMP_2025 and rejects unsupported cases)dsl",
       {{R"dsl(xsmp.project)dsl",
         R"dsl(project 'Demo' using 'ECSS_SMP_2025'
source 'src'
)dsl"},
        {R"dsl(src/types.xsmpcat)dsl", R"dsl(catalogue DemoTypes

namespace demo
{
    public array IntPair = Smp.Int32[2]
    @SimpleArray
    public array SimpleIntQuad = Smp.Int32[4]
    public integer TinyCounter extends Smp.UInt8 in 1 ... 3
    public float SmallRatio extends Smp.Float32 in 0.0 ..< 2.0

    public struct Counters
    {
        field Smp.Int32 count
        field Smp.Bool enabled
        field IntPair values
    }

    public model Child
    {
        field Smp.Int32 value
    }

    public model Root
    {
        field Smp.UInt8 byteValue
        field TinyCounter tiny
        field SmallRatio smallRatio
        field SimpleIntQuad simpleValues
        field Counters state
        field Smp.Bool flag
        field Smp.Float64 ratio
        container Child child
    }
}
)dsl"},
        {R"dsl(src/assembly.xsmpasb)dsl",
         R"dsl(assembly <Lane = "Ops"> DemoAsm
Root: demo.Root
{
    child += Child{Lane}: demo.Child
}
)dsl"},
        {R"dsl(src/other.xsmpcfg)dsl",
         R"dsl(configuration Other /Other {})dsl"}},
       R"dsl(configuration DemoStartIndexOk
/Root: demo.Root
{
    simpleValues = [1: 2, 3]
}
)dsl",
       {{R"dsl(exact)dsl", {}, false, {}, false, 0}}},
      {R"dsl(xsmpcfg)dsl",
       R"dsl(accepts StartIndex for simple arrays in ECSS_SMP_2025 and rejects unsupported cases)dsl",
       {{R"dsl(xsmp.project)dsl",
         R"dsl(project 'Demo' using 'ECSS_SMP_2025'
source 'src'
)dsl"},
        {R"dsl(src/types.xsmpcat)dsl", R"dsl(catalogue DemoTypes

namespace demo
{
    public array IntPair = Smp.Int32[2]
    @SimpleArray
    public array SimpleIntQuad = Smp.Int32[4]
    public integer TinyCounter extends Smp.UInt8 in 1 ... 3
    public float SmallRatio extends Smp.Float32 in 0.0 ..< 2.0

    public struct Counters
    {
        field Smp.Int32 count
        field Smp.Bool enabled
        field IntPair values
    }

    public model Child
    {
        field Smp.Int32 value
    }

    public model Root
    {
        field Smp.UInt8 byteValue
        field TinyCounter tiny
        field SmallRatio smallRatio
        field SimpleIntQuad simpleValues
        field Counters state
        field Smp.Bool flag
        field Smp.Float64 ratio
        container Child child
    }
}
)dsl"},
        {R"dsl(src/assembly.xsmpasb)dsl",
         R"dsl(assembly <Lane = "Ops"> DemoAsm
Root: demo.Root
{
    child += Child{Lane}: demo.Child
}
)dsl"},
        {R"dsl(src/other.xsmpcfg)dsl",
         R"dsl(configuration Other /Other {})dsl"}},
       R"dsl(configuration DemoStartIndexInvalid2025
/Root: demo.Root
{
    state = {
        values = [1: 2]
    }
}
)dsl",
       {{R"dsl(exact)dsl",
         {R"dsl(StartIndex is only allowed for SimpleArray values.)dsl"},
         false,
         {},
         false,
         0}}},
      {R"dsl(xsmpcfg)dsl",
       R"dsl(accepts StartIndex for simple arrays in ECSS_SMP_2025 and rejects unsupported cases)dsl",
       {{R"dsl(xsmp.project)dsl",
         R"dsl(project 'Demo' using 'ECSS_SMP_2020'
source 'src'
)dsl"},
        {R"dsl(src/types.xsmpcat)dsl", R"dsl(catalogue DemoTypes

namespace demo
{
    public array IntPair = Smp.Int32[2]
    @SimpleArray
    public array SimpleIntQuad = Smp.Int32[4]
    public integer TinyCounter extends Smp.UInt8 in 1 ... 3
    public float SmallRatio extends Smp.Float32 in 0.0 ..< 2.0

    public struct Counters
    {
        field Smp.Int32 count
        field Smp.Bool enabled
        field IntPair values
    }

    public model Child
    {
        field Smp.Int32 value
    }

    public model Root
    {
        field Smp.UInt8 byteValue
        field TinyCounter tiny
        field SmallRatio smallRatio
        field SimpleIntQuad simpleValues
        field Counters state
        field Smp.Bool flag
        field Smp.Float64 ratio
        container Child child
    }
}
)dsl"},
        {R"dsl(src/assembly.xsmpasb)dsl",
         R"dsl(assembly <Lane = "Ops"> DemoAsm
Root: demo.Root
{
    child += Child{Lane}: demo.Child
}
)dsl"},
        {R"dsl(src/other.xsmpcfg)dsl",
         R"dsl(configuration Other /Other {})dsl"}},
       R"dsl(configuration DemoStartIndexInvalid2020
/Root: demo.Root
{
    simpleValues = [1: 2, 3]
}
)dsl",
       {{R"dsl(exact)dsl",
         {R"dsl(StartIndex is only available in ECSS_SMP_2025.)dsl"},
         false,
         {},
         false,
         0}}},
      {R"dsl(xsmpcfg)dsl",
       R"dsl(infers types for positional structure and array values)dsl",
       {{R"dsl(xsmp.project)dsl",
         R"dsl(project 'Demo' using 'ECSS_SMP_2025'
source 'src'
)dsl"},
        {R"dsl(src/types.xsmpcat)dsl", R"dsl(catalogue DemoTypes

namespace demo
{
    public array IntPair = Smp.Int32[2]
    @SimpleArray
    public array SimpleIntQuad = Smp.Int32[4]
    public integer TinyCounter extends Smp.UInt8 in 1 ... 3
    public float SmallRatio extends Smp.Float32 in 0.0 ..< 2.0

    public struct Counters
    {
        field Smp.Int32 count
        field Smp.Bool enabled
        field IntPair values
    }

    public model Child
    {
        field Smp.Int32 value
    }

    public model Root
    {
        field Smp.UInt8 byteValue
        field TinyCounter tiny
        field SmallRatio smallRatio
        field SimpleIntQuad simpleValues
        field Counters state
        field Smp.Bool flag
        field Smp.Float64 ratio
        container Child child
    }
}
)dsl"},
        {R"dsl(src/assembly.xsmpasb)dsl",
         R"dsl(assembly <Lane = "Ops"> DemoAsm
Root: demo.Root
{
    child += Child{Lane}: demo.Child
}
)dsl"},
        {R"dsl(src/other.xsmpcfg)dsl",
         R"dsl(configuration Other /Other {})dsl"}},
       R"dsl(configuration Demo
/Root: demo.Root
{
    state = {
        1,
        true,
        [1, 2]
    }
}
)dsl",
       {{R"dsl(exact)dsl", {}, false, {}, false, 0}}},
      {R"dsl(xsmpcfg)dsl",
       R"dsl(reports array overflow, duplicate structure fields, and excess positional structure values)dsl",
       {{R"dsl(xsmp.project)dsl",
         R"dsl(project 'Demo' using 'ECSS_SMP_2025'
source 'src'
)dsl"},
        {R"dsl(src/types.xsmpcat)dsl", R"dsl(catalogue DemoTypes

namespace demo
{
    public array IntPair = Smp.Int32[2]
    @SimpleArray
    public array SimpleIntQuad = Smp.Int32[4]
    public integer TinyCounter extends Smp.UInt8 in 1 ... 3
    public float SmallRatio extends Smp.Float32 in 0.0 ..< 2.0

    public struct Counters
    {
        field Smp.Int32 count
        field Smp.Bool enabled
        field IntPair values
    }

    public model Child
    {
        field Smp.Int32 value
    }

    public model Root
    {
        field Smp.UInt8 byteValue
        field TinyCounter tiny
        field SmallRatio smallRatio
        field SimpleIntQuad simpleValues
        field Counters state
        field Smp.Bool flag
        field Smp.Float64 ratio
        container Child child
    }
}
)dsl"},
        {R"dsl(src/assembly.xsmpasb)dsl",
         R"dsl(assembly <Lane = "Ops"> DemoAsm
Root: demo.Root
{
    child += Child{Lane}: demo.Child
}
)dsl"},
        {R"dsl(src/other.xsmpcfg)dsl",
         R"dsl(configuration Other /Other {})dsl"}},
       R"dsl(configuration Demo
/Root: demo.Root
{
    simpleValues = [2: 7, 8, 9]
    state = {
        count = 1i32,
        count = 2i32,
        true,
        [1i32, 2i32],
        false
    }
}
)dsl",
       {{R"dsl(contains)dsl",
         {R"dsl(The array value shall not exceed 4 item(s) when StartIndex is applied.)dsl",
          R"dsl(The structure field 'count' shall not be initialized more than once.)dsl",
          R"dsl(The structure value shall not contain more values than the fields of demo.Counters.)dsl"},
         false,
         {},
         false,
         0}}},
      {R"dsl(xsmpcfg)dsl",
       R"dsl(rejects non-collection values for typed array and structure fields)dsl",
       {{R"dsl(xsmp.project)dsl",
         R"dsl(project 'Demo' using 'ECSS_SMP_2025'
source 'src'
)dsl"},
        {R"dsl(src/types.xsmpcat)dsl", R"dsl(catalogue DemoTypes

namespace demo
{
    public array IntPair = Smp.Int32[2]
    @SimpleArray
    public array SimpleIntQuad = Smp.Int32[4]
    public integer TinyCounter extends Smp.UInt8 in 1 ... 3
    public float SmallRatio extends Smp.Float32 in 0.0 ..< 2.0

    public struct Counters
    {
        field Smp.Int32 count
        field Smp.Bool enabled
        field IntPair values
    }

    public model Child
    {
        field Smp.Int32 value
    }

    public model Root
    {
        field Smp.UInt8 byteValue
        field TinyCounter tiny
        field SmallRatio smallRatio
        field SimpleIntQuad simpleValues
        field Counters state
        field Smp.Bool flag
        field Smp.Float64 ratio
        container Child child
    }
}
)dsl"},
        {R"dsl(src/assembly.xsmpasb)dsl",
         R"dsl(assembly <Lane = "Ops"> DemoAsm
Root: demo.Root
{
    child += Child{Lane}: demo.Child
}
)dsl"},
        {R"dsl(src/other.xsmpcfg)dsl",
         R"dsl(configuration Other /Other {})dsl"}},
       R"dsl(configuration Demo
/Root: demo.Root
{
    simpleValues = true
    state = false
}
)dsl",
       {{R"dsl(contains)dsl",
         {R"dsl(The value shall be compatible with type demo.SimpleIntQuad.)dsl", R"dsl(The value shall be compatible with type demo.Counters.)dsl"},
         false,
         {},
         false,
         0}}},
      {R"dsl(xsmpcfg)dsl",
       R"dsl(resolves include paths from an assembly context and reports missing placeholders)dsl",
       {{R"dsl(xsmp.project)dsl",
         R"dsl(project 'Demo' using 'ECSS_SMP_2025'
source 'src'
)dsl"},
        {R"dsl(src/types.xsmpcat)dsl", R"dsl(catalogue DemoTypes

namespace demo
{
    public array IntPair = Smp.Int32[2]
    @SimpleArray
    public array SimpleIntQuad = Smp.Int32[4]
    public integer TinyCounter extends Smp.UInt8 in 1 ... 3
    public float SmallRatio extends Smp.Float32 in 0.0 ..< 2.0

    public struct Counters
    {
        field Smp.Int32 count
        field Smp.Bool enabled
        field IntPair values
    }

    public model Child
    {
        field Smp.Int32 value
    }

    public model Root
    {
        field Smp.UInt8 byteValue
        field TinyCounter tiny
        field SmallRatio smallRatio
        field SimpleIntQuad simpleValues
        field Counters state
        field Smp.Bool flag
        field Smp.Float64 ratio
        container Child child
    }
}
)dsl"},
        {R"dsl(src/assembly.xsmpasb)dsl",
         R"dsl(assembly <Lane = "Ops"> DemoAsm
Root: demo.Root
{
    child += Child{Lane}: demo.Child
}
)dsl"},
        {R"dsl(src/other.xsmpcfg)dsl",
         R"dsl(configuration Other /Other {})dsl"}},
       R"dsl(configuration DemoIncludeOk
/Root: DemoAsm
{
    include Other at Child{Lane}
}
)dsl",
       {{R"dsl(exact)dsl", {}, false, {}, false, 0}}},
      {R"dsl(xsmpcfg)dsl",
       R"dsl(resolves include paths from an assembly context and reports missing placeholders)dsl",
       {{R"dsl(xsmp.project)dsl",
         R"dsl(project 'Demo' using 'ECSS_SMP_2025'
source 'src'
)dsl"},
        {R"dsl(src/types.xsmpcat)dsl", R"dsl(catalogue DemoTypes

namespace demo
{
    public array IntPair = Smp.Int32[2]
    @SimpleArray
    public array SimpleIntQuad = Smp.Int32[4]
    public integer TinyCounter extends Smp.UInt8 in 1 ... 3
    public float SmallRatio extends Smp.Float32 in 0.0 ..< 2.0

    public struct Counters
    {
        field Smp.Int32 count
        field Smp.Bool enabled
        field IntPair values
    }

    public model Child
    {
        field Smp.Int32 value
    }

    public model Root
    {
        field Smp.UInt8 byteValue
        field TinyCounter tiny
        field SmallRatio smallRatio
        field SimpleIntQuad simpleValues
        field Counters state
        field Smp.Bool flag
        field Smp.Float64 ratio
        container Child child
    }
}
)dsl"},
        {R"dsl(src/assembly.xsmpasb)dsl",
         R"dsl(assembly <Lane = "Ops"> DemoAsm
Root: demo.Root
{
    child += Child{Lane}: demo.Child
}
)dsl"},
        {R"dsl(src/other.xsmpcfg)dsl",
         R"dsl(configuration Other /Other {})dsl"}},
       R"dsl(configuration DemoIncludeInvalid
/Root: DemoAsm
{
    include Other at Child{Missing}
}
)dsl",
       {{R"dsl(exact)dsl",
         {R"dsl(The placeholder '{Missing}' shall resolve to a Template Argument of the enclosing Assembly context.)dsl"},
         false,
         {},
         false,
         0}}},
      {R"dsl(xsmplnk)dsl",
       R"dsl(validates anchored component link bases and link endpoints by instance name and honors unsafe)dsl",
       {{R"dsl(xsmp.project)dsl",
         R"dsl(project 'Demo' using 'ECSS_SMP_2025'
source 'src'
)dsl"},
        {R"dsl(src/types.xsmpcat)dsl", R"dsl(catalogue DemoTypes

namespace demo
{
    public event FlagEvent extends Smp.Bool

    public model Child
    {
        input field Smp.Int32 inValue
        output field Smp.Int32 outValue

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
        reference Smp.IComponent backLogger
    }

    public model Root
    {
        field Smp.Bool enabledState
        output field Smp.Int32 outValue
        input field Smp.Int32 inValue
        container Child child = demo.Child
        reference Smp.IComponent logger

        public property Smp.Bool enabled -> enabledState

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }
}
)dsl"},
        {R"dsl(src/assembly.xsmpasb)dsl", R"dsl(assembly DemoAsm

Root: demo.Root
{
    child += Leaf: demo.Child
}
)dsl"}},
       R"dsl(link Demo for DemoAsm

/
{
    field link outValue -> Leaf.outValue
    field link unsafe outValue -> unsafe Leaf.outValue
    event link outbound -> Leaf.inbound

    enabled
    {
        event link outbound -> inbound
    }
}
)dsl",
       {{R"dsl(contains)dsl",
         {R"dsl(The path segment 'outValue' shall resolve to a Field marked as Input of the current Component.)dsl",
          R"dsl(The path segment 'enabled' shall resolve to a child Model Instance or Assembly Instance.)dsl"},
         false,
         {},
         false,
         0},
        {R"dsl(none)dsl",
         {},
         false,
         {R"dsl(unsafe)dsl"},
         true,
         0}}},
      {R"dsl(xsmplnk)dsl",
       R"dsl(rejects component type names when an anchored assembly instance has a different name)dsl",
       {{R"dsl(xsmp.project)dsl",
         R"dsl(project 'Demo' using 'ECSS_SMP_2025'
source 'src'
)dsl"},
        {R"dsl(src/types.xsmpcat)dsl", R"dsl(catalogue DemoTypes

namespace demo
{
    public event FlagEvent extends Smp.Bool

    public model Child
    {
        input field Smp.Int32 inValue
        output field Smp.Int32 outValue

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
        reference Smp.IComponent backLogger
    }

    public model Root
    {
        field Smp.Bool enabledState
        output field Smp.Int32 outValue
        input field Smp.Int32 inValue
        container Child child = demo.Child
        reference Smp.IComponent logger

        public property Smp.Bool enabled -> enabledState

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }
}
)dsl"},
        {R"dsl(src/assembly.xsmpasb)dsl", R"dsl(assembly DemoAsm

Root: demo.Root
{
    child += Leaf: demo.Child
}
)dsl"}},
       R"dsl(link Demo for DemoAsm

/
{
    event link outbound -> Child.inbound
}
)dsl",
       {{R"dsl(exact)dsl",
         {R"dsl(The path segment 'Child' shall resolve to a child Model Instance or Assembly Instance.)dsl"},
         false,
         {},
         false,
         0}}},
      {R"dsl(xsmplnk)dsl",
       R"dsl(resolves absolute nested link paths from the assembly root)dsl",
       {{R"dsl(xsmp.project)dsl",
         R"dsl(project 'Demo' using 'ECSS_SMP_2025'
source 'src'
)dsl"},
        {R"dsl(src/types.xsmpcat)dsl", R"dsl(catalogue DemoTypes

namespace demo
{
    public event FlagEvent extends Smp.Bool

    public model Child
    {
        input field Smp.Int32 inValue
        output field Smp.Int32 outValue

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
        reference Smp.IComponent backLogger
    }

    public model Root
    {
        field Smp.Bool enabledState
        output field Smp.Int32 outValue
        input field Smp.Int32 inValue
        container Child child = demo.Child
        reference Smp.IComponent logger

        public property Smp.Bool enabled -> enabledState

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }
}
)dsl"},
        {R"dsl(src/assembly.xsmpasb)dsl", R"dsl(assembly DemoAsm

Root: demo.Root
{
    child += Leaf: demo.Child
}
)dsl"}},
       R"dsl(link Demo for DemoAsm

/
{
    event link outbound -> inbound

    Leaf
    {
        field link /outValue -> inValue
        interface link /logger -> .
    }
}
)dsl",
       {{R"dsl(exact)dsl", {}, false, {}, false, 0}}},
      {R"dsl(xsmplnk)dsl",
       R"dsl(keeps unanchored link bases in text mode)dsl",
       {{R"dsl(xsmp.project)dsl",
         R"dsl(project 'Demo' using 'ECSS_SMP_2025'
source 'src'
)dsl"},
        {R"dsl(src/types.xsmpcat)dsl", R"dsl(catalogue DemoTypes

namespace demo
{
    public event FlagEvent extends Smp.Bool

    public model Child
    {
        input field Smp.Int32 inValue
        output field Smp.Int32 outValue

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
        reference Smp.IComponent backLogger
    }

    public model Root
    {
        field Smp.Bool enabledState
        output field Smp.Int32 outValue
        input field Smp.Int32 inValue
        container Child child = demo.Child
        reference Smp.IComponent logger

        public property Smp.Bool enabled -> enabledState

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }
}
)dsl"}},
       R"dsl(link Demo

Unanchored
{
    field link missing -> missing
}
)dsl",
       {{R"dsl(exact)dsl", {}, false, {}, false, 0}}},
      {R"dsl(xsmplnk)dsl",
       R"dsl(requires an assembly anchor for templated link base paths)dsl",
       {{R"dsl(xsmp.project)dsl",
         R"dsl(project 'Demo' using 'ECSS_SMP_2025'
source 'src'
)dsl"},
        {R"dsl(src/types.xsmpcat)dsl", R"dsl(catalogue DemoTypes

namespace demo
{
    public event FlagEvent extends Smp.Bool

    public model Child
    {
        input field Smp.Int32 inValue
        output field Smp.Int32 outValue

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
        reference Smp.IComponent backLogger
    }

    public model Root
    {
        field Smp.Bool enabledState
        output field Smp.Int32 outValue
        input field Smp.Int32 inValue
        container Child child = demo.Child
        reference Smp.IComponent logger

        public property Smp.Bool enabled -> enabledState

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }
}
)dsl"}},
       R"dsl(link Demo

/
{
    field link outValue -> {Target}.inValue
}
)dsl",
       {{R"dsl(exact)dsl",
         {R"dsl(A Link Base using templated paths shall declare an Assembly anchor with 'for <Assembly>'.)dsl"},
         false,
         {},
         false,
         0}}},
      {R"dsl(xsmplnk)dsl",
       R"dsl(rejects duplicate document names across document kinds)dsl",
       {{R"dsl(xsmp.project)dsl",
         R"dsl(project 'Demo' using 'ECSS_SMP_2025'
source 'src'
)dsl"},
        {R"dsl(src/types.xsmpcat)dsl", R"dsl(catalogue DemoTypes

namespace demo
{
    public event FlagEvent extends Smp.Bool

    public model Child
    {
        input field Smp.Int32 inValue
        output field Smp.Int32 outValue

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
        reference Smp.IComponent backLogger
    }

    public model Root
    {
        field Smp.Bool enabledState
        output field Smp.Int32 outValue
        input field Smp.Int32 inValue
        container Child child = demo.Child
        reference Smp.IComponent logger

        public property Smp.Bool enabled -> enabledState

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }
}
)dsl"},
        {R"dsl(src/assembly.xsmpasb)dsl", R"dsl(assembly DemoAsm

Root: demo.Root
{
    child += Leaf: demo.Child
}
)dsl"}},
       R"dsl(link DemoAsm for DemoAsm

/
{
    field link outValue -> Leaf.inValue
}
)dsl",
       {{R"dsl(contains)dsl",
         {R"dsl(Duplicated Document name.)dsl"},
         false,
         {},
         false,
         0}}},
      {R"dsl(xsmplnk)dsl",
       R"dsl(resolves templated link paths with imported assembly defaults)dsl",
       {{R"dsl(xsmp.project)dsl",
         R"dsl(project 'Demo' using 'ECSS_SMP_2025'
source 'src'
)dsl"},
        {R"dsl(src/types.xsmpcat)dsl", R"dsl(catalogue DemoTypes

namespace demo
{
    public event FlagEvent extends Smp.Bool

    public model Child
    {
        input field Smp.Int32 inValue
        output field Smp.Int32 outValue

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
        reference Smp.IComponent backLogger
    }

    public model Root
    {
        field Smp.Bool enabledState
        output field Smp.Int32 outValue
        input field Smp.Int32 inValue
        container Child child = demo.Child
        reference Smp.IComponent logger

        public property Smp.Bool enabled -> enabledState

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }
}
)dsl"},
        {R"dsl(src/assembly.xsmpasb)dsl",
         R"dsl(assembly <Target = "Leaf"> DemoAsm

Root: demo.Root
{
    child += Leaf: demo.Child
}
)dsl"}},
       R"dsl(link Demo for DemoAsm

/
{
    field link outValue -> {Target}.inValue
}
)dsl",
       {{R"dsl(exact)dsl", {}, false, {}, false, 0}}},
      {R"dsl(xsmplnk)dsl",
       R"dsl(resolves templated link paths without spaces inside instance names)dsl",
       {{R"dsl(xsmp.project)dsl",
         R"dsl(project 'Demo' using 'ECSS_SMP_2025'
source 'src'
)dsl"},
        {R"dsl(src/types.xsmpcat)dsl", R"dsl(catalogue DemoTypes

namespace demo
{
    public event FlagEvent extends Smp.Bool

    public model Child
    {
        input field Smp.Int32 inValue
        output field Smp.Int32 outValue

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
        reference Smp.IComponent backLogger
    }

    public model Root
    {
        field Smp.Bool enabledState
        output field Smp.Int32 outValue
        input field Smp.Int32 inValue
        container Child child = demo.Child
        reference Smp.IComponent logger

        public property Smp.Bool enabled -> enabledState

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }
}
)dsl"},
        {R"dsl(src/assembly.xsmpasb)dsl",
         R"dsl(assembly <Lane = "Ops"> DemoAsm

Root: demo.Root
{
    child += Leaf{Lane}: demo.Child
}
)dsl"}},
       R"dsl(link Demo for DemoAsm

/
{
    field link outValue -> Leaf{Lane}.inValue
}
)dsl",
       {{R"dsl(exact)dsl", {}, false, {}, false, 0}}},
      {R"dsl(xsmplnk)dsl",
       R"dsl(reports unknown placeholders from the anchored assembly)dsl",
       {{R"dsl(xsmp.project)dsl",
         R"dsl(project 'Demo' using 'ECSS_SMP_2025'
source 'src'
)dsl"},
        {R"dsl(src/types.xsmpcat)dsl", R"dsl(catalogue DemoTypes

namespace demo
{
    public event FlagEvent extends Smp.Bool

    public model Child
    {
        input field Smp.Int32 inValue
        output field Smp.Int32 outValue

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
        reference Smp.IComponent backLogger
    }

    public model Root
    {
        field Smp.Bool enabledState
        output field Smp.Int32 outValue
        input field Smp.Int32 inValue
        container Child child = demo.Child
        reference Smp.IComponent logger

        public property Smp.Bool enabled -> enabledState

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }
}
)dsl"},
        {R"dsl(src/assembly.xsmpasb)dsl",
         R"dsl(assembly <Target = "child"> DemoAsm

Root: demo.Root
{
    child += Leaf: demo.Child
}
)dsl"}},
       R"dsl(link Demo for DemoAsm

/
{
    field link outValue -> {Missing}.inValue
}
)dsl",
       {{R"dsl(exact)dsl",
         {R"dsl(The placeholder '{Missing}' shall resolve to a Template Argument of the anchored Assembly.)dsl"},
         false,
         {},
         false,
         0}}},
      {R"dsl(xsmplnk)dsl",
       R"dsl(reports when a reference upper bound is exceeded in a link base)dsl",
       {{R"dsl(xsmp.project)dsl",
         R"dsl(project 'Demo' using 'ECSS_SMP_2025'
source 'src'
)dsl"},
        {R"dsl(src/types.xsmpcat)dsl", R"dsl(catalogue DemoTypes

namespace demo
{
    public event FlagEvent extends Smp.Bool

    public model Child
    {
        input field Smp.Int32 inValue
        output field Smp.Int32 outValue

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
        reference Smp.IComponent backLogger
    }

    public model Root
    {
        field Smp.Bool enabledState
        output field Smp.Int32 outValue
        input field Smp.Int32 inValue
        container Child child = demo.Child
        reference Smp.IComponent logger

        public property Smp.Bool enabled -> enabledState

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }
}
)dsl"},
        {R"dsl(src/assembly.xsmpasb)dsl", R"dsl(assembly DemoAsm

Root: demo.Root
{
    child += Leaf: demo.Child
}
)dsl"}},
       R"dsl(link Demo for DemoAsm

/
{
    interface link logger -> Leaf:backLogger
    interface link logger -> .
}
)dsl",
       {{R"dsl(exact)dsl",
         {R"dsl(The Reference 'logger' of component path '/' shall not be connected more than 1 time(s).)dsl",
          R"dsl(The Reference 'logger' of component path '/' shall not be connected more than 1 time(s).)dsl"},
         false,
         {},
         false,
         0}}},
      {R"dsl(xsmplnk)dsl",
       R"dsl(reports when an owner-prefixed source reference upper bound is exceeded in a link base)dsl",
       {{R"dsl(xsmp.project)dsl",
         R"dsl(project 'Demo' using 'ECSS_SMP_2025'
source 'src'
)dsl"},
        {R"dsl(src/types.xsmpcat)dsl", R"dsl(catalogue DemoTypes

namespace demo
{
    public event FlagEvent extends Smp.Bool

    public model Child
    {
        input field Smp.Int32 inValue
        output field Smp.Int32 outValue

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
        reference Smp.IComponent backLogger
    }

    public model Root
    {
        field Smp.Bool enabledState
        output field Smp.Int32 outValue
        input field Smp.Int32 inValue
        container Child child = demo.Child
        reference Smp.IComponent logger

        public property Smp.Bool enabled -> enabledState

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }
}
)dsl"},
        {R"dsl(src/assembly.xsmpasb)dsl", R"dsl(assembly DemoAsm

Root: demo.Root
{
    child += Leaf: demo.Child
}
)dsl"}},
       R"dsl(link Demo for DemoAsm

/
{
    interface link Leaf.backLogger -> .
    interface link Leaf.backLogger -> Leaf
}
)dsl",
       {{R"dsl(count)dsl",
         {},
         false,
         {R"dsl('backLogger')dsl",
          R"dsl(shall not be connected more than 1 time(s).)dsl"},
         true,
         2}}},
      {R"dsl(xsmplnk)dsl",
       R"dsl(rejects an event link owner path that selects no member)dsl",
       {{R"dsl(xsmp.project)dsl",
         R"dsl(project 'Demo' using 'ECSS_SMP_2025'
source 'src'
)dsl"},
        {R"dsl(src/types.xsmpcat)dsl", R"dsl(catalogue DemoTypes

namespace demo
{
    public event FlagEvent extends Smp.Bool

    public model Child
    {
        input field Smp.Int32 inValue
        output field Smp.Int32 outValue

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
        reference Smp.IComponent backLogger
    }

    public model Root
    {
        field Smp.Bool enabledState
        output field Smp.Int32 outValue
        input field Smp.Int32 inValue
        container Child child = demo.Child
        reference Smp.IComponent logger

        public property Smp.Bool enabled -> enabledState

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }
}
)dsl"},
        {R"dsl(src/assembly.xsmpasb)dsl", R"dsl(assembly DemoAsm

Root: demo.Root
{
    child += Leaf: demo.Child
}
)dsl"}},
       R"dsl(link Demo for DemoAsm

/
{
    event link . -> Leaf.inbound
}
)dsl",
       {{R"dsl(contains)dsl",
         {R"dsl(The path shall resolve to a supported member of the current Component.)dsl"},
         false,
         {},
         false,
         0}}},
      {R"dsl(xsmplnk)dsl",
       R"dsl(rejects parent traversal in component, owner, client, and source link paths)dsl",
       {{R"dsl(xsmp.project)dsl",
         R"dsl(project 'Demo' using 'ECSS_SMP_2025'
source 'src'
)dsl"},
        {R"dsl(src/types.xsmpcat)dsl", R"dsl(catalogue DemoTypes

namespace demo
{
    public event FlagEvent extends Smp.Bool

    public model Child
    {
        input field Smp.Int32 inValue
        output field Smp.Int32 outValue

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
        reference Smp.IComponent backLogger
    }

    public model Root
    {
        field Smp.Bool enabledState
        output field Smp.Int32 outValue
        input field Smp.Int32 inValue
        container Child child = demo.Child
        reference Smp.IComponent logger

        public property Smp.Bool enabled -> enabledState

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }
}
)dsl"},
        {R"dsl(src/assembly.xsmpasb)dsl", R"dsl(assembly DemoAsm

Root: demo.Root
{
    child += Leaf: demo.Child
}
)dsl"}},
       R"dsl(link Demo for DemoAsm

../child
{
    field link ../outValue -> child/../inValue
    interface link ../logger -> ../child:backLogger
}
)dsl",
       {{R"dsl(count)dsl",
         {},
         false,
         {R"dsl(Paths shall not contain '..'.)dsl"},
         false,
         5}}},
      {R"dsl(xsmplnk)dsl",
       R"dsl(requires at least one component link base)dsl",
       {{R"dsl(xsmp.project)dsl",
         R"dsl(project 'Demo' using 'ECSS_SMP_2025'
source 'src'
)dsl"},
        {R"dsl(src/types.xsmpcat)dsl", R"dsl(catalogue DemoTypes

namespace demo
{
    public event FlagEvent extends Smp.Bool

    public model Child
    {
        input field Smp.Int32 inValue
        output field Smp.Int32 outValue

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
        reference Smp.IComponent backLogger
    }

    public model Root
    {
        field Smp.Bool enabledState
        output field Smp.Int32 outValue
        input field Smp.Int32 inValue
        container Child child = demo.Child
        reference Smp.IComponent logger

        public property Smp.Bool enabled -> enabledState

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }
}
)dsl"},
        {R"dsl(src/assembly.xsmpasb)dsl", R"dsl(assembly DemoAsm

Root: demo.Root
{
    child += Leaf: demo.Child
}
)dsl"}},
       R"dsl(link Demo for DemoAsm
)dsl",
       {{R"dsl(exact)dsl",
         {R"dsl(A Link Base shall contain at least one Component Link Base.)dsl"},
         false,
         {},
         false,
         0}}},
      {R"dsl(xsmplnk)dsl",
       R"dsl(requires anchored instance paths and at least one link per component link base)dsl",
       {{R"dsl(xsmp.project)dsl",
         R"dsl(project 'Demo' using 'ECSS_SMP_2025'
source 'src'
)dsl"},
        {R"dsl(src/types.xsmpcat)dsl", R"dsl(catalogue DemoTypes

namespace demo
{
    public event FlagEvent extends Smp.Bool

    public model Child
    {
        input field Smp.Int32 inValue
        output field Smp.Int32 outValue

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
        reference Smp.IComponent backLogger
    }

    public model Root
    {
        field Smp.Bool enabledState
        output field Smp.Int32 outValue
        input field Smp.Int32 inValue
        container Child child = demo.Child
        reference Smp.IComponent logger

        public property Smp.Bool enabled -> enabledState

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }
}
)dsl"},
        {R"dsl(src/assembly.xsmpasb)dsl", R"dsl(assembly DemoAsm

Root: demo.Root
{
    child += Leaf: demo.Child
}
)dsl"}},
       R"dsl(link Demo for DemoAsm

logger
{
}
)dsl",
       {{R"dsl(contains)dsl",
         {R"dsl(The path segment 'logger' shall resolve to a child Model Instance or Assembly Instance.)dsl",
          R"dsl(A Component Link Base shall contain at least one Link.)dsl"},
         false,
         {},
         false,
         0}}},
      {R"dsl(xsmpsed)dsl",
       R"dsl(validates typed schedule paths, execute compatibility and honors unsafe)dsl",
       {{R"dsl(xsmp.project)dsl",
         R"dsl(project 'Demo' using 'ECSS_SMP_2025'
source 'src'
)dsl"},
        {R"dsl(src/types.xsmpcat)dsl", R"dsl(catalogue DemoTypes

namespace demo
{
    public event FlagEvent extends Smp.Bool

    public model Child
    {
        input field Smp.Int32 inValue
        output field Smp.Int32 outValue

        /** child reset */
        public def void reset()

        /** child trigger */
        entrypoint step
        {
            in inValue
            out outValue
        }

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }

    public model SpecializedChild extends Child
    {
        input field Smp.Int32 implIn
    }

    public model Root
    {
        field Smp.Bool enabledState
        field Smp.Int32 countState
        output field Smp.Int32 outValue
        input field Smp.Int32 inValue
        container Child child = demo.Child

        /** root enabled */
        public property Smp.Bool enabled -> enabledState
        /** root count */
        public property Smp.Int32 count -> countState

        /** root reset */
        public def void reset()
        /** root apply */
        public def void apply(in Smp.Int32 nextCount, in Smp.Float64 nextRatio)

        /** root trigger */
        entrypoint step
        {
            in inValue
            out outValue
        }

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }

    public model RootWithDefault
    {
        output field Smp.Int32 outValue
        container Child child = demo.SpecializedChild
    }
}
)dsl"}},
       R"dsl(schedule Demo

task Main on demo.Root
{
    call enabled()
    call unsafe enabled()
    property child = true
    transfer outValue -> Child.outValue
    trig missing
    execute Worker at /
}

task Worker on demo.Child
{
    call reset()
}

event Main mission "PT1S"
)dsl",
       {{R"dsl(contains)dsl",
         {R"dsl(The path segment 'enabled' shall resolve to a supported member of the current Component.)dsl",
          R"dsl(The path segment 'child' shall resolve to a supported member of the current Component.)dsl",
          R"dsl(The path segment 'Child' shall resolve to a Field marked as Input of the current Component.)dsl",
          R"dsl(The path segment 'missing' shall resolve to a supported member of the current Component.)dsl",
          R"dsl(The root path shall resolve to a Component compatible with the execution context of task Worker.)dsl"},
         false,
         {},
         false,
         0},
        {R"dsl(none)dsl",
         {},
         false,
         {R"dsl(unsafe)dsl"},
         true,
         0}}},
      {R"dsl(xsmpsed)dsl",
       R"dsl(does not treat container types or default components as child instances in typed schedule paths)dsl",
       {{R"dsl(xsmp.project)dsl",
         R"dsl(project 'Demo' using 'ECSS_SMP_2025'
source 'src'
)dsl"},
        {R"dsl(src/types.xsmpcat)dsl", R"dsl(catalogue DemoTypes

namespace demo
{
    public event FlagEvent extends Smp.Bool

    public model Child
    {
        input field Smp.Int32 inValue
        output field Smp.Int32 outValue

        /** child reset */
        public def void reset()

        /** child trigger */
        entrypoint step
        {
            in inValue
            out outValue
        }

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }

    public model SpecializedChild extends Child
    {
        input field Smp.Int32 implIn
    }

    public model Root
    {
        field Smp.Bool enabledState
        field Smp.Int32 countState
        output field Smp.Int32 outValue
        input field Smp.Int32 inValue
        container Child child = demo.Child

        /** root enabled */
        public property Smp.Bool enabled -> enabledState
        /** root count */
        public property Smp.Int32 count -> countState

        /** root reset */
        public def void reset()
        /** root apply */
        public def void apply(in Smp.Int32 nextCount, in Smp.Float64 nextRatio)

        /** root trigger */
        entrypoint step
        {
            in inValue
            out outValue
        }

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }

    public model RootWithDefault
    {
        output field Smp.Int32 outValue
        container Child child = demo.SpecializedChild
    }
}
)dsl"}},
       R"dsl(schedule <Root = "root"> Demo

task Main on demo.RootWithDefault
{
    transfer outValue -> Child.inValue
    transfer outValue -> SpecializedChild.implIn
}
)dsl",
       {{R"dsl(contains)dsl",
         {R"dsl(The path segment 'Child' shall resolve to a Field marked as Input of the current Component.)dsl",
          R"dsl(The path segment 'SpecializedChild' shall resolve to a Field marked as Input of the current Component.)dsl"},
         false,
         {},
         false,
         0}}},
      {R"dsl(xsmpsed)dsl",
       R"dsl(resolves templated schedule paths with schedule defaults)dsl",
       {{R"dsl(xsmp.project)dsl",
         R"dsl(project 'Demo' using 'ECSS_SMP_2025'
source 'src'
)dsl"},
        {R"dsl(src/types.xsmpcat)dsl", R"dsl(catalogue DemoTypes

namespace demo
{
    public event FlagEvent extends Smp.Bool

    public model Child
    {
        input field Smp.Int32 inValue
        output field Smp.Int32 outValue

        /** child reset */
        public def void reset()

        /** child trigger */
        entrypoint step
        {
            in inValue
            out outValue
        }

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }

    public model SpecializedChild extends Child
    {
        input field Smp.Int32 implIn
    }

    public model Root
    {
        field Smp.Bool enabledState
        field Smp.Int32 countState
        output field Smp.Int32 outValue
        input field Smp.Int32 inValue
        container Child child = demo.Child

        /** root enabled */
        public property Smp.Bool enabled -> enabledState
        /** root count */
        public property Smp.Int32 count -> countState

        /** root reset */
        public def void reset()
        /** root apply */
        public def void apply(in Smp.Int32 nextCount, in Smp.Float64 nextRatio)

        /** root trigger */
        entrypoint step
        {
            in inValue
            out outValue
        }

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }

    public model RootWithDefault
    {
        output field Smp.Int32 outValue
        container Child child = demo.SpecializedChild
    }
}
)dsl"},
        {R"dsl(src/assembly.xsmpasb)dsl",
         R"dsl(assembly <Lane = "Ops"> DemoAsm

Root: demo.Root
{
    child += Child{Lane}: demo.Child
}
)dsl"}},
       R"dsl(schedule <Target = "ChildOps"> Demo

task Main on DemoAsm
{
    call {Target}.reset()
}
)dsl",
       {{R"dsl(exact)dsl", {}, false, {}, false, 0}}},
      {R"dsl(xsmpsed)dsl",
       R"dsl(accepts templated task names as atomic schedule names)dsl",
       {{R"dsl(xsmp.project)dsl",
         R"dsl(project 'Demo' using 'ECSS_SMP_2025'
source 'src'
)dsl"},
        {R"dsl(src/types.xsmpcat)dsl", R"dsl(catalogue DemoTypes

namespace demo
{
    public event FlagEvent extends Smp.Bool

    public model Child
    {
        input field Smp.Int32 inValue
        output field Smp.Int32 outValue

        /** child reset */
        public def void reset()

        /** child trigger */
        entrypoint step
        {
            in inValue
            out outValue
        }

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }

    public model SpecializedChild extends Child
    {
        input field Smp.Int32 implIn
    }

    public model Root
    {
        field Smp.Bool enabledState
        field Smp.Int32 countState
        output field Smp.Int32 outValue
        input field Smp.Int32 inValue
        container Child child = demo.Child

        /** root enabled */
        public property Smp.Bool enabled -> enabledState
        /** root count */
        public property Smp.Int32 count -> countState

        /** root reset */
        public def void reset()
        /** root apply */
        public def void apply(in Smp.Int32 nextCount, in Smp.Float64 nextRatio)

        /** root trigger */
        entrypoint step
        {
            in inValue
            out outValue
        }

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }

    public model RootWithDefault
    {
        output field Smp.Int32 outValue
        container Child child = demo.SpecializedChild
    }
}
)dsl"}},
       R"dsl(schedule <Lane = "Ops"> Demo

task Task{Lane} on demo.Root
{
    call reset()
}

event Task{Lane} mission "PT1S"
)dsl",
       {{R"dsl(exact)dsl", {}, false, {}, false, 0}}},
      {R"dsl(xsmpsed)dsl",
       R"dsl(rejects duplicate document names across document kinds)dsl",
       {{R"dsl(xsmp.project)dsl",
         R"dsl(project 'Demo' using 'ECSS_SMP_2025'
source 'src'
)dsl"},
        {R"dsl(src/types.xsmpcat)dsl", R"dsl(catalogue DemoTypes

namespace demo
{
    public event FlagEvent extends Smp.Bool

    public model Child
    {
        input field Smp.Int32 inValue
        output field Smp.Int32 outValue

        /** child reset */
        public def void reset()

        /** child trigger */
        entrypoint step
        {
            in inValue
            out outValue
        }

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }

    public model SpecializedChild extends Child
    {
        input field Smp.Int32 implIn
    }

    public model Root
    {
        field Smp.Bool enabledState
        field Smp.Int32 countState
        output field Smp.Int32 outValue
        input field Smp.Int32 inValue
        container Child child = demo.Child

        /** root enabled */
        public property Smp.Bool enabled -> enabledState
        /** root count */
        public property Smp.Int32 count -> countState

        /** root reset */
        public def void reset()
        /** root apply */
        public def void apply(in Smp.Int32 nextCount, in Smp.Float64 nextRatio)

        /** root trigger */
        entrypoint step
        {
            in inValue
            out outValue
        }

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }

    public model RootWithDefault
    {
        output field Smp.Int32 outValue
        container Child child = demo.SpecializedChild
    }
}
)dsl"},
        {R"dsl(src/assembly.xsmpasb)dsl",
         R"dsl(assembly <Lane = "Ops"> DemoAsm

Root: demo.Root
{
    child += Child{Lane}: demo.Child
}
)dsl"}},
       R"dsl(schedule <Root = "root"> DemoAsm

task Main on DemoAsm
{
    call Child{Lane}.reset()
}
)dsl",
       {{R"dsl(contains)dsl",
         {R"dsl(Duplicated Document name.)dsl"},
         false,
         {},
         false,
         0}}},
      {R"dsl(xsmpsed)dsl",
       R"dsl(allows unsuffixed numeric values when the safe resolved target type is known)dsl",
       {{R"dsl(xsmp.project)dsl",
         R"dsl(project 'Demo' using 'ECSS_SMP_2025'
source 'src'
)dsl"},
        {R"dsl(src/types.xsmpcat)dsl", R"dsl(catalogue DemoTypes

namespace demo
{
    public event FlagEvent extends Smp.Bool

    public model Child
    {
        input field Smp.Int32 inValue
        output field Smp.Int32 outValue

        /** child reset */
        public def void reset()

        /** child trigger */
        entrypoint step
        {
            in inValue
            out outValue
        }

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }

    public model SpecializedChild extends Child
    {
        input field Smp.Int32 implIn
    }

    public model Root
    {
        field Smp.Bool enabledState
        field Smp.Int32 countState
        output field Smp.Int32 outValue
        input field Smp.Int32 inValue
        container Child child = demo.Child

        /** root enabled */
        public property Smp.Bool enabled -> enabledState
        /** root count */
        public property Smp.Int32 count -> countState

        /** root reset */
        public def void reset()
        /** root apply */
        public def void apply(in Smp.Int32 nextCount, in Smp.Float64 nextRatio)

        /** root trigger */
        entrypoint step
        {
            in inValue
            out outValue
        }

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }

    public model RootWithDefault
    {
        output field Smp.Int32 outValue
        container Child child = demo.SpecializedChild
    }
}
)dsl"}},
       R"dsl(schedule <Root = "root"> Demo

task Main on demo.Root
{
    property count = 2
    call apply(nextCount = 3, nextRatio = 1.5)
}
)dsl",
       {{R"dsl(exact)dsl", {}, false, {}, false, 0}}},
      {R"dsl(xsmpsed)dsl",
       R"dsl(reports unknown placeholders and invalid expanded schedule path segments)dsl",
       {{R"dsl(xsmp.project)dsl",
         R"dsl(project 'Demo' using 'ECSS_SMP_2025'
source 'src'
)dsl"},
        {R"dsl(src/types.xsmpcat)dsl", R"dsl(catalogue DemoTypes

namespace demo
{
    public event FlagEvent extends Smp.Bool

    public model Child
    {
        input field Smp.Int32 inValue
        output field Smp.Int32 outValue

        /** child reset */
        public def void reset()

        /** child trigger */
        entrypoint step
        {
            in inValue
            out outValue
        }

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }

    public model SpecializedChild extends Child
    {
        input field Smp.Int32 implIn
    }

    public model Root
    {
        field Smp.Bool enabledState
        field Smp.Int32 countState
        output field Smp.Int32 outValue
        input field Smp.Int32 inValue
        container Child child = demo.Child

        /** root enabled */
        public property Smp.Bool enabled -> enabledState
        /** root count */
        public property Smp.Int32 count -> countState

        /** root reset */
        public def void reset()
        /** root apply */
        public def void apply(in Smp.Int32 nextCount, in Smp.Float64 nextRatio)

        /** root trigger */
        entrypoint step
        {
            in inValue
            out outValue
        }

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }

    public model RootWithDefault
    {
        output field Smp.Int32 outValue
        container Child child = demo.SpecializedChild
    }
}
)dsl"}},
       R"dsl(schedule <Root = "child"> DemoMissingPlaceholder

task Main on demo.Root
{
    call {Missing}.reset()
}
)dsl",
       {{R"dsl(exact)dsl",
         {R"dsl(The placeholder '{Missing}' shall resolve to a Template Argument of the enclosing Schedule.)dsl"},
         false,
         {},
         false,
         0}}},
      {R"dsl(xsmpsed)dsl",
       R"dsl(reports unknown placeholders and invalid expanded schedule path segments)dsl",
       {{R"dsl(xsmp.project)dsl",
         R"dsl(project 'Demo' using 'ECSS_SMP_2025'
source 'src'
)dsl"},
        {R"dsl(src/types.xsmpcat)dsl", R"dsl(catalogue DemoTypes

namespace demo
{
    public event FlagEvent extends Smp.Bool

    public model Child
    {
        input field Smp.Int32 inValue
        output field Smp.Int32 outValue

        /** child reset */
        public def void reset()

        /** child trigger */
        entrypoint step
        {
            in inValue
            out outValue
        }

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }

    public model SpecializedChild extends Child
    {
        input field Smp.Int32 implIn
    }

    public model Root
    {
        field Smp.Bool enabledState
        field Smp.Int32 countState
        output field Smp.Int32 outValue
        input field Smp.Int32 inValue
        container Child child = demo.Child

        /** root enabled */
        public property Smp.Bool enabled -> enabledState
        /** root count */
        public property Smp.Int32 count -> countState

        /** root reset */
        public def void reset()
        /** root apply */
        public def void apply(in Smp.Int32 nextCount, in Smp.Float64 nextRatio)

        /** root trigger */
        entrypoint step
        {
            in inValue
            out outValue
        }

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }

    public model RootWithDefault
    {
        output field Smp.Int32 outValue
        container Child child = demo.SpecializedChild
    }
}
)dsl"}},
       R"dsl(schedule <Target = "1bad"> DemoInvalidExpansion

task Main on demo.Root
{
    call {Target}.reset()
}
)dsl",
       {{R"dsl(exact)dsl",
         {R"dsl(The expanded path segment '1bad' is not valid for SMP Level 2.)dsl"},
         false,
         {},
         false,
         0}}},
      {R"dsl(xsmpsed)dsl",
       R"dsl(warns when a schedule template parameter is not used)dsl",
       {{R"dsl(xsmp.project)dsl",
         R"dsl(project 'Demo' using 'ECSS_SMP_2025'
source 'src'
)dsl"},
        {R"dsl(src/types.xsmpcat)dsl", R"dsl(catalogue DemoTypes

namespace demo
{
    public event FlagEvent extends Smp.Bool

    public model Child
    {
        input field Smp.Int32 inValue
        output field Smp.Int32 outValue

        /** child reset */
        public def void reset()

        /** child trigger */
        entrypoint step
        {
            in inValue
            out outValue
        }

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }

    public model SpecializedChild extends Child
    {
        input field Smp.Int32 implIn
    }

    public model Root
    {
        field Smp.Bool enabledState
        field Smp.Int32 countState
        output field Smp.Int32 outValue
        input field Smp.Int32 inValue
        container Child child = demo.Child

        /** root enabled */
        public property Smp.Bool enabled -> enabledState
        /** root count */
        public property Smp.Int32 count -> countState

        /** root reset */
        public def void reset()
        /** root apply */
        public def void apply(in Smp.Int32 nextCount, in Smp.Float64 nextRatio)

        /** root trigger */
        entrypoint step
        {
            in inValue
            out outValue
        }

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }

    public model RootWithDefault
    {
        output field Smp.Int32 outValue
        container Child child = demo.SpecializedChild
    }
}
)dsl"},
        {R"dsl(src/assembly.xsmpasb)dsl",
         R"dsl(assembly <Lane = "Ops"> DemoAsm

Root: demo.Root
{
    child += Child{Lane}: demo.Child
}
)dsl"}},
       R"dsl(schedule <Target = "ChildOps", Unused = 7> Demo

task Main on DemoAsm
{
    call {Target}.reset()
}
)dsl",
       {{R"dsl(exact)dsl",
         {R"dsl(The Template Parameter 'Unused' is not used.)dsl"},
         false,
         {},
         false,
         0}}},
      {R"dsl(xsmpsed)dsl",
       R"dsl(accepts assembly execution contexts and resolves inherited template parameters in paths)dsl",
       {{R"dsl(xsmp.project)dsl",
         R"dsl(project 'Demo' using 'ECSS_SMP_2025'
source 'src'
)dsl"},
        {R"dsl(src/types.xsmpcat)dsl", R"dsl(catalogue DemoTypes

namespace demo
{
    public event FlagEvent extends Smp.Bool

    public model Child
    {
        input field Smp.Int32 inValue
        output field Smp.Int32 outValue

        /** child reset */
        public def void reset()

        /** child trigger */
        entrypoint step
        {
            in inValue
            out outValue
        }

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }

    public model SpecializedChild extends Child
    {
        input field Smp.Int32 implIn
    }

    public model Root
    {
        field Smp.Bool enabledState
        field Smp.Int32 countState
        output field Smp.Int32 outValue
        input field Smp.Int32 inValue
        container Child child = demo.Child

        /** root enabled */
        public property Smp.Bool enabled -> enabledState
        /** root count */
        public property Smp.Int32 count -> countState

        /** root reset */
        public def void reset()
        /** root apply */
        public def void apply(in Smp.Int32 nextCount, in Smp.Float64 nextRatio)

        /** root trigger */
        entrypoint step
        {
            in inValue
            out outValue
        }

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }

    public model RootWithDefault
    {
        output field Smp.Int32 outValue
        container Child child = demo.SpecializedChild
    }
}
)dsl"},
        {R"dsl(src/assembly.xsmpasb)dsl",
         R"dsl(assembly <Lane = "Ops"> DemoAsm

Root: demo.Root
{
    child += Child{Lane}: demo.Child
}
)dsl"}},
       R"dsl(schedule <Root = "root"> Demo

task Main on DemoAsm
{
    call Child{Lane}.reset()
    trig Child{Lane}.step
}
)dsl",
       {{R"dsl(exact)dsl", {}, false, {}, false, 0}}},
      {R"dsl(xsmpsed)dsl",
       R"dsl(requires a root String8 parameter even when the schedule only uses task-scoped paths)dsl",
       {{R"dsl(xsmp.project)dsl",
         R"dsl(project 'Demo' using 'ECSS_SMP_2025'
source 'src'
)dsl"},
        {R"dsl(src/types.xsmpcat)dsl", R"dsl(catalogue DemoTypes

namespace demo
{
    public event FlagEvent extends Smp.Bool

    public model Child
    {
        input field Smp.Int32 inValue
        output field Smp.Int32 outValue

        /** child reset */
        public def void reset()

        /** child trigger */
        entrypoint step
        {
            in inValue
            out outValue
        }

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }

    public model SpecializedChild extends Child
    {
        input field Smp.Int32 implIn
    }

    public model Root
    {
        field Smp.Bool enabledState
        field Smp.Int32 countState
        output field Smp.Int32 outValue
        input field Smp.Int32 inValue
        container Child child = demo.Child

        /** root enabled */
        public property Smp.Bool enabled -> enabledState
        /** root count */
        public property Smp.Int32 count -> countState

        /** root reset */
        public def void reset()
        /** root apply */
        public def void apply(in Smp.Int32 nextCount, in Smp.Float64 nextRatio)

        /** root trigger */
        entrypoint step
        {
            in inValue
            out outValue
        }

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }

    public model RootWithDefault
    {
        output field Smp.Int32 outValue
        container Child child = demo.SpecializedChild
    }
}
)dsl"}},
       R"dsl(schedule Demo

task Main on demo.Root
{
    call reset()
}
)dsl",
       {{R"dsl(exact)dsl",
         {R"dsl(A Schedule shall declare at least one String8 Template Argument for the root path.)dsl"},
         false,
         {},
         false,
         0}}},
      {R"dsl(xsmpsed)dsl",
       R"dsl(rejects parent traversal in schedule paths)dsl",
       {{R"dsl(xsmp.project)dsl",
         R"dsl(project 'Demo' using 'ECSS_SMP_2025'
source 'src'
)dsl"},
        {R"dsl(src/types.xsmpcat)dsl", R"dsl(catalogue DemoTypes

namespace demo
{
    public event FlagEvent extends Smp.Bool

    public model Child
    {
        input field Smp.Int32 inValue
        output field Smp.Int32 outValue

        /** child reset */
        public def void reset()

        /** child trigger */
        entrypoint step
        {
            in inValue
            out outValue
        }

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }

    public model SpecializedChild extends Child
    {
        input field Smp.Int32 implIn
    }

    public model Root
    {
        field Smp.Bool enabledState
        field Smp.Int32 countState
        output field Smp.Int32 outValue
        input field Smp.Int32 inValue
        container Child child = demo.Child

        /** root enabled */
        public property Smp.Bool enabled -> enabledState
        /** root count */
        public property Smp.Int32 count -> countState

        /** root reset */
        public def void reset()
        /** root apply */
        public def void apply(in Smp.Int32 nextCount, in Smp.Float64 nextRatio)

        /** root trigger */
        entrypoint step
        {
            in inValue
            out outValue
        }

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }

    public model RootWithDefault
    {
        output field Smp.Int32 outValue
        container Child child = demo.SpecializedChild
    }
}
)dsl"}},
       R"dsl(schedule <Root = "root"> Demo

task Main on demo.Root
{
    call ../child.reset()
    execute Worker at ../child
}

task Worker on demo.Child
{
    call reset()
}
)dsl",
       {{R"dsl(count)dsl",
         {},
         false,
         {R"dsl(Paths shall not contain '..'.)dsl"},
         false,
         2}}},
      {R"dsl(xsmpsed)dsl",
       R"dsl(requires explicit schedule template values and unique names)dsl",
       {{R"dsl(xsmp.project)dsl",
         R"dsl(project 'Demo' using 'ECSS_SMP_2025'
source 'src'
)dsl"},
        {R"dsl(src/types.xsmpcat)dsl", R"dsl(catalogue DemoTypes

namespace demo
{
    public event FlagEvent extends Smp.Bool

    public model Child
    {
        input field Smp.Int32 inValue
        output field Smp.Int32 outValue

        /** child reset */
        public def void reset()

        /** child trigger */
        entrypoint step
        {
            in inValue
            out outValue
        }

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }

    public model SpecializedChild extends Child
    {
        input field Smp.Int32 implIn
    }

    public model Root
    {
        field Smp.Bool enabledState
        field Smp.Int32 countState
        output field Smp.Int32 outValue
        input field Smp.Int32 inValue
        container Child child = demo.Child

        /** root enabled */
        public property Smp.Bool enabled -> enabledState
        /** root count */
        public property Smp.Int32 count -> countState

        /** root reset */
        public def void reset()
        /** root apply */
        public def void apply(in Smp.Int32 nextCount, in Smp.Float64 nextRatio)

        /** root trigger */
        entrypoint step
        {
            in inValue
            out outValue
        }

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }

    public model RootWithDefault
    {
        output field Smp.Int32 outValue
        container Child child = demo.SpecializedChild
    }
}
)dsl"}},
       R"dsl(schedule <Root: string, Root = "root", Lane: int32> Demo

task Main on demo.Root
{
}
)dsl",
       {{R"dsl(contains)dsl",
         {R"dsl(Duplicated template argument name.)dsl",
          R"dsl(A template parameter needs a value.)dsl"},
         false,
         {},
         false,
         0},
        {R"dsl(count)dsl",
         {},
         false,
         {R"dsl(A template parameter needs a value.)dsl"},
         false,
         2}}},
      {R"dsl(xsmpsed)dsl",
       R"dsl(validates duplicate and unknown schedule operation parameters)dsl",
       {{R"dsl(xsmp.project)dsl",
         R"dsl(project 'Demo' using 'ECSS_SMP_2025'
source 'src'
)dsl"},
        {R"dsl(src/types.xsmpcat)dsl", R"dsl(catalogue DemoTypes

namespace demo
{
    public event FlagEvent extends Smp.Bool

    public model Child
    {
        input field Smp.Int32 inValue
        output field Smp.Int32 outValue

        /** child reset */
        public def void reset()

        /** child trigger */
        entrypoint step
        {
            in inValue
            out outValue
        }

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }

    public model SpecializedChild extends Child
    {
        input field Smp.Int32 implIn
    }

    public model Root
    {
        field Smp.Bool enabledState
        field Smp.Int32 countState
        output field Smp.Int32 outValue
        input field Smp.Int32 inValue
        container Child child = demo.Child

        /** root enabled */
        public property Smp.Bool enabled -> enabledState
        /** root count */
        public property Smp.Int32 count -> countState

        /** root reset */
        public def void reset()
        /** root apply */
        public def void apply(in Smp.Int32 nextCount, in Smp.Float64 nextRatio)

        /** root trigger */
        entrypoint step
        {
            in inValue
            out outValue
        }

        eventsink demo.FlagEvent inbound
        eventsource demo.FlagEvent outbound
    }

    public model RootWithDefault
    {
        output field Smp.Int32 outValue
        container Child child = demo.SpecializedChild
    }
}
)dsl"}},
       R"dsl(schedule <Root = "root"> Demo

task Main on demo.Root
{
    call apply(nextCount = 1i32, nextCount = 2i32, missing = 3i32)
}
)dsl",
       {{R"dsl(contains)dsl",
         {R"dsl(Duplicated parameter name.)dsl",
          R"dsl(The parameter 'missing' shall resolve to a Parameter of operation apply.)dsl"},
         false,
         {},
         false,
         0}}},
  };
  return cases;
}
}  // namespace xsmp
