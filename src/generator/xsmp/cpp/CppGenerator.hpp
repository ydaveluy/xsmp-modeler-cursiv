#pragma once

#include <cursiv/text/TextBuilder.hpp>
#include <filesystem>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <variant>
#include <vector>
#include <xsmp/AttributeHelper.hpp>
#include <xsmp/Traversal.hpp>
#include <xsmp/XsmpcatCoreServices.hpp>
#include <xsmp/ast.hpp>
#include <xsmp/utils/XsmpDocumentation.hpp>

namespace xsmp {

/// Shared C++ generation and rendering services. Concrete profiles implement
/// runtime-specific type bodies, registration and publication.
class CppGenerator {
 public:
  explicit CppGenerator(const XsmpcatCoreServices& services);
  virtual ~CppGenerator() = default;
  CppGenerator(const CppGenerator&) = delete;
  CppGenerator& operator=(const CppGenerator&) = delete;

  /// Generates every file for `catalogue` under `projectDir`: the
  /// namespace/type tree plus the catalogue-level package files.
  void generate(const Catalogue& catalogue,
                const std::filesystem::path& projectDir) const;

 protected:
  /// The C++ standard that the generated code targets, which decides a few
  /// spellings.
  [[nodiscard]] virtual unsigned cxxStandard() const { return 17; }

  /// A fixed header, a type's generated header, or a forward declaration.
  struct ForwardedType {
    const Type* type = nullptr;
    // The full header comes with it, for a use in a signature.
    bool withHeader = false;
  };

  using Include = std::variant<std::string, const Type*, ForwardedType>;

  /// Generates `type`'s own files under `projectDir`.
  virtual void generateType(const Type& type,
                            const std::filesystem::path& projectDir) const = 0;

  // Writes `<Catalogue>.h`, `<Catalogue>.cpp` and `<Catalogue>.pkg.cpp`.
  virtual void generatePackage(const Catalogue& catalogue,
                               const std::filesystem::path& projectDir) const;

  // The same files, under the given folders.
  void generatePackageFiles(const Catalogue& catalogue,
                            const std::filesystem::path& projectDir,
                            std::string_view headerFolder,
                            std::string_view packageSourceFolder) const;

  // `<Catalogue>.h`: declares `Initialise_<Catalogue>` and
  // `Finalise_<Catalogue>`, the entry points of a static library.
  virtual void generatePackageHeader(cursiv::TextBuilder& out,
                                     const Catalogue& catalogue) const;

  // `<Catalogue>.cpp`: defines those entry points. Initialising runs the
  // dependent packages, then registers the types and the components.
  virtual void generatePackageSource(cursiv::TextBuilder& out,
                                     const Catalogue& catalogue) const;

  // `<Catalogue>.pkg.cpp`: the exported `Initialise` and `Finalise` of a shared
  // library, which forward to the entry points above.
  virtual void generateDynamicPackageSource(cursiv::TextBuilder& out,
                                            const Catalogue& catalogue) const;

  /// `catalogue.name`, the base file name of its package files.
  [[nodiscard]] static std::string_view catalogueFileName(
      const Catalogue& catalogue);

  /// The includes that a `<Catalogue>.cpp` starts from.
  [[nodiscard]] std::vector<Include> sourceIncludesCatalogue(
      const Catalogue& catalogue) const;

  /// Extra includes needed to create a factory.
  [[nodiscard]] virtual std::vector<Include> factoryIncludes() const;

  /// The registration of `component` (a Model or a Service) inside
  /// `Initialise_<Catalogue>`.
  void registerComponent(cursiv::TextBuilder& out,
                         const Component& component) const;

  /// The factory-registration statement for `model`.
  virtual void registerModel(cursiv::TextBuilder& out,
                             const Model& model) const = 0;

  /// The registration statement for `service`.
  virtual void registerService(cursiv::TextBuilder& out,
                               const Service& service) const;

  /// The registration of `type` inside `Initialise_<Catalogue>`.
  void registerType(cursiv::TextBuilder& out, const Type& type) const;

