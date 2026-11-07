#pragma once

#include <xsmp/cpp/CppGenerator.hpp>

namespace xsmp {

/// Adds the "Generation Gap" pattern to `CppGenerator`: a `src-gen/` tree that
/// is always regenerated (and never hand-edited) and a `src/` tree that is
/// written once, if it does not exist, and never touched again. Interfaces and
/// Components (Models and Services) always use the split. The other types split
/// only when a hand-edited header already exists.
///
/// `Gen` is the regenerated half of a type, `gen` the flag that selects it in
/// the per-type bodies below. A missing body means that the type needs no file
/// of that kind.
class GapPatternCppGenerator : public CppGenerator {
 public:
  using CppGenerator::CppGenerator;

 protected:
  // The package files are regenerated, so they go to the `Gen` folders.
  void generatePackage(const Catalogue& catalogue,
                       const std::filesystem::path& projectDir) const override;

  void generateType(const Type& type,
                    const std::filesystem::path& projectDir) const override;

  /// True when `type` always uses the split: `XGen.h` and `XGen.cpp`
  /// regenerated, `X.h` and `X.cpp` hand-edited. Other types use it only once a
  /// hand-edited header exists.
  [[nodiscard]] virtual bool useGenerationGapPattern(const Type& type) const;

  /// `baseName`, `Gen`-suffixed when `gen` is true. Also usable on a path or a
  /// qualified identifier: the suffix lands on its last segment.
  [[nodiscard]] static std::string name(std::string_view baseName, bool gen);

  // The bodies that depend on the runtime library of the profile.
  virtual void generateStringHeaderGen(cursiv::TextBuilder& out,
                                       const String& type, bool gen) const = 0;

  virtual void generateArrayHeaderGen(cursiv::TextBuilder& out,
                                      const Array& type, bool gen) const = 0;

  // A structure with a field that has a default value gets a constructor that
  // takes all its fields.
  virtual void generateStructureHeaderGen(cursiv::TextBuilder& out,
                                          const Structure& type,
                                          bool gen) const;

  /// `_Register_<Name>` of an Enumeration, with one `AddLiteral` per literal.
  virtual void generateEnumerationSourceGen(cursiv::TextBuilder& out,
                                            const Enumeration& type) const;

  /// One `AddLiteral` per literal of `type`.
  void addLiterals(cursiv::TextBuilder& out, const Enumeration& type) const;

  /// Component generation is profile-specific: lifecycle methods, request
  /// dispatch and the field publication of the runtime.
  virtual void generateComponentHeaderGen(cursiv::TextBuilder& out,
                                          const Component& type,
                                          bool gen) const = 0;

  virtual void generateComponentSourceGen(cursiv::TextBuilder& out,
                                          const Component& type,
                                          bool gen) const = 0;

  /// The hand-edited header and source of a Component.
  virtual void generateComponentHeader(cursiv::TextBuilder& out,
                                       const Component& type) const = 0;

  virtual void generateComponentSource(cursiv::TextBuilder& out,
                                       const Component& type, bool gen) const;

  /// The name of the declaration that directly encloses `node`.
  [[nodiscard]] static std::string ownerName(const cursiv::AstNode& node,
                                             bool gen = false);

  /// The fields that each instance of `type` holds.
  [[nodiscard]] static std::vector<const Field*> instanceFields(
      const Structure& type);

  /// The UUID and the comment of `type`, then its class up to the `friend` of
  /// its hand-edited half.
  void openComponentClass(cursiv::TextBuilder& out, const Component& type,
                          bool gen) const;

  /// What the SMP interface of a Component declares. `destructor` follows the
  /// name of the destructor.
  static void declareComponentInterface(cursiv::TextBuilder& out,
                                        const Component& type, bool gen,
                                        std::string_view destructor);

  static void declareInvoke(cursiv::TextBuilder& out);

  /// Declare generated members in a fixed order per kind, preserving each
  /// member's visibility and ordering dependent constants before their users.
  /// `initial` is the section that the class body is in.
  void declareMembersGen(cursiv::TextBuilder& out,
                         std::span<NamedElement* const> elements,
                         Visibility initial, bool gen) const;

  /// The out-of-line `.cpp` definitions for `elements`' members: static
  /// fields, string constants and attached properties.
  void defineMembersGen(cursiv::TextBuilder& out,
                        std::span<NamedElement* const> elements,
                        bool gen) const;

  // The declaration of a Field. Its default is only written here outside a
  // Component, and not for a static field.
  virtual void declareFieldGen(cursiv::TextBuilder& out, const Field& element,
                               bool gen) const;

