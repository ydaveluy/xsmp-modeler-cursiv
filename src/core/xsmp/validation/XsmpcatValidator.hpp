#pragma once

#include <cursiv/syntax-tree/AstNode.hpp>
#include <cursiv/utils/DocumentCache.hpp>
#include <cursiv/utils/FunctionRef.hpp>
#include <cursiv/utils/WorkspaceCache.hpp>
#include <cursiv/validation/ValidationAcceptor.hpp>
#include <cursiv/validation/ValidationRegistry.hpp>
#include <cursiv/workspace/AstNodeDescription.hpp>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>
#include <xsmp/ast.hpp>
#include <xsmp/utils/XsmpDocumentation.hpp>

namespace xsmp {

// Forward-declared (not included) to avoid a header cycle:
// `XsmpcatCoreServices.hpp` fully includes this header for the concrete
// `validator` member type.
struct XsmpcatCoreServices;

}  // namespace xsmp

namespace xsmp {

/// xsmpcat semantic validation. Each check works from the AST plus the
/// already-resolved cross references, the constant solver, and (for doc-comment
/// tags such as `@uuid`) the shared `commentProvider`.
class XsmpcatValidator final {
 public:
  // `services` is the concrete `XsmpcatCoreServices` container: its `shared`
  // member is the typed `XsmpSharedCoreServices`, so `.shared.projectManager`
  // needs no cast. The constructor is defined in the .cpp because it reads
  // `services.shared`, and that type is only forward-declared here.
  explicit XsmpcatValidator(const XsmpcatCoreServices& services);

  // Name pattern + C++-reserved-keyword rejection, for every named element: no
  // grammar rule produces a bare `NamedElement`, so the check reaches them all
  // through subtype dispatch. Also validates annotation applicability,
  // repetition and values.
  void checkNamedElement(const NamedElement& element,
                         cursiv::ValidationAcceptor& accept) const;

  void checkEnumeration(const Enumeration& enumeration,
                        cursiv::ValidationAcceptor& accept) const;

  void checkInteger(const Integer& integer,
                    cursiv::ValidationAcceptor& accept) const;

  void checkFloat(const Float& floatType,
                  cursiv::ValidationAcceptor& accept) const;

  void checkConstant(const Constant& constant,
                     cursiv::ValidationAcceptor& accept) const;

  void checkStringType(const String& stringType,
                       cursiv::ValidationAcceptor& accept) const;

  void checkArrayType(const Array& arrayType,
                      cursiv::ValidationAcceptor& accept) const;

  void checkStructure(const Structure& structure,
                      cursiv::ValidationAcceptor& accept) const;

  // Also runs for an Exception, which extends Class.
  void checkClass(const Class& clazz, cursiv::ValidationAcceptor& accept) const;

  void checkInterface(const Interface& interface,
                      cursiv::ValidationAcceptor& accept) const;

  // Runs for both kinds of Component: Model and Service.
  void checkComponent(const Component& component,
                      cursiv::ValidationAcceptor& accept) const;

  void checkNamespace(const Namespace& ns,
                      cursiv::ValidationAcceptor& accept) const;

  // The `@date` tag, a name unique in the workspace, a file written by it alone
  // in its project, the membership of a project, and no circular dependency
  // with another catalogue.
  void checkCatalogue(const Catalogue& catalogue,
                      cursiv::ValidationAcceptor& accept) const;

  // A Service name, without its namespace, is unique in the workspace.
  void checkService(const Service& service,
                    cursiv::ValidationAcceptor& accept) const;

  // The elements that name a type: its visibility, its deprecation, and the
  // rules of each kind of element.
  void checkField(const Field& field, cursiv::ValidationAcceptor& accept) const;
  void checkProperty(const Property& property,
                     cursiv::ValidationAcceptor& accept) const;

  void checkAssociation(const Association& association,
                        cursiv::ValidationAcceptor& accept) const;

  void checkContainer(const Container& container,
                      cursiv::ValidationAcceptor& accept) const;

  void checkReference(const Reference& reference,
                      cursiv::ValidationAcceptor& accept) const;

  void checkEventSink(const EventSink& eventSink,
                      cursiv::ValidationAcceptor& accept) const;

  void checkEventSource(const EventSource& eventSource,
                        cursiv::ValidationAcceptor& accept) const;

  void checkValueReference(const ValueReference& valueReference,
                           cursiv::ValidationAcceptor& accept) const;

  void checkEventType(const EventType& eventType,
                      cursiv::ValidationAcceptor& accept) const;

  void checkAttributeType(const AttributeType& attributeType,
                          cursiv::ValidationAcceptor& accept) const;

  void checkNativeType(const NativeType& nativeType,
                       cursiv::ValidationAcceptor& accept) const;

  void checkPrimitiveType(const PrimitiveType& primitiveType,
                          cursiv::ValidationAcceptor& accept) const;