  /// The includes of `<Catalogue>.cpp`.
  [[nodiscard]] std::vector<Include> packageSourceIncludes(
      const Catalogue& catalogue) const;

  /// The statements of `Initialise_<Catalogue>` for `dependencies`, and the end
  /// of `Finalise_<Catalogue>`, which passes `argument` on to them.
  static void initialiseDependencies(
      cursiv::TextBuilder& out, std::span<const Catalogue* const> dependencies);

  static void finaliseDependencies(
      cursiv::TextBuilder& out, std::span<const Catalogue* const> dependencies,
      std::string_view argument);

  /// The registrations of every exported type and of every component.
  void registerTypes(cursiv::TextBuilder& out,
                     const Catalogue& catalogue) const;

  void registerComponents(cursiv::TextBuilder& out,
                          const Catalogue& catalogue) const;

  /// Every ValueType declared in `catalogue`, dependency-ordered: a type comes
  /// after the item type of an array, the target of a value reference and the
  /// types of the public fields of a structure.
  [[nodiscard]] std::vector<const Type*> exportedTypes(
      const Catalogue& catalogue) const;

  /// Every non-abstract Model and Service declared in `catalogue`.
  [[nodiscard]] std::vector<const Component*> components(
      const Catalogue& catalogue) const;

  /// The other catalogues that `catalogue` depends on by the type of a field or
  /// a property, the base of a type or the item type of an array, sorted by
  /// name. Not a standard builtin library.
  [[nodiscard]] std::vector<const Catalogue*> dependentPackages(
      const Catalogue& catalogue) const;

  /// Every `Constant` directly in `elements`, dependency-ordered (a constant
  /// referencing another comes after it).
  [[nodiscard]] std::vector<const Constant*> sortedConstants(
      std::span<NamedElement* const> elements) const;

  /// The copyright notice of the document of `node`, if any, as `//` comments.
  void notice(cursiv::TextBuilder& out, const cursiv::AstNode& node) const;

  /// The banner of a generated file, between two lines of `separator`.
  static void fileBanner(cursiv::TextBuilder& out, std::string_view fileName,
                         std::string_view separator);

  /// Writes the finished file `out`, through clang-format when it is there, to
  /// `projectDir/folder/relativePath`, creating parent directories as needed,
  /// unless the file already has the same content. Within `generate`, the file
  /// may appear only when `generate` ends.
  void generateFile(const std::filesystem::path& projectDir,
                    std::string_view folder, std::string_view relativePath,
                    cursiv::TextBuilder& out) const;

  /// The same for a file the user edits: written only where it is missing.
  void generateUserFile(const std::filesystem::path& projectDir,
                        std::string_view folder, std::string_view relativePath,
                        cursiv::TextBuilder& out) const;

  /// Re-emit all documentation paragraphs and tags as C++ documentation,
  /// excluding the model identity tags `@uuid` and `@id`.
  void comment(cursiv::TextBuilder& out, const NamedElement& element) const;

  /// `::`-joined fully-qualified C++ name of `node` (with a leading `::`), or
  /// `defaultFqn` when `node` is null.
  [[nodiscard]] std::string fqn(const NamedElement* node,
                                std::string_view defaultFqn = {}) const;

  /// `node`'s fully-qualified name with `separator` joining each segment (no
  /// leading separator, unlike `fqn()`): `qualifiedPath(type, "/")` for a
  /// relative file path, `qualifiedPath(type, "_")` for an include guard.
  /// Empty when `node` is null.
  [[nodiscard]] std::string qualifiedPath(const NamedElement* node,
                                          std::string_view separator) const;

  /// The UUID expression to pass when registering `type` with the type
  /// registry: `::Smp::Uuids::Uuid_<Name>` for the types the SMP SDK
  /// predeclares, else `<ContainerFQN>::Uuid_<Name>`. `::Smp::Uuids::Uuid_Void`
  /// when `type` is null.
  [[nodiscard]] std::string uuid(const Type* type) const;

  /// `text` as it stands between the quotes of a C++ string, reading back as
  /// the same bytes.
  [[nodiscard]] std::string escapedText(std::string_view text) const;

