#include <algorithm>
#include <xsmp/AttributeHelper.hpp>
#include <xsmp/esa-cdk/EsaCdkGenerator.hpp>
#include <xsmp/utils/XsmpUtils.hpp>

namespace xsmp {

namespace {

template <class T>
bool has(const Component& type) {
  return std::ranges::any_of(type.elements, [](const NamedElement* e) {
    return cursiv::ast_cast<const T>(e);
  });
}

}  // namespace

void EsaCdkGenerator::registerModel(cursiv::TextBuilder& out,
                                    const Model& model) const {
  out.line("// Register factory for Model ", model.name);
  out.open("simulator->RegisterFactory(new ::esa::ecss::smp::cdk::Factory<",
           fqn(&model), ">(");
  out.line("\"", model.name, "\", // Name");
  out.line(description(model), ", // Description");
  out.line("simulator, // Simulator");
  out.line(uuid(&model), " // UUID");
  out.close("));");
}

std::vector<CppGenerator::Include> EsaCdkGenerator::factoryIncludes() const {
  return {std::string("esa/ecss/smp/cdk/Factory.h")};
}

void EsaCdkGenerator::generateArrayHeaderGen(cursiv::TextBuilder& out,
                                             const Array& type,
                                             bool gen) const {
  comment(out, type);
  out.line("using ", name(type.name, gen), " = ::esa::ecss::smp::cdk::Array<",
           fqn(type.itemType.get()), ", ", expression(type.size), ">;");
  out.line();
  uuidDeclaration(out, type);
  out.line();
  out.line("void _Register_", type.name,
           "(::Smp::Publication::ITypeRegistry* registry);");
}

std::vector<CppGenerator::Include> EsaCdkGenerator::headerIncludesArray(
    const Array& type) const {
  auto result = GapPatternCppGenerator::headerIncludesArray(type);
  result.emplace_back(std::string("esa/ecss/smp/cdk/Array.h"));
  return result;
}

void EsaCdkGenerator::generateStringHeaderGen(cursiv::TextBuilder& out,
                                              const String& type,
                                              bool gen) const {
  comment(out, type);
  out.line("using ", name(type.name, gen), " = ::esa::ecss::smp::cdk::String<",
           expression(type.length), ">;");
  out.line();
  uuidDeclaration(out, type);
  out.line();
  out.line("void _Register_", type.name,
           "(::Smp::Publication::ITypeRegistry* registry);");
}

std::vector<CppGenerator::Include> EsaCdkGenerator::headerIncludesString(
    const String& type) const {
  auto result = GapPatternCppGenerator::headerIncludesString(type);
  result.emplace_back(std::string("esa/ecss/smp/cdk/String.h"));
  return result;
}

std::vector<CppGenerator::Include> EsaCdkGenerator::headerIncludesContainer(
    const Container& element) const {
  auto result = GapPatternCppGenerator::headerIncludesContainer(element);
  result.emplace_back(std::string("esa/ecss/smp/cdk/Container.h"));
  return result;
}

void EsaCdkGenerator::declareContainerGen(cursiv::TextBuilder& out,
                                          const Container& element) const {
  comment(out, element);
  out.line("::esa::ecss::smp::cdk::Container<", fqn(element.type.get()), ">* ",
           element.name, ";");
}

void EsaCdkGenerator::initializeContainer(cursiv::TextBuilder& out,
                                          const Container& element) const {
  out.line("// Container: ", element.name);
  out.open(element.name, " {new ::esa::ecss::smp::cdk::Container<",
           fqn(element.type.get()), ">(");
  out.line("\"", element.name, "\",");
  out.line(description(element), ",");
  out.line("this,");
  out.line("simulator,");
  out.line(lower(element), ",");
  out.write(upper(element), ")}");
  out.close();
}

void EsaCdkGenerator::constructMembers(
    cursiv::TextBuilder& out, std::span<NamedElement* const> elements) const {
  for (const auto* element : elements) {
    const std::string_view name = element->name;
    if (cursiv::ast_cast<const Container>(element)) {
      out.line("// Add container ", name);
      out.line("this->AddContainer(", name, ");");
    } else if (cursiv::ast_cast<const EntryPoint>(element)) {
      out.line("if (!this->GetEntryPoint(\"", name, "\"))");
      out.open("{");
      out.line("// Use existing implementation to manage Entry Points");
      out.line("this->AddEntryPoint(", name, ");");
      out.close("}");
      out.line("else");
      out.open("{");
      out.line("Log(Smp::Services::ILogger::LMK_Error, \"EntryPoint ", name,
               " redeclared\");");
      out.close("}");
    } else if (cursiv::ast_cast<const EventSink>(element)) {
      out.line("// Add event sink ", name);
      out.line("this->AddEventSink(", name, ");");
    } else if (cursiv::ast_cast<const EventSource>(element)) {
      out.line("// Add event source ", name);
      out.line("this->AddEventSource(", name, ");");
    } else if (cursiv::ast_cast<const Reference>(element)) {
      out.line("// Add reference ", name);
      out.line("this->AddReference(", name, ");");
    }
  }
}

void EsaCdkGenerator::declareReferenceGen(cursiv::TextBuilder& out,
                                          const Reference& element) const {
  comment(out, element);
  out.line("::esa::ecss::smp::cdk::Reference<", fqn(element.interface_.get()),
           ">* ", element.name, ";");
}

std::vector<CppGenerator::Include> EsaCdkGenerator::headerIncludesReference(
    const Reference& element) const {
  auto result = GapPatternCppGenerator::headerIncludesReference(element);
  result.emplace_back(std::string("esa/ecss/smp/cdk/Reference.h"));
  return result;
}

void EsaCdkGenerator::initializeReference(cursiv::TextBuilder& out,
                                          const Reference& element) const {
  out.line("// Reference: ", element.name);
  out.open(element.name, " {new ::esa::ecss::smp::cdk::Reference<",
           fqn(element.interface_.get()), ">(");
  out.line("\"", element.name, "\",");
  out.line(description(element), ",");
  out.line("this,");
  out.line("simulator,");
  out.line(lower(element), ",");
  out.write(upper(element), ")}");
  out.close();
}

std::vector<CppGenerator::Include> EsaCdkGenerator::headerIncludesEntryPoint(
    const EntryPoint& element) const {
  auto result = GapPatternCppGenerator::headerIncludesEntryPoint(element);
  result.emplace_back(std::string("Smp/IEntryPoint.h"));
  return result;
}

std::vector<CppGenerator::Include> EsaCdkGenerator::sourceIncludesEntryPoint(
    const EntryPoint& element) const {
  auto result = GapPatternCppGenerator::sourceIncludesEntryPoint(element);
  result.emplace_back(std::string("esa/ecss/smp/cdk/EntryPoint.h"));
  return result;
}

void EsaCdkGenerator::initializeEntryPoint(cursiv::TextBuilder& out,
                                           const EntryPoint& element,
                                           bool gen) const {
  out.line("// EntryPoint: ", element.name);
  out.open(element.name, "{ new ::esa::ecss::smp::cdk::EntryPoint(");
  out.line("\"", element.name, "\", // Name");
  out.line(description(element), ", // Description");
  out.line("this,");
  out.line("simulator,");
  out.write("&", ownerName(element, gen), "::_", element.name, ")}");
  out.close();
}

std::vector<CppGenerator::Include> EsaCdkGenerator::sourceIncludesEventSink(
    const EventSink& element) const {
  auto result = GapPatternCppGenerator::sourceIncludesEventSink(element);
  result.emplace_back(std::string(eventType(element)
                                      ? "esa/ecss/smp/cdk/EventSinkArg.h"
                                      : "esa/ecss/smp/cdk/EventSinkVoid.h"));
  return result;
}

void EsaCdkGenerator::initializeEventSink(cursiv::TextBuilder& out,
                                          const EventSink& element,
                                          bool gen) const {
  out.line("// Event Sink: ", element.name);
  out.write(element.name, "{ new ::esa::ecss::smp::cdk::");
  if (const auto* payload = eventType(element))
    out.write("EventSinkArg<", fqn(payload), ">");
  else
    out.write("EventSinkVoid");
  out.open("(");
  out.line("\"", element.name, "\", // Name");
  out.line(description(element), ", // Description");
  out.line("this,");
  out.line("simulator,");
  out.write("&", ownerName(element, gen), "::_", element.name, ")}");
  out.close();
}

void EsaCdkGenerator::declareEventSourceGen(cursiv::TextBuilder& out,
                                            const EventSource& element) const {
  comment(out, element);
  out.write("::esa::ecss::smp::cdk::");
  if (const auto* payload = eventType(element))
    out.write("EventSourceArg<", fqn(payload), ">");
  else
    out.write("EventSourceVoid");
  out.line(" *", element.name, ";");
}

std::vector<CppGenerator::Include> EsaCdkGenerator::headerIncludesEventSource(
    const EventSource& element) const {
  auto result = GapPatternCppGenerator::headerIncludesEventSource(element);
  result.emplace_back(std::string(eventType(element)
                                      ? "esa/ecss/smp/cdk/EventSourceArg.h"
                                      : "esa/ecss/smp/cdk/EventSourceVoid.h"));
  return result;
}

void EsaCdkGenerator::initializeEventSource(cursiv::TextBuilder& out,
                                            const EventSource& element) const {
  out.line("// Event Source: ", element.name);
  out.write(element.name, "{ new ::esa::ecss::smp::cdk::");
  if (const auto* payload = eventType(element))
    out.write("EventSourceArg<", fqn(payload), ">");
  else
    out.write("EventSourceVoid");
  out.open("(");
  out.line("\"", element.name, "\", // Name");
  out.line(description(element), ", // Description");
  out.line("this,");
  out.write("simulator)}");
  out.close();
}

std::vector<CppGenerator::Include> EsaCdkGenerator::headerIncludesComponent(
    const Component& type) const {
  auto result = GapPatternCppGenerator::headerIncludesComponent(type);
  result.emplace_back(std::string("Smp/ISimulator.h"));
  result.emplace_back(std::string("Smp/IComposite.h"));
  result.emplace_back(std::string("Smp/PrimitiveTypes.h"));
  if (!type.base.has_value())
    result.emplace_back("esa/ecss/smp/cdk/" + std::string(componentKind(type))
                        + ".h");
  if (hasInvokableMembers(type)) {
    for (const auto* include :
         {"map", "Smp/IRequest.h", "esa/ecss/smp/cdk/Request.h",
          "esa/ecss/smp/cdk/RequestContainer.h"})
      result.emplace_back(std::string(include));
  }
  if (has<Container>(type))
    result.emplace_back(std::string("esa/ecss/smp/cdk/Composite.h"));
  if (has<Reference>(type))
    result.emplace_back(std::string("esa/ecss/smp/cdk/Aggregate.h"));
  if (has<EventSource>(type))
    result.emplace_back(std::string("esa/ecss/smp/cdk/EventProvider.h"));
  if (has<EventSink>(type))
    result.emplace_back(std::string("esa/ecss/smp/cdk/EventConsumer.h"));
  if (has<EntryPoint>(type))
    result.emplace_back(std::string("esa/ecss/smp/cdk/EntryPointPublisher.h"));
  return result;
}

std::vector<CppGenerator::Include> EsaCdkGenerator::sourceIncludesComponent(
    const Component& type) const {
  auto result = GapPatternCppGenerator::sourceIncludesComponent(type);
  if (hasInvokableMembers(type)) {
    result.emplace_back(std::string("Smp/IPublication.h"));
    result.emplace_back(std::string("esa/ecss/smp/cdk/Request.h"));
  }
  return result;
}

void EsaCdkGenerator::generateComponentHeader(cursiv::TextBuilder& out,
                                              const Component& type) const {
  const auto kind = componentKind(type);
  comment(out, type);
  out.line("class ", type.name, ": public ", type.name, "Gen {");
  out.open("public:");
  out.line("/// Re-use parent constructor");
  out.line("using ", type.name, "Gen::", type.name, "Gen;");
  out.line();
  out.line("/// Virtual destructor to release memory.");
  out.line("~", type.name, "() noexcept override = default;");
  out.close();
  out.line();
  out.open("private:");
  out.line("// ", type.name,
           "Gen call DoPublish/DoConfigure/DoConnect/DoDisconnect");
  out.line("friend class ", fqn(&type), "Gen;");
  out.line();
  out.line("/// Publish fields, operations and properties of the ", kind, ".");
  out.line("/// @param receiver Publication receiver.");
  out.line("void DoPublish(::Smp::IPublication* receiver);");
  out.line();
  out.line("/// Perform any custom configuration of the ", kind, ".");
  out.line(
      "/// @param logger Logger to use for log messages during Configure().");
  out.line("/// @param linkRegistry Link Registry to use for registration of");
  out.line("///         links created during Configure() or later.");
  out.line(
      "void DoConfigure(::Smp::Services::ILogger* logger, "
      "::Smp::Services::ILinkRegistry* linkRegistry);");
  out.line();
  out.line("/// Connect the ", kind, " to the simulator and its simulation");
  out.line("/// services.");
  out.line("/// @param simulator Simulation Environment that hosts the ", kind,
           ".");
  out.line("void DoConnect(::Smp::ISimulator* simulator);");
  out.line();
  out.line("/// Disconnect the ", kind, " from the simulator and all its");
  out.line("/// simulation services.");
  out.line("void DoDisconnect();");
  out.line();
  declareMembers(out, type.elements, Visibility::Private);
  out.close("};");
}

std::string EsaCdkGenerator::componentBase(const Component& type) const {
  return type.base.has_value()
             ? fqn(type.base->get())
             : "::esa::ecss::smp::cdk::" + std::string(componentKind(type));
}

bool EsaCdkGenerator::hasInvokableMembers(const Component& type) const {
  return std::ranges::any_of(
      type.elements, [this](const NamedElement* element) {
        const auto* operation = cursiv::ast_cast<const Operation>(element);
        return operation && isInvokable(*operation);
      });
}

std::optional<std::string> EsaCdkGenerator::notInvokableReason(
    const Operation& operation) const {
  if (auto reason = GapPatternCppGenerator::notInvokableReason(operation))
    return reason;
  if (!getViewKind(operation)) return "it has no @View attribute";
  // A static member has no member pointer to give `AddIfMissing`.
  if (isStaticAttribute(operation)) return "it is static";
  for (const auto* parameter : operation.parameter) {
    if (cursiv::ast_cast<const String>(parameter->type.get()))
      return "its '" + std::string(parameter->name)
             + "' parameter is of a String type, which is not supported for "
               "dynamic invocation";
    if (parameter->direction == "out" || parameter->direction == "inout")
      return "its '" + std::string(parameter->name) + "' "
             + std::string(parameter->direction)
             + " parameter is not supported for dynamic invocation (only 'in' "
               "parameters are supported)";
  }
  return std::nullopt;
}

std::vector<std::string> EsaCdkGenerator::componentBases(
    const Component& type) const {
  auto bases = GapPatternCppGenerator::componentBases(type);
  if (has<Container>(type))
    bases.emplace_back("public virtual ::esa::ecss::smp::cdk::Composite");
  if (has<Reference>(type))
    bases.emplace_back("public virtual ::esa::ecss::smp::cdk::Aggregate");
  if (has<EventSource>(type))
    bases.emplace_back("public virtual ::esa::ecss::smp::cdk::EventProvider");
  if (has<EventSink>(type))
    bases.emplace_back("public virtual ::esa::ecss::smp::cdk::EventConsumer");
  if (has<EntryPoint>(type))
    bases.emplace_back(
        "public virtual ::esa::ecss::smp::cdk::EntryPointPublisher");
  return bases;
}

void EsaCdkGenerator::generateComponentHeaderGen(cursiv::TextBuilder& out,
                                                 const Component& type,
                                                 bool gen) const {
  const auto className = name(type.name, gen);
  const auto invokable = hasInvokableMembers(type);
  openComponentClass(out, type, gen);
  out.open();
  declareComponentInterface(out, type, gen, "() override;");
  if (invokable) {
    out.line("private:");
    out.line(
        "template <typename _Type> static void "
        "PopulateRequestHandlers(_Type* bluePrint, typename "
        "::esa::ecss::smp::cdk::RequestContainer<_Type>::Map& handlers);");
    out.line("static ::esa::ecss::smp::cdk::RequestContainer<", className,
             ">::Map requestHandlers;");
    out.line();
    declareInvoke(out);
  }
  out.line();
  declareMembersGen(out, type.elements, Visibility::Public, gen);
  out.close("};");
  if (!invokable) return;
  out.line();
  out.line("template <typename _Type>");
  out.line("void ", className,
           "::PopulateRequestHandlers(_Type* bluePrint, typename "
           "::esa::ecss::smp::cdk::RequestContainer<_Type>::Map& handlers)");
  out.open("{");
  out.line("typedef ::esa::ecss::smp::cdk::RequestContainer<_Type> Help;");
  out.line();
  out.line("// ---- Operations ----");
  for (const auto* element : type.elements)
    if (const auto* operation = cursiv::ast_cast<const Operation>(element);
        operation && isInvokable(*operation))
      requestHandlerOperation(out, *operation, gen);
  out.line();
  out.line("::esa::ecss::smp::cdk::", componentKind(type),
           "::PopulateRequestHandlers<_Type>(bluePrint, handlers);");
  out.close("}");
}

void EsaCdkGenerator::generateComponentSourceGen(cursiv::TextBuilder& out,
                                                 const Component& type,
                                                 bool gen) const {
  const auto className = name(type.name, gen);
  const auto base = componentBase(type);
  const auto qualifiedName = fqn(&type);
  const auto invokable = hasInvokableMembers(type);
  out.line(
      "//--------------------------- Constructor -------------------------");
  out.open(className, "::", className, "(");
  out.line("::Smp::String8 name,");
  out.line("::Smp::String8 description,");
  out.line("::Smp::IComposite* parent,");
  out.line("::Smp::ISimulator* simulator):");
  out.line("// Base class");
  out.write(base, "(name, description, parent, simulator)");
  initializeMembers(out, type.elements, gen);
  out.line(" {");
  constructMembers(out, type.elements);
  out.close("}");
  out.line();
  out.line(
      "/// Virtual destructor that is called by inherited classes as well.");
  out.open(className, "::~", className, "() {");
  finalizeMembers(out, type.elements);
  out.close("}");
  out.line();
  out.open("void ", className, "::Publish(::Smp::IPublication* receiver) {");
  out.line("// Call parent class implementation first");
  out.line(base, "::Publish(receiver);");
  out.line();
  if (invokable) {
    out.open("if (requestHandlers.empty()) {");
    out.line("PopulateRequestHandlers<", className,
             ">(this, requestHandlers);");
    out.close("}");
    out.line();
  }
  publishMembers(out, type.elements);
  out.line();
  out.line("dynamic_cast<", qualifiedName, "*>(this)->DoPublish(receiver);");
  out.close("}");
  out.line();
  out.open("void ", className,
           "::Configure(::Smp::Services::ILogger* logger, "
           "::Smp::Services::ILinkRegistry* linkRegistry) {");
  out.line("// Call parent implementation first");
  out.line(base, "::Configure(logger, linkRegistry);");
  out.line();
  out.line("dynamic_cast<", qualifiedName,
           "*>(this)->DoConfigure(logger, linkRegistry);");
  out.close("}");
  out.line();
  out.open("void ", className, "::Connect(::Smp::ISimulator* simulator) {");
  out.line("// Call parent implementation first");
  out.line(base, "::Connect(simulator);");
  out.line();
  out.line("dynamic_cast<", qualifiedName, "*>(this)->DoConnect(simulator);");
  out.close("}");
  out.line();
  out.open("void ", className, "::Disconnect() {");
  out.open(
      "if (this->GetState() == ::Smp::ComponentStateKind::CSK_Connected) "
      "{");
  out.line("dynamic_cast<", qualifiedName, "*>(this)->DoDisconnect();");
  out.close("}");
  out.line();
  out.line(
      "// Call parent implementation last, to remove references to the "
      "Simulator and its services");
  out.line(base, "::Disconnect();");
  out.close("}");
  out.line();
  if (invokable) {
    out.line("::esa::ecss::smp::cdk::RequestContainer<", className, ">::Map ",
             className, "::requestHandlers;");
    out.line();
    out.open("void ", className, "::Invoke(::Smp::IRequest* request) {");
    out.open("if (!request) {");
    out.line("return;");
    out.close("}");
    out.line("auto it = requestHandlers.find(request->GetOperationName());");
    out.open("if (it != requestHandlers.end()) {");
    out.line("it->second->Execute(*this, request);");
    out.close("} else {");
    out.line("    // pass the request down to the base class");
    out.line("    ", base, "::Invoke(request);");
    out.line("}");
    out.close("}");
    out.line();
  }
  out.open("const ::Smp::Uuid& ", className, "::GetUuid() const {");
  out.line("return Uuid_", type.name, ";");
  out.close("}");
  out.line();
  defineMembersGen(out, type.elements, gen);
}

void EsaCdkGenerator::requestHandlerOperation(cursiv::TextBuilder& out,
                                              const Operation& op,
                                              bool gen) const {
  const auto* returned = op.returnParameter;
  const auto returnType = type(returned);
  std::string parameterTypes;
  std::string_view separator;
  for (const auto* parameter : op.parameter) {
    parameterTypes += separator;
    parameterTypes += type(parameter);
    separator = ", ";
  }
  const auto owner = ownerName(op, gen);
  out.line("Help::template AddIfMissing<", returnType,
           op.parameter.empty() ? "" : ", ", parameterTypes, ">(");
  out.line("    handlers,");
  out.line("    \"", op.name, "\",");
  out.line("    ", primitiveTypeKind(returned ? returned->type.get() : nullptr),
           ",");
  out.line("    static_cast<", returnType, " (", owner, "::*)(", parameterTypes,
           ")", isConst(op) ? " const" : "", ">(&", owner,
           "::", operationName(op), "));");
}

}  // namespace xsmp
