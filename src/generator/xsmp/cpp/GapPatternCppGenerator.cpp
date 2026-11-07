#include <algorithm>
#include <limits>
#include <ranges>
#include <xsmp/AttributeHelper.hpp>
#include <xsmp/XsmpcatCoreServices.hpp>
#include <xsmp/cpp/GapPatternCppGenerator.hpp>
#include <xsmp/solver/Solver.hpp>
#include <xsmp/utils/FileWriter.hpp>
#include <xsmp/utils/TreeUtils.hpp>
#include <xsmp/utils/XsmpUtils.hpp>

namespace xsmp {

namespace {

constexpr std::string_view bannerSeparator =
    "// -----------------------------------------------------------------------"
    "------";

std::string_view visibilityKeyword(Visibility visibility) {
  switch (visibility) {
    case Visibility::Public:
      return "public";
    case Visibility::Protected:
      return "protected";
    case Visibility::Private:
      return "private";
  }
  return "private";
}

// Writes a visibility label before a member declaration whenever the visibility
// changes.
class MemberList {
 public:
  MemberList(cursiv::TextBuilder& out, Visibility initial)
      : _out(out), _visibility(initial) {}

  void next(const NamedElement& element) {
    const auto visibility = getRealVisibility(element);
    if (visibility != _visibility) {
      _out.line(visibilityKeyword(visibility), ":");
    }
    _visibility = visibility;
  }