  // UUID (missing/invalid) — dispatches for every Type subtype.
  void checkType(const Type& type, cursiv::ValidationAcceptor& accept) const;

  // Multiplicity bounds — dispatches for Container and Reference.
  void checkNamedElementWithMultiplicity(
      const NamedElementWithMultiplicity& element,
      cursiv::ValidationAcceptor& accept) const;

  void checkOperation(const Operation& operation,
                      cursiv::ValidationAcceptor& accept) const;

  void checkParameter(const Parameter& parameter,
                      cursiv::ValidationAcceptor& accept) const;

  void checkReturnParameter(const ReturnParameter& parameter,
                            cursiv::ValidationAcceptor& accept) const;

  void checkEntryPoint(const EntryPoint& entryPoint,
                       cursiv::ValidationAcceptor& accept) const;

 private:
  // "Duplicated Type name" — workspace-wide by fully-qualified name, filtered
  // to `type`'s visible documents (via `services.shared.projectManager`).
  // Shared by `checkType`.
  void checkDuplicatedTypeName(const Type& type,
                               cursiv::ValidationAcceptor& accept) const;

  // Gives each other exported NamedElement with the qualified name of `element`
  // among the documents it sees.
  void visitSameName(
      const NamedElement& element,
      cursiv::function_ref<void(const cursiv::AstNode&,
                                const cursiv::AstNodeDescription&)>
          visitor) const;

  // The SMP standard revision in effect for `node`'s document: its owning
  // project's `standard` (defaulting to "ECSS_SMP_2020" when the project has no
  // `using` clause) when one owns the document, else the `@<standard>` tag on a
  // builtin's own URI, else "ECSS_SMP_2020". Shared by `checkReference`.
  std::string getSmpStandard(const cursiv::AstNode& node) const;

  // What the comment of an AttributeType says of its use.
  struct AttributeTypeDoc {
    std::vector<std::string> usages;
    bool allowMultiple = false;
  };

  // Every annotation asks it, and a comment is costly to parse.
  [[nodiscard]] std::shared_ptr<const AttributeTypeDoc> attributeTypeDoc(
      const AttributeType& type) const;

  // For each `@Foo(1)` usage on `element`, its visibility, its value, whether
  // the AttributeType allows the kind of `element` and allows a repetition.
  template <typename T>
  void checkAttributeUsages(const T& element,
                            cursiv::ValidationAcceptor& accept) const;

  // What the checks of the types and catalogues ask of the whole workspace.
  struct WorkspaceFacts {
    // The Types by folded UUID, in index order, when several have it.
    std::unordered_map<std::string, std::vector<cursiv::AstNodeDescription>>
        duplicatedUuids;
    // The Services by name, without their namespace, in index order.
    std::unordered_map<std::string_view,
                       std::vector<cursiv::AstNodeDescription>>
        servicesByName;
    // For every Catalogue in a circular dependency, the others of that cycle.
    std::map<cursiv::DocumentId, std::vector<cursiv::AstNodeDescription>>
        catalogueCycles;
  };

  [[nodiscard]] std::shared_ptr<const WorkspaceFacts> workspaceFacts() const;
  [[nodiscard]] std::shared_ptr<const WorkspaceFacts> readWorkspaceFacts()
      const;

  // What the fields of a type hold, through its arrays, structures and base:
  // the first field whose type is not a ValueType, and the first of String8.
  // `isString8` when the type is String8, or an array of it.
  struct FieldSummary {
    const Field* nonValueTypeField = nullptr;
    const Field* string8Field = nullptr;
    bool isString8 = false;
  };

  // For one state of the workspace: the summaries of the types with no cycle
  // below them, read once each, then those of the types of a cycle, whose
  // fields depend on where the walk starts, and the types with no cycle below
  // them through fields, items and base, which no field can make recursive.
  struct TypeGraphs {
    std::mutex mutex;
    std::unordered_map<const Type*, FieldSummary> summaries;
    std::unordered_map<const Type*, FieldSummary> cyclicSummaries;
    std::unordered_set<const Type*> acyclic;
  };

  [[nodiscard]] FieldSummary fieldSummary(const Type& type) const;

  // Whether `owner`, through a member of type `type`, holds itself.
  [[nodiscard]] bool isRecursiveMember(const Type& owner,
                                       const Type* type) const;

  // The `@uuid` of `type`, read once per parse of its document.
  [[nodiscard]] std::shared_ptr<const std::optional<DocTagRange>> uuidOf(
      const cursiv::AstNode& type, cursiv::DocumentId document) const;

  [[nodiscard]] std::map<cursiv::DocumentId,
                         std::vector<cursiv::AstNodeDescription>>
  computeCatalogueCycles(
      const std::map<cursiv::DocumentId, cursiv::AstNodeDescription>&
          catalogues) const;