  /// `text` with its trigraphs kept from reading as one, which C++ does before
  /// anything else up to C++14.
  [[nodiscard]] std::string protectTrigraphs(std::string text) const;

  /// `"<escaped description>"`: the description of `node`, or `""` when it has
  /// none.
  [[nodiscard]] std::string description(const cursiv::AstNode& node) const;

  /// The `::Smp::ViewKind::VK_*` expression for `element`'s `@Attributes.View`
  /// attribute, or `defaultViewKind` when absent or not a constant.
  [[nodiscard]] std::string viewKind(
      const NamedElement& element,
      std::string_view defaultViewKind = "::Smp::ViewKind::VK_All") const;

  /// The include guard of the header at `path`, joined by `_`.
  [[nodiscard]] static std::string includeGuard(std::string path);

  /// Render the model expression as C++, preserving operators, references and
  /// composite initializers. Resolved values supply math calls, which C++
  /// cannot evaluate in a constant, and scalar literals where the written
  /// literal needs a conversion to C++.
  [[nodiscard]] std::string expression(const Expression* expr) const;

  /// Render a direct-list initializer, retaining structure initializer
  /// braces and adding the outer braces required by array and scalar values.
  [[nodiscard]] std::string directListInitializer(const Expression* expr) const;

  /// Opens the Namespace of `type`, written as one `namespace A::B` block from
  /// C++17 on and as one block per level before. Only the innermost level can
  /// leave its comments out.
  void openNamespace(cursiv::TextBuilder& out, const Type& type,
                     bool comments = true) const;

  void closeNamespace(cursiv::TextBuilder& out, const Type& type) const;

  /// `[inline ]constexpr ::Smp::Uuid Uuid_<Name> { 0x.., 0x..,... };` from
  /// `type`'s `@uuid` tag.
  void uuidDeclaration(cursiv::TextBuilder& out, const Type& type) const;

  /// `::Smp::PrimitiveTypeKind::PTK_<Kind>` for `type`'s primitive storage
  /// kind (`Enum` maps to `PTK_Int32`, the storage of enumeration literals).
  [[nodiscard]] std::string primitiveTypeKind(const Type* type) const;

  /// The lower multiplicity bound.
  [[nodiscard]] std::string lower(
      const NamedElementWithMultiplicity& element) const;

  /// The upper multiplicity bound (`-1` is unbounded).
  [[nodiscard]] std::string upper(
      const NamedElementWithMultiplicity& element) const;

  /// The event payload type of `element`: the `eventArgs` of its EventType, or
  /// `nullptr` for an event without payload.
  [[nodiscard]] static const Type* eventType(const EventSink& element) {
    return payloadOf(element.type.get());
  }

  [[nodiscard]] static const Type* eventType(const EventSource& element) {
    return payloadOf(element.type.get());
  }

  /// `value` as a C++ literal.
  [[nodiscard]] static constexpr std::string_view boolText(bool value) {
    return value ? "true" : "false";
  }

  /// `Model` or `Service`.
  [[nodiscard]] static std::string_view componentKind(
      const Component& component);

  /// The include-header path for `type`: the well-known SMP header for a
  /// builtin type, else `qualifiedPath(type, "/") + ".h"`.
  [[nodiscard]] std::string typeInclude(const Type& type) const;

  /// Render sorted, deduplicated headers and forward declarations after
  /// exclusions, and an empty line. A full type header supersedes its forward
  /// declaration. Nothing when none is left.
  void includes(cursiv::TextBuilder& out,
                const std::vector<Include>& includeList,
                const std::vector<Include>& excludeList = {}) const;

  // The includes of the regenerated `.h` of an element, by its kind.
  [[nodiscard]] std::vector<Include> headerIncludes(
      const NamedElement& element) const;

  /// The includes of the regenerated `.h` of `type` as they are written: a type
  /// used in a signature only stays declared when its header needs the
  /// definition of `type`. A class would then derive from one still undefined.
  [[nodiscard]] std::vector<Include> headerIncludesWithoutCycle(
      const Type& type) const;

  [[nodiscard]] std::vector<Include> headerIncludesParameter(
      const Parameter& element) const;

