#include <algorithm>
#include <xsmp/AttributeHelper.hpp>
#include <xsmp/XsmpcatCoreServices.hpp>
#include <xsmp/solver/Solver.hpp>
#include <xsmp/utils/XsmpUtils.hpp>
#include <xsmp/xsmp-sdk/XsmpSdkGenerator.hpp>

namespace xsmp {

namespace {

template <typename T>
bool hasMember(const Component& type) {
  return std::ranges::any_of(type.elements, [](const NamedElement* element) {
    return cursiv::ast_cast<const T>(element) != nullptr;
  });
}

// The `::transient::input::output::forcible::failure` qualifiers of an
// `::Xsmp::Field` type.
std::string fieldQualifiers(const Field& field) {
  std::string result;
  if (!isState(field)) result += "::transient";
  if (isInput(field)) result += "::input";
  if (isOutput(field)) result += "::output";
  if (attributeBoolValue(field, "Attributes.Forcible").value_or(false))
    result += "::forcible";
  if (attributeBoolValue(field, "Attributes.Failure").value_or(false))
    result += "::failure";
  return result;
}

}  // namespace

void XsmpSdkGenerator::registerModel(cursiv::TextBuilder& out,
                                     const Model& model) const {
  out.line("// Register factory for Model ", model.name);
  out.open("simulator->RegisterFactory(::Xsmp::Factory::Create<", fqn(&model),
           ">(");
  out.line("\"", model.name, "\", // Name");
  out.line(description(model), ", // Description");
  out.line("simulator, // Simulator");
  out.line(uuid(&model), " // UUID");
  out.close("));");
}

std::vector<CppGenerator::Include> XsmpSdkGenerator::factoryIncludes() const {
  return {std::string("Xsmp/Factory.h")};
}

void XsmpSdkGenerator::generateArrayHeaderGen(cursiv::TextBuilder& out,
                                              const Array& type,
                                              bool gen) const {
  comment(out, type);
  out.line("using ", name(type.name, gen), " = ::Xsmp::Array<",
           fqn(type.itemType.get()), ", ", expression(type.size), ">",
           isSimpleArrayAttribute(type) ? "::simple" : "", ";");
  out.line();
  uuidDeclaration(out, type);
  out.line();
  out.line("void _Register_", type.name,
           "(::Smp::Publication::ITypeRegistry* registry);");
}

std::vector<CppGenerator::Include> XsmpSdkGenerator::headerIncludesArray(
    const Array& type) const {
  auto result = GapPatternCppGenerator::headerIncludesArray(type);
  result.emplace_back(std::string("Xsmp/Array.h"));
  return result;
}

void XsmpSdkGenerator::generateStringHeaderGen(cursiv::TextBuilder& out,
                                               const String& type,
                                               bool gen) const {
  comment(out, type);
  out.line("using ", name(type.name, gen), " = ::Xsmp::String<",
           expression(type.length), ">;");
  out.line();
  uuidDeclaration(out, type);
  out.line();
  out.line("void _Register_", type.name,
           "(::Smp::Publication::ITypeRegistry* registry);");
}

std::vector<CppGenerator::Include> XsmpSdkGenerator::headerIncludesString(
    const String& type) const {
  auto result = GapPatternCppGenerator::headerIncludesString(type);
  result.emplace_back(std::string("Xsmp/String.h"));
  return result;
}

std::vector<CppGenerator::Include> XsmpSdkGenerator::headerIncludesContainer(
    const Container& element) const {
  auto result = GapPatternCppGenerator::headerIncludesContainer(element);
  result.emplace_back(std::string("Xsmp/Container.h"));
  return result;
}

void XsmpSdkGenerator::declareContainerGen(cursiv::TextBuilder& out,
                                           const Container& element) const {
  comment(out, element);
  out.line("::Xsmp::Container<", fqn(element.type.get()), "> ", element.name,
           ";");
}

void XsmpSdkGenerator::initializeContainer(cursiv::TextBuilder& out,
                                           const Container& element) const {
  out.line("// Container ", element.name);
  out.open(element.name, " {");
  out.line("\"", element.name, "\", // Name");
  out.line(description(element), ", // Description");
  out.line("this, // Parent");
  out.line(lower(element), ", // Lower bound");
  out.line(upper(element), " // Upper bound");
  out.close().write("}");
}

void XsmpSdkGenerator::declareReferenceGen(cursiv::TextBuilder& out,
                                           const Reference& element) const {
  comment(out, element);
  out.line("::Xsmp::Reference<", fqn(element.interface_.get()), "> ",
           element.name, ";");
}

std::vector<CppGenerator::Include> XsmpSdkGenerator::headerIncludesReference(
    const Reference& element) const {
  auto result = GapPatternCppGenerator::headerIncludesReference(element);
  result.emplace_back(std::string("Xsmp/Reference.h"));
  return result;
}

void XsmpSdkGenerator::initializeReference(cursiv::TextBuilder& out,
                                           const Reference& element) const {
  out.line("// Reference ", element.name);
  out.open(element.name, "{");
  out.line("\"", element.name, "\", // Name");
  out.line(description(element), ", // Description");
  out.line("this, // Parent");
  out.line(lower(element), ", // Lower bound");
  out.line(upper(element), " // Upper bound");
  out.close().write("}");
}

void XsmpSdkGenerator::declareEntryPointGen(cursiv::TextBuilder& out,
                                            const EntryPoint& element,
                                            bool gen) const {
  comment(out, element);
  out.line("::Xsmp::EntryPoint ", element.name, ";");
  out.line("virtual void _", element.name, "()", gen ? " = 0" : "", ";");
}

std::vector<CppGenerator::Include> XsmpSdkGenerator::headerIncludesEntryPoint(
    const EntryPoint& /*element*/) const {
  return {std::string("Xsmp/EntryPoint.h")};
}

void XsmpSdkGenerator::initializeEntryPoint(cursiv::TextBuilder& out,
                                            const EntryPoint& element,
                                            bool /*gen*/) const {
  out.line("// EntryPoint ", element.name);
  out.open(element.name, "{");
  out.line("\"", element.name, "\", // Name");
  out.line(description(element), ", // Description");
  out.line("this, // Parent");
  out.line("[this]{this->_", element.name, "();} // Callback");
  out.close().write("}");
}

void XsmpSdkGenerator::declareEventSinkGen(cursiv::TextBuilder& out,
                                           const EventSink& element,
                                           bool gen) const {
  const auto* payload = eventType(element);
  const auto payloadType = fqn(payload);
  comment(out, element);
  out.line("::Xsmp::EventSink<", payloadType, "> ", element.name, ";");
  out.write("virtual void _", element.name, "(::Smp::IObject* sender");
  if (payload) out.write(", ", payloadType);
  out.line(")", gen ? " = 0" : "", ";");
}

std::vector<CppGenerator::Include> XsmpSdkGenerator::headerIncludesEventSink(
    const EventSink& element) const {
  auto result = GapPatternCppGenerator::headerIncludesEventSink(element);
  result.emplace_back(std::string("Xsmp/EventSink.h"));
  return result;
}

void XsmpSdkGenerator::initializeEventSink(cursiv::TextBuilder& out,
                                           const EventSink& element,
                                           bool /*gen*/) const {
  out.line("// EventSink ", element.name);
  out.open(element.name, "{");
  out.line("\"", element.name, "\", // Name");
  out.line(description(element), ", // Description");
  out.line("this, // Parent");
  if (const auto* payload = eventType(element)) {
    out.line("[this](::Smp::IObject *sender, ", fqn(payload),
             " value) {this->_", element.name,
             "(sender, value);}, // Callback");
    out.line(primitiveTypeKind(payload), " // Primitive Type Kind");
  } else {
    out.line("[this](::Smp::IObject *sender) {this->_", element.name,
             "(sender);} // Callback");
  }
  out.close().write("}");
}

void XsmpSdkGenerator::declareEventSourceGen(cursiv::TextBuilder& out,
                                             const EventSource& element) const {
  comment(out, element);
  out.line("::Xsmp::EventSource<", fqn(eventType(element)), "> ", element.name,
           ";");
}

std::vector<CppGenerator::Include> XsmpSdkGenerator::headerIncludesEventSource(
    const EventSource& element) const {
  auto result = GapPatternCppGenerator::headerIncludesEventSource(element);
  result.emplace_back(std::string("Xsmp/EventSource.h"));
  return result;
}

void XsmpSdkGenerator::initializeEventSource(cursiv::TextBuilder& out,
                                             const EventSource& element) const {
  out.line("// EventSource ", element.name);
  out.open(element.name, "{");
  out.line("\"", element.name, "\", // Name");
  out.line(description(element), ", // Description");
  if (const auto* payload = eventType(element)) {
    out.line("this, // Parent");
    out.line(primitiveTypeKind(payload), " // Primitive Type Kind");
  } else {
    out.line("this // Parent");
  }
  out.close().write("}");
}

void XsmpSdkGenerator::generateStructureHeaderGen(cursiv::TextBuilder& out,
                                                  const Structure& type,
                                                  bool gen) const {
  const auto fields = instanceFields(type);
  const auto raw = name(fqn(&type), gen);
  comment(out, type);
  out.open("struct ", name(type.name, gen), " {");
  declareMembersGen(out, type.elements, Visibility::Public, gen);
  out.line();
  out.line(
      "static void _Register(::Smp::Publication::ITypeRegistry* registry);");
  out.line();
  out.open("template<typename _BASE> struct _Field : public _BASE {");
  out.write(R"(_Field(::Smp::Publication::ITypeRegistry *typeRegistry,
       ::Smp::Uuid typeUuid, ::Smp::String8 name,
       ::Smp::String8 description = "", ::Smp::IObject *parent = nullptr,
       ::Smp::ViewKind view = ::Smp::ViewKind::VK_All)
    : _Field(typeRegistry, typeUuid, name, description, parent, view,
)");
  out.line("             ", raw, "{}) {}");
  out.write(
      R"(_Field(::Smp::Publication::ITypeRegistry *typeRegistry, ::Smp::Uuid typeUuid,
       ::Smp::String8 name, ::Smp::String8 description, ::Smp::IObject *parent,
)");
  out.open("       ::Smp::ViewKind view, const ", raw, " &value) :");
  out.write("_BASE(typeRegistry, typeUuid, name, description, parent, view)");
  for (const auto* field : fields) {
    out.line(",");
    out.line("// Field ", field->name);
    out.open(field->name, "{");
    out.line("typeRegistry, // Type Registry");
    out.line(uuid(field->type.get()), ", //Type UUID");
    out.line("\"", field->name, "\", // Name");
    out.line(description(*field), ", // Description");
    out.line("this, // Parent");
    out.line(viewKind(*field, "view"), ", // View Kind");
    out.line("value.", field->name, " // Value");
    out.close().write("}");
  }
  out.line();
  out.close("{");
  out.line("}");
  out.line("_Field(const _Field&) = delete;");
  out.line("_Field& operator=(const _Field&) = delete;");
  out.line();
  out.line("// copy operator from ", raw);
  out.line("_Field & operator=(const ", raw, " &other) {");
  for (const auto* field : fields)
    out.line("    this->", field->name, " = other.", field->name, ";");
  out.line("    return *this;");
  out.line("}");
  out.line();
  out.line("// implicit convertion to ", raw);
  out.line("operator ", raw, "() const noexcept {");
  out.line("    return {", cursiv::joined(fields, ", ", &Field::name), "};");
  out.line("}");
  out.line();
  if (!fields.empty()) out.line("// Fields declaration");
  for (const auto* field : fields) {
    comment(out, *field);
    out.line(isMutable(*field) ? "mutable " : "",
             "typename _BASE::template Field<", fqn(field->type.get()), ">",
             fieldQualifiers(*field), " ", field->name, ";");
  }
  out.close("};");
  out.close("};");
  out.line();
  uuidDeclaration(out, type);
}

void XsmpSdkGenerator::generateEnumerationSourceGen(
    cursiv::TextBuilder& out, const Enumeration& type) const {
  out.open("void _Register_", type.name,
           "(::Smp::Publication::ITypeRegistry* registry) {");
  out.open("auto *type = registry->AddEnumerationType(");
  out.line("\"", type.name, "\", // Name");
  out.line(description(type), ", // Description");
  out.line(uuid(&type), " // UUID");
  out.close("#if ECSS_SMP_VERSION < 202503L");
  out.line("    , sizeof(", fqn(&type), ") // Size");
  out.line("#endif");
  out.line(");");
  out.line();
  addLiterals(out, type);
  out.close("}");
}

void XsmpSdkGenerator::sourcePrologue(cursiv::TextBuilder& out,
                                      const Type& type) const {
  const auto* component = cursiv::ast_cast<const Component>(&type);
  if (cursiv::ast_cast<const Enumeration>(&type)
      || (component && hasInvokableMembers(*component)))
    smpVersionPrologue(out);
}

std::vector<CppGenerator::Include> XsmpSdkGenerator::sourceIncludesComponent(
    const Component& type) const {
  auto result = GapPatternCppGenerator::sourceIncludesComponent(type);
  result.emplace_back(std::string("Xsmp/ComponentHelper.h"));
  if (hasInvokableMembers(type)) {
    result.emplace_back(std::string("Xsmp/Request.h"));
    result.emplace_back(std::string("string"));
  }
  return result;
}

std::vector<CppGenerator::Include> XsmpSdkGenerator::headerIncludesComponent(
    const Component& type) const {
  auto result = GapPatternCppGenerator::headerIncludesComponent(type);
  result.emplace_back(std::string("Smp/ISimulator.h"));
  result.emplace_back(std::string("Smp/IComposite.h"));
  result.emplace_back(std::string("Smp/PrimitiveTypes.h"));
  if (!type.base.has_value())
    result.emplace_back("Xsmp/" + std::string(componentKind(type)) + ".h");
  if (hasInvokableMembers(type)) {
    for (const auto* include :
         {"map", "Smp/IRequest.h", "functional", "string_view"})
      result.emplace_back(std::string(include));
  }
  if (hasMember<Container>(type))
    result.emplace_back(std::string("Xsmp/Composite.h"));
  if (hasMember<Reference>(type))
    result.emplace_back(std::string("Xsmp/Aggregate.h"));
  if (hasMember<EventSource>(type))
    result.emplace_back(std::string("Xsmp/EventProvider.h"));
  if (hasMember<EventSink>(type))
    result.emplace_back(std::string("Xsmp/EventConsumer.h"));
  if (hasMember<EntryPoint>(type))
    result.emplace_back(std::string("Xsmp/EntryPointPublisher.h"));
  if (hasFailureField(type))
    result.emplace_back(std::string("Xsmp/FallibleModel.h"));
  return result;
}

void XsmpSdkGenerator::generateComponentHeader(cursiv::TextBuilder& out,
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
  out.line();
  out.close("private:");
  out.line(
      "    // visibility to call DoPublish / DoConfigure / DoConnect / "
      "DoDisconnect");
  out.line("    friend class ::Xsmp::Component::Helper;");
  out.line();
  out.line("    /// Publish fields, operations and properties of the ", kind,
           ".");
  out.line("    /// @param receiver Publication receiver.");
  out.line("    void DoPublish(::Smp::IPublication* receiver);");
  out.line();
  out.line("    /// Perform any custom configuration of the ", kind, ".");
  out.line(
      "    /// @param logger Logger to use for log messages during "
      "Configure().");
  out.line(
      "    /// @param linkRegistry Link Registry to use for registration of");
  out.line("    ///         links created during Configure() or later.");
  out.line(
      "    void DoConfigure(::Smp::Services::ILogger* logger, "
      "::Smp::Services::ILinkRegistry* linkRegistry);");
  out.line();
  out.line("    /// Connect the ", kind,
           " to the simulator and its simulation");
  out.line("    /// services.");
  out.line("    /// @param simulator Simulation Environment that hosts the ",
           kind, ".");
  out.line("    void DoConnect(::Smp::ISimulator* simulator);");
  out.line();
  out.line("    /// Disconnect the ", kind, " from the simulator and all its");
  out.line("    /// simulation services.");
  out.line("    void DoDisconnect();");
  out.line();
  out.open();
  declareMembers(out, type.elements, Visibility::Private);
  out.close("};");
}

void XsmpSdkGenerator::generateComponentHeaderGen(cursiv::TextBuilder& out,
                                                  const Component& type,
                                                  bool gen) const {
  openComponentClass(out, type, gen);
  out.open();
  declareComponentInterface(out, type, gen, "() override = default;");
  if (hasInvokableMembers(type)) {
    out.line("private:");
    out.line("static std::map<std::string_view, std::function<void(",
             name(type.name, gen), "*, ::Smp::IRequest*)>> _requestHandlers;");
    out.line();
    declareInvoke(out);
  }
  out.line();
  declareMembersGen(out, type.elements, Visibility::Public, gen);
  out.close("};");
}

std::string XsmpSdkGenerator::componentBase(const Component& type) const {
  return type.base.has_value() ? fqn(type.base->get())
                               : "::Xsmp::" + std::string(componentKind(type));
}

std::vector<std::string> XsmpSdkGenerator::componentBases(
    const Component& type) const {
  auto bases = GapPatternCppGenerator::componentBases(type);
  if (hasMember<Container>(type))
    bases.emplace_back("public virtual ::Xsmp::Composite");
  if (hasMember<Reference>(type))
    bases.emplace_back("public virtual ::Xsmp::Aggregate");
  if (hasMember<EventSource>(type))
    bases.emplace_back("public virtual ::Xsmp::EventProvider");
  if (hasMember<EventSink>(type))
    bases.emplace_back("public virtual ::Xsmp::EventConsumer");
  if (hasMember<EntryPoint>(type))
    bases.emplace_back("public virtual ::Xsmp::EntryPointPublisher");
  if (hasFailureField(type))
    bases.emplace_back("public virtual ::Xsmp::FallibleModel");
  return bases;
}

void XsmpSdkGenerator::generateComponentSourceGen(cursiv::TextBuilder& out,
                                                  const Component& type,
                                                  bool gen) const {
  const auto className = name(type.name, gen);
  const auto base = componentBase(type);
  const auto typeFqn = fqn(&type);
  out.line(
      "//--------------------------- Constructor -------------------------");
  out.line(className, "::", className, "(");
  out.line("    ::Smp::String8 name,");
  out.line("    ::Smp::String8 description,");
  out.line("    ::Smp::IComposite* parent,");
  out.line("    ::Smp::ISimulator* simulator):");
  out.open("    // Base class");
  out.write(base, "(name, description, parent, simulator)");
  initializeMembers(out, type.elements, gen);
  out.line(" { }");
  out.close();
  out.line();
  out.line("void ", className, "::Publish(::Smp::IPublication* receiver) {");
  out.line("    // Call parent class implementation first");
  out.line("    ", base, "::Publish(receiver);");
  out.line();
  out.open();
  publishMembers(out, type.elements);
  out.line();
  out.close("    // Call user DoPublish if any");
  out.line("    ::Xsmp::Component::Helper::Publish<", typeFqn,
           ">(this, receiver);");
  out.line("}");
  out.line();
  out.open("void ", className,
           "::Configure(::Smp::Services::ILogger* logger, "
           "::Smp::Services::ILinkRegistry* linkRegistry) {");
  out.line("// Call parent implementation first");
  out.line(base, "::Configure(logger, linkRegistry);");
  out.line();
  out.line("// Call user DoConfigure if any");
  out.line("::Xsmp::Component::Helper::Configure<", typeFqn,
           ">(this, logger, linkRegistry);");
  out.close("}");
  out.line();
  out.open("void ", className, "::Connect(::Smp::ISimulator* simulator) {");
  out.line("// Call parent implementation first");
  out.line(base, "::Connect(simulator);");
  out.line();
  out.line("// Call user DoConnect if any");
  out.line("::Xsmp::Component::Helper::Connect<", typeFqn,
           ">(this, simulator);");
  out.close("}");
  out.line();
  out.open("void ", className, "::Disconnect() {");
  out.open(
      "if (this->::Xsmp::Component::GetState() == "
      "::Smp::ComponentStateKind::CSK_Connected) {");
  out.line("// Call user DoDisconnect if any");
  out.line("::Xsmp::Component::Helper::Disconnect<", typeFqn, ">(this);");
  out.close("}");
  out.line();
  out.line(
      "// Call parent implementation last, to remove references to the "
      "Simulator and its services");
  out.line(base, "::Disconnect();");
  out.close("}");
  out.line();
  if (hasInvokableMembers(type))
    defineRequestHandlers(out, type, className, base, gen);
  out.open("const ::Smp::Uuid& ", className, "::GetUuid() const {");
  out.line("return Uuid_", type.name, ";");
  out.close("}");
  defineMembersGen(out, type.elements, gen);
}

void XsmpSdkGenerator::defineRequestHandlers(cursiv::TextBuilder& out,
                                             const Component& type,
                                             std::string_view className,
                                             std::string_view base,
                                             bool gen) const {
  out.open("std::map<std::string_view, std::function<void(", className,
           "*, ::Smp::IRequest*)>> ", className, "::_requestHandlers{");
  for (const auto* element : type.elements)
    if (const auto* operation = cursiv::ast_cast<const Operation>(element);
        operation && isInvokable(*operation)) {
      requestHandlerOperation(out, *operation, gen);
      out.line();
    }
  for (const auto* element : type.elements)
    if (const auto* property = cursiv::ast_cast<const Property>(element);
        property && isInvokable(*property)) {
      requestHandlerProperty(out, *property, gen);
      out.line();
    }
  out.close("};");
  out.line();
  out.line("void ", className, "::Invoke(::Smp::IRequest* request) {");
  out.write(R"(    if (!request) {
        return;
    }
#if ECSS_SMP_VERSION >= 202503L
    // the request carries the bare property name and tells a getter
    // from a setter apart through its type
    const std::string key =
            (request->GetType() == ::Smp::RequestType::RT_Get ? "get_"
             : request->GetType() == ::Smp::RequestType::RT_Set ? "set_"
             : "") + std::string{request->GetName()};
#else
    // the request already names the property accessor
    const std::string_view key{request->GetOperationName()};
#endif
    if (auto it = _requestHandlers.find(key);
            it != _requestHandlers.end()) {
        it->second(this, request);
    } else {
        // pass the request down to the base class
)");
  out.line("        ", base, "::Invoke(request);");
  out.line("    }");
  out.line("}");
  out.line();
}

void XsmpSdkGenerator::requestHandlerOperation(cursiv::TextBuilder& out,
                                               const Operation& op,
                                               bool gen) const {
  const auto* returned = op.returnParameter;
  out.line("// Handler for Operation ", op.name);
  out.line("{\"", op.name, "\",");
  out.open("[](", ownerName(op, gen), "* component, ::Smp::IRequest *",
           returned || !op.parameter.empty() ? "request" : "", ") {");
  for (const auto* parameter : op.parameter) initParameter(out, *parameter);
  out.line();
  if (returned)
    out.write("::Xsmp::Request::setReturnValue(request, ",
              primitiveTypeKind(returned->type.get()), ", ");
  out.write("component->", operationName(op), "(");
  std::string_view separator;
  for (const auto* parameter : op.parameter) {
    out.write(separator);
    separator = ", ";
    if (isOptionalPointer(*parameter)) {
      out.write("has_", parameter->name, " ? &p_", parameter->name,
                " : nullptr");
      continue;
    }
    out.write(isByPointer(*parameter) ? "&" : "");
    if (isString8(parameter->type.get()))
      out.write("static_cast<", fqn(parameter->type.get()), ">(p_",
                parameter->name, ")");
    else
      out.write("p_", parameter->name);
  }
  out.line(returned ? "));" : ");");
  for (const auto* parameter : op.parameter) setParameter(out, *parameter);
  out.close("}},");
}

void XsmpSdkGenerator::initParameter(cursiv::TextBuilder& out,
                                     const Parameter& parameter) const {
  const auto* parameterType = parameter.type.get();
  if (isOptionalPointer(parameter)) {
    const std::string_view name = parameter.name;
    const auto keepString =
        isString8(parameterType) && parameter.direction != "out";
    const auto read = [&](std::string_view type) {
      return "::Xsmp::Request::get<" + std::string(type)
             + ">(component, request, \"" + std::string(name) + "\", "
             + primitiveTypeKind(parameterType) + ")";
    };
    out.line("const bool has_", name, " = request->GetParameterIndex(\"", name,
             "\") >= 0;");
    out.line(fqn(parameterType), " p_", name, "{};");
    if (parameter.direction == "out") return;
    if (keepString) out.line("::Smp::AnySimple storage_", name, ";");
    out.open("if (has_", name, ") {");
    if (keepString) {
      out.line("storage_", name, " = ", read("::Smp::AnySimple"), ";");
      out.line("p_", name, " = static_cast<", fqn(parameterType), ">(storage_",
               name, ");");
    } else {
      out.line("p_", name, " = ", read(fqn(parameterType)), ";");
    }
    out.close("}");
    return;
  }
  if (parameter.direction == "out") {
    out.line(fqn(parameterType), " p_", parameter.name,
             directListInitializer(parameter.default_), ";");
    return;
  }
  if (isString8(parameterType)) {
    out.line("auto p_", parameter.name,
             " = request->GetParameterValue(request->GetParameterIndex(\"",
             parameter.name, "\"));");
    return;
  }
  out.write("auto p_", parameter.name, " = ::Xsmp::Request::get<",
            fqn(parameterType), ">(component, request, \"", parameter.name,
            "\", ", primitiveTypeKind(parameterType));
  if (parameter.default_) out.write(", ", expression(parameter.default_));
  out.line(");");
}

void XsmpSdkGenerator::setParameter(cursiv::TextBuilder& out,
                                    const Parameter& parameter) const {
  if (parameter.direction != "out" && parameter.direction != "inout") return;
  const auto optional = isOptionalPointer(parameter);
  out.line(optional ? "if (has_" : "", optional ? parameter.name : "",
           optional ? ") { " : "",
           "::Xsmp::Request::set(component, request, \"", parameter.name,
           "\", ", primitiveTypeKind(parameter.type.get()), ", p_",
           parameter.name, ");", optional ? " }" : "");
}

void XsmpSdkGenerator::requestHandlerProperty(cursiv::TextBuilder& out,
                                              const Property& property,
                                              bool gen) const {
  const auto owner = ownerName(property, gen);
  const auto kind = primitiveTypeKind(property.type.get());
  const auto access = getAccessKind(property).value_or(AccessKind::ReadWrite);
  if (access != AccessKind::WriteOnly) {
    out.line("// Getter handler for Property ", property.name);
    out.line("{\"get_", property.name, "\",");
    out.open("[](", owner, "* component, ::Smp::IRequest *request) {");
    out.line("::Xsmp::Request::setReturnValue(request, ", kind,
             ", component->get_", property.name, "());");
    out.close("}},");
    out.line();
  }
  if (access != AccessKind::ReadOnly) {
    out.line("// Setter handler for Property ", property.name);
    out.line("{\"set_", property.name, "\",");
    out.open("[](", owner, "* component, ::Smp::IRequest *request) {");
    // A reference binds to a local. A String8 points into the value of the
    // request, which a local keeps.
    const auto byReference = isByReference(property);
    const auto keepString = isString8(property.type.get());
    const auto read = "::Xsmp::Request::get<"
                      + (keepString ? std::string("::Smp::AnySimple")
                                    : fqn(property.type.get()))
                      + ">(component, request, \"" + std::string(property.name)
                      + "\", " + kind + ")";
    if (keepString) {
      out.line("const auto storage = ", read, ";");
      out.line("auto value = static_cast<", fqn(property.type.get()),
               ">(storage);");
    } else if (byReference) {
      out.line("auto value = ", read, ";");
    }
    out.line("component->set_", property.name, "(",
             byReference || keepString ? std::string("value") : read, ");");
    out.close("}},");
  }
}

std::optional<std::string> XsmpSdkGenerator::notInvokableReason(
    const Operation& operation) const {
  if (operation.returnParameter
      && isString8(operation.returnParameter->type.get()))
    return "its return type is Smp.String8 (its memory ownership cannot be "
           "determined, so it cannot be safely deallocated)";
  for (const auto* parameter : operation.parameter)
    if (isString8(parameter->type.get()) && !parameter->direction.empty()
        && parameter->direction != "in")
      return "its '" + std::string(parameter->name) + "' "
             + std::string(parameter->direction)
             + " parameter is of type Smp.String8 (its memory ownership cannot "
               "be determined, so it cannot be safely deallocated)";
  return GapPatternCppGenerator::notInvokableReason(operation);
}

bool XsmpSdkGenerator::hasInvokableMembers(const Component& type) const {
  for (const auto* element : type.elements) {
    if (const auto* operation = cursiv::ast_cast<const Operation>(element);
        operation && isInvokable(*operation))
      return true;
    if (const auto* property = cursiv::ast_cast<const Property>(element);
        property && isInvokable(*property))
      return true;
  }
  return false;
}

bool XsmpSdkGenerator::hasFailureField(const Component& type) {
  if (!cursiv::ast_cast<const Model>(&type)) return false;
  return std::ranges::any_of(type.elements, [](const NamedElement* element) {
    const auto* field = cursiv::ast_cast<const Field>(element);
    return field
           && attributeBoolValue(*field, "Attributes.Failure").value_or(false);
  });
}

bool XsmpSdkGenerator::isCdkField(const Field& field) const {
  if (!cursiv::ast_cast<const Component>(field.parent)
      || isStaticAttribute(field)
      || cursiv::ast_cast<const Class>(field.type.get()))
    return false;
  return isOutput(field)
         || attributeBoolValue(field, "Attributes.Failure").value_or(false)
         || attributeBoolValue(field, "Attributes.Forcible").value_or(false);
}

std::vector<CppGenerator::Include> XsmpSdkGenerator::headerIncludesField(
    const Field& field) const {
  auto result = GapPatternCppGenerator::headerIncludesField(field);
  if (isCdkField(field)) result.emplace_back(std::string("Xsmp/Field.h"));
  return result;
}

void XsmpSdkGenerator::declareFieldGen(cursiv::TextBuilder& out,
                                       const Field& field, bool gen) const {
  if (!isCdkField(field)) {
    GapPatternCppGenerator::declareFieldGen(out, field, gen);
    return;
  }
  comment(out, field);
  out.line(isMutable(field) ? "mutable " : "", "::Xsmp::Field<",
           fqn(field.type.get()), ">", fieldQualifiers(field), " ", field.name,
           ";");
}

void XsmpSdkGenerator::initializeField(cursiv::TextBuilder& out,
                                       const Field& field) const {
  if (!isCdkField(field)) {
    GapPatternCppGenerator::initializeField(out, field);
    return;
  }
  const auto view = viewKind(field);
  out.line("// Field ", field.name);
  out.open(field.name, "{");
  out.line("simulator->GetTypeRegistry(), // Type Registry");
  out.line(uuid(field.type.get()), ", // Type UUID");
  out.line("\"", field.name, "\", // Name");
  out.line(description(field), ", // Description");
  out.line("this, // Parent");
  if (field.default_) {
    out.line(view, ", // View Kind");
    // That of an array goes to the constructor of its field in braces.
    out.line(cursiv::ast_cast<const Array>(field.type.get())
                 ? directListInitializer(field.default_)
                 : expression(field.default_),
             " // Default value");
  } else {
    out.line(view, " // View Kind");
  }
  out.close().write("}");
}

}  // namespace xsmp
