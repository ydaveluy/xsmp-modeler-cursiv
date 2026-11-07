#include <algorithm>
#include <cctype>
#include <charconv>
#include <cmath>
#include <cstring>
#include <cursiv/documentation/DocComment.hpp>
#include <ranges>
#include <unordered_map>
#include <unordered_set>
#include <xsmp/AlphabeticalOrder.hpp>
#include <xsmp/AttributeHelper.hpp>
#include <xsmp/CopyrightNoticeProvider.hpp>
#include <xsmp/LiteralConversions.hpp>
#include <xsmp/XsmpcatCoreServices.hpp>
#include <xsmp/cpp/ClangFormat.hpp>
#include <xsmp/cpp/CppGenerator.hpp>
#include <xsmp/solver/Solver.hpp>
#include <xsmp/utils/TreeUtils.hpp>
#include <xsmp/utils/Utf8.hpp>
#include <xsmp/utils/XsmpUtils.hpp>
#include <xsmp/workspace/ProjectManager.hpp>

namespace xsmp {

namespace {

// ECSS-E-ST-40-07C (2 March 2020) declares no version of its own, so it is
// identified by the absence of `Smp/Version.h` and given a version here. The
// generated code then compares revisions plainly.
constexpr std::string_view smpVersionDetection =
    R"(#if defined(__has_include)
#if __has_include(<Smp/Version.h>)
#include <Smp/Version.h>
#endif
#endif

#ifndef ECSS_SMP_VERSION
#define ECSS_SMP_VERSION 202003L
#endif
)";

// The lines around the banner of a package file.
constexpr std::string_view packageHeaderSeparator =
    "// -----------------------------------------------------------"
    "----------------";
constexpr std::string_view packageSourceSeparator =
    "// -----------------------------------------------------------"
    "------------------";

// `text` with one blank line in a row at most and none at its ends, after an
// opening brace or before a closing one, and with a line break at its end. The
// generators can then separate two parts with a blank line whether or not they
// are empty.
std::string tidied(std::string text) {
  if (!text.ends_with('\n')) text += '\n';
  std::size_t kept = 0;
  auto blank = false;
  for (std::size_t at = 0, end = 0; at < text.size(); at = end) {
    end = text.find('\n', at) + 1;
    if (end - at == 1) {
      blank = true;
      continue;
    }
    if (blank && kept > 0 && text[kept - 2] != '{'
        && text[text.find_first_not_of(' ', at)] != '}')
      text[kept++] = '\n';
    blank = false;
    std::memmove(text.data() + kept, text.data() + at, end - at);
    kept += end - at;
  }
  text.resize(kept);
  return text;
}

// `value` as a C++ floating literal, with the fewest digits that read back the
// same.
std::string floatingLiteral(double value, PTK kind) {
  const auto isFloat32 = kind == PTK::Float32;
  char buffer[64];
  const auto end =
      isFloat32
          ? std::to_chars(std::begin(buffer), std::end(buffer),
                          static_cast<float>(value))
                .ptr
          : std::to_chars(std::begin(buffer), std::end(buffer), value).ptr;
  std::string text(buffer, end);
  if (text.find_first_of(".e") == std::string::npos) text += ".0";
  return isFloat32 ? text + "f" : text;
}

// The SMP header of a primitive type, which is not named after the type.
std::optional<std::string_view> primitiveTypeInclude(
    std::string_view dottedFqn) {
  static const std::unordered_map<std::string_view, std::string_view> table{
      {"Smp.Char8", "Smp/PrimitiveTypes.h"},
      {"Smp.String8", "Smp/PrimitiveTypes.h"},
      {"Smp.Float32", "Smp/PrimitiveTypes.h"},
      {"Smp.Float64", "Smp/PrimitiveTypes.h"},
      {"Smp.Int8", "Smp/PrimitiveTypes.h"},
      {"Smp.UInt8", "Smp/PrimitiveTypes.h"},
      {"Smp.Int16", "Smp/PrimitiveTypes.h"},
      {"Smp.UInt16", "Smp/PrimitiveTypes.h"},
      {"Smp.Int32", "Smp/PrimitiveTypes.h"},
      {"Smp.UInt32", "Smp/PrimitiveTypes.h"},
      {"Smp.Int64", "Smp/PrimitiveTypes.h"},
      {"Smp.UInt64", "Smp/PrimitiveTypes.h"},
      {"Smp.Bool", "Smp/PrimitiveTypes.h"},
      {"Smp.DateTime", "Smp/PrimitiveTypes.h"},
      {"Smp.Duration", "Smp/PrimitiveTypes.h"},
      {"Smp.PrimitiveTypeKind", "Smp/PrimitiveTypes.h"},
  };
  const auto it = table.find(dottedFqn);
  return it == table.end() ? std::nullopt : std::make_optional(it->second);
}

// ECSS SMP 2020 declares a collection in the header of its item. From ECSS SMP
// 2025 on, a collection has a header of its own, named after it.
std::optional<std::string_view> collectionInclude2020(
    std::string_view dottedFqn) {
  static const std::unordered_map<std::string_view, std::string_view> table{
      {"Smp.EventSourceCollection", "Smp/IEventSource.h"},
      {"Smp.EntryPointCollection", "Smp/IEntryPoint.h"},
      {"Smp.FactoryCollection", "Smp/IFactory.h"},
      {"Smp.FailureCollection", "Smp/IFailure.h"},
      {"Smp.FieldCollection", "Smp/IField.h"},
      {"Smp.ComponentCollection", "Smp/IComponent.h"},
      {"Smp.OperationCollection", "Smp/IOperation.h"},
      {"Smp.ParameterCollection", "Smp/IParameter.h"},
      {"Smp.PropertyCollection", "Smp/IProperty.h"},
      {"Smp.ModelCollection", "Smp/IModel.h"},
      {"Smp.ServiceCollection", "Smp/IService.h"},
      {"Smp.ReferenceCollection", "Smp/IReference.h"},
      {"Smp.ContainerCollection", "Smp/IContainer.h"},
      {"Smp.EventSinkCollection", "Smp/IEventSink.h"},
  };
  const auto it = table.find(dottedFqn);
  return it == table.end() ? std::nullopt : std::make_optional(it->second);
}

// Types the SMP SDK's own `::Smp::Uuids` namespace predeclares a UUID constant
// for (so referencing types don't need to reach into the defining type's own
// `Uuid_X`).
const std::unordered_set<std::string_view>& smpUuidsTypes() {
  static const std::unordered_set<std::string_view> names{
      "Smp.Uuid",
      "Smp.Char8",
      "Smp.Bool",
      "Smp.Int8",
      "Smp.UInt8",
      "Smp.Int16",
      "Smp.UInt16",
      "Smp.Int32",
      "Smp.UInt32",
      "Smp.Int64",
      "Smp.UInt64",
      "Smp.Float32",
      "Smp.Float64",
      "Smp.Duration",
      "Smp.DateTime",
      "Smp.String8",
      "Smp.PrimitiveTypeKind",
      "Smp.Services.EventId",
      "Smp.Services.LogMessageKind",
      "Smp.Services.TimeKind",
      "Smp.ViewKind",
      "Smp.Publication.ParameterDirectionKind",
      "Smp.ComponentStateKind",
      "Smp.AccessKind",
      "Smp.SimulatorStateKind"};
  return names;
}

// `items` ordered so that each item comes after those of them `dependenciesOf`
// returns for it: first the items that depend on none, in their original order,
// then each item once the last of its dependencies is placed. The items of a
// cycle, which validation refuses, are left out.
template <class T, class DependenciesOf>
std::vector<const T*> dependencyOrder(const std::vector<const T*>& items,
                                      DependenciesOf dependenciesOf) {
  std::unordered_map<const T*, std::size_t> waitingFor;
  for (const auto* item : items) waitingFor.emplace(item, 0);
  std::unordered_map<const T*, std::vector<const T*>> dependents;
  for (const auto* item : items) {
    auto dependencies = dependenciesOf(*item);
    std::ranges::sort(dependencies);
    const auto [first, last] = std::ranges::unique(dependencies);
    dependencies.erase(first, last);
    for (const auto* dependency : dependencies)
      if (waitingFor.contains(dependency)) {
        ++waitingFor[item];
        dependents[dependency].push_back(item);
      }
  }

  std::vector<const T*> result;
  result.reserve(items.size());
  std::ranges::copy_if(items, std::back_inserter(result), [&](const T* item) {
    return waitingFor[item] == 0;
  });
  // The result is also the queue of the items to release dependents of.
  for (std::size_t next = 0; next < result.size(); ++next)
    if (const auto found = dependents.find(result[next]);
        found != dependents.end())
      for (const auto* dependent : found->second)
        if (--waitingFor[dependent] == 0) result.push_back(dependent);
  return result;
}

// The name of the kind of `type`, as in the comment above its registration.
std::string_view nodeTypeName(const Type& type) {
  if (cursiv::ast_cast<const Exception>(&type)) return "Exception";
  if (cursiv::ast_cast<const Class>(&type)) return "Class";
  if (cursiv::ast_cast<const Structure>(&type)) return "Structure";
  if (cursiv::ast_cast<const Array>(&type)) return "Array";
  if (cursiv::ast_cast<const String>(&type)) return "String";
  if (cursiv::ast_cast<const Integer>(&type)) return "Integer";
  if (cursiv::ast_cast<const Float>(&type)) return "Float";
  if (cursiv::ast_cast<const Enumeration>(&type)) return "Enumeration";
  return "PrimitiveType";
}

// Whether the type of `operation` or one of its bases has an Operation of that
// name with another signature: a request only says the name.
bool isOverloaded(const Operation& operation) {
  std::unordered_set<std::string> signatures;
  std::unordered_set<const cursiv::AstNode*> visited;
  const auto visit = [&](const auto& self,
                         const cursiv::AstNode* type) -> void {
    if (!type || !visited.insert(type).second) return;
    const auto members = [&](std::span<NamedElement* const> elements) {
      for (const auto* element : elements)
        if (const auto* other = cursiv::ast_cast<const Operation>(element);
            other && other->name == operation.name
            && !isConstructorAttribute(*other))
          signatures.insert(getSignature(*other));
    };
    if (const auto* component = cursiv::ast_cast<const Component>(type)) {
      members(component->elements);
      if (component->base.has_value()) self(self, component->base->get());
      for (const auto& base : component->interface_) self(self, base.get());
    } else if (const auto* interface =
                   cursiv::ast_cast<const Interface>(type)) {
      members(interface->elements);
      for (const auto& base : interface->base) self(self, base.get());
    } else if (const auto* clazz = cursiv::ast_cast<const Class>(type)) {
      members(clazz->elements);
      if (clazz->base.has_value()) self(self, clazz->base->get());
    }
  };
  visit(visit, operation.parent);
  return signatures.size() > 1;
}

// Whether a declaration of `type` can stand in for its definition.
bool isForwardable(const Type* type) {
  return cursiv::ast_cast<const Structure>(type)
         || cursiv::ast_cast<const ReferenceType>(type);
}

// The catalogue that holds `node`, or `nullptr`.
const Catalogue* catalogueOf(const cursiv::AstNode* node) {
  for (; node != nullptr; node = node->parent)
    if (const auto* catalogue = cursiv::ast_cast<const Catalogue>(node))
      return catalogue;
  return nullptr;
}

// The nearest Namespace above `node`, or `nullptr`.
const Namespace* namespaceOf(const cursiv::AstNode* node) {
  for (; node != nullptr; node = node->parent)
    if (const auto* ns = cursiv::ast_cast<const Namespace>(node)) return ns;
  return nullptr;
}

}  // namespace