  [[nodiscard]] std::vector<Include> headerIncludesOperation(
      const Operation& element) const;

  [[nodiscard]] virtual std::vector<Include> headerIncludesContainer(
      const Container& element) const;

  [[nodiscard]] virtual std::vector<Include> headerIncludesEntryPoint(
      const EntryPoint& element) const;

  [[nodiscard]] virtual std::vector<Include> headerIncludesEventSink(
      const EventSink& element) const;

  [[nodiscard]] virtual std::vector<Include> headerIncludesEventSource(
      const EventSource& element) const;

  [[nodiscard]] virtual std::vector<Include> headerIncludesField(
      const Field& element) const;

  [[nodiscard]] virtual std::vector<Include> headerIncludesReference(
      const Reference& element) const;

  [[nodiscard]] virtual std::vector<Include> headerIncludesStructure(
      const Structure& type) const;

  [[nodiscard]] virtual std::vector<Include> headerIncludesComponent(
      const Component& type) const;

  [[nodiscard]] virtual std::vector<Include> headerIncludesArray(
      const Array& type) const;

  [[nodiscard]] virtual std::vector<Include> headerIncludesString(
      const String& type) const;

  /// What comes between the includes and the namespaces of a generated source
  /// file, for the preprocessor definitions its body relies on.
  virtual void sourcePrologue(cursiv::TextBuilder& out, const Type& type) const;

  /// The block that defines `ECSS_SMP_VERSION`, and the blank line after it.
  static void smpVersionPrologue(cursiv::TextBuilder& out);

  // The includes of the regenerated `.cpp` of an element, by its kind.
  [[nodiscard]] std::vector<Include> sourceIncludes(
      const NamedElement& element) const;

  [[nodiscard]] virtual std::vector<Include> sourceIncludesEntryPoint(
      const EntryPoint& element) const;

  [[nodiscard]] virtual std::vector<Include> sourceIncludesEventSink(
      const EventSink& element) const;

  [[nodiscard]] std::vector<Include> sourceIncludesParameter(
      const Parameter& element) const;
  [[nodiscard]] std::vector<Include> sourceIncludesParameter(
      const ReturnParameter& element) const;

  [[nodiscard]] virtual std::vector<Include> sourceIncludesOperation(
      const Operation& element) const;

  [[nodiscard]] virtual std::vector<Include> sourceIncludesEnumeration(
      const Enumeration& type) const;

  [[nodiscard]] virtual std::vector<Include> sourceIncludesComponent(
      const Component& type) const;

  /// The header that the value of `expr` needs: that of the type of what it
  /// names, and `<cmath>` for a builtin constant or function.
  [[nodiscard]] std::vector<Include> expressionIncludes(
      const Expression* expr) const;

  /// `delete x;` and `x = nullptr;` for each member that a component owns by
  /// pointer.
  void finalizeMembers(cursiv::TextBuilder& out,
                       std::span<NamedElement* const> elements) const;

  /// True when `op` or `property` can be published and invoked dynamically
  /// through `IPublication` and `IRequest`: `notInvokableReason` has nothing
  /// against it.
  [[nodiscard]] bool isInvokable(const Operation& op) const;
  [[nodiscard]] bool isInvokable(const Property& property) const;

  /// Why the profile cannot invoke an element, or `std::nullopt` when it can: a
  /// Property needs a SimpleType passed by value. An Operation cannot be a
  /// constructor, return a pointer or have overloads, and needs a return type
  /// and parameter types that are SimpleTypes.
  [[nodiscard]] virtual std::optional<std::string> notInvokableReason(
      const Operation& operation) const;
  [[nodiscard]] std::optional<std::string> notInvokableReason(
      const Property& property) const;

  /// The base class `type` derives from: its `extends` type, or the default of
  /// the profile.
  [[nodiscard]] virtual std::string componentBase(const Component& type) const;

  /// `componentBase()` (as `public <base>`) plus each implemented interface (as
  /// `public virtual <FQN>`).
  [[nodiscard]] virtual std::vector<std::string> componentBases(
      const Component& type) const;

