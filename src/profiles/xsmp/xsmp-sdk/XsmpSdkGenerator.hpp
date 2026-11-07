#pragma once

#include <xsmp/cpp/GapPatternCppGenerator.hpp>

namespace xsmp {

/// The C++ generator of the "xsmp-sdk" profile. It implements what depends on
/// the XSMP runtime library (types, components, members, request handlers) and
/// shares the rest with the other C++ profiles.
class XsmpSdkGenerator final : public GapPatternCppGenerator {
 public:
  using GapPatternCppGenerator::GapPatternCppGenerator;

 protected:
  void registerModel(cursiv::TextBuilder& out,
                     const Model& model) const override;

  [[nodiscard]] std::vector<Include> factoryIncludes() const override;

  void generateArrayHeaderGen(cursiv::TextBuilder& out, const Array& type,
                              bool gen) const override;

  [[nodiscard]] std::vector<Include> headerIncludesArray(
      const Array& type) const override;

  void generateStringHeaderGen(cursiv::TextBuilder& out, const String& type,
                               bool gen) const override;

  [[nodiscard]] std::vector<Include> headerIncludesString(
      const String& type) const override;

  // Each member is an object of the runtime (`::Xsmp::Container` and so on),
  // set up by a constructor initializer.
  [[nodiscard]] std::vector<Include> headerIncludesContainer(
      const Container& element) const override;

  void declareContainerGen(cursiv::TextBuilder& out,
                           const Container& element) const override;

  void initializeContainer(cursiv::TextBuilder& out,
                           const Container& element) const override;

  void declareReferenceGen(cursiv::TextBuilder& out,
                           const Reference& element) const override;

  [[nodiscard]] std::vector<Include> headerIncludesReference(
      const Reference& element) const override;

  void initializeReference(cursiv::TextBuilder& out,
                           const Reference& element) const override;

  void declareEntryPointGen(cursiv::TextBuilder& out, const EntryPoint& element,
                            bool gen) const override;

  [[nodiscard]] std::vector<Include> headerIncludesEntryPoint(
      const EntryPoint& element) const override;

  void initializeEntryPoint(cursiv::TextBuilder& out, const EntryPoint& element,
                            bool gen) const override;

  void declareEventSinkGen(cursiv::TextBuilder& out, const EventSink& element,
                           bool gen) const override;

  [[nodiscard]] std::vector<Include> headerIncludesEventSink(
      const EventSink& element) const override;

  void initializeEventSink(cursiv::TextBuilder& out, const EventSink& element,
                           bool gen) const override;

  void declareEventSourceGen(cursiv::TextBuilder& out,
                             const EventSource& element) const override;

  [[nodiscard]] std::vector<Include> headerIncludesEventSource(
      const EventSource& element) const override;

  void initializeEventSource(cursiv::TextBuilder& out,
                             const EventSource& element) const override;

  // Adds `_Field<_BASE>` to the struct, with one runtime field per member.
  void generateStructureHeaderGen(cursiv::TextBuilder& out,
                                  const Structure& type,
                                  bool gen) const override;

  // SMP 2025 dropped the memory size of an enumeration: it is always backed by
  // an `Smp::Int32`. The preprocessor keeps the two calls apart, so that one
  // generated file compiles against either revision.
  void generateEnumerationSourceGen(cursiv::TextBuilder& out,
                                    const Enumeration& type) const override;

  void sourcePrologue(cursiv::TextBuilder& out,
                      const Type& type) const override;

  [[nodiscard]] std::vector<Include> sourceIncludesComponent(
      const Component& type) const override;

  [[nodiscard]] std::vector<Include> headerIncludesComponent(
      const Component& type) const override;

  /// The hand-edited header: it befriends `::Xsmp::Component::Helper`, which
  /// calls the `DoPublish`, `DoConfigure`, `DoConnect` and `DoDisconnect`
  /// hooks.
  void generateComponentHeader(cursiv::TextBuilder& out,
                               const Component& type) const override;

  void generateComponentHeaderGen(cursiv::TextBuilder& out,
                                  const Component& type,
                                  bool gen) const override;

  [[nodiscard]] std::string componentBase(const Component& type) const override;

  [[nodiscard]] std::vector<std::string> componentBases(
      const Component& type) const override;

  void generateComponentSourceGen(cursiv::TextBuilder& out,
                                  const Component& type,
                                  bool gen) const override;

  // Beyond the base rule, `Smp.String8` is refused as a return type and as the
  // type of an `out` or `inout` parameter: the ownership of its memory is
  // unknown.
  [[nodiscard]] std::optional<std::string> notInvokableReason(
      const Operation& operation) const override;

  // Whether a field is declared as an `::Xsmp::Field` wrapper: a non-static
  // field of a Component, not of a Class type, that is an output,
  // `@Attributes.Failure` or `@Attributes.Forcible`.
  [[nodiscard]] bool isCdkField(const Field& field) const override;

  [[nodiscard]] std::vector<Include> headerIncludesField(
      const Field& field) const override;

  void declareFieldGen(cursiv::TextBuilder& out, const Field& field,
                       bool gen) const override;

  void initializeField(cursiv::TextBuilder& out,
                       const Field& field) const override;

 private:
  // The `_requestHandlers` table and the `Invoke` function of `type`.
  void defineRequestHandlers(cursiv::TextBuilder& out, const Component& type,
                             std::string_view className, std::string_view base,
                             bool gen) const;

  // An entry of `_requestHandlers` per invokable Operation, and per `get_` and
  // `set_` of an invokable Property.
  void requestHandlerOperation(cursiv::TextBuilder& out, const Operation& op,
                               bool gen) const;

  void requestHandlerProperty(cursiv::TextBuilder& out,
                              const Property& property, bool gen) const;

  void initParameter(cursiv::TextBuilder& out,
                     const Parameter& parameter) const;

  void setParameter(cursiv::TextBuilder& out, const Parameter& parameter) const;

  // Whether the component gets a `_requestHandlers` table and an `Invoke`
  // function.
  [[nodiscard]] bool hasInvokableMembers(const Component& type) const;

  // True for a Model with an `@Attributes.Failure` field: it derives from
  // `::Xsmp::FallibleModel`.
  [[nodiscard]] static bool hasFailureField(const Component& type);
};

}  // namespace xsmp