 private:
  cursiv::TextBuilder& _out;
  Visibility _visibility;
};

// The elements of kind `T`, in their order.
template <class T>
auto ofKind(std::span<NamedElement* const> elements) {
  return elements | std::views::filter([](const NamedElement* element) {
           return cursiv::ast_cast<const T>(element) != nullptr;
         })
         | std::views::transform([](const NamedElement* element) {
             return cursiv::ast_cast<const T>(element);
           });
}

// The fields that `_Register` adds.
std::vector<const Field*> registeredFields(const Structure& type) {
  std::vector<const Field*> fields;
  for (const auto* element : type.elements)
    if (const auto* field = cursiv::ast_cast<const Field>(element);
        field && !isStaticAttribute(*field) && !isString8(field->type.get()))
      fields.push_back(field);
  return fields;
}

// Whether the hand-edited class overrides a Property: it is virtual, not
// abstract and has no attached field.
bool isOverridden(const Property& property) {
  return !property.attachedField.has_value() && isVirtual(property)
         && !isAbstract(property);
}

// Whether the hand-edited `.cpp` defines `element`.
bool hasDefinition(const NamedElement& element) {
  if (const auto* property = cursiv::ast_cast<const Property>(&element))
    return isOverridden(*property);
  if (const auto* operation = cursiv::ast_cast<const Operation>(&element))
    return !isConstructorAttribute(*operation) && !isAbstract(*operation);
  return cursiv::ast_cast<const EntryPoint>(&element)
         || cursiv::ast_cast<const EventSink>(&element);
}

// Whether a type has a hand-edited `.h`, a regenerated `.h` and a regenerated
// `.cpp`. Primitive, event and attribute types have no code.
bool hasHeader(const Type& type) {
  return cursiv::ast_cast<const LanguageType>(&type)
         && !cursiv::ast_cast<const PrimitiveType>(&type);
}

bool hasHeaderGen(const Type& type) {
  // Without a `@type` there is nothing to generate: the validator reports it.
  if (cursiv::ast_cast<const NativeType>(&type))
    return docTag(type, "type").has_value();
  return hasHeader(type);
}

bool hasSourceGen(const Type& type) {
  // Only the String constants of an Interface have to be defined somewhere: any
  // other is `constexpr`, and a String constant would lose its value.
  if (const auto* interface = cursiv::ast_cast<const Interface>(&type))
    return std::ranges::any_of(
        interface->elements, [](const NamedElement* element) {
          const auto* constant = cursiv::ast_cast<const Constant>(element);
          return constant
                 && cursiv::ast_cast<const String>(constant->type.get());
        });
  return hasHeader(type) && !cursiv::ast_cast<const NativeType>(&type)
         && !cursiv::ast_cast<const ValueReference>(&type);
}

}  // namespace

void GapPatternCppGenerator::generatePackage(
    const Catalogue& catalogue, const std::filesystem::path& projectDir) const {
  generatePackageFiles(catalogue, projectDir, includeGenFolder,
                       sourceGenFolder);
}

bool GapPatternCppGenerator::useGenerationGapPattern(const Type& type) const {
  return cursiv::ast_cast<const ReferenceType>(&type) != nullptr;
}

std::string GapPatternCppGenerator::name(std::string_view baseName, bool gen) {
  return gen ? std::string(baseName) + "Gen" : std::string(baseName);
}

std::string GapPatternCppGenerator::ownerName(const cursiv::AstNode& node,
                                              bool gen) {
  const auto* owner = cursiv::ast_cast<const NamedElement>(node.parent);
  return name(owner ? std::string_view(owner->name) : std::string_view{}, gen);
}

std::vector<const Field*> GapPatternCppGenerator::instanceFields(
    const Structure& type) {
  std::vector<const Field*> fields;
  for (const auto* member : type.elements)
    if (const auto* field = cursiv::ast_cast<const Field>(member);
        field && !isStaticAttribute(*field))
      fields.push_back(field);
  return fields;
}

void GapPatternCppGenerator::generateType(
    const Type& type, const std::filesystem::path& projectDir) const {
  const auto relativePath = qualifiedPath(&type, "/");

  const auto includePath = projectDir / includeFolder / (relativePath + ".h");
  const auto includePathExist = fileExists(includePath);
  const auto useSplit = includePathExist || useGenerationGapPattern(type);
  cursiv::TextBuilder out;

  // The hand-edited header is written once, when it does not exist yet.
  if (useSplit && !includePathExist && hasHeader(type)) {
    writeHeaderFile(out, type);
    generateUserFile(projectDir, includeFolder, relativePath + ".h", out);
  }

  // The hand-edited source is written once too, whether the type is split or
  // not, so that class method bodies exist in both layouts.
  if (hasSource(type)
      && !fileExists(projectDir / sourceFolder / (relativePath + ".cpp"))) {
    writeSourceFile(out, type, useSplit);
    generateUserFile(projectDir, sourceFolder, relativePath + ".cpp", out);
  }

  const auto qualifiedNameGen = name(relativePath, useSplit);
  if (hasHeaderGen(type)) {
    writeHeaderGenFile(out, type, useSplit);
    generateFile(projectDir, includeGenFolder, qualifiedNameGen + ".h", out);
  }
  if (hasSourceGen(type)) {
    writeSourceGenFile(out, type, useSplit);
    generateFile(projectDir, sourceGenFolder, qualifiedNameGen + ".cpp", out);
  }
}

void GapPatternCppGenerator::generateHeaderBody(cursiv::TextBuilder& out,
                                                const Type& type) const {
  if (const auto* classType = cursiv::ast_cast<const Class>(&type))
    return generateClassHeader(out, *classType);
  if (const auto* structure = cursiv::ast_cast<const Structure>(&type))
    return generateStructureHeader(out, *structure);
  if (const auto* component = cursiv::ast_cast<const Component>(&type))
    return generateComponentHeader(out, *component);
  if (const auto* interface = cursiv::ast_cast<const Interface>(&type))
    return generateInterfaceHeader(out, *interface);
  if (cursiv::ast_cast<const Array>(&type)) {
    comment(out, type);
    out.line("using ", type.name, " = ::", type.name, "Gen;");
    return;
  }
  generateTypeHeader(out, type);
}

void GapPatternCppGenerator::generateSourceBody(cursiv::TextBuilder& out,
                                                const Type& type,
                                                bool gen) const {
  if (const auto* classType = cursiv::ast_cast<const Class>(&type))
    defineMembers(out, classType->elements, gen);
  else
    generateComponentSource(out, static_cast<const Component&>(type), gen);
}

void GapPatternCppGenerator::generateHeaderGenBody(cursiv::TextBuilder& out,
                                                   const Type& type,
                                                   bool gen) const {
  if (const auto* classType = cursiv::ast_cast<const Class>(&type))
    return generateClassHeaderGen(out, *classType, gen);
  if (const auto* structure = cursiv::ast_cast<const Structure>(&type))
    return generateStructureHeaderGen(out, *structure, gen);
  if (const auto* integer = cursiv::ast_cast<const Integer>(&type))
    return generateIntegerHeaderGen(out, *integer, gen);
  if (const auto* floating = cursiv::ast_cast<const Float>(&type))
    return generateFloatHeaderGen(out, *floating, gen);
  if (const auto* component = cursiv::ast_cast<const Component>(&type))
    return generateComponentHeaderGen(out, *component, gen);
  if (const auto* interface = cursiv::ast_cast<const Interface>(&type))
    return generateInterfaceHeaderGen(out, *interface, gen);
  if (const auto* array = cursiv::ast_cast<const Array>(&type))
    return generateArrayHeaderGen(out, *array, gen);
  if (const auto* enumeration = cursiv::ast_cast<const Enumeration>(&type))
    return generateEnumerationHeaderGen(out, *enumeration, gen);
  if (const auto* string = cursiv::ast_cast<const String>(&type))
    return generateStringHeaderGen(out, *string, gen);
  if (const auto* native = cursiv::ast_cast<const NativeType>(&type))
    return generateNativeTypeHeaderGen(out, *native, gen);
  generateValueReferenceHeaderGen(out, static_cast<const ValueReference&>(type),
                                  gen);
}

void GapPatternCppGenerator::generateSourceGenBody(cursiv::TextBuilder& out,
                                                   const Type& type,
                                                   bool gen) const {
  if (const auto* classType = cursiv::ast_cast<const Class>(&type))
    return generateClassSourceGen(out, *classType, gen);
  if (const auto* structure = cursiv::ast_cast<const Structure>(&type))
    return generateStructureSourceGen(out, *structure, gen);
  if (const auto* integer = cursiv::ast_cast<const Integer>(&type))
    return generateIntegerSourceGen(out, *integer);
  if (const auto* floating = cursiv::ast_cast<const Float>(&type))
    return generateFloatSourceGen(out, *floating);
  if (const auto* component = cursiv::ast_cast<const Component>(&type))
    return generateComponentSourceGen(out, *component, gen);
  if (const auto* interface = cursiv::ast_cast<const Interface>(&type))
    return generateInterfaceSourceGen(out, *interface, gen);
  if (const auto* array = cursiv::ast_cast<const Array>(&type))
    return generateArraySourceGen(out, *array);
  if (const auto* enumeration = cursiv::ast_cast<const Enumeration>(&type))
    return generateEnumerationSourceGen(out, *enumeration);
  generateStringSourceGen(out, static_cast<const String&>(type));
}

void GapPatternCppGenerator::writeHeaderFile(cursiv::TextBuilder& out,
                                             const Type& type) const {
  const auto path = qualifiedPath(&type, "/");
  const auto guard = includeGuard(qualifiedPath(&type, "_"));
  notice(out, type);
  fileBanner(out, std::string(type.name) + ".h", bannerSeparator);
  out.line("/// @file ", path, ".h");
  out.line();
  out.line("#ifndef ", guard);
  out.line("#define ", guard);
  out.line();
  out.line("// Include the generated header file");
  out.line("#include <", path, "Gen.h>");
  out.line();
  openNamespace(out, type);
  generateHeaderBody(out, type);
  closeNamespace(out, type);
  out.line();
  out.line("#endif // ", guard);
}

void GapPatternCppGenerator::writeSourceFile(cursiv::TextBuilder& out,
                                             const Type& type, bool gen) const {
  const auto path = qualifiedPath(&type, "/");
  notice(out, type);
  fileBanner(out, std::string(type.name) + ".cpp", bannerSeparator);
  out.line("/// @file ", path, ".cpp");
  out.line();
  out.line("#include <", path, ".h>");
  std::span<NamedElement* const> elements;
  if (const auto* structure = cursiv::ast_cast<const Structure>(&type))
    elements = structure->elements;
  else if (const auto* component = cursiv::ast_cast<const Component>(&type))
    elements = component->elements;
  else if (const auto* interface = cursiv::ast_cast<const Interface>(&type))
    elements = interface->elements;
  // The types that the signatures only declare, and what the stubs that
  // cannot return a reference throw.
  std::vector<Include> signatures;
  auto throws = false;
  for (const auto* element : elements) {
    if (const auto* op = cursiv::ast_cast<const Operation>(element)) {
      if (const auto* returned = op->returnParameter) {
        appendTo(signatures, sourceIncludesParameter(*returned));
        throws = throws || isByReference(*returned);
      }
      for (const auto* parameter : op->parameter)
        appendTo(signatures, sourceIncludesParameter(*parameter));
    } else if (const auto* property =
                   cursiv::ast_cast<const Property>(element)) {
      throws = throws || isByReference(*property);
    }
  }
  includes(out, signatures, {&type});
  if (throws) out.line("#include <stdexcept>");
  out.line();
  openNamespace(out, type);
  generateSourceBody(out, type, gen);
  closeNamespace(out, type);
}

void GapPatternCppGenerator::writeHeaderGenFile(cursiv::TextBuilder& out,
                                                const Type& type,
                                                bool gen) const {
  const auto guard = includeGuard(name(qualifiedPath(&type, "_"), gen));
  notice(out, type);
  fileBanner(out, name(type.name, gen) + ".h", bannerSeparator);
  out.line("/// @file ", name(qualifiedPath(&type, "/"), gen), ".h");
  out.line(autoGeneratedLine);
  out.line();
  out.line("#ifndef ", guard);
  out.line("#define ", guard);
  out.line();
  includes(out, headerIncludesWithoutCycle(type), {&type});
  out.write(
      R"(// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

)");
  openNamespace(out, type);
  generateHeaderGenBody(out, type, gen);
  closeNamespace(out, type);
  out.line();
  out.line("#endif // ", guard);
}

void GapPatternCppGenerator::writeSourceGenFile(cursiv::TextBuilder& out,
                                                const Type& type,
                                                bool gen) const {
  auto includeList = sourceIncludes(type);
  includeList.emplace_back(&type);
  notice(out, type);
  fileBanner(out, name(type.name, gen) + ".cpp", bannerSeparator);
  out.line("/// @file ", name(qualifiedPath(&type, "/"), gen), ".cpp");
  out.line(autoGeneratedLine);
  out.line();
  includes(out, includeList);
  out.line();
  sourcePrologue(out, type);
  openNamespace(out, type);
  generateSourceGenBody(out, type, gen);
  closeNamespace(out, type);
}

void GapPatternCppGenerator::generateTypeHeader(cursiv::TextBuilder& out,
                                                const Type& type) const {
  comment(out, type);
  out.line("using ", type.name, " = ", type.name, "Gen;");
}

void GapPatternCppGenerator::generateNativeTypeHeaderGen(
    cursiv::TextBuilder& out, const NativeType& type, bool gen) const {
  const auto nativeNamespace = docTag(type, "namespace");
  comment(out, type);
  out.write("using ", name(type.name, gen), " = ");
  if (nativeNamespace && !nativeNamespace->empty())
    out.write(*nativeNamespace, "::");
  out.line(docTag(type, "type").value_or(""), ";");
}

void GapPatternCppGenerator::generateValueReferenceHeaderGen(
    cursiv::TextBuilder& out, const ValueReference& type, bool gen) const {
  comment(out, type);
  out.line("using ", name(type.name, gen), " = ", fqn(type.type.get()), " *;");
  out.line();
  uuidDeclaration(out, type);
}

void GapPatternCppGenerator::generateClassHeader(cursiv::TextBuilder& out,
                                                 const Class& type) const {
  comment(out, type);
  out.line("class ", type.name, ": public ", type.name, "Gen");
  out.open("{");
  out.line("friend class ", fqn(&type), "Gen;");
  out.line("public:");
  out.line(type.name, "() = default;");
  out.line(type.name, "(const ", type.name, "&) = default;");
  declareMembers(out, type.elements, Visibility::Public);
  out.close("};");
}

void GapPatternCppGenerator::generateClassHeaderGen(cursiv::TextBuilder& out,
                                                    const Class& type,
                                                    bool gen) const {
  const auto isException = cursiv::ast_cast<const Exception>(&type) != nullptr;
  const auto base = fqn(type.base.has_value() ? type.base->get() : nullptr,
                        isException ? "::Smp::Exception" : "");
  const auto className = name(type.name, gen);
  // A constructor Operation without parameter replaces the defaulted
  // constructor.
  const auto constructor =
      !attributeBoolValue(type, "Attributes.NoConstructor").value_or(false)
      && std::ranges::none_of(type.elements, [](const NamedElement* element) {
           const auto* operation = cursiv::ast_cast<const Operation>(element);
           return operation != nullptr && isConstructorAttribute(*operation)
                  && operation->parameter.empty();
         });
  const auto destructor =
      !attributeBoolValue(type, "Attributes.NoDestructor").value_or(false);

  if (gen) out.line("class ", type.name, ";");
  out.line();
  comment(out, type);
  out.line("class ", className, base.empty() ? "" : ": public ", base);
  out.open("{");
  if (gen) out.line("friend class ", fqn(&type), ";");
  out.line("public:");
  out.line(
      "static void _Register(::Smp::Publication::ITypeRegistry* registry);");
  out.line();
  if (constructor) out.line(className, "() = default;");
  if (destructor) out.line("~", className, "() noexcept = default;");
  out.line(className, "(const ", className, "&) = default;");
  out.line();
  declareMembersGen(out, type.elements, Visibility::Public, gen);
  out.close("};");
  out.line();
  uuidDeclaration(out, type);
}

void GapPatternCppGenerator::addFields(cursiv::TextBuilder& out,
                                       const Structure& type) const {
  const auto fields = registeredFields(type);
  if (!fields.empty()) out.line("// Register the Fields of the Class");
  for (const auto* field : fields) {
    out.open("type->AddField(");
    out.line("\"", field->name, "\", // Name");
    out.line(description(*field), ", // Description");
    out.line(uuid(field->type.get()), ", // Type UUID");
    out.line("offsetof(", type.name, ", ", field->name, "), // Field offset");
    out.line(viewKind(*field), ", // View Kind");
    out.line(boolText(isState(*field)), ", // State");
    out.line(boolText(isInput(*field)), ", // Input");
    out.line(boolText(isOutput(*field)), " // Output");
    out.close(");");
  }
}

bool GapPatternCppGenerator::hasDefinitions(
    std::span<NamedElement* const> elements) {
  return std::ranges::any_of(elements, [](const NamedElement* element) {
    return hasDefinition(*element);
  });
}

bool GapPatternCppGenerator::hasSource(const Type& type) {
  if (const auto* classType = cursiv::ast_cast<const Class>(&type))
    return hasDefinitions(classType->elements);
  return cursiv::ast_cast<const Component>(&type);
}

void GapPatternCppGenerator::generateClassSourceGen(cursiv::TextBuilder& out,
                                                    const Class& type,
                                                    bool gen) const {
  defineMembersGen(out, type.elements, gen);
  out.line("void ", name(type.name, gen),
           "::_Register(::Smp::Publication::ITypeRegistry* registry)");
  out.open("{");
  out.open(registeredFields(type).empty() ? "" : "auto *type = ",
           "registry->AddClassType(");
  out.line("\"", type.name, "\", // Name");
  out.line(description(type), ", // Description");
  out.line(uuid(&type), ", // UUID");
  out.line(uuid(type.base.has_value() ? type.base->get() : nullptr),
           " // Base Class UUID");
  out.close(");");
  out.line();
  addFields(out, type);
  out.close("}");
}

void GapPatternCppGenerator::generateStructureHeader(
    cursiv::TextBuilder& out, const Structure& type) const {
  comment(out, type);
  out.line("struct ", type.name, " : public ", type.name, "Gen");
  out.open("{");
  declareMembers(out, type.elements, Visibility::Public);
  out.close("};");
}

void GapPatternCppGenerator::generateStructureSourceGen(
    cursiv::TextBuilder& out, const Structure& type, bool gen) const {
  defineMembersGen(out, type.elements, gen);
  out.line("void ", name(type.name, gen),
           "::_Register(::Smp::Publication::ITypeRegistry* registry)");
  out.open("{");
  out.open(registeredFields(type).empty() ? "" : "auto *type = ",
           "registry->AddStructureType(");
  out.line("\"", type.name, "\", // Name");
  out.line(description(type), ", // Description");
  out.line(uuid(&type), " // UUID");
  out.close(");");
  out.line();
  addFields(out, type);
  out.close("}");
}

void GapPatternCppGenerator::generateStructureHeaderGen(
    cursiv::TextBuilder& out, const Structure& type, bool gen) const {
  const auto fields = instanceFields(type);
  const auto className = name(type.name, gen);
  comment(out, type);
  out.line("struct ", className);
  out.open("{");
  declareMembersGen(out, type.elements, Visibility::Public, gen);
  out.line();
  if (std::ranges::any_of(fields, [](const Field* field) {
        return field->default_ != nullptr;
      })) {
    out.line(className, "() = default;");
    out.line("~", className, "() = default;");
    out.line(className, "(const ", className, "&) = default;");
    out.line(className, "(", className, "&&) = default;");
    out.write(className, "(");
    std::string_view separator;
    for (const auto* field : fields) {
      out.write(separator, fqn(field->type.get()), " ", field->name);
      separator = ", ";
    }
    out.line("):");
    separator = {};
    for (const auto* field : fields) {
      out.write(separator, field->name, "(", field->name, ")");
      separator = ", ";
    }
    out.line(" {}");
    out.line(className, "& operator=(const ", className, "&) = default;");
    out.line();
  }
  out.line(
      "static void _Register(::Smp::Publication::ITypeRegistry* registry);");
  out.close("};");
  out.line();
  uuidDeclaration(out, type);
}

void GapPatternCppGenerator::generateIntegerHeaderGen(cursiv::TextBuilder& out,
                                                      const Integer& type,
                                                      bool gen) const {
  uuidDeclaration(out, type);
  comment(out, type);
  out.line(
      "using ", name(type.name, gen), " = ",
      fqn(type.primitiveType.has_value() ? type.primitiveType->get() : nullptr,
          "::Smp::Int32"),
      ";");
  out.line("void _Register_", type.name,
           "(::Smp::Publication::ITypeRegistry* registry);");
}

void GapPatternCppGenerator::generateIntegerSourceGen(
    cursiv::TextBuilder& out, const Integer& type) const {
  const auto primitive =
      fqn(type.primitiveType.has_value() ? type.primitiveType->get() : nullptr,
          "::Smp::Int32");
  // `AddIntegerType` takes `Int64` bounds, and a larger `UInt64` would wrap
  // around to a negative one.
  const auto wrapsAround = [&] {
    if (getPTK(&type) != PTK::UInt64) return false;
    if (type.maximum == nullptr) return true;
    const auto maximum = getValueAs(type.maximum, PTK::UInt64);
    return maximum
           && maximum->integral > static_cast<std::uint64_t>(
                  std::numeric_limits<std::int64_t>::max());
  }();
  out.open("void _Register_", type.name,
           "(::Smp::Publication::ITypeRegistry* registry) {");
  out.open("registry->AddIntegerType(");
  out.line("\"", type.name, "\", // Name");
  out.line(description(type), ", // Description");
  out.line(uuid(&type), ", // UUID");
  out.line(type.minimum != nullptr
               ? expression(type.minimum)
               : "std::numeric_limits<" + primitive + ">::min()",
           ", // Minimum");
  out.line(wrapsAround ? std::string("9223372036854775807LL")
           : type.maximum != nullptr
               ? expression(type.maximum)
               : "std::numeric_limits<" + primitive + ">::max()",
           ", // Maximum");
  out.line("\"", escapedText(docTag(type, "unit").value_or("")), "\", // Unit");
  out.line(primitiveTypeKind(&type), " // Primitive Type Kind");
  out.close(");");
  out.close("}");
}

void GapPatternCppGenerator::generateFloatHeaderGen(cursiv::TextBuilder& out,
                                                    const Float& type,
                                                    bool gen) const {
  uuidDeclaration(out, type);
  comment(out, type);
  out.line(
      "using ", name(type.name, gen), " = ",
      fqn(type.primitiveType.has_value() ? type.primitiveType->get() : nullptr,
          "::Smp::Float64"),
      ";");
  out.line("void _Register_", type.name,
           "(::Smp::Publication::ITypeRegistry* registry);");
}

void GapPatternCppGenerator::generateFloatSourceGen(cursiv::TextBuilder& out,
                                                    const Float& type) const {
  const auto primitive =
      fqn(type.primitiveType.has_value() ? type.primitiveType->get() : nullptr,
          "::Smp::Float64");
  // `range` is empty or one of "...", "<..", "..<" and "<.<": a "<" marks an
  // exclusive bound.
  const auto minInclusive = type.range != "<.." && type.range != "<.<";
  const auto maxInclusive = type.range != "..<" && type.range != "<.<";
  out.open("void _Register_", type.name,
           "(::Smp::Publication::ITypeRegistry* registry) {");
  out.open("registry->AddFloatType(");
  out.line("\"", type.name, "\", // Name");
  out.line(description(type), ", // Description");
  out.line(uuid(&type), ", // UUID");
  out.line(type.minimum != nullptr
               ? expression(type.minimum)
               : "std::numeric_limits<" + primitive + ">::lowest()",
           ", // Minimum");
  out.line(type.maximum != nullptr
               ? expression(type.maximum)
               : "std::numeric_limits<" + primitive + ">::max()",
           ", // Maximum");
  out.line(boolText(minInclusive), ", // Minimum ",
           minInclusive ? "inclusive" : "exclusive");
  out.line(boolText(maxInclusive), ", // Maximum ",
           maxInclusive ? "inclusive" : "exclusive");
  out.line("\"", escapedText(docTag(type, "unit").value_or("")), "\", // Unit");
  out.line(primitiveTypeKind(&type), " // Primitive Type Kind");
  out.close(");");
  out.close("}");
}

void GapPatternCppGenerator::openComponentClass(cursiv::TextBuilder& out,
                                                const Component& type,
                                                bool gen) const {
  uuidDeclaration(out, type);
  out.line();
  comment(out, type);
  const auto bases = componentBases(type);
  out.line("class ", name(type.name, gen), bases.empty() ? "" : ": ",
           cursiv::joined(bases, ", "));
  out.line("{");
  if (gen) out.line("    friend class ", fqn(&type), ";");
}

void GapPatternCppGenerator::declareComponentInterface(
    cursiv::TextBuilder& out, const Component& type, bool gen,
    std::string_view destructor) {
  const auto className = name(type.name, gen);
  const auto kind = componentKind(type);
  out.line("public:");
  out.line("/// Constructor setting name, description, parent and simulator.");
  out.line("/// @param name Name of new ", kind, " instance.");
  out.line("/// @param description Description of new ", kind, " instance.");
  out.line("/// @param parent Parent of new ", kind, " instance.");
  out.line("/// @param simulator The simulator instance.");
  out.open(className, "(::Smp::String8 name,");
  out.line("    ::Smp::String8 description,");
  out.line("    ::Smp::IComposite* parent,");
  out.line("    ::Smp::ISimulator* simulator);");
  out.close("/// deleted copy constructor");
  out.line(className, "(const ", className, "&) = delete;");
  out.line("/// deleted move constructor");
  out.line(className, "(", className, "&&) = delete;");
  out.line("/// deleted copy assignment");
  out.line(className, "& operator=(const ", className, "&) = delete;");
  out.line("/// deleted move assignment");
  out.line(className, "& operator=(", className, "&&) = delete;");
  out.line();
  out.line("/// Virtual destructor to release memory.");
  out.line("~", className, destructor);
  out.line();
  out.line("/// Request the ", kind, " to publish its fields, properties and");
  out.line("/// operations against the provided publication receiver.");
  out.line("/// @param   receiver Publication receiver.");
  out.line("/// @throws  Smp::InvalidComponentState");
  out.line("void Publish(::Smp::IPublication* receiver) override;");
  out.line();
  out.line("/// Request the ", kind,
           " to perform any custom configuration. The");
  out.line(
      "/// component can create and configure other components using the "
      "field");
  out.line("/// values of its published fields.");
  out.line(
      "/// @param   logger Logger service for logging of error messages "
      "during");
  out.line("///          configuration.");
  out.line(
      "/// @param   linkRegistry Reference to the link registry services, so");
  out.line("///          that the ", kind,
           " can register links that it creates");
  out.line("///          during configuration.");
  out.line("/// @throws  Smp::InvalidComponentState");
  out.line(
      "void Configure(::Smp::Services::ILogger* logger, "
      "::Smp::Services::ILinkRegistry* linkRegistry) override;");
  out.line();
  out.line("/// Allow the ", kind,
           " to connect to the simulator and its simulation");
  out.line("/// services.");
  out.line("/// @param   simulator Simulation Environment that hosts the ",
           kind, ".");
  out.line("/// @throws  Smp::InvalidComponentState");
  out.line("void Connect(::Smp::ISimulator* simulator) override;");
  out.line();
  out.line("/// Ask the ", kind,
           " to disconnect from the simulator and all its");
  out.line("/// simulation services.");
  out.line("/// @throws  Smp::InvalidComponentState");
  out.line("void Disconnect() override;");
  out.line();
  out.line("/// Get Universally Unique Identifier of the ", kind, ".");
  out.line("/// @return  Universally Unique Identifier of the ", kind, ".");
  out.line("const ::Smp::Uuid& GetUuid() const override;");
}

void GapPatternCppGenerator::declareInvoke(cursiv::TextBuilder& out) {
  out.write(R"(public:
/// Dynamically invoke an operation using a request object that has
/// been created and filled with parameter values by the caller.
/// @param   request Request object to invoke.
/// @throws  Smp::InvalidOperationName
/// @throws  Smp::InvalidParameterCount
/// @throws  Smp::InvalidParameterType
void Invoke(::Smp::IRequest* request) override;
)");
}

void GapPatternCppGenerator::generateComponentSource(cursiv::TextBuilder& out,
                                                     const Component& type,
                                                     bool gen) const {
  out.line("void ", type.name, "::DoPublish(::Smp::IPublication* receiver) {");
  out.line("}");
  out.line();
  out.line("void ", type.name,
           "::DoConfigure(::Smp::Services::ILogger* logger, "
           "::Smp::Services::ILinkRegistry* linkRegistry) {");
  out.line("}");
  out.line();
  out.line("void ", type.name, "::DoConnect(::Smp::ISimulator* simulator) {");
  out.line("}");
  out.line();
  out.line("void ", type.name, "::DoDisconnect() {");
  out.line("}");
  out.line();
  defineMembers(out, type.elements, gen);
}

void GapPatternCppGenerator::generateInterfaceHeader(
    cursiv::TextBuilder& out, const Interface& type) const {
  comment(out, type);
  out.line("class ", type.name, ": public ", type.name, "Gen {");
  out.line("public:");
  out.line("    ~", type.name, "() override = default;");
  out.line("};");
}

void GapPatternCppGenerator::generateInterfaceHeaderGen(
    cursiv::TextBuilder& out, const Interface& type, bool gen) const {
  const auto className = name(type.name, gen);
  comment(out, type);
  out.write("class ", className);
  std::string_view separator = ": ";
  for (const auto& base : type.base) {
    out.write(separator, "public virtual ", fqn(base.get()));
    separator = ", ";
  }
  out.line(" {");
  out.open("public:");
  out.line("virtual ~", className, "() = default;");
  declareMembersGen(out, type.elements, Visibility::Public, gen);
  out.close("};");
  out.line();
  uuidDeclaration(out, type);
}

void GapPatternCppGenerator::generateInterfaceSourceGen(
    cursiv::TextBuilder& out, const Interface& type, bool gen) const {
  for (const auto* element : type.elements)
    if (const auto* constant = cursiv::ast_cast<const Constant>(element);
        constant && cursiv::ast_cast<const String>(constant->type.get()))
      defineConstantGen(out, *constant, gen);
}

void GapPatternCppGenerator::generateArraySourceGen(cursiv::TextBuilder& out,
                                                    const Array& type) const {
  out.open("void _Register_", type.name,
           "(::Smp::Publication::ITypeRegistry* registry) {");
  out.open("registry->AddArrayType(");
  out.line("\"", type.name, "\", // Name");
  out.line(description(type), ", // Description");
  out.line(uuid(&type), ", // UUID");
  out.line(uuid(type.itemType.get()), ", // Item Type UUID");
  out.line("sizeof(", fqn(type.itemType.get()), "), // Item Type size");
  out.line(expression(type.size), ", // Number of elements");
  out.line(boolText(isSimpleArrayAttribute(type)), " // Simple array");
  out.close(");");
  out.close("}");
}

void GapPatternCppGenerator::generateEnumerationHeaderGen(
    cursiv::TextBuilder& out, const Enumeration& type, bool gen) const {
  const auto className = name(type.name, gen);
  comment(out, type);
  out.open("enum class ", className, ": ::Smp::Int32 {");
  std::string_view separator;
  for (const auto* literal : type.literal) {
    out.line(separator);
    separator = ",";
    comment(out, *literal);
    out.write(literal->name, " = ", expression(literal->value));
  }
  out.line();
  out.close("};");
  out.line();
  uuidDeclaration(out, type);
  out.line();
  out.line("void _Register_", type.name,
           "(::Smp::Publication::ITypeRegistry* registry);");
  out.line();
  out.open("const std::map<", className, ", std::string> ", type.name,
           "_name_map = {");
  separator = {};
  for (const auto* literal : type.literal) {
    out.write(separator, "{ ", className, "::", literal->name, ", \"",
              literal->name, "\" }");
    separator = ", ";
  }
  out.line();
  out.close("};");
  out.line();
  out.open("const std::map<", className, ", std::string> ", type.name,
           "_descr_map = {");
  separator = {};
  for (const auto* literal : type.literal) {
    out.write(separator, "{ ", className, "::", literal->name, ", ",
              description(*literal), " }");
    separator = ", ";
  }
  out.line();
  out.close("};");
}

void GapPatternCppGenerator::generateEnumerationSourceGen(
    cursiv::TextBuilder& out, const Enumeration& type) const {
  out.open("void _Register_", type.name,
           "(::Smp::Publication::ITypeRegistry* registry) {");
  out.open("auto *type = registry->AddEnumerationType(");
  out.line("\"", type.name, "\", // Name");
  out.line(description(type), ", // Description");
  out.line(uuid(&type), ", // UUID");
  out.line("sizeof(", fqn(&type), ") // Size");
  out.close(");");
  out.line();
  addLiterals(out, type);
  out.close("}");
}

void GapPatternCppGenerator::addLiterals(cursiv::TextBuilder& out,
                                         const Enumeration& type) const {
  out.line("// Register the Literals of the Enumeration");
  for (const auto* literal : type.literal)
    out.line("type->AddLiteral(\"", literal->name, "\", ",
             description(*literal), ", static_cast<::Smp::Int32>(",
             fqn(literal), "));");
}

void GapPatternCppGenerator::generateStringSourceGen(cursiv::TextBuilder& out,
                                                     const String& type) const {
  out.open("void _Register_", type.name,
           "(::Smp::Publication::ITypeRegistry* registry) {");
  out.open("registry->AddStringType(");
  out.line("\"", type.name, "\", // Name");
  out.line(description(type), ", // Description");
  out.line(uuid(&type), ", // UUID");
  out.line(expression(type.length), " // Length");
  out.close(");");
  out.close("}");
}

void GapPatternCppGenerator::declareMembersGen(
    cursiv::TextBuilder& out, std::span<NamedElement* const> elements,
    Visibility initial, bool gen) const {
  MemberList members(out, initial);
  for (const auto* constant : sortedConstants(elements)) {
    members.next(*constant);
    declareConstantGen(out, *constant);
  }
  for (const auto* property : ofKind<Property>(elements)) {
    members.next(*property);
    declarePropertyGen(out, *property, gen);
  }
  for (const auto* operation : ofKind<Operation>(elements)) {
    members.next(*operation);
    declareOperationGen(out, *operation, gen);
  }
  for (const auto* entryPoint : ofKind<EntryPoint>(elements)) {
    members.next(*entryPoint);
    declareEntryPointGen(out, *entryPoint, gen);
  }
  for (const auto* eventSink : ofKind<EventSink>(elements)) {
    members.next(*eventSink);
    declareEventSinkGen(out, *eventSink, gen);
  }
  for (const auto* eventSource : ofKind<EventSource>(elements)) {
    members.next(*eventSource);
    declareEventSourceGen(out, *eventSource);
  }
  for (const auto* field : ofKind<Field>(elements)) {
    members.next(*field);
    declareFieldGen(out, *field, gen);
  }
  for (const auto* association : ofKind<Association>(elements)) {
    members.next(*association);
    declareAssociationGen(out, *association);
  }
  for (const auto* container : ofKind<Container>(elements)) {
    members.next(*container);
    declareContainerGen(out, *container);
  }
  for (const auto* reference : ofKind<Reference>(elements)) {
    members.next(*reference);
    declareReferenceGen(out, *reference);
  }
}

void GapPatternCppGenerator::defineMembersGen(
    cursiv::TextBuilder& out, std::span<NamedElement* const> elements,
    bool gen) const {
  for (const auto* element : elements) {
    if (const auto* constant = cursiv::ast_cast<const Constant>(element)) {
      // A constexpr static member needs a definition before C++17.
      if (!cursiv::ast_cast<const String>(constant->type.get())
          && cxxStandard() >= 17)
        continue;
      defineConstantGen(out, *constant, gen);
    } else if (const auto* property =
                   cursiv::ast_cast<const Property>(element)) {
      if (!property->attachedField.has_value()) continue;
      definePropertyGen(out, *property, gen);
    } else if (const auto* field = cursiv::ast_cast<const Field>(element)) {
      if (!isStaticAttribute(*field)) continue;
      defineFieldGen(out, *field, gen);
    } else if (const auto* association =
                   cursiv::ast_cast<const Association>(element)) {
      if (!isStaticAttribute(*association)) continue;
      out.line(type(association), " ", ownerName(*association, gen),
               "::", association->name, "{};");
    } else {
      continue;
    }
    out.line();
  }
}

void GapPatternCppGenerator::declareConstantGen(cursiv::TextBuilder& out,
                                                const Constant& element) const {
  const auto* type = element.type.get();
  comment(out, element);
  if (cursiv::ast_cast<const String>(type))
    out.line("static const ", fqn(type), " ", element.name, ";");
  else
    out.line("static constexpr ", fqn(type), " ", element.name,
             directListInitializer(element.value), ";");
}

void GapPatternCppGenerator::defineConstantGen(cursiv::TextBuilder& out,
                                               const Constant& element,
                                               bool gen) const {
  const auto* type = element.type.get();
  const auto owner = ownerName(element, gen);
  comment(out, element);
  if (cursiv::ast_cast<const String>(type))
    out.line("const ", fqn(type), " ", owner, "::", element.name,
             directListInitializer(element.value), ";");
  else
    out.line("constexpr ", fqn(type), " ", owner, "::", element.name, ";");
}

void GapPatternCppGenerator::declareFieldGen(cursiv::TextBuilder& out,
                                             const Field& element,
                                             bool /*gen*/) const {
  const auto isStatic = isStaticAttribute(element);
  // A component field gets its default from the constructor, a static field
  // from its definition outside the class.
  const auto inComponent =
      cursiv::ast_cast<const Component>(element.parent) != nullptr;
  comment(out, element);
  out.write(isStatic ? "static " : "", isMutable(element) ? "mutable " : "",
            fqn(element.type.get()), " ", element.name);
  if (element.default_ != nullptr && !inComponent && !isStatic)
    out.write(directListInitializer(element.default_));
  out.line(";");
}

void GapPatternCppGenerator::defineFieldGen(cursiv::TextBuilder& out,
                                            const Field& element,
                                            bool gen) const {
  out.line("// Field ", element.name);
  out.line(fqn(element.type.get()), " ", ownerName(element, gen),
           "::", element.name, directListInitializer(element.default_), ";");
}

void GapPatternCppGenerator::declareAssociationGen(
    cursiv::TextBuilder& out, const Association& element) const {
  comment(out, element);
  out.line(isStaticAttribute(element) ? "static " : "",
           isMutable(element) ? "mutable " : "", type(&element), " ",
           element.name, ";");
}

void GapPatternCppGenerator::declareOperationGen(cursiv::TextBuilder& out,
                                                 const Operation& op,
                                                 bool gen) const {
  const auto params = parameterList(op, true);
  comment(out, op);
  if (isConstructorAttribute(op)) {
    out.line(ownerName(op, gen), "(", params, ");");
  } else if (isStaticAttribute(op)) {
    out.line("static ", type(op.returnParameter), " ", operationName(op), "(",
             params, ");");
  } else {
    const auto virtual_ = isVirtual(op);
    out.line(virtual_ ? "virtual " : "", type(op.returnParameter), " ",
             operationName(op), "(", params, ")", isConst(op) ? " const" : "",
             virtual_ && (isAbstract(op) || gen) ? "=0" : "", ";");
  }
}

void GapPatternCppGenerator::declarePropertyGen(cursiv::TextBuilder& out,
                                                const Property& element,
                                                bool gen) const {
  const auto access = getAccessKind(element).value_or(AccessKind::ReadWrite);
  if (access != AccessKind::WriteOnly)
    declarePropertyGetterGen(out, element, gen);
  if (access != AccessKind::ReadOnly)
    declarePropertySetterGen(out, element, gen);
}

void GapPatternCppGenerator::declarePropertyGetterGen(cursiv::TextBuilder& out,
                                                      const Property& element,
                                                      bool gen) const {
  // The regenerated class implements the accessors of an attached field, and
  // leaves the others to the hand-edited class.
  const auto abstract =
      isAbstract(element)
      || (isVirtual(element) && gen && !element.attachedField.has_value());
  out.line("/// Get ", element.name, ".");
  comment(out, element);
  out.line("/// @return Current value of property ", element.name, ".");
  out.line(isVirtual(element) ? "virtual " : "",
           isStaticAttribute(element) ? "static " : "", type(&element), " get_",
           element.name, "()", isConstGetter(element) ? " const" : "",
           abstract ? "=0" : "", ";");
}

void GapPatternCppGenerator::declarePropertySetterGen(cursiv::TextBuilder& out,
                                                      const Property& element,
                                                      bool gen) const {
  const auto abstract =
      isAbstract(element)
      || (isVirtual(element) && gen && !element.attachedField.has_value());
  out.line("/// Set ", element.name, ".");
  comment(out, element);
  out.line("/// @param value New value of property ", element.name, " to set.");
  out.line(isVirtual(element) ? "virtual " : "",
           isStaticAttribute(element) ? "static " : "", "void set_",
           element.name, "(", type(&element), " value)", abstract ? "=0" : "",
           ";");
}

void GapPatternCppGenerator::definePropertyGen(cursiv::TextBuilder& out,
                                               const Property& element,
                                               bool gen) const {
  const auto owner = ownerName(element, gen);
  const auto self = isStaticAttribute(element) ? owner + "::" : "this->";
  const std::string_view field = element.attachedField->get()->name;
  const auto access = getAccessKind(element).value_or(AccessKind::ReadWrite);
  if (access != AccessKind::WriteOnly) {
    out.open(type(&element), " ", owner, "::get_", element.name, "()",
             isConstGetter(element) ? " const" : "", "{");
    out.line("return ", self, field, ";");
    out.close("}");
  }
  if (access != AccessKind::ReadOnly) {
    out.open("void ", owner, "::set_", element.name, "(", type(&element),
             " value) {");
    out.line(self, field, " = value;");
    out.close("}");
  }
}

void GapPatternCppGenerator::declareEntryPointGen(cursiv::TextBuilder& out,
                                                  const EntryPoint& element,
                                                  bool gen) const {
  comment(out, element);
  out.line("::Smp::IEntryPoint* ", element.name, ";");
  out.line("virtual void _", element.name, "()", gen ? " = 0" : "", ";");
}

void GapPatternCppGenerator::declareEventSinkGen(cursiv::TextBuilder& out,
                                                 const EventSink& element,
                                                 bool gen) const {
  const auto* payload = eventType(element);
  comment(out, element);
  out.line("::Smp::IEventSink* ", element.name, ";");
  out.write("virtual void _", element.name, "(::Smp::IObject* sender");
  if (payload) out.write(", ", fqn(payload));
  out.line(")", gen ? " = 0" : "", ";");
}

void GapPatternCppGenerator::initializeField(cursiv::TextBuilder& out,
                                             const Field& element) const {
  out.line("// Field ", element.name);
  out.write(element.name, " ", directListInitializer(element.default_));
}

void GapPatternCppGenerator::initializeMembers(
    cursiv::TextBuilder& out, std::span<NamedElement* const> elements,
    bool gen) const {
  for (const auto* entryPoint : ofKind<EntryPoint>(elements)) {
    out.line(",");
    initializeEntryPoint(out, *entryPoint, gen);
  }
  for (const auto* eventSink : ofKind<EventSink>(elements)) {
    out.line(",");
    initializeEventSink(out, *eventSink, gen);
  }
  for (const auto* eventSource : ofKind<EventSource>(elements)) {
    out.line(",");
    initializeEventSource(out, *eventSource);
  }
  for (const auto* field : ofKind<Field>(elements)) {
    if (isStaticAttribute(*field)) continue;
    out.line(",");
    initializeField(out, *field);
  }
  for (const auto* association : ofKind<Association>(elements)) {
    if (isStaticAttribute(*association)) continue;
    out.line(",");
    out.line("// Association ", association->name);
    out.write(association->name, "{ }");
  }
  for (const auto* container : ofKind<Container>(elements)) {
    out.line(",");
    initializeContainer(out, *container);
  }
  for (const auto* reference : ofKind<Reference>(elements)) {
    out.line(",");
    initializeReference(out, *reference);
  }
}

void GapPatternCppGenerator::declareMembers(
    cursiv::TextBuilder& out, std::span<NamedElement* const> elements,
    Visibility initial) const {
  MemberList members(out, initial);
  for (const auto* property : ofKind<Property>(elements)) {
    if (!isOverridden(*property)) continue;
    members.next(*property);
    declareProperty(out, *property);
  }
  auto constructor = false;
  for (const auto* operation : ofKind<Operation>(elements)) {
    if (isConstructorAttribute(*operation)) {
      // A component already inherits them in the head of its class.
      if (constructor || cursiv::ast_cast<const Component>(operation->parent))
        continue;
      constructor = true;
      members.next(*operation);
      const auto base = ownerName(*operation, true);
      out.line("using ", base, "::", base, ";");
      continue;
    }
    if (!isVirtual(*operation) || isAbstract(*operation)) continue;
    members.next(*operation);
    declareOperation(out, *operation);
  }
  for (const auto* entryPoint : ofKind<EntryPoint>(elements)) {
    members.next(*entryPoint);
    declareEntryPoint(out, *entryPoint);
  }
  for (const auto* eventSink : ofKind<EventSink>(elements)) {
    members.next(*eventSink);
    declareEventSink(out, *eventSink);
  }
}

void GapPatternCppGenerator::defineMembers(
    cursiv::TextBuilder& out, std::span<NamedElement* const> elements,
    bool gen) const {
  for (const auto* element : elements) {
    if (!hasDefinition(*element)) continue;
    if (const auto* property = cursiv::ast_cast<const Property>(element))
      defineProperty(out, *property, gen);
    else if (const auto* operation = cursiv::ast_cast<const Operation>(element))
      defineOperation(out, *operation, gen);
    else if (const auto* entryPoint =
                 cursiv::ast_cast<const EntryPoint>(element))
      defineEntryPoint(out, *entryPoint);
    else
      defineEventSink(out, *cursiv::ast_cast<const EventSink>(element));
    out.line();
  }
}

void GapPatternCppGenerator::declareOperation(cursiv::TextBuilder& out,
                                              const Operation& op) const {
  comment(out, op);
  out.line(type(op.returnParameter), " ", operationName(op), "(",
           parameterList(op, true), ")", isConst(op) ? " const" : "",
           " override;");
}

void GapPatternCppGenerator::defaultReturn(cursiv::TextBuilder& out,
                                           const ReturnParameter& param,
                                           const cursiv::AstNode* self) const {
  const auto* returned = param.type.get();
  const auto isSelf = returned != nullptr && returned == self;
  if (isByPointer(param)) {
    out.line(isSelf ? "return this;" : "return nullptr;");
  } else if (isSelf) {
    out.line("return *this;");
  } else {
    out.line("return ", defaultValueForType(returned), ";");
  }
}

void GapPatternCppGenerator::defineOperation(cursiv::TextBuilder& out,
                                             const Operation& op,
                                             bool gen) const {
  const auto isStatic = isStaticAttribute(op);
  const auto generatedOwner = gen && (isStatic || !isVirtual(op));
  // Only a member of `X` called on an object can give itself as an `X`.
  const auto* self = generatedOwner || isStatic ? nullptr : op.parent;
  const auto* result = op.returnParameter;
  // A reference to anything but the object itself has nothing to refer to.
  const auto throws =
      result && isByReference(*result)
      && (!self || result->type.get() != self || isByPointer(*result)
          || (isConst(op) && !isConst(*result)));
  out.open(type(result), " ", ownerName(op, generatedOwner),
           "::", operationName(op), "(", parameterList(op, false), ")",
           !isStatic && isConst(op) ? " const" : "", " {");
  out.line("// TODO");
  if (throws)
    out.line(R"(throw std::logic_error("Operation is not implemented.");)");
  else if (result)
    defaultReturn(out, *result, self);
  out.close("}");
}

void GapPatternCppGenerator::declareProperty(cursiv::TextBuilder& out,
                                             const Property& element) const {
  const auto access = getAccessKind(element).value_or(AccessKind::ReadWrite);
  if (access != AccessKind::WriteOnly)
    out.line(type(&element), " get_", element.name, "()",
             isConstGetter(element) ? " const" : "", " override;");
  if (access != AccessKind::ReadOnly)
    out.line("void set_", element.name, "(", type(&element),
             " value) override;");
}

void GapPatternCppGenerator::defineProperty(cursiv::TextBuilder& out,
                                            const Property& element,
                                            bool gen) const {
  const auto owner = definingClass(element, gen);
  const auto access = getAccessKind(element).value_or(AccessKind::ReadWrite);
  if (access != AccessKind::WriteOnly) {
    out.line(type(&element), " ", owner, "::get_", element.name, "()",
             isConstGetter(element) ? " const" : "", "{");
    // A reference has nothing to refer to.
    if (isByReference(element)) {
      out.line(
          R"(    throw std::logic_error("Property getter is not implemented.");)");
    } else {
      out.line("    ", type(&element), " ret {};");
      out.line("    return ret;");
    }
    out.line("}");
  }
  if (access != AccessKind::ReadOnly) {
    out.open("void ", owner, "::set_", element.name, "(", type(&element),
             " value){");
    out.line("// TODO");
    out.close("}");
  }
}

std::string GapPatternCppGenerator::definingClass(const NamedElement& member,
                                                  bool gen) {
  return ownerName(member, gen && !isVirtual(member));
}

void GapPatternCppGenerator::declareEntryPoint(
    cursiv::TextBuilder& out, const EntryPoint& element) const {
  out.line("void _", element.name, "() override;");
}

void GapPatternCppGenerator::defineEntryPoint(cursiv::TextBuilder& out,
                                              const EntryPoint& element) const {
  out.line("void ", ownerName(element), "::_", element.name, "() {");
  out.open("    // TODO implement EntryPoint ", element.name);
  comment(out, element);
  out.close("}");
}

void GapPatternCppGenerator::declareEventSink(cursiv::TextBuilder& out,
                                              const EventSink& element) const {
  const auto* payload = eventType(element);
  out.write("void _", element.name, "(::Smp::IObject* sender");
  if (payload) out.write(", ", fqn(payload));
  out.line(") override;");
}

void GapPatternCppGenerator::defineEventSink(cursiv::TextBuilder& out,
                                             const EventSink& element) const {
  const auto* payload = eventType(element);
  out.write("void ", ownerName(element), "::_", element.name,
            "(::Smp::IObject* sender");
  if (payload) out.write(", ", fqn(payload));
  out.line(") {");
  out.open("    // TODO implement EventSink ", element.name);
  comment(out, element);
  out.close("}");
}

}  // namespace xsmp
