#pragma once

#include <cursiv/syntax-tree/AstNode.hpp>

// XSMP catalogue (xsmpcat) abstract syntax tree. Cross references use
// `reference<T>` / `multi_reference<T>` and are resolved by the linker; owned
// children use `T*` (a raw pointer owned by the parse result) and `vector<>`.
// A reference member declares the broad class of the types it may name. The
// grammar narrows each site to the class it accepts with `xref`. C++ keyword
// clashes are suffixed with `_`: `interface_`, `default_`, `optional_`.
namespace xsmp {

struct NamedNode : cursiv::Extends<cursiv::AstNode> {
  string_view name;
};

// ---------------------------------------------------------------------------
// Forward declarations (cross references and owned-child pointers only need an
// incomplete type).
// ---------------------------------------------------------------------------
struct Attribute;
struct Expression;
struct Type;
struct Field;
struct Constant;
struct EnumerationLiteral;
struct Multiplicity;
struct ReturnParameter;
struct Parameter;
struct NamedElement;
struct Namespace;

// ---------------------------------------------------------------------------
// Expressions
// ---------------------------------------------------------------------------
// The constant expressions of a catalogue, in C++ syntax. `getValue` computes
// their value.
struct Expression : cursiv::Extends<cursiv::AstNode> {};

// `feature` is the operator as written, here and in `UnaryOperation`.
struct BinaryOperation : cursiv::Extends<Expression> {
  Expression* leftOperand = nullptr;
  string_view feature;
  Expression* rightOperand = nullptr;
};

struct UnaryOperation : cursiv::Extends<Expression> {
  string_view feature;
  Expression* operand = nullptr;
};

struct NamedElementReference : cursiv::Extends<Expression> {
  reference<Constant, EnumerationLiteral> value;
};

struct ParenthesizedExpression : cursiv::Extends<Expression> {
  Expression* expr = nullptr;
};

// `.field = expr` in a `CollectionLiteral`. `expr` may be missing. Fields have
// to come in declaration order, so the position still tells the field.
struct DesignatedInitializer : cursiv::Extends<Expression> {
  reference<Field> field;
  Expression* expr = nullptr;
};

// `{ ... }`: the value of an array or a structure.
struct CollectionLiteral : cursiv::Extends<Expression> {
  vector<Expression*> elements;
};

// `text` is as written: `integer_value` reads its time unit.
struct IntegerLiteral : cursiv::Extends<Expression> {
  string_view text;
};

struct FloatingLiteral : cursiv::Extends<Expression> {
  string_view text;
};

// Adjacent strings make one literal. `value` holds each piece, decoded.
struct StringLiteral : cursiv::Extends<Expression> {
  vector<string> value;
};

struct CharacterLiteral : cursiv::Extends<Expression> {
  string value;
};

// `$name`: a constant the language defines, such as `$PI`.
struct BuiltInConstant : cursiv::Extends<Expression> {
  string_view name;
};

// `$name(argument)`: a function the language defines, such as `$cos(x)`.
struct BuiltInFunction : cursiv::Extends<Expression> {
  string_view name;
  Expression* argument = nullptr;
};

struct BooleanLiteral : cursiv::Extends<Expression> {
  bool isTrue = false;
};

// `nullptr`, the only keyword that is an expression.
struct KeywordExpression : cursiv::Extends<Expression> {
  string_view name;
};

// ---------------------------------------------------------------------------
// Named-element bases
// ---------------------------------------------------------------------------
// An SMP element: it has a name and can carry attributes.
struct NamedElement : cursiv::Extends<NamedNode> {
  vector<Attribute*> attributes;
};

// The root of a modeling document: a catalogue, an assembly, a configuration, a
// link base or a schedule.
struct Document : cursiv::Extends<NamedElement> {};

// An element with modifiers (`public`, `abstract`, `input`, `readOnly`, ...),
// kept as written. `getRealVisibility` reads the visibility they give.
struct VisibilityElement : cursiv::Extends<NamedElement> {
  vector<string_view> modifiers;
};

// ---------------------------------------------------------------------------
// Metadata / multiplicity
// ---------------------------------------------------------------------------
// Metadata on an element: `@Type` or `@Type(value)`.
struct Attribute : cursiv::Extends<cursiv::AstNode> {
  // @Type (resolves to an AttributeType)
  reference<Type> type;
  Expression* value = nullptr;
};

// How many components a container or a reference holds: `[lower ... upper]`.
// With no bound written, `aux` tells `+` from `*`. With a lower bound, it marks
// the open form `lower ... *`. `getLower` and `getUpper` give the numbers.
struct Multiplicity : cursiv::Extends<cursiv::AstNode> {
  Expression* lower = nullptr;
  Expression* upper = nullptr;
  bool aux = false;
};

// ---------------------------------------------------------------------------
// Type hierarchy
// ---------------------------------------------------------------------------
// A type is a language type, an event type or an attribute type. Only language
// types can type a field, a property or a parameter.
struct Type : cursiv::Extends<VisibilityElement> {};

struct LanguageType : cursiv::Extends<Type> {};

// What is used by reference: interfaces and components.
struct ReferenceType : cursiv::Extends<LanguageType> {};

// What holds a value: simple types, structures, classes and arrays.
struct ValueType : cursiv::Extends<LanguageType> {};

// A value type with a single value. Only these can type a constant or the
// argument of an event.
struct SimpleType : cursiv::Extends<ValueType> {};

// A base type of the SMP standard, such as `Smp.Float64`, which the built-in
// catalogues declare.
struct PrimitiveType : cursiv::Extends<SimpleType> {};

struct Enumeration : cursiv::Extends<SimpleType> {
  vector<EnumerationLiteral*> literal;
};

// An integer restricted to a range and stored as `primitiveType`. Each of the
// three is optional.
struct Integer : cursiv::Extends<SimpleType> {
  optional<reference<Type>> primitiveType;
  Expression* minimum = nullptr;
  Expression* maximum = nullptr;
};

// The same for a floating-point number. `range` spells which bounds count:
// `...` both, `..<` not the upper one, `<..` not the lower one, `<.<` neither.
struct Float : cursiv::Extends<SimpleType> {
  optional<reference<Type>> primitiveType;
  Expression* minimum = nullptr;
  Expression* maximum = nullptr;
  string_view range;
};

struct String : cursiv::Extends<SimpleType> {
  Expression* length = nullptr;
};

// `elements` holds constants and fields. A `Class` adds properties, operations
// and associations.
struct Structure : cursiv::Extends<ValueType> {
  vector<NamedElement*> elements;
};

struct Class : cursiv::Extends<Structure> {
  optional<reference<Type>> base;
};

struct Exception : cursiv::Extends<Class> {};

struct Array : cursiv::Extends<ValueType> {
  reference<Type> itemType;
  Expression* size = nullptr;
};

// A contract that components implement. `elements` holds constants, properties
// and operations.
struct Interface : cursiv::Extends<ReferenceType> {
  vector<reference<Type>> base;
  vector<NamedElement*> elements;
};

// What a simulation instantiates: a `Model` or a `Service`. `interface_` lists
// the interfaces it implements.
struct Component : cursiv::Extends<ReferenceType> {
  vector<reference<Type>> interface_;
  optional<reference<Type>> base;
  vector<NamedElement*> elements;
};

struct Model : cursiv::Extends<Component> {};

struct Service : cursiv::Extends<Component> {};

// A pointer to a value type: `using Ref = Value*`.
struct ValueReference : cursiv::Extends<LanguageType> {
  reference<Type> type;
};

// A type of the platform, opaque to XSMP.
struct NativeType : cursiv::Extends<LanguageType> {};

// `eventArgs` is the type of the value an event carries, if it carries one.
struct EventType : cursiv::Extends<Type> {
  optional<reference<Type>> eventArgs;
};

// Declares an attribute: `type` is the type of its value, and `default_` the
// value when a use gives none.
struct AttributeType : cursiv::Extends<Type> {
  reference<Type> type;
  Expression* default_ = nullptr;
};

// ---------------------------------------------------------------------------
// Members
// ---------------------------------------------------------------------------
struct Constant : cursiv::Extends<VisibilityElement> {
  reference<Type> type;
  Expression* value = nullptr;
};

struct Field : cursiv::Extends<VisibilityElement> {
  reference<Type> type;
  Expression* default_ = nullptr;
};

// A member read and written through a getter and a setter. `attachedField` is
// the field behind it, `getRaises` and `setRaises` what each accessor throws.
struct Property : cursiv::Extends<VisibilityElement> {
  reference<Type> type;
  optional<reference<Field>> attachedField;
  vector<reference<Type>> getRaises;
  vector<reference<Type>> setRaises;
};

// Not a `NamedElement`: its name is optional, and the checks on named elements
// do not apply to it. See `attributeUsageName` for how attributes see it.
struct ReturnParameter : cursiv::Extends<cursiv::AstNode> {
  vector<Attribute*> attributes;
  string_view name;
  reference<Type> type;
};

// `direction` is `in`, `out`, `inout`, or empty when not written.
struct Parameter : cursiv::Extends<NamedElement> {
  reference<Type> type;
  Expression* default_ = nullptr;
  string_view direction;
};

// `returnParameter` is null for `void`.
struct Operation : cursiv::Extends<VisibilityElement> {
  ReturnParameter* returnParameter = nullptr;
  vector<Parameter*> parameter;
  vector<reference<Type>> raisedException;
};

struct Association : cursiv::Extends<VisibilityElement> {
  reference<Type> type;
};

// `optional_` is the `?` form, which has no `multiplicity` node.
struct NamedElementWithMultiplicity : cursiv::Extends<NamedElement> {
  Multiplicity* multiplicity = nullptr;
  bool optional_ = false;
};

// A slot that owns child components. `defaultComponent` is the implementation
// of a child that names none.
struct Container : cursiv::Extends<NamedElementWithMultiplicity> {
  reference<Type> type;
  optional<reference<Type>> defaultComponent;
};

// A slot that points at components owned elsewhere. `interface_` is its type:
// an interface or a component.
struct Reference : cursiv::Extends<NamedElementWithMultiplicity> {
  reference<Type> interface_;
};

// What a scheduler or an event can trigger. `input` and `output` are the fields
// it reads and writes.
struct EntryPoint : cursiv::Extends<NamedElement> {
  vector<reference<Field>> input;
  vector<reference<Field>> output;
};

// The two ends of an event link: a source emits and its sinks receive.
struct EventSink : cursiv::Extends<NamedElement> {
  reference<Type> type;
};

struct EventSource : cursiv::Extends<NamedElement> {
  reference<Type> type;
};

struct EnumerationLiteral : cursiv::Extends<NamedElement> {
  Expression* value = nullptr;
};

// ---------------------------------------------------------------------------
// Containers / root
// ---------------------------------------------------------------------------
struct Namespace : cursiv::Extends<NamedElement> {
  vector<NamedElement*> elements;  // (Namespace | Type)
};

// The root of a catalogue document. Types only live inside its namespaces.
struct Catalogue : cursiv::Extends<Document> {
  vector<Namespace*> elements;
};

// Xsmp.project language (`xsmp.project` / `.xsmpprofile` / `.xsmptool`). A file
// is exactly one of Project / Profile / Tool (the entry ordered choice). Cross
// references (`profile`/`tool`/`dependency`) use the `STRING` value as the name
// to resolve.
struct Profile;
struct Tool;
struct Project;

// One line of a project: a profile, a tool, a dependency or a source.
struct ProjectElement : cursiv::Extends<cursiv::AstNode> {};

struct ProfileReference : cursiv::Extends<ProjectElement> {
  reference<Profile> profile;
};

struct ToolReference : cursiv::Extends<ProjectElement> {
  reference<Tool> tool;
};

struct Dependency : cursiv::Extends<ProjectElement> {
  reference<Project> project;
};

// `path` is relative to the project directory and names a folder or a file.
struct Source : cursiv::Extends<ProjectElement> {
  string_view path;
};

// `standard` is the SMP revision the project targets: see `kSmpStandards`.
struct Project : cursiv::Extends<NamedNode> {
  string_view standard = "ECSS_SMP_2020";
  vector<ProjectElement*> elements;
};

// What a `.xsmpprofile` or a `.xsmptool` file declares. Installed extensions
// usually provide them.
struct Profile : cursiv::Extends<NamedNode> {};

struct Tool : cursiv::Extends<NamedNode> {};

// ---------------------------------------------------------------------------
// Deployment documents (xsmpasb, xsmpcfg, xsmplnk, xsmpsed)
// ---------------------------------------------------------------------------
// They share the catalogue's named types and a common path and value
// vocabulary. Children belong to the parse result.
struct Assembly;
struct Configuration;
struct LinkBase;
struct Task;
struct Value;
struct SimpleValue;
struct PathSegment;
struct PathElement;
struct TemplateParameter;
struct TemplateArgument;
struct ModelInstance;
struct AssemblyComponentConfiguration;
struct ComponentConfiguration;
struct ComponentLinkBase;
struct Activity;
struct Event;

// A name read among the members of the current component. `unsafe` leaves it
// unlinked and unchecked.
struct LocalNamedReference : cursiv::Extends<cursiv::AstNode> {
  bool unsafe = false;
  cursiv::reference<NamedElement> reference;
};

// One step of a `Path`: a name, a name with `{parameter}` placeholders, `..` or
// `.`.
struct PathSegment : cursiv::Extends<cursiv::AstNode> {};

// `unsafe`, written only for a field of a configuration's structure value,
// leaves the name unlinked.
struct ConcretePathNamedSegment : cursiv::Extends<PathSegment> {
  bool unsafe = false;
  cursiv::reference<NamedElement> reference;
};

// A name with placeholders. It is no reference: it is expanded, then looked up
// by `segmentResolution`.
struct PatternPathNamedSegment : cursiv::Extends<PathSegment> {
  std::string_view text;
};

struct PathParentSegment : cursiv::Extends<PathSegment> {};

struct PathSelfSegment : cursiv::Extends<PathSegment> {};

// What follows the head of a path: `.name` or `/name` (`PathMember`), or `[2]`
// (`PathIndex`).
struct PathElement : cursiv::Extends<cursiv::AstNode> {};

struct PathMember : cursiv::Extends<PathElement> {
  std::string_view separator;
  PathSegment* segment = nullptr;
};

struct PathIndex : cursiv::Extends<PathElement> {
  std::int64_t index = 0;
};

// `absolute` is a leading `/`, and the path may stop there, without a `head`.
// `unsafe` turns off the checks on the path and on the value it receives.
// `resolvePathWithBindings` gives what the path names.
struct Path : cursiv::Extends<cursiv::AstNode> {
  bool unsafe = false;
  bool absolute = false;
  PathSegment* head = nullptr;
  std::vector<PathElement*> elements;
};

// The values of the deployment languages. A suffix names the SMP type of a
// number: `1i8`, `2u16`, `1.5f32`. Integers are held in 64 bits whatever their
// suffix, and validation checks that they fit.
struct Value : cursiv::Extends<cursiv::AstNode> {};

struct SimpleValue : cursiv::Extends<Value> {};

struct Int8Value : cursiv::Extends<SimpleValue> {
  std::int64_t value = 0;
};

struct Int16Value : cursiv::Extends<SimpleValue> {
  std::int64_t value = 0;
};

struct Int32Value : cursiv::Extends<SimpleValue> {
  std::int64_t value = 0;
};

struct Int64Value : cursiv::Extends<SimpleValue> {
  std::int64_t value = 0;
};

struct UInt8Value : cursiv::Extends<SimpleValue> {
  std::uint64_t value = 0;
};

struct UInt16Value : cursiv::Extends<SimpleValue> {
  std::uint64_t value = 0;
};

struct UInt32Value : cursiv::Extends<SimpleValue> {
  std::uint64_t value = 0;
};

struct UInt64Value : cursiv::Extends<SimpleValue> {
  std::uint64_t value = 0;
};

// An unsuffixed integer keeps its decimal spelling until the target type is
// known: a UInt64 is not narrowed to Int64 by parsing, and a value beyond 64
// bits is reported against its target type.
struct IntValue : cursiv::Extends<SimpleValue> {
  std::string_view value;
};

// A literal by name, or by number: `2 lit`.
struct EnumerationValue : cursiv::Extends<SimpleValue> {
  std::optional<std::int64_t> value;
  cursiv::reference<EnumerationLiteral> reference;
};

struct BoolValue : cursiv::Extends<SimpleValue> {
  bool value = false;
};

struct Float32Value : cursiv::Extends<SimpleValue> {
  double value = 0;
};

struct Float64Value : cursiv::Extends<SimpleValue> {
  double value = 0;
};

struct FloatValue : cursiv::Extends<SimpleValue> {
  double value = 0;
};

struct Char8Value : cursiv::Extends<SimpleValue> {
  std::string value;
};

struct String8Value : cursiv::Extends<SimpleValue> {
  std::string value;
};

// `"PT1S"d` here and `"1970-01-01T00:00:00Z"dt` below: `value` is the text
// between the quotes.
struct DurationValue : cursiv::Extends<SimpleValue> {
  std::string_view value;
};

struct DateTimeValue : cursiv::Extends<SimpleValue> {
  std::string_view value;
};

// `[1, 2]`, or `[3: 1, 2]` to start at index 3.
struct ArrayValue : cursiv::Extends<Value> {
  std::optional<std::int64_t> startIndex;
  std::vector<Value*> elements;
};

// `{ ... }`. An element names its field or takes the next one still free: see
// `inferDeploymentValueType`.
struct StructureValue : cursiv::Extends<Value> {
  std::vector<Value*> elements;
};

// `path = value`. Inside a structure value, the path may be missing.
struct FieldValue : cursiv::Extends<Value> {
  Path* field = nullptr;
  Value* value = nullptr;
};

// The form of `FieldValue` inside a configuration's structure value, where one
// name designates the field.
struct CfgStructureFieldValue : cursiv::Extends<Value> {
  ConcretePathNamedSegment* field = nullptr;
  Value* value = nullptr;
};

// A parameter of an assembly or a schedule, used in names as `{name}`. A
// default value is optional: the `value` of a `StringParameter` has a null
// `data()` when there is none.
struct TemplateParameter : cursiv::Extends<cursiv::AstNode> {
  std::string_view name;
};

struct StringParameter : cursiv::Extends<TemplateParameter> {
  std::string_view value;
};

struct Int32Parameter : cursiv::Extends<TemplateParameter> {
  std::optional<std::int64_t> value;
};

// The value given to a parameter by an `AssemblyInstance` or an `ExecuteTask`.
struct TemplateArgument : cursiv::Extends<cursiv::AstNode> {
  cursiv::reference<TemplateParameter> parameter;
};

struct StringArgument : cursiv::Extends<TemplateArgument> {
  std::string_view value;
};

struct Int32Argument : cursiv::Extends<TemplateArgument> {
  std::int64_t value = 0;
};

// `name = value` in a call. `parameter` is plain text, compared with the names
// of the operation's parameters.
struct ParameterValue : cursiv::Extends<cursiv::AstNode> {
  std::string_view parameter;
  SimpleValue* value = nullptr;
};

// `subscribe entryPoint -> "event"`: a simulator-wide event triggers an entry
// point.
struct GlobalEventHandler : cursiv::Extends<cursiv::AstNode> {
  LocalNamedReference* entryPoint = nullptr;
  std::string globalEventName;
};

// What an assembly does on an instance: call an operation or set a property.
struct Invocation : cursiv::Extends<cursiv::AstNode> {};

struct OperationCall : cursiv::Extends<Invocation> {
  LocalNamedReference* operation = nullptr;
  std::vector<ParameterValue*> parameters;
};

struct PropertyValue : cursiv::Extends<Invocation> {
  LocalNamedReference* property = nullptr;
  SimpleValue* value = nullptr;
};

// A connection from `ownerPath` to `clientPath`.
struct Link : cursiv::Extends<cursiv::AstNode> {
  Path* ownerPath = nullptr;
  Path* clientPath = nullptr;
};

struct EventLink : cursiv::Extends<Link> {};

struct FieldLink : cursiv::Extends<Link> {};

// Starts at `sourcePath`, which ends on the published reference, and leaves
// `ownerPath` null. `backReference`, if any, is a reference of the client that
// points back at the owner.
struct InterfaceLink : cursiv::Extends<Link> {
  Path* sourcePath = nullptr;
  LocalNamedReference* backReference = nullptr;
};

// A model in the tree of an assembly. Its implementation is a component, or a
// string in `strImplementation`, or neither: then its container gives it. See
// `targetType`.
struct ModelInstance : cursiv::Extends<NamedElement> {
  cursiv::reference<Component> implementation;
  std::optional<std::string> strImplementation;
  std::vector<cursiv::AstNode*> elements;
};

// Another assembly used in this tree, with its arguments and optionally the
// configuration and the link base it comes with.
struct AssemblyInstance : cursiv::Extends<NamedElement> {
  cursiv::reference<Assembly> assembly;
  std::vector<TemplateArgument*> arguments;
  cursiv::reference<Configuration> configuration;
  cursiv::reference<LinkBase> linkBase;
  std::vector<AssemblyComponentConfiguration*> elements;
};

// `container += instance`: a child instance in a container of its parent.
struct SubInstance : cursiv::Extends<cursiv::AstNode> {
  LocalNamedReference* container = nullptr;
  NamedElement* instance = nullptr;
};

// `configure path { ... }`: what an assembly sets on an instance of its tree.
struct AssemblyComponentConfiguration : cursiv::Extends<cursiv::AstNode> {
  Path* name = nullptr;
  std::vector<cursiv::AstNode*> elements;
};

// `model` is the root instance. `configurations` stand before or after it.
struct Assembly : cursiv::Extends<Document> {
  std::vector<TemplateParameter*> parameters;
  std::vector<AssemblyComponentConfiguration*> configurations;
  ModelInstance* model = nullptr;
};

// `include configuration [at path]`.
struct ConfigurationUsage : cursiv::Extends<cursiv::AstNode> {
  Path* path = nullptr;
  cursiv::reference<Configuration> configuration;
};

// `name [: context] { ... }`: what a configuration sets on a component. At the
// top level, `name` is only a label and `context` applies to the children.
struct ComponentConfiguration : cursiv::Extends<cursiv::AstNode> {
  Path* name = nullptr;
  cursiv::reference<Component, Assembly> context;
  std::vector<cursiv::AstNode*> elements;
};

struct Configuration : cursiv::Extends<Document> {
  std::vector<cursiv::AstNode*> elements;
};

// `path { ... }`: the links of one component instance, and nested blocks.
struct ComponentLinkBase : cursiv::Extends<cursiv::AstNode> {
  Path* name = nullptr;
  std::vector<cursiv::AstNode*> elements;
};

// `assembly`, when given, anchors the paths of the link base.
struct LinkBase : cursiv::Extends<Document> {
  cursiv::reference<Assembly> assembly;
  std::vector<ComponentLinkBase*> elements;
};

// One step of a `Task`.
struct Activity : cursiv::Extends<cursiv::AstNode> {};

// Runs another task. `root`, when given, is where that task runs.
struct ExecuteTask : cursiv::Extends<Activity> {
  cursiv::reference<Task> task;
  std::vector<TemplateArgument*> parameter;
  Path* root = nullptr;
};

struct EmitGlobalEvent : cursiv::Extends<Activity> {
  std::string eventName;
  bool asynchronous = false;
};

struct CallOperation : cursiv::Extends<Activity> {
  Path* operationPath = nullptr;
  std::vector<ParameterValue*> parameters;
};

struct SetProperty : cursiv::Extends<Activity> {
  Path* propertyPath = nullptr;
  SimpleValue* value = nullptr;
};

struct Transfer : cursiv::Extends<Activity> {
  Path* outputFieldPath = nullptr;
  Path* inputFieldPath = nullptr;
};

// `trig path`: runs an entry point.
struct Trigger : cursiv::Extends<Activity> {
  Path* entryPoint = nullptr;
};

// `context` is the component or assembly its paths are read against.
struct Task : cursiv::Extends<NamedElement> {
  cursiv::reference<Component, Assembly> context;
  std::vector<Activity*> elements;
};

// Starts `task`, then repeats it every `cycleTime`, `repeatCount` times when
// given. Each derived class says when it starts: mission and simulation times
// are durations, epoch and zulu times are dates.
struct Event : cursiv::Extends<cursiv::AstNode> {
  cursiv::reference<Task> task;
  std::string_view cycleTime = "PT0S";
  std::optional<std::int64_t> repeatCount;
};

// Starts when the global event `startEvent` is emitted, and stops listening at
// `stopEvent`.
struct GlobalEventTriggeredEvent : cursiv::Extends<Event> {
  std::string_view delay;
  std::string startEvent;
  std::string stopEvent;
  std::string_view timeKind;
};

struct MissionEvent : cursiv::Extends<Event> {
  std::string_view missionTime;
};

struct EpochEvent : cursiv::Extends<Event> {
  std::string_view epochTime;
};

struct SimulationEvent : cursiv::Extends<Event> {
  std::string_view simulationTime;
};

struct ZuluEvent : cursiv::Extends<Event> {
  std::string_view zuluTime;
};

// `elements` holds tasks and events. `epochTime` and `missionStart` are dates.
struct Schedule : cursiv::Extends<Document> {
  std::vector<TemplateParameter*> parameters;
  std::vector<cursiv::AstNode*> elements;
  std::string_view epochTime;
  std::string_view missionStart;
};

}  // namespace xsmp