CppGenerator::CppGenerator(const XsmpcatCoreServices& services)
    : services(services) {}

void CppGenerator::generate(const Catalogue& catalogue,
                            const std::filesystem::path& projectDir) const {
  GeneratedCppBatch batch;
  for (const auto* ns : catalogue.elements) {
    generateNamespace(*ns, projectDir);
  }
  generatePackage(catalogue, projectDir);
  batch.run();
}

void CppGenerator::generateNamespace(
    const Namespace& ns, const std::filesystem::path& projectDir) const {
  for (const auto* element : ns.elements) {
    if (const auto* nested = cursiv::ast_cast<const Namespace>(element)) {
      generateNamespace(*nested, projectDir);
    } else if (const auto* type = cursiv::ast_cast<const Type>(element)) {
      generateType(*type, projectDir);
    }
  }
}

void CppGenerator::generatePackage(
    const Catalogue& catalogue, const std::filesystem::path& projectDir) const {
  generatePackageFiles(catalogue, projectDir, includeFolder, sourceFolder);
}

void CppGenerator::generatePackageFiles(
    const Catalogue& catalogue, const std::filesystem::path& projectDir,
    std::string_view headerFolder, std::string_view packageSourceFolder) const {
  const auto name = std::string(catalogueFileName(catalogue));
  cursiv::TextBuilder out;
  generatePackageHeader(out, catalogue);
  generateFile(projectDir, headerFolder, name + ".h", out);
  generatePackageSource(out, catalogue);
  generateFile(projectDir, packageSourceFolder, name + ".cpp", out);
  generateDynamicPackageSource(out, catalogue);
  generateFile(projectDir, packageSourceFolder, name + ".pkg.cpp", out);
}

std::string_view CppGenerator::catalogueFileName(const Catalogue& catalogue) {
  return catalogue.name;
}

std::vector<CppGenerator::Include> CppGenerator::sourceIncludesCatalogue(
    const Catalogue& catalogue) const {
  return {std::string("unordered_set"),
          std::string(catalogueFileName(catalogue)) + ".h"};
}

std::vector<CppGenerator::Include> CppGenerator::factoryIncludes() const {
  return {};
}

void CppGenerator::registerComponent(cursiv::TextBuilder& out,
                                     const Component& component) const {
  if (const auto* model = cursiv::ast_cast<const Model>(&component))
    registerModel(out, *model);
  else
    registerService(out, static_cast<const Service&>(component));
}

void CppGenerator::registerService(cursiv::TextBuilder& out,
                                   const Service& service) const {
  out.line("// Register Service ", service.name);
  out.open("simulator->AddService( new ", fqn(&service), "(");
  out.line("\"", service.name, "\", // Name");
  out.line(description(service), ", // Description");
  out.line("simulator, // Parent");
  out.line("simulator // Simulator");
  out.close("));");
}

void CppGenerator::generatePackageHeader(cursiv::TextBuilder& out,
                                         const Catalogue& catalogue) const {
  const auto file = std::string(catalogueFileName(catalogue)) + ".h";
  const auto guard = includeGuard(std::string(catalogue.name));
  const std::string_view name = catalogue.name;

  notice(out, catalogue);
  fileBanner(out, file, packageHeaderSeparator);
  out.line("/// @file ", file);
  out.line(autoGeneratedLine);
  out.line();
  out.line("#ifndef ", guard);
  out.line("#define ", guard);
  out.line();
  out.write(
      R"(// ----------------------------------------------------------------------------
// --------------------------------- Includes ---------------------------------
// ----------------------------------------------------------------------------
#include <Smp/ISimulator.h>
#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/ITypeRegistry.h>

)");
  out.write(smpVersionDetection);
  out.line();
  out.line("// Entry points for static library");
  out.line("extern \"C\" {");
  out.line("    /// Initialise Package ", name, ".");
  out.write(R"(    /// @param simulator Simulator for registration of factories.
    /// @param typeRegistry Type Registry for registration of types.
    /// @return True if initialisation was successful, false otherwise.
)");
  out.line("    bool Initialise_", name, "(");
  out.write(R"(        ::Smp::ISimulator* simulator,
        ::Smp::Publication::ITypeRegistry* typeRegistry);

#if ECSS_SMP_VERSION >= 202503L
)");
  out.line("    /// Finalise Package ", name, ".");
  out.write(R"(    /// @param simulator Simulator to finalise.
    /// @return True if finalisation was successful, false otherwise.
)");
  out.line("    bool Finalise_", name, "(::Smp::ISimulator* simulator);");
  out.line();
  out.line("    /// Get the revision of the SMP standard Package ", name,
           " was");
  out.write(R"(    /// built against.
    /// @return The revision of the SMP standard.
)");
  out.line("    ::Smp::UInt64 GetSmpVersion_", name, "();");
  out.open("#else");
  out.line("/// Finalise Package ", name, ".");
  out.line("/// @return True if finalisation was successful, false otherwise.");
  out.line("bool Finalise_", name, "();");
  out.close("#endif");
  out.line("}");
  out.line();
  out.line("#endif // ", guard);
}

std::vector<const Type*> CppGenerator::exportedTypes(
    const Catalogue& catalogue) const {
  std::vector<const Type*> types;
  for (const auto* type : allContent<ValueType>(catalogue)) {
    types.push_back(type);
  }
  return dependencyOrder<Type>(types, [](const Type& type) {
    std::vector<const Type*> dependencies;
    if (const auto* array = cursiv::ast_cast<const Array>(&type)) {
      dependencies.push_back(array->itemType.get());
    } else if (const auto* structure =
                   cursiv::ast_cast<const Structure>(&type)) {
      for (const auto* field : getAllFields(*structure)) {
        dependencies.push_back(field->type.get());
      }
      // A class registers with the UUID of its base, which must be known.
      if (const auto* derived = cursiv::ast_cast<const Class>(structure);
          derived && derived->base) {
        dependencies.push_back(derived->base->get());
      }
    }
    return dependencies;
  });
}

std::vector<const Constant*> CppGenerator::sortedConstants(
    std::span<NamedElement* const> elements) const {
  std::vector<const Constant*> constants;
  for (const auto* element : elements) {
    if (const auto* constant = cursiv::ast_cast<const Constant>(element)) {
      constants.push_back(constant);
    }
  }
  return dependencyOrder<Constant>(
      constants, [](const Constant& constant) {
        std::vector<const Constant*> dependencies;
        if (constant.value == nullptr) {
          return dependencies;
        }
        const auto add = [&](const NamedElementReference& reference) {
          if (const auto* dependency = reference.value.get<Constant>()) {
            dependencies.push_back(dependency);
          }
        };
        if (const auto* root =
                cursiv::ast_cast<const NamedElementReference>(constant.value)) {
          add(*root);
        }
        for (const auto* reference :
             allContent<NamedElementReference>(*constant.value)) {
          add(*reference);
        }
        return dependencies;
      });
}