  const XsmpcatCoreServices& services;

  // Read once per build, when validation first asks, and emptied when the next
  // build is announced.
  cursiv::WorkspaceCache<std::monostate, std::shared_ptr<const WorkspaceFacts>>
      _workspaceFacts;
  mutable std::mutex _workspaceFactsMutex;

  // Read once per parse of the document of the AttributeType.
  cursiv::DocumentCache<const AttributeType*,
                        std::shared_ptr<const AttributeTypeDoc>>
      _attributeTypeDocs;

  // Emptied when a build is announced, as the workspace facts.
  cursiv::WorkspaceCache<std::monostate, std::shared_ptr<TypeGraphs>>
      _typeGraphs;
};

template <typename Services>
void registerXsmpcatValidationChecks(Services& services) {
  using Registry = cursiv::ValidationRegistry;
  auto& registry = *services.validationRegistry;
  auto& validator = *services.validator;
  registry.registerChecks({
      Registry::makeValidationCheck<&XsmpcatValidator::checkNamedElement>(
          validator),
      Registry::makeValidationCheck<&XsmpcatValidator::checkEnumeration>(
          validator),
      Registry::makeValidationCheck<&XsmpcatValidator::checkInteger>(validator),
      Registry::makeValidationCheck<&XsmpcatValidator::checkFloat>(validator),
      Registry::makeValidationCheck<&XsmpcatValidator::checkConstant>(
          validator),
      Registry::makeValidationCheck<&XsmpcatValidator::checkStringType>(
          validator),
      Registry::makeValidationCheck<&XsmpcatValidator::checkArrayType>(
          validator),
      // `Class`/`Exception` are C++ subtypes of `Structure` (field/member
      // reuse) and cursiv's reflection wires a real `Class`/`Exception` →
      // `Structure` edge for that C++ relationship (even though the grammar
      // never nests one rule inside the other) — so the plain
      // `makeValidationCheck` form would ALSO dispatch this check to
      // `Class`/`Exception`, duplicating `checkClass`'s own diagnostics and
      // misapplying `Structure`'s narrower member-modifier rules. `Structure`
      // is exclusively for a literal `struct`; `Class`/`Exception` are owned
      // by `checkClass`. Use `makeExactValidationCheck` to opt out of subtype
      // dispatch.
      Registry::makeExactValidationCheck<&XsmpcatValidator::checkStructure>(
          validator),
      Registry::makeValidationCheck<&XsmpcatValidator::checkClass>(validator),
      Registry::makeValidationCheck<&XsmpcatValidator::checkInterface>(
          validator),
      Registry::makeValidationCheck<&XsmpcatValidator::checkComponent>(
          validator),
      Registry::makeValidationCheck<&XsmpcatValidator::checkNamespace>(
          validator),
      Registry::makeValidationCheck<&XsmpcatValidator::checkCatalogue>(
          validator),
      Registry::makeValidationCheck<&XsmpcatValidator::checkService>(validator),
      Registry::makeValidationCheck<&XsmpcatValidator::checkField>(validator),
      Registry::makeValidationCheck<&XsmpcatValidator::checkProperty>(
          validator),
      Registry::makeValidationCheck<&XsmpcatValidator::checkAssociation>(
          validator),
      Registry::makeValidationCheck<&XsmpcatValidator::checkContainer>(
          validator),
      Registry::makeValidationCheck<&XsmpcatValidator::checkReference>(
          validator),
      Registry::makeValidationCheck<&XsmpcatValidator::checkEventSink>(
          validator),
      Registry::makeValidationCheck<&XsmpcatValidator::checkEventSource>(
          validator),
      Registry::makeValidationCheck<&XsmpcatValidator::checkValueReference>(
          validator),
      Registry::makeValidationCheck<&XsmpcatValidator::checkEventType>(
          validator),
      Registry::makeValidationCheck<&XsmpcatValidator::checkAttributeType>(
          validator),
      Registry::makeValidationCheck<&XsmpcatValidator::checkNativeType>(
          validator),
      Registry::makeValidationCheck<&XsmpcatValidator::checkPrimitiveType>(
          validator),
      Registry::makeValidationCheck<&XsmpcatValidator::checkType>(validator),
      Registry::makeValidationCheck<
          &XsmpcatValidator::checkNamedElementWithMultiplicity>(validator),
      Registry::makeValidationCheck<&XsmpcatValidator::checkOperation>(
          validator),
      Registry::makeValidationCheck<&XsmpcatValidator::checkParameter>(
          validator),
      Registry::makeValidationCheck<&XsmpcatValidator::checkReturnParameter>(
          validator),
      Registry::makeValidationCheck<&XsmpcatValidator::checkEntryPoint>(
          validator),
  });
}

}  // namespace xsmp