  // These two declare the runtime member and its `_<name>` callback, which is
  // pure when `gen` is true: the hand-edited class implements it.
  virtual void declareEntryPointGen(cursiv::TextBuilder& out,
                                    const EntryPoint& element, bool gen) const;

  virtual void declareEventSinkGen(cursiv::TextBuilder& out,
                                   const EventSink& element, bool gen) const;

  // Component-only member kinds: declaration in the regenerated class and
  // constructor initializer. Both depend on the runtime library.
  virtual void declareContainerGen(cursiv::TextBuilder& out,
                                   const Container& element) const = 0;

  virtual void declareReferenceGen(cursiv::TextBuilder& out,
                                   const Reference& element) const = 0;

  virtual void declareEventSourceGen(cursiv::TextBuilder& out,
                                     const EventSource& element) const = 0;

  /// The constructor member-initializer-list entry of a member. A comma follows
  /// its last line. A static field has none.
  virtual void initializeField(cursiv::TextBuilder& out,
                               const Field& element) const;

  virtual void initializeContainer(cursiv::TextBuilder& out,
                                   const Container& element) const = 0;

  virtual void initializeReference(cursiv::TextBuilder& out,
                                   const Reference& element) const = 0;

  virtual void initializeEntryPoint(cursiv::TextBuilder& out,
                                    const EntryPoint& element,
                                    bool gen) const = 0;

  virtual void initializeEventSink(cursiv::TextBuilder& out,
                                   const EventSink& element,
                                   bool gen) const = 0;

  virtual void initializeEventSource(cursiv::TextBuilder& out,
                                     const EventSource& element) const = 0;

  /// Every initializer of `elements`, in EntryPoint, EventSink, EventSource,
  /// Field, Association, Container and Reference order, each after a comma that
  /// ends the current line. The last line goes on.
  void initializeMembers(cursiv::TextBuilder& out,
                         std::span<NamedElement* const> elements,
                         bool gen) const;

  /// Declarations of the members that the hand-edited class overrides:
  /// Operations that are virtual but not abstract, Properties that also have no
  /// attached field, and every EntryPoint and EventSink callback.
  void declareMembers(cursiv::TextBuilder& out,
                      std::span<NamedElement* const> elements,
                      Visibility initial) const;

  /// The out-of-line `.cpp` definitions for `elements`' hand-edited members,
  /// with an empty line between them. `gen` is true when these members belong
  /// to a type with an `XGen` half.
  void defineMembers(cursiv::TextBuilder& out,
                     std::span<NamedElement* const> elements, bool gen) const;

  /// Whether `defineMembers` has something to write.
  [[nodiscard]] static bool hasDefinitions(
      std::span<NamedElement* const> elements);

  // The folders of the regenerated files, under the project directory.
  std::string includeGenFolder = "src-gen";
  std::string sourceGenFolder = "src-gen";

 private:
  // Whether `type` has a hand-edited `.cpp`.
  [[nodiscard]] static bool hasSource(const Type& type);

  // The body of the hand-edited `.h` and `.cpp` of `type`, by its kind. Those
  // of the regenerated `.h` and `.cpp`. Each writes whole lines.
  void generateHeaderBody(cursiv::TextBuilder& out, const Type& type) const;

  void generateSourceBody(cursiv::TextBuilder& out, const Type& type,
                          bool gen) const;

  void generateHeaderGenBody(cursiv::TextBuilder& out, const Type& type,
                             bool gen) const;

  void generateSourceGenBody(cursiv::TextBuilder& out, const Type& type,
                             bool gen) const;

  /// A whole file of each kind: banner, guard, includes, and the namespace
  /// around its body.
  void writeHeaderFile(cursiv::TextBuilder& out, const Type& type) const;

  void writeSourceFile(cursiv::TextBuilder& out, const Type& type,
                       bool gen) const;

  void writeHeaderGenFile(cursiv::TextBuilder& out, const Type& type,
                          bool gen) const;

  void writeSourceGenFile(cursiv::TextBuilder& out, const Type& type,
                          bool gen) const;

  // `using X = XGen;`, the hand-edited header of most types.
  void generateTypeHeader(cursiv::TextBuilder& out, const Type& type) const;

  void generateNativeTypeHeaderGen(cursiv::TextBuilder& out,
                                   const NativeType& type, bool gen) const;

  void generateValueReferenceHeaderGen(cursiv::TextBuilder& out,
                                       const ValueReference& type,
                                       bool gen) const;

  void generateClassHeader(cursiv::TextBuilder& out, const Class& type) const;

  void generateClassHeaderGen(cursiv::TextBuilder& out, const Class& type,
                              bool gen) const;

  void generateClassSourceGen(cursiv::TextBuilder& out, const Class& type,
                              bool gen) const;

  void generateStructureHeader(cursiv::TextBuilder& out,
                               const Structure& type) const;