void CppGenerator::registerType(cursiv::TextBuilder& out,
                                const Type& type) const {
  out.line("// register ", nodeTypeName(type), " ", type.name);
  if (cursiv::ast_cast<const Structure>(&type))
    out.line(fqn(&type), "::_Register(typeRegistry);");
  else
    out.line(fqn(cursiv::ast_cast<const NamedElement>(type.parent)),
             "::_Register_", type.name, "(typeRegistry);");
}

void CppGenerator::generatePackageSource(cursiv::TextBuilder& out,
                                         const Catalogue& catalogue) const {
  const auto deps = dependentPackages(catalogue);
  const auto file = std::string(catalogueFileName(catalogue)) + ".cpp";
  const std::string_view name = catalogue.name;
  const auto maybeUnused =
      cxxStandard() >= 17 && deps.empty() && exportedTypes(catalogue).empty()
          ? "[[maybe_unused]] "
          : "";

  notice(out, catalogue);
  fileBanner(out, file, packageSourceSeparator);
  out.line("/// @file ", file);
  out.line(autoGeneratedLine);
  out.line();
  includes(out, packageSourceIncludes(catalogue));
  out.line();
  out.write(
      R"(// -----------------------------------------------------------------------------
// ----------------------------- Global variables ------------------------------
// -----------------------------------------------------------------------------

namespace {
/// Simulators set.
std::unordered_set<::Smp::ISimulator*> simulators { };
} // namespace


// -----------------------------------------------------------------------------
// --------------------------- Initialise Function -----------------------------
// -----------------------------------------------------------------------------

extern "C"
{
)");
  out.line("    /// Initialise Package ", name, ".");
  out.write(R"(    /// @param simulator Simulator for registration of factories.
    /// @param typeRegistry Type Registry for registration of types.
    /// @return True if initialisation was successful, false otherwise.
)");
  out.line("    bool Initialise_", name, "(");
  out.line("            ::Smp::ISimulator* simulator,");
  out.line("            ", maybeUnused,
           "::Smp::Publication::ITypeRegistry* typeRegistry) {");
  out.write(R"(        // check simulator validity
        if (!simulator) {
            return false;
        }
        // avoid double initialisation
        if (!::simulators.emplace(simulator).second) {
            return true;
        }
)");
  {
    const auto body = out.indented(8);
    out.line();
    initialiseDependencies(out, deps);
    out.line();
    registerTypes(out, catalogue);
    out.line();
    registerComponents(out, catalogue);
  }
  out.line();
  out.write(R"(        return true;
    }
}

// ---------------------------------------------------------------------------------
// ---------------------------- Finalise Function ------------------------------
// ---------------------------------------------------------------------------------

extern "C"
{
#if ECSS_SMP_VERSION >= 202503L
)");
  out.line("    /// Finalise Package ", name, ".");
  out.write(R"(    /// @param simulator Simulator to finalise.
    /// @return True if finalisation was successful, false otherwise.
)");
  out.line("    bool Finalise_", name, "(::Smp::ISimulator* simulator) {");
  out.write(R"(        // avoid double finalisation of that simulator
        if (::simulators.erase(simulator) == 0) {
            return true;
        }
)");
  {
    const auto body = out.indented(8);
    finaliseDependencies(out, deps, "simulator");
  }
  out.line("    }");
  out.line();
  out.line("    /// Get the revision of the SMP standard Package ", name,
           " was");
  out.write(R"(    /// built against.
    /// @return The revision of the SMP standard.
)");
  out.line("    ::Smp::UInt64 GetSmpVersion_", name, "() {");
  out.write(R"(        return ECSS_SMP_VERSION;
    }
#else
)");
  out.line("    /// Finalise Package ", name, ".");
  out.line(
      "    /// @return True if finalisation was successful, false otherwise.");
  out.line("    bool Finalise_", name, "() {");
  out.write(R"(        // avoid double finalisation
        if (::simulators.empty()) {
            return true;
        }
        ::simulators.clear();

)");
  {
    const auto body = out.indented(8);
    finaliseDependencies(out, deps, "");
  }
  out.write(R"(    }
#endif
}
)");
}

std::vector<CppGenerator::Include> CppGenerator::packageSourceIncludes(
    const Catalogue& catalogue) const {
  const auto comps = components(catalogue);
  auto result = sourceIncludesCatalogue(catalogue);
  for (const auto* comp : comps) result.emplace_back(comp);
  if (std::ranges::any_of(comps, [](const Component* comp) {
        return cursiv::ast_cast<const Model>(comp) != nullptr;
      }))
    appendTo(result, factoryIncludes());
  for (const auto* type : exportedTypes(catalogue)) result.emplace_back(type);
  for (const auto* dep : dependentPackages(catalogue))
    result.emplace_back(std::string(dep->name) + ".h");
  return result;
}

void CppGenerator::initialiseDependencies(
    cursiv::TextBuilder& out, std::span<const Catalogue* const> dependencies) {
  if (dependencies.empty()) return;
  out.line("// Initialisation of dependent Package(s)");
  out.write("if(");
  std::string_view separator;
  for (const auto* dep : dependencies) {
    out.write(separator, "!Initialise_", dep->name,
              "(simulator, typeRegistry)");
    separator = " || ";
  }
  out.line(") {");
  out.line("    return false;");
  out.line("}");
}

void CppGenerator::finaliseDependencies(
    cursiv::TextBuilder& out, std::span<const Catalogue* const> dependencies,
    std::string_view argument) {
  if (dependencies.empty()) {
    out.line("return true;");
    return;
  }
  out.line("// Finalisation of dependent Package(s)");
  out.write("return ");
  std::string_view separator;
  for (const auto* dep : dependencies) {
    out.write(separator, "Finalise_", dep->name, "(", argument, ")");
    separator = " && ";
  }
  out.line(";");
}

void CppGenerator::registerTypes(cursiv::TextBuilder& out,
                                 const Catalogue& catalogue) const {
  for (const auto* type : exportedTypes(catalogue)) registerType(out, *type);
}

void CppGenerator::registerComponents(cursiv::TextBuilder& out,
                                      const Catalogue& catalogue) const {
  for (const auto* comp : components(catalogue)) {
    registerComponent(out, *comp);
    out.line();
  }
}