  /// Whether a field is published as an object of the runtime: only a profile
  /// has those.
  [[nodiscard]] virtual bool isCdkField(const Field& field) const;

  /// Publication statements for fields and supported operations and properties.
  /// A member that cannot be published gets a `// WARNING` comment instead.
  void publishField(cursiv::TextBuilder& out, const Field& field) const;

  void publishOperation(cursiv::TextBuilder& out, const Operation& op) const;

  void publishProperty(cursiv::TextBuilder& out,
                       const Property& property) const;

  /// The publication statements of every Field, Operation and Property in
  /// `elements`, in declaration order.
  void publishMembers(cursiv::TextBuilder& out,
                      std::span<NamedElement* const> elements) const;

  /// `[const ]<FQN>[*][&]`: the C++ parameter or member type of `elem`, driven
  /// by its `isConst`, `isByPointer` and `isByReference` attributes, or
  /// `"void"` when `elem` is null.
  template <typename T>
  [[nodiscard]] std::string type(const T* elem) const {
    if (elem == nullptr) {
      return "void";
    }
    std::string result;
    if (isConst(*elem)) {
      result += "const ";
    }
    result += fqn(elem->type.get());
    if (isByPointer(*elem)) {
      result += "*";
    }
    if constexpr (!std::is_same_v<T, Association>) {
      if (isByReference(*elem)) {
        result += "&";
      }
    }
    return result;
  }

  /// A parameter passed by pointer whose default is `nullptr`: a request may
  /// leave it out, and the operation then gets no pointer.
  [[nodiscard]] static bool isOptionalPointer(const Parameter& param);

  /// The parameter list of `op`, `<type> <name>` separated by commas. A
  /// declaration also shows the default values, which are not allowed on an
  /// out-of-line definition, as far as C++ accepts them.
  [[nodiscard]] std::string parameterList(const Operation& op,
                                          bool declaration) const;

  /// The default value of `type`, to return from a stub.
  [[nodiscard]] std::string defaultValueForType(const Type* type) const;

  /// `op.name`, or the matching `operator...` spelling when `op` has an
  /// `@Attributes.Operator` attribute (`Add` gives `operator+=`).
  [[nodiscard]] std::string operationName(const Operation& op) const;

  /// The line that keeps a reader from editing a generated file.
  static constexpr std::string_view autoGeneratedLine =
      "// This file is auto-generated, Do not edit otherwise your changes will "
      "be lost";

  // The folders of headers and of sources, under the project directory.
  std::string includeFolder = "src";
  std::string sourceFolder = "src";

  const XsmpcatCoreServices& services;

 private:
  void generateNamespace(const Namespace& ns,
                         const std::filesystem::path& projectDir) const;

  // A `namespace` block: the Namespace whose comment it gets, and its name.
  struct NamespaceBlock {
    const Namespace* ns;
    std::string name;
  };

  // The blocks around `type`, outermost first.
  [[nodiscard]] std::vector<NamespaceBlock> namespaceBlocks(
      const Type& type) const;

  template <class T>
  [[nodiscard]] Include pointerMemberInclude(const T& member) const;

  template <class T>
  [[nodiscard]] std::vector<Include> signatureIncludes(const T& element) const;

  template <class T>
  [[nodiscard]] std::vector<Include> signatureSourceIncludes(
      const T& element) const;

  // Whether the regenerated `.h` of `from` needs the definition of `target`,
  // directly or through the headers it includes.
  [[nodiscard]] bool needsDefinitionOf(const Type& from, const Type& target,
                                       std::vector<const Type*>& seen) const;

  [[nodiscard]] static const Type* payloadOf(const Type* declared) {
    const auto* event = cursiv::ast_cast<const EventType>(declared);
    return event != nullptr && event->eventArgs.has_value()
               ? event->eventArgs->get()
               : nullptr;
  }
};

/// Appends `from` to `to`.
template <class T>
void appendTo(std::vector<T>& to, std::vector<T> from) {
  to.insert(to.end(), std::make_move_iterator(from.begin()),
            std::make_move_iterator(from.end()));
}

}  // namespace xsmp