  void generateStructureSourceGen(cursiv::TextBuilder& out,
                                  const Structure& type, bool gen) const;

  // One `AddField` per non-static field of `type` that is not a `Smp.String8`.
  void addFields(cursiv::TextBuilder& out, const Structure& type) const;

  void generateIntegerHeaderGen(cursiv::TextBuilder& out, const Integer& type,
                                bool gen) const;

  void generateIntegerSourceGen(cursiv::TextBuilder& out,
                                const Integer& type) const;

  void generateFloatHeaderGen(cursiv::TextBuilder& out, const Float& type,
                              bool gen) const;

  void generateFloatSourceGen(cursiv::TextBuilder& out,
                              const Float& type) const;

  void generateInterfaceHeader(cursiv::TextBuilder& out,
                               const Interface& type) const;

  void generateInterfaceHeaderGen(cursiv::TextBuilder& out,
                                  const Interface& type, bool gen) const;

  void generateInterfaceSourceGen(cursiv::TextBuilder& out,
                                  const Interface& type, bool gen) const;

  void generateArraySourceGen(cursiv::TextBuilder& out,
                              const Array& type) const;

  // The `.h` of an Enumeration also has two maps, from each literal to its name
  // and to its description.
  void generateEnumerationHeaderGen(cursiv::TextBuilder& out,
                                    const Enumeration& type, bool gen) const;

  void generateStringSourceGen(cursiv::TextBuilder& out,
                               const String& type) const;

  // A String constant is only declared here, and defined by
  // `defineConstantGen`. Any other is `static constexpr` with its value.
  void declareConstantGen(cursiv::TextBuilder& out,
                          const Constant& element) const;

  // The definition of a String constant, or of a `constexpr` one before C++17,
  // which needs a definition then.
  void defineConstantGen(cursiv::TextBuilder& out, const Constant& element,
                         bool gen) const;

  // The definition of a static field.
  void defineFieldGen(cursiv::TextBuilder& out, const Field& element,
                      bool gen) const;

  void declareAssociationGen(cursiv::TextBuilder& out,
                             const Association& element) const;

  // The declaration in the regenerated class. A virtual Operation is pure when
  // `gen` is true: the hand-edited class overrides it.
  void declareOperationGen(cursiv::TextBuilder& out, const Operation& op,
                           bool gen) const;

  // The accessors of a Property in the regenerated class. With an attached
  // field they have a body here, which reads or writes that field and which
  // `definePropertyGen` defines. Without one, a virtual accessor is pure when
  // `gen` is true.
  void declarePropertyGen(cursiv::TextBuilder& out, const Property& element,
                          bool gen) const;

  void declarePropertyGetterGen(cursiv::TextBuilder& out,
                                const Property& element, bool gen) const;

  void declarePropertySetterGen(cursiv::TextBuilder& out,
                                const Property& element, bool gen) const;

  void definePropertyGen(cursiv::TextBuilder& out, const Property& element,
                         bool gen) const;

  // The declaration in the hand-edited class: an `override` for a virtual,
  // non-abstract Operation.
  void declareOperation(cursiv::TextBuilder& out, const Operation& op) const;

  // A `// TODO` stub in the hand-edited `.cpp`, except for an abstract
  // Operation and for a constructor.
  void defineOperation(cursiv::TextBuilder& out, const Operation& op,
                       bool gen) const;

  // The `return` of a stub of `op`: `this` or `*this` when it returns `self`,
  // `nullptr` for any other pointer, else the default value of its type.
  void defaultReturn(cursiv::TextBuilder& out, const ReturnParameter& param,
                     const cursiv::AstNode* self) const;

  // The overrides and the stubs in the hand-edited class, only for a Property
  // that is virtual, not abstract and has no attached field.
  void declareProperty(cursiv::TextBuilder& out, const Property& element) const;

  void defineProperty(cursiv::TextBuilder& out, const Property& element,
                      bool gen) const;

  // The class that defines `member` in the hand-edited `.cpp`: the hand-edited
  // one overrides a virtual member, and the others stay in `XGen` when `gen` is
  // true.
  [[nodiscard]] static std::string definingClass(const NamedElement& member,
                                                 bool gen);

  // The overrides and the `// TODO` stubs of the callbacks in the hand-edited
  // class.
  void declareEntryPoint(cursiv::TextBuilder& out,
                         const EntryPoint& element) const;
  void defineEntryPoint(cursiv::TextBuilder& out,
                        const EntryPoint& element) const;
  void declareEventSink(cursiv::TextBuilder& out,
                        const EventSink& element) const;
  void defineEventSink(cursiv::TextBuilder& out,
                       const EventSink& element) const;
};

}  // namespace xsmp