void CppGenerator::generateDynamicPackageSource(
    cursiv::TextBuilder& out, const Catalogue& catalogue) const {
  const auto catalogueFile = catalogueFileName(catalogue);
  const auto file = std::string(catalogueFile) + ".pkg.cpp";
  const std::string_view name = catalogue.name;

  notice(out, catalogue);
  fileBanner(out, file, packageSourceSeparator);
  out.line("/// @file ", file);
  out.line(autoGeneratedLine);
  out.line();
  out.write(
      R"(// -----------------------------------------------------------------------------
// --------------------------------- Includes ----------------------------------
// -----------------------------------------------------------------------------
)");
  out.line("#include <", catalogueFile, ".h>");
  out.write(R"(#include <Smp/ISimulator.h>
#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/ITypeRegistry.h>

)");
  out.write(smpVersionDetection);
  out.line();
  out.write(R"(#ifdef WIN32
#define DLL_EXPORT __declspec(dllexport) // %RELAX<mconst> Visual Studio requires a define
#else
#define DLL_EXPORT
#endif

// -----------------------------------------------------------------------------
// -------------------------- Initialise Function ------------------------------
// -----------------------------------------------------------------------------

extern "C" {
)");
  out.line("    /// Global Initialise function of Package ", name, ".");
  out.write(R"(    /// @param simulator Simulator for registration of factories.
    /// @param typeRegistry Type Registry for registration of types.
    /// @return True if initialisation was successful, false otherwise.
    DLL_EXPORT bool Initialise(
            ::Smp::ISimulator* simulator,
            ::Smp::Publication::ITypeRegistry* typeRegistry) {
)");
  out.line("        return Initialise_", name, "(simulator, typeRegistry);");
  out.write(R"(    }
}

// -----------------------------------------------------------------------------
// ---------------------------- Finalise Function ------------------------------
// -----------------------------------------------------------------------------

extern "C" {
)");
  out.line("    /// Global Finalise function of Package ", name, ".");
  out.write(R"(    /// @param simulator Simulator.
    /// @return True if finalisation was successful, false otherwise.
    DLL_EXPORT bool Finalise(::Smp::ISimulator* simulator) {
#if ECSS_SMP_VERSION >= 202503L
)");
  out.line("        return Finalise_", name, "(simulator);");
  out.line("#else");
  out.line("        static_cast<void>(simulator);");
  out.line("        return Finalise_", name, "();");
  out.write(R"(#endif
    }
}

// -----------------------------------------------------------------------------
// ------------------------- GetSmpVersion Function ----------------------------
// -----------------------------------------------------------------------------

#if ECSS_SMP_VERSION >= 202503L
extern "C" {
)");
  out.line("    /// Global GetSmpVersion function of Package ", name, ".");
  out.write(
      R"(    /// The simulator refuses to load a package built against another
    /// revision of the standard than its own.
    /// @return The revision of the SMP standard this package was built
    /// against.
    DLL_EXPORT ::Smp::UInt64 GetSmpVersion() {
)");
  out.line("        return GetSmpVersion_", name, "();");
  out.write(R"(    }
}
#endif
)");
}

std::vector<const Component*> CppGenerator::components(
    const Catalogue& catalogue) const {
  std::vector<const Component*> result;
  for (const auto* component : allContent<Component>(catalogue)) {
    if (!isAbstractType(*component)) {
      result.push_back(component);
    }
  }
  return result;
}

std::vector<const Catalogue*> CppGenerator::dependentPackages(
    const Catalogue& catalogue) const {
  std::vector<const Catalogue*> result;
  const auto add = [&](const cursiv::AstNode* target) {
    const auto* dependency = catalogueOf(target);
    if (dependency == nullptr || dependency == &catalogue
        || isBuiltinUri(documentOf(*dependency).uri)
        || std::ranges::find(result, dependency) != result.end())
      return;
    result.push_back(dependency);
  };
  for (const auto* node : descendantsOf(catalogue)) {
    if (const auto* field = cursiv::ast_cast<const Field>(node)) {
      add(field->type.get());
    } else if (const auto* property = cursiv::ast_cast<const Property>(node)) {
      add(property->type.get());
    } else if (const auto* component =
                   cursiv::ast_cast<const Component>(node)) {
      if (component->base.has_value()) add(component->base->get());
    } else if (const auto* clazz = cursiv::ast_cast<const Class>(node)) {
      if (clazz->base.has_value()) add(clazz->base->get());
    } else if (const auto* array = cursiv::ast_cast<const Array>(node)) {
      add(array->itemType.get());
    }
  }
  std::ranges::stable_sort(result, alphabeticalLess,
                           [](const Catalogue* dependency) {
                             return std::string_view(dependency->name);
                           });
  return result;
}

void CppGenerator::notice(cursiv::TextBuilder& out,
                          const cursiv::AstNode& node) const {
  if (const auto text =
          getCopyrightNotice(*services.commentProvider, node, "// "))
    out.write(*text).line();
}

void CppGenerator::fileBanner(cursiv::TextBuilder& out,
                              std::string_view fileName,
                              std::string_view separator) {
  out.line(separator);
  out.line("// File Name    : ", fileName);
  out.line(separator);
}

void CppGenerator::generateFile(const std::filesystem::path& projectDir,
                                std::string_view folder,
                                std::string_view relativePath,
                                cursiv::TextBuilder& out) const {
  writeGeneratedCpp(projectDir / folder / relativePath,
                    tidied(utf8Text(out.take())));
}

void CppGenerator::generateUserFile(const std::filesystem::path& projectDir,
                                    std::string_view folder,
                                    std::string_view relativePath,
                                    cursiv::TextBuilder& out) const {
  writeUserCpp(projectDir / folder / relativePath,
               tidied(utf8Text(out.take())));
}

namespace {

// Where a backslash ends `line` before spaces only, or `npos`. C++ joins such a
// line to the next one, even with the spaces. The spaces are those of
// JavaScript's `\s`, as the other XSMP tools test them, and a `\r` before the
// line break.
std::size_t spliceBackslash(std::string_view line, bool lastLine) {
  static constexpr std::string_view spaces[] = {" ",
                                                "\t",
                                                "\v",
                                                "\f",
                                                "\xC2\xA0",
                                                "\xE1\x9A\x80",
                                                "\xE2\x80\x80",
                                                "\xE2\x80\x81",
                                                "\xE2\x80\x82",
                                                "\xE2\x80\x83",
                                                "\xE2\x80\x84",
                                                "\xE2\x80\x85",
                                                "\xE2\x80\x86",
                                                "\xE2\x80\x87",
                                                "\xE2\x80\x88",
                                                "\xE2\x80\x89",
                                                "\xE2\x80\x8A",
                                                "\xE2\x80\xA8",
                                                "\xE2\x80\xA9",
                                                "\xE2\x80\xAF",
                                                "\xE2\x81\x9F",
                                                "\xE3\x80\x80",
                                                "\xEF\xBB\xBF"};
  if (!lastLine && line.ends_with('\r')) line.remove_suffix(1);
  for (auto trimmed = true; trimmed;) {
    trimmed = false;
    for (const auto space : spaces)
      if (line.ends_with(space)) {
        line.remove_suffix(space.size());
        trimmed = true;
        break;
      }
  }
  return line.ends_with('\\') ? line.size() - 1 : std::string_view::npos;
}

}  // namespace

void CppGenerator::comment(cursiv::TextBuilder& out,
                           const NamedElement& element) const {
  const auto parsed = docComment(element);
  if (!parsed) return;
  std::string result;
  auto first = true;
  const auto append = [&](std::string text) {
    while (!text.empty()
           && std::isspace(static_cast<unsigned char>(text.back())))
      text.pop_back();
    if (!first) result += "\n///";
    first = false;
    for (const auto c : text) {
      result += c;
      if (c == '\n') result += "///";
    }
  };
  for (const auto& part : parsed->elements) {
    if (part.kind != cursiv::DocCommentNode::Kind::Tag) {
      append(part.toString());
      continue;
    }
    if (!part.inlineTag && (part.name == "uuid" || part.name == "id")) continue;
    auto text = part.toString();
    // The text of a parameter stays on the line of its tag.
    if (part.name == "param") {
      auto content = part;
      content.kind = cursiv::DocCommentNode::Kind::Paragraph;
      text = "@param " + content.toString();
    }
    append(" " + text);
  }
  if (result.empty()) return;
  const auto protectedText = protectTrigraphs(std::move(result));
  std::string_view rest = protectedText;
  std::string_view prefix = "///";
  while (true) {
    const auto end = rest.find('\n');
    const auto line = rest.substr(0, end);
    // A final backslash would carry the comment over to the next line of the
    // generated code: it is written as its HTML entity.
    if (const auto backslash =
            spliceBackslash(line, end == std::string_view::npos);
        backslash != std::string_view::npos)
      out.line(prefix, line.substr(0, backslash), "&#92;",
               line.substr(backslash + 1));
    else
      out.line(prefix, line);
    prefix = {};
    if (end == std::string_view::npos) break;
    rest.remove_prefix(end + 1);
  }
}

std::string CppGenerator::fqn(const NamedElement* node,
                              std::string_view defaultFqn) const {
  if (node == nullptr) {
    return std::string(defaultFqn);
  }
  return "::" + qualifiedPath(node, "::");
}

std::string CppGenerator::qualifiedPath(const NamedElement* node,
                                        std::string_view separator) const {
  if (node == nullptr) {
    return {};
  }
  const auto dotted = qualifiedName(*node);
  std::string result;
  for (const auto c : dotted) {
    if (c == '.') {
      result += separator;
    } else {
      result += c;
    }
  }
  return result;
}

std::string CppGenerator::uuid(const Type* type) const {
  if (type == nullptr) {
    return "::Smp::Uuids::Uuid_Void";
  }
  const auto dotted = qualifiedName(*type);
  if (smpUuidsTypes().contains(dotted)) {
    return "::Smp::Uuids::Uuid_" + std::string(type->name);
  }
  return fqn(cursiv::ast_cast<const NamedElement>(type->parent)) + "::Uuid_"
         + std::string(type->name);
}

std::string CppGenerator::protectTrigraphs(std::string text) const {
  if (cxxStandard() >= 17) return text;
  for (auto at = text.find("??"); at != std::string::npos;
       at = text.find("??", at + 1))
    if (at + 2 < text.size()
        && std::string_view("=/'()!<>-").find(text[at + 2])
               != std::string_view::npos)
      text.replace(at, 2, "?\\?");
  return text;
}

std::string CppGenerator::escapedText(std::string_view text) const {
  std::string result;
  for (const auto c : cppEscaped(text)) {
    if (c == '\'') result += '\\';
    result += c;
  }
  return protectTrigraphs(std::move(result));
}

std::string CppGenerator::description(const cursiv::AstNode& node) const {
  return "\"" + escapedText(docDescription(node).value_or("")) + "\"";
}

std::string CppGenerator::viewKind(const NamedElement& element,
                                   std::string_view defaultViewKind) const {
  const auto kind = getViewKind(element);
  return kind ? "::Smp::ViewKind::VK_" + std::string(*kind)
              : std::string(defaultViewKind);
}

std::string CppGenerator::includeGuard(std::string path) {
  std::ranges::transform(path, path.begin(), [](unsigned char c) {
    return static_cast<char>(std::toupper(c));
  });
  return path + "_H_";
}

std::string CppGenerator::expression(const Expression* expr) const {
  if (!expr) return {};
  if (const auto* collection =
          cursiv::ast_cast<const CollectionLiteral>(expr)) {
    const auto* expected = services.typeProvider->getType(*expr);
    const auto* array = cursiv::ast_cast<const Array>(expected);
    const auto nestedArray =
        array && cursiv::ast_cast<const Array>(array->itemType.get());
    const bool structure = cursiv::ast_cast<const Structure>(expected);
    std::string result = "{ ";
    std::string_view separator;
    for (const auto* element : collection->elements) {
      result += separator;
      separator = ", ";
      // An array nested in an array or in a structure needs its own braces.
      if (nestedArray
          || (structure
              && cursiv::ast_cast<const Array>(
                  services.typeProvider->getType(*element))))
        result += "{" + expression(element) + "}";
      else
        result += expression(element);
    }
    return result + "}";
  }
  if (const auto* designated =
          cursiv::ast_cast<const DesignatedInitializer>(expr))
    return "/* ." + std::string(designated->field.refText) + " = */"
           + expression(designated->expr);
  if (const auto* unary = cursiv::ast_cast<const UnaryOperation>(expr)) {
    // `- -1` must not become the decrement `--1`.
    const auto operand = expression(unary->operand);
    return std::string(unary->feature)
           + (operand.starts_with(unary->feature) ? " " : "") + operand;
  }
  if (const auto* binary = cursiv::ast_cast<const BinaryOperation>(expr))
    return expression(binary->leftOperand) + " " + std::string(binary->feature)
           + " " + expression(binary->rightOperand);
  if (const auto* parenthesized =
          cursiv::ast_cast<const ParenthesizedExpression>(expr))
    return "(" + expression(parenthesized->expr) + ")";
  if (const auto* builtin = cursiv::ast_cast<const BuiltInConstant>(expr))
    return "M_" + std::string(builtin->name);
  if (const auto* builtin = cursiv::ast_cast<const BuiltInFunction>(expr)) {
    // A math function is not `constexpr` in C++, and a constant has to be: the
    // generated code holds its value. A value with no literal stays a call.
    if (const auto value = getValue(expr);
        value && std::isfinite(value->floating))
      return floatingLiteral(value->floating, value->type);
    return "std::" + std::string(builtin->name) + "("
           + expression(builtin->argument) + ")";
  }
  if (const auto* integer = cursiv::ast_cast<const IntegerLiteral>(expr)) {
    // An integer given to an Enumeration is written as its literal.
    const auto* type = services.typeProvider->getType(*expr);
    if (cursiv::ast_cast<const Enumeration>(type))
      if (const auto value = getValueAs(expr, *type); value && value->literal)
        return fqn(value->literal);
    auto text = integer_value(integer->text);
    // Digit separators need C++14.
    if (cxxStandard() < 14) std::erase(text, '\'');
    return text;
  }
  if (const auto* literal = cursiv::ast_cast<const FloatingLiteral>(expr)) {
    std::string text(literal->text);
    std::erase(text, '\'');
    return text;
  }
  if (cursiv::ast_cast<const StringLiteral>(expr)) {
    // A DateTime or a Duration written as a string is an integer in C++.
    const auto kind = getPTK(services.typeProvider->getType(*expr));
    if (kind == PTK::DateTime || kind == PTK::Duration)
      if (const auto value = getValueAs(expr, kind))
        return std::to_string(static_cast<std::int64_t>(value->integral)) + "L";
  }
  if (const auto* keyword = cursiv::ast_cast<const KeywordExpression>(expr))
    return std::string(keyword->name);
  if (const auto* ref = cursiv::ast_cast<const NamedElementReference>(expr)) {
    const NamedElement* target = ref->value.get<Constant>();
    if (!target) target = ref->value.get<EnumerationLiteral>();
    // A name stays short inside its own type, and is qualified anywhere else.
    const auto* owner = expr->parent;
    while (owner && !cursiv::ast_cast<const Type>(owner)) owner = owner->parent;
    return target && target->parent == owner ? std::string(target->name)
                                             : fqn(target);
  }

  // Only a Boolean, a String and a Character literal are left.
  const auto value = getValue(expr);
  if (!value) return {};
  switch (value->type) {
    case PTK::Bool:
      return value->boolean ? "true" : "false";
    case PTK::String8:
      return "\"" + protectTrigraphs(cppEscaped(value->text)) + "\"";
    case PTK::Char8:
      return "'" + cppEscaped(value->text, '\'') + "'";
    default:
      return {};
  }
}

std::string CppGenerator::directListInitializer(const Expression* expr) const {
  if (expr == nullptr) {
    return "{}";
  }
  const auto* type = services.typeProvider->getType(*expr);
  if (cursiv::ast_cast<const Structure>(type) != nullptr) {
    return expression(expr);
  }
  return "{" + expression(expr) + "}";
}

void CppGenerator::openNamespace(cursiv::TextBuilder& out, const Type& type,
                                 bool comments) const {
  const auto blocks = namespaceBlocks(type);
  for (std::size_t i = 0; i < blocks.size(); ++i) {
    if (comments || i + 1 < blocks.size()) comment(out, *blocks[i].ns);
    out.line("namespace ", blocks[i].name);
    out.line("{");
  }
}

void CppGenerator::closeNamespace(cursiv::TextBuilder& out,
                                  const Type& type) const {
  for (const auto& block : std::views::reverse(namespaceBlocks(type)))
    out.line("} // namespace ", block.name);
}

std::vector<CppGenerator::NamespaceBlock> CppGenerator::namespaceBlocks(
    const Type& type) const {
  const auto* ns = namespaceOf(&type);
  if (cxxStandard() >= 17) return {{ns, qualifiedPath(ns, "::")}};
  std::vector<NamespaceBlock> blocks;
  for (; ns != nullptr; ns = namespaceOf(ns->parent))
    blocks.insert(blocks.begin(), {ns, std::string(ns->name)});
  return blocks;
}

void CppGenerator::uuidDeclaration(cursiv::TextBuilder& out,
                                   const Type& type) const {
  std::string groups;
  if (const auto value = docTag(type, "uuid")) {
    groups = "0x";
    for (const auto c : *value) {
      if (c == '-') {
        groups += "U, 0x";
      } else {
        groups += c;
      }
    }
    groups += "U";
  }
  out.line("/// Universally unique identifier of type ", type.name, ".");
  out.line(cxxStandard() >= 17 ? "inline " : "", "constexpr ::Smp::Uuid Uuid_",
           type.name, " { ", groups, " };");
}

std::string CppGenerator::primitiveTypeKind(const Type* type) const {
  const auto kind = getPTK(type);
  if (kind == PTK::Enum) {
    return "::Smp::PrimitiveTypeKind::PTK_Int32";
  }
  return "::Smp::PrimitiveTypeKind::PTK_" + std::string(ptkName(kind));
}

std::string CppGenerator::lower(
    const NamedElementWithMultiplicity& element) const {
  if (element.optional_) return "0";
  const auto* multiplicity = element.multiplicity;
  if (multiplicity == nullptr) return "1";
  if (multiplicity->lower == nullptr && multiplicity->upper == nullptr)
    return multiplicity->aux ? "1" : "0";
  return expression(multiplicity->lower);
}

std::string CppGenerator::upper(
    const NamedElementWithMultiplicity& element) const {
  const auto* multiplicity = element.multiplicity;
  if (element.optional_ || multiplicity == nullptr) return "1";
  if (multiplicity->lower == nullptr && multiplicity->upper == nullptr)
    return "-1";
  if (multiplicity->upper == nullptr)
    return multiplicity->aux ? "-1" : expression(multiplicity->lower);
  return expression(multiplicity->upper);
}

std::string_view CppGenerator::componentKind(const Component& component) {
  return cursiv::ast_cast<const Model>(&component) ? "Model" : "Service";
}

std::string CppGenerator::typeInclude(const Type& type) const {
  const auto dotted = qualifiedName(type);
  if (const auto primitive = primitiveTypeInclude(dotted)) {
    return std::string(*primitive);
  }
  if (builtinStandard(documentOf(type).uri) == "ECSS_SMP_2020") {
    if (const auto collection = collectionInclude2020(dotted)) {
      return std::string(*collection);
    }
  }
  return qualifiedPath(&type, "/") + ".h";
}

void CppGenerator::includes(cursiv::TextBuilder& out,
                            const std::vector<Include>& includeList,
                            const std::vector<Include>& excludeList) const {
  std::vector<std::string> declarations;
  std::vector<const Type*> forwarded;
  for (const auto& include : includeList) {
    if (const auto* literal = std::get_if<std::string>(&include))
      declarations.push_back(*literal);
    else if (const auto* type = std::get_if<const Type*>(&include);
             type && *type)
      declarations.push_back(typeInclude(**type));
    else if (const auto* forward = std::get_if<ForwardedType>(&include);
             forward && forward->type) {
      if (std::ranges::find(forwarded, forward->type) == forwarded.end())
        forwarded.push_back(forward->type);
      if (forward->withHeader)
        declarations.push_back(typeInclude(*forward->type));
    }
  }

  for (const auto& exclude : excludeList) {
    if (const auto* literal = std::get_if<std::string>(&exclude))
      std::erase(declarations, *literal);
    else if (const auto* type = std::get_if<const Type*>(&exclude);
             type && *type)
      std::erase(declarations, typeInclude(**type));
    else if (const auto* forward = std::get_if<ForwardedType>(&exclude))
      std::erase(forwarded, forward->type);
  }

  std::ranges::stable_sort(declarations, alphabeticalLess);
  const auto duplicates = std::ranges::unique(declarations);
  declarations.erase(duplicates.begin(), duplicates.end());

  if (!forwarded.empty()) {
    out.write(
        R"(// ----------------------------------------------------------------------------
// --------------------------- Forward Declarations ---------------------------
// ----------------------------------------------------------------------------

)");
    for (const auto* type : forwarded) {
      openNamespace(out, *type, false);
      out.line(cursiv::ast_cast<const Structure>(type)
                       && !cursiv::ast_cast<const Class>(type)
                   ? "struct "
                   : "class ",
               type->name, ";");
      closeNamespace(out, *type);
      out.line();
    }
  }
  if (!declarations.empty()) {
    out.write(
        R"(// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

)");
    for (const auto& header : declarations) {
      out.line("#include <", header, ">");
    }
  }
  out.line();
}

template <class T>
CppGenerator::Include CppGenerator::pointerMemberInclude(
    const T& member) const {
  const auto* type = member.type.get();
  return isByPointer(member) && isForwardable(type)
             ? Include{ForwardedType{type}}
             : Include{type};
}

// A type used in a signature is declared first: its header may lead back here,
// and is then skipped by its include guard. By pointer or by reference, the
// declaration is enough, unless a default value needs the type.
template <class T>
std::vector<CppGenerator::Include> CppGenerator::signatureIncludes(
    const T& element) const {
  const auto* type = element.type.get();
  if (!isForwardable(type)) return {type};
  auto withDefault = false;
  if constexpr (std::is_same_v<T, Parameter>)
    withDefault = element.default_ != nullptr;
  const auto declarationOnly =
      isByPointer(element) || (isByReference(element) && !withDefault);
  return {ForwardedType{type, !declarationOnly}};
}

std::vector<CppGenerator::Include> CppGenerator::headerIncludes(
    const NamedElement& element) const {
  // Each member of a class, an interface or a component, by its kind.
  const auto ofMembers = [this](std::vector<Include> result,
                                std::span<NamedElement* const> elements) {
    for (const auto* member : elements)
      appendTo(result, headerIncludes(*member));
    return result;
  };
  const auto bounds = [this](const auto& type) {
    std::vector<Include> result{std::string("Smp/Publication/ITypeRegistry.h"),
                                std::string("Smp/PrimitiveTypes.h")};
    appendTo(result, expressionIncludes(type.minimum));
    appendTo(result, expressionIncludes(type.maximum));
    return result;
  };
  if (const auto* type = cursiv::ast_cast<const Class>(&element)) {
    auto result = ofMembers({std::string("Smp/Publication/ITypeRegistry.h")},
                            type->elements);
    if (type->base.has_value()) result.emplace_back(type->base->get());
    return result;
  }
  if (const auto* type = cursiv::ast_cast<const Structure>(&element))
    return headerIncludesStructure(*type);
  if (const auto* type = cursiv::ast_cast<const Integer>(&element))
    return bounds(*type);
  if (const auto* type = cursiv::ast_cast<const Float>(&element))
    return bounds(*type);
  if (const auto* type = cursiv::ast_cast<const Component>(&element))
    return headerIncludesComponent(*type);
  if (const auto* type = cursiv::ast_cast<const Interface>(&element)) {
    auto result = ofMembers({std::string("Smp/Uuid.h")}, type->elements);
    for (const auto& base : type->base) result.emplace_back(base.get());
    result.emplace_back(ForwardedType{type});
    return result;
  }
  if (const auto* type = cursiv::ast_cast<const Array>(&element))
    return headerIncludesArray(*type);
  if (cursiv::ast_cast<const Enumeration>(&element))
    return {std::string("Smp/Publication/ITypeRegistry.h"),
            std::string("Smp/PrimitiveTypes.h"), std::string("map"),
            std::string("string")};
  if (const auto* type = cursiv::ast_cast<const String>(&element))
    return headerIncludesString(*type);
  if (const auto* type = cursiv::ast_cast<const NativeType>(&element)) {
    std::vector<Include> result{std::string("Smp/Publication/ITypeRegistry.h")};
    if (auto location = docTag(*type, "location"))
      result.emplace_back(std::move(*location));
    return result;
  }
  if (const auto* member = cursiv::ast_cast<const Association>(&element))
    return {pointerMemberInclude(*member)};
  if (const auto* member = cursiv::ast_cast<const Constant>(&element)) {
    std::vector<Include> result{member->type.get()};
    appendTo(result, expressionIncludes(member->value));
    return result;
  }
  if (const auto* member = cursiv::ast_cast<const Container>(&element))
    return headerIncludesContainer(*member);
  if (const auto* member = cursiv::ast_cast<const EntryPoint>(&element))
    return headerIncludesEntryPoint(*member);
  if (const auto* member = cursiv::ast_cast<const EventSink>(&element))
    return headerIncludesEventSink(*member);
  if (const auto* member = cursiv::ast_cast<const EventSource>(&element))
    return headerIncludesEventSource(*member);
  if (const auto* member = cursiv::ast_cast<const Field>(&element))
    return headerIncludesField(*member);
  if (const auto* member = cursiv::ast_cast<const Operation>(&element))
    return headerIncludesOperation(*member);
  if (const auto* member = cursiv::ast_cast<const Property>(&element))
    return {pointerMemberInclude(*member)};
  if (const auto* member = cursiv::ast_cast<const Reference>(&element))
    return headerIncludesReference(*member);
  if (const auto* type = cursiv::ast_cast<const ValueReference>(&element))
    return {type->type.get()};
  return {};
}

std::vector<CppGenerator::Include> CppGenerator::headerIncludesParameter(
    const Parameter& element) const {
  auto result = signatureIncludes(element);
  appendTo(result, expressionIncludes(element.default_));
  return result;
}

std::vector<CppGenerator::Include> CppGenerator::headerIncludesOperation(
    const Operation& element) const {
  std::vector<Include> result;
  if (const auto* returned = element.returnParameter)
    appendTo(result, signatureIncludes(*returned));
  for (const auto* parameter : element.parameter)
    appendTo(result, headerIncludesParameter(*parameter));
  return result;
}

std::vector<CppGenerator::Include> CppGenerator::headerIncludesWithoutCycle(
    const Type& type) const {
  auto result = headerIncludes(type);
  for (auto& include : result) {
    auto* forward = std::get_if<ForwardedType>(&include);
    if (!forward || !forward->withHeader) continue;
    std::vector<const Type*> seen;
    forward->withHeader = !needsDefinitionOf(*forward->type, type, seen);
  }
  return result;
}

bool CppGenerator::needsDefinitionOf(const Type& from, const Type& target,
                                     std::vector<const Type*>& seen) const {
  if (std::ranges::find(seen, &from) != seen.end()) return false;
  seen.push_back(&from);
  for (const auto& include : headerIncludes(from)) {
    const Type* next = nullptr;
    if (const auto* type = std::get_if<const Type*>(&include)) {
      if (*type == &target) return true;
      next = *type;
    } else if (const auto* forward = std::get_if<ForwardedType>(&include);
               forward && forward->withHeader) {
      next = forward->type;
    }
    if (next && needsDefinitionOf(*next, target, seen)) return true;
  }
  return false;
}

std::vector<CppGenerator::Include> CppGenerator::headerIncludesContainer(
    const Container& element) const {
  return {element.type.get()};
}

std::vector<CppGenerator::Include> CppGenerator::headerIncludesEntryPoint(
    const EntryPoint& /*element*/) const {
  return {};
}

std::vector<CppGenerator::Include> CppGenerator::headerIncludesEventSink(
    const EventSink& element) const {
  if (const auto* payload = eventType(element)) return {payload};
  return {};
}

std::vector<CppGenerator::Include> CppGenerator::headerIncludesEventSource(
    const EventSource& element) const {
  if (const auto* payload = eventType(element)) return {payload};
  return {};
}

std::vector<CppGenerator::Include> CppGenerator::headerIncludesField(
    const Field& element) const {
  std::vector<Include> result{element.type.get()};
  appendTo(result, expressionIncludes(element.default_));
  return result;
}

std::vector<CppGenerator::Include> CppGenerator::headerIncludesReference(
    const Reference& element) const {
  return {element.interface_.get()};
}

std::vector<CppGenerator::Include> CppGenerator::headerIncludesStructure(
    const Structure& type) const {
  // The Publish declaration defaults its view kind.
  std::vector<Include> result{std::string("Smp/Publication/ITypeRegistry.h"),
                              std::string("Smp/ViewKind.h")};
  for (const auto* member : type.elements)
    appendTo(result, headerIncludes(*member));
  return result;
}

std::vector<CppGenerator::Include> CppGenerator::headerIncludesComponent(
    const Component& type) const {
  std::vector<Include> result{std::string("Smp/Publication/ITypeRegistry.h")};
  for (const auto* member : type.elements)
    appendTo(result, headerIncludes(*member));
  if (type.base.has_value()) result.emplace_back(type.base->get());
  for (const auto& interface : type.interface_)
    result.emplace_back(interface.get());
  result.emplace_back(ForwardedType{&type});
  return result;
}

std::vector<CppGenerator::Include> CppGenerator::headerIncludesArray(
    const Array& type) const {
  std::vector<Include> result{std::string("Smp/Publication/ITypeRegistry.h"),
                              type.itemType.get()};
  appendTo(result, expressionIncludes(type.size));
  return result;
}

std::vector<CppGenerator::Include> CppGenerator::headerIncludesString(
    const String& type) const {
  std::vector<Include> result{std::string("Smp/Publication/ITypeRegistry.h")};
  appendTo(result, expressionIncludes(type.length));
  return result;
}

void CppGenerator::sourcePrologue(cursiv::TextBuilder& /*out*/,
                                  const Type& /*type*/) const {}

void CppGenerator::smpVersionPrologue(cursiv::TextBuilder& out) {
  out.write(smpVersionDetection);
  out.line();
}

std::vector<CppGenerator::Include> CppGenerator::sourceIncludes(
    const NamedElement& element) const {
  const auto ofMembers = [this](std::vector<Include> result,
                                std::span<NamedElement* const> elements) {
    for (const auto* member : elements)
      appendTo(result, sourceIncludes(*member));
    return result;
  };
  const auto pointerMember = [](const auto& member) -> std::vector<Include> {
    const auto* type = member.type.get();
    if (isByPointer(member)
        && (cursiv::ast_cast<const Structure>(type)
            || cursiv::ast_cast<const ReferenceType>(type)))
      return {type};
    return {};
  };
  const auto bounds = [this](const auto& type) {
    auto result = expressionIncludes(type.minimum);
    appendTo(result, expressionIncludes(type.maximum));
    if (type.minimum == nullptr || type.maximum == nullptr)
      result.emplace_back(std::string("limits"));
    return result;
  };
  if (const auto* type = cursiv::ast_cast<const Class>(&element))
    return ofMembers(
        {std::string("cstddef"), std::string("Smp/Publication/IClassType.h")},
        type->elements);
  // `offsetof` needs `<cstddef>`.
  if (const auto* type = cursiv::ast_cast<const Structure>(&element))
    return ofMembers({std::string("cstddef"),
                      std::string("Smp/Publication/IStructureType.h"),
                      std::string("Smp/ViewKind.h")},
                     type->elements);
  if (const auto* type = cursiv::ast_cast<const Integer>(&element))
    return bounds(*type);
  if (const auto* type = cursiv::ast_cast<const Float>(&element))
    return bounds(*type);
  if (const auto* type = cursiv::ast_cast<const Component>(&element))
    return sourceIncludesComponent(*type);
  if (const auto* type = cursiv::ast_cast<const Interface>(&element))
    return ofMembers({}, type->elements);
  if (const auto* type = cursiv::ast_cast<const Array>(&element)) {
    std::vector<Include> result{std::string("Smp/Publication/IArrayType.h")};
    appendTo(result, expressionIncludes(type->size));
    return result;
  }
  if (const auto* type = cursiv::ast_cast<const Enumeration>(&element))
    return sourceIncludesEnumeration(*type);
  if (const auto* type = cursiv::ast_cast<const String>(&element)) {
    std::vector<Include> result{std::string("Smp/Publication/IType.h")};
    appendTo(result, expressionIncludes(type->length));
    return result;
  }
  if (const auto* member = cursiv::ast_cast<const Association>(&element))
    return pointerMember(*member);
  if (const auto* member = cursiv::ast_cast<const EntryPoint>(&element))
    return sourceIncludesEntryPoint(*member);
  if (const auto* member = cursiv::ast_cast<const EventSink>(&element))
    return sourceIncludesEventSink(*member);
  if (const auto* member = cursiv::ast_cast<const Operation>(&element))
    return sourceIncludesOperation(*member);
  if (const auto* member = cursiv::ast_cast<const Property>(&element))
    return pointerMember(*member);
  return {};
}

std::vector<CppGenerator::Include> CppGenerator::sourceIncludesEntryPoint(
    const EntryPoint& /*element*/) const {
  return {};
}

std::vector<CppGenerator::Include> CppGenerator::sourceIncludesEventSink(
    const EventSink& /*element*/) const {
  return {};
}

// The header only declares a type taken by pointer or by reference: the source
// that uses it includes it.
template <class T>
std::vector<CppGenerator::Include> CppGenerator::signatureSourceIncludes(
    const T& element) const {
  const auto* type = element.type.get();
  if ((isByPointer(element) || isByReference(element)) && isForwardable(type))
    return {type};
  return {};
}

std::vector<CppGenerator::Include> CppGenerator::sourceIncludesParameter(
    const Parameter& element) const {
  return signatureSourceIncludes(element);
}

std::vector<CppGenerator::Include> CppGenerator::sourceIncludesParameter(
    const ReturnParameter& element) const {
  return signatureSourceIncludes(element);
}

std::vector<CppGenerator::Include> CppGenerator::sourceIncludesOperation(
    const Operation& element) const {
  std::vector<Include> result;
  if (const auto* returned = element.returnParameter)
    appendTo(result, sourceIncludesParameter(*returned));
  for (const auto* parameter : element.parameter)
    appendTo(result, sourceIncludesParameter(*parameter));
  // The IPublishOperation returned by PublishOperation is dereferenced to
  // publish the parameters, and `Smp/IPublication.h` only forward declares it.
  if (isInvokable(element)
      && (element.returnParameter != nullptr || !element.parameter.empty()))
    result.emplace_back(std::string("Smp/Publication/IPublishOperation.h"));
  return result;
}

std::vector<CppGenerator::Include> CppGenerator::sourceIncludesEnumeration(
    const Enumeration& /*type*/) const {
  return {std::string("Smp/Publication/IEnumerationType.h")};
}

std::vector<CppGenerator::Include> CppGenerator::sourceIncludesComponent(
    const Component& type) const {
  // The members are published with a view kind.
  std::vector<Include> result{std::string("Smp/ViewKind.h")};
  for (const auto* member : type.elements)
    appendTo(result, sourceIncludes(*member));
  return result;
}

std::vector<CppGenerator::Include> CppGenerator::expressionIncludes(
    const Expression* expr) const {
  std::vector<Include> result;
  if (expr == nullptr) return result;
  const auto add = [&](const cursiv::AstNode& node) {
    if (const auto* reference =
            cursiv::ast_cast<const NamedElementReference>(&node)) {
      const cursiv::AstNode* target = reference->value.get<Constant>();
      if (!target) target = reference->value.get<EnumerationLiteral>();
      if (target)
        if (const auto* owner = cursiv::ast_cast<const Type>(target->parent))
          result.emplace_back(owner);
    } else if (cursiv::ast_cast<const BuiltInConstant>(&node)
               || cursiv::ast_cast<const BuiltInFunction>(&node)) {
      result.emplace_back(std::string("cmath"));
    }
  };
  add(*expr);
  for (const auto* node : descendantsOf(*expr)) add(*node);
  return result;
}

void CppGenerator::finalizeMembers(
    cursiv::TextBuilder& out, std::span<NamedElement* const> elements) const {
  for (const auto* element : elements)
    if (cursiv::ast_cast<const Container>(element)
        || cursiv::ast_cast<const EntryPoint>(element)
        || cursiv::ast_cast<const EventSink>(element)
        || cursiv::ast_cast<const EventSource>(element)
        || cursiv::ast_cast<const Reference>(element)) {
      out.line("delete ", element->name, ";");
      out.line(element->name, " = nullptr;");
    }
}

bool CppGenerator::isInvokable(const Operation& op) const {
  return !notInvokableReason(op);
}

bool CppGenerator::isInvokable(const Property& property) const {
  return !notInvokableReason(property);
}

std::optional<std::string> CppGenerator::notInvokableReason(
    const Operation& operation) const {
  if (isConstructorAttribute(operation))
    return "constructors cannot be dynamically invoked";
  if (const auto* returned = operation.returnParameter;
      returned && isByPointer(*returned))
    return "pointer return values cannot be represented by Smp::AnySimple";
  if (isOverloaded(operation))
    return "overloaded operation names cannot be distinguished by "
           "Smp::IRequest";
  if (const auto* returned = operation.returnParameter;
      returned && !cursiv::ast_cast<const SimpleType>(returned->type.get()))
    return "its return type is not a SimpleType";
  for (const auto* parameter : operation.parameter)
    if (!cursiv::ast_cast<const SimpleType>(parameter->type.get()))
      return "parameter '" + std::string(parameter->name)
             + "' is not a SimpleType";
  return std::nullopt;
}

std::optional<std::string> CppGenerator::notInvokableReason(
    const Property& property) const {
  if (isByPointer(property))
    return "pointer properties cannot be represented by Smp::AnySimple";
  if (!cursiv::ast_cast<const SimpleType>(property.type.get()))
    return "its type is not a SimpleType";
  return std::nullopt;
}

std::string CppGenerator::componentBase(const Component& type) const {
  return type.base.has_value() ? fqn(type.base->get()) : std::string{};
}

std::vector<std::string> CppGenerator::componentBases(
    const Component& type) const {
  std::vector<std::string> bases;
  if (const auto base = componentBase(type); !base.empty())
    bases.push_back("public " + base);
  for (const auto& interface : type.interface_)
    bases.push_back("public virtual " + fqn(interface.get()));
  return bases;
}

bool CppGenerator::isCdkField(const Field& /*field*/) const { return false; }

void CppGenerator::publishField(cursiv::TextBuilder& out,
                                const Field& field) const {
  if (isCdkField(field)) {
    out.line("// Publish field ", field.name);
    out.line("receiver->PublishField(&", field.name, ");");
    return;
  }
  const auto* type = field.type.get();
  const auto kind = getPTK(type);
  const bool primitive = cursiv::ast_cast<const PrimitiveType>(type);
  if (primitive && kind == PTK::String8) {
    out.line("// WARNING: Field ", field.name,
             " of type ::Smp::String8 is not publicable.");
    return;
  }
  out.line("// Publish field ", field.name);
  out.line("receiver->PublishField(");
  out.line("    \"", field.name, "\", // Name");
  out.line("    ", description(field), ", // Description");
  out.line("    &", field.name, ", // Address");
  // Otherwise published directly with the address of the field. A DateTime and
  // a Duration are identical to an Int64, so only their UUID tells them from
  // one.
  if (!primitive || kind == PTK::None || kind == PTK::DateTime
      || kind == PTK::Duration)
    out.line("    ", uuid(type), ", // Type UUID");
  out.line("    ", viewKind(field), ", // View Kind");
  out.line("    ", boolText(isState(field)), ", // State");
  out.line("    ", boolText(isInput(field)), ", // Input");
  out.line("    ", boolText(isOutput(field)), " // Output");
  out.line(");");
}

void CppGenerator::publishOperation(cursiv::TextBuilder& out,
                                    const Operation& op) const {
  if (const auto reason = notInvokableReason(op)) {
    out.line("// WARNING: Operation ", op.name, " is not invokable: ", *reason,
             ".");
    return;
  }
  const auto publishParameter =
      [&](std::string_view name, const cursiv::AstNode& documented,
          const Type* type, std::string_view direction) {
        out.open("op_", op.name, "->PublishParameter(");
        out.line("\"", name, "\", // Name");
        out.line(description(documented), ", // Description");
        out.line(uuid(type), ", // Type UUID");
        out.line("Smp::Publication::ParameterDirectionKind::", direction,
                 " // Parameter Direction Kind");
        out.close(");");
      };
  const auto* returned = op.returnParameter;
  out.line("// Publish operation ", op.name);
  if (returned != nullptr || !op.parameter.empty())
    out.write("auto* op_", op.name, " = ");
  out.open("receiver->PublishOperation(");
  out.line("\"", op.name, "\", // Name");
  out.line(description(op), ", // Description");
  out.line(viewKind(op), " // View Kind");
  out.close(");");
  for (const auto* param : op.parameter)
    publishParameter(param->name, *param, param->type.get(),
                     param->direction == "out"     ? "PDK_Out"
                     : param->direction == "inout" ? "PDK_InOut"
                                                   : "PDK_In");
  if (returned != nullptr)
    publishParameter(
        returned->name.empty() ? std::string_view("return") : returned->name,
        *returned, returned->type.get(), "PDK_Return");
}

void CppGenerator::publishProperty(cursiv::TextBuilder& out,
                                   const Property& property) const {
  if (const auto reason = notInvokableReason(property)) {
    out.line("// WARNING: Property ", property.name,
             " is not invokable: ", *reason, ".");
    return;
  }
  const auto access = getAccessKind(property).value_or(AccessKind::ReadWrite);
  out.line("// Publish Property ", property.name);
  out.open("receiver->PublishProperty(");
  out.line("\"", property.name, "\", // Name");
  out.line(description(property), ", // Description");
  out.line(uuid(property.type.get()), ", // Type UUID");
  out.line(access == AccessKind::ReadOnly ? "::Smp::AccessKind::AK_ReadOnly"
           : access == AccessKind::WriteOnly
               ? "::Smp::AccessKind::AK_WriteOnly"
               : "::Smp::AccessKind::AK_ReadWrite",
           ", // Access Kind");
  out.line(viewKind(property), " // View Kind");
  out.close(");");
}

void CppGenerator::publishMembers(
    cursiv::TextBuilder& out, std::span<NamedElement* const> elements) const {
  for (const auto* element : elements) {
    if (const auto* field = cursiv::ast_cast<const Field>(element))
      publishField(out, *field);
    else if (const auto* operation = cursiv::ast_cast<const Operation>(element))
      publishOperation(out, *operation);
    else if (const auto* property = cursiv::ast_cast<const Property>(element))
      publishProperty(out, *property);
    else
      continue;
    out.line();
  }
}

bool CppGenerator::isOptionalPointer(const Parameter& param) {
  return isByPointer(param) && isNullptr(param.default_);
}

std::string CppGenerator::parameterList(const Operation& op,
                                        bool declaration) const {
  // C++ only allows defaults on the last parameters.
  auto firstDefault = op.parameter.size();
  while (declaration && firstDefault > 0
         && op.parameter[firstDefault - 1]->default_ != nullptr) {
    --firstDefault;
  }

  std::string result;
  for (std::size_t i = 0; i < op.parameter.size(); ++i) {
    const auto* param = op.parameter[i];
    if (!result.empty()) {
      result += ", ";
    }
    result += type(param) + " " + std::string(param->name);
    if (declaration && i >= firstDefault) {
      result += " = "
                + (isOptionalPointer(*param) ? "nullptr"
                                             : expression(param->default_));
    }
  }
  return result;
}

std::string CppGenerator::defaultValueForType(const Type* type) const {
  if (type == nullptr) return {};
  if (const auto* array = cursiv::ast_cast<const Array>(type)) {
    const auto size = getValueAs(array->size, PTK::Int64);
    const auto count = size ? integralValueOf(*size) : std::nullopt;
    if (!count || *count == 0) return "{}";
    const auto item = defaultValueForType(array->itemType.get());
    // Zero is already the default of a number: only enumerations, strings and
    // composites are written out.
    const auto itemKind = getPTK(array->itemType.get());
    if (item == "{}"
        || (itemKind != PTK::None && itemKind != PTK::Enum
            && itemKind != PTK::String8))
      return "{}";
    // Every item of a huge array would make a stub of many megabytes.
    constexpr std::uint64_t largestValue = 65536;
    if (static_cast<std::uint64_t>(*count) * (item.size() + 2) > largestValue)
      return "{}";
    // An array type holds a C array: one pair of braces for each.
    std::string result = "{{";
    for (std::int64_t i = 0; i < *count; ++i) result += (i ? ", " : "") + item;
    return result + "}}";
  }
  if (const auto* structure = cursiv::ast_cast<const Structure>(type)) {
    std::string result = "{";
    std::string_view separator;
    for (const auto* field : getAllFields(*structure)) {
      result += separator;
      separator = ", ";
      result += "/*." + std::string(field->name) + " = */"
                + defaultValueForType(field->type.get());
    }
    return result + "}";
  }
  if (const auto* enumeration = cursiv::ast_cast<const Enumeration>(type))
    return enumeration->literal.empty() ? "0" : fqn(enumeration->literal[0]);
  switch (getPTK(type)) {
    case PTK::Bool:
      return "false";
    case PTK::Float32:
      return "0.0f";
    case PTK::Float64:
      return "0.0";
    case PTK::Int8:
    case PTK::Int16:
    case PTK::Int32:
      return "0";
    case PTK::DateTime:
    case PTK::Duration:
    case PTK::Int64:
      return "0L";
    case PTK::UInt8:
    case PTK::UInt16:
    case PTK::UInt32:
      return "0U";
    case PTK::UInt64:
      return "0UL";
    case PTK::Char8:
      return R"('\0')";
    case PTK::String8:
      return "\"\"";
    default:
      return "{}";
  }
}

std::string CppGenerator::operationName(const Operation& op) const {
  static constexpr std::pair<std::string_view, std::string_view> spellings[] = {
      {"Positive", "operator+"},   {"Negative", "operator-"},
      {"Assign", "operator="},     {"Add", "operator+="},
      {"Subtract", "operator-="},  {"Multiply", "operator*="},
      {"Divide", "operator/="},    {"Remainder", "operator%="},
      {"Greater", "operator>"},    {"Less", "operator<"},
      {"Equal", "operator=="},     {"NotGreater", "operator<="},
      {"NotLess", "operator>="},   {"NotEqual", "operator!="},
      {"Indexer", "operator[]"},   {"Sum", "operator+"},
      {"Difference", "operator-"}, {"Product", "operator*"},
      {"Quotient", "operator/"},   {"Module", "operator%"}};
  const auto literal = operatorLiteral(op);
  for (const auto& [name, spelling] : spellings) {
    if (name == literal) {
      return std::string(spelling);
    }
  }
  return std::string(op.name);
}

}  // namespace xsmp
