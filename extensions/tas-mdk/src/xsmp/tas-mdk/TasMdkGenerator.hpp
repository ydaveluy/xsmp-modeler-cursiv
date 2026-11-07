#pragma once

#include <xsmp/cpp/GapPatternCppGenerator.hpp>

namespace xsmp {

// The C++ generator of the "tas-mdk" profile. Its output builds with a C++11
// compiler that may not know `__has_include`, so it includes `Smp/Version.h`
// without testing for it.
class TasMdkGenerator final : public GapPatternCppGenerator {
 public:
  explicit TasMdkGenerator(const XsmpcatCoreServices& services)
      : GapPatternCppGenerator(services) {
    includeFolder = "include";
    includeGenFolder = "include-gen";
  }

 protected:
  [[nodiscard]] unsigned cxxStandard() const override { return 11; }

  void generateEnumerationSourceGen(cursiv::TextBuilder& out,
                                    const Enumeration& type) const override;

  void generatePackageHeader(cursiv::TextBuilder& out,
                             const Catalogue& catalogue) const override;

  void generatePackageSource(cursiv::TextBuilder& out,
                             const Catalogue& catalogue) const override;

  void generateDynamicPackageSource(cursiv::TextBuilder& out,
                                    const Catalogue& catalogue) const override;

  [[nodiscard]] std::vector<Include> sourceIncludesComponent(
      const Component& type) const override;

  [[nodiscard]] std::vector<Include> sourceIncludesEnumeration(
      const Enumeration& type) const override;

  [[nodiscard]] std::vector<Include> sourceIncludesOperation(
      const Operation& element) const override;

  void registerModel(cursiv::TextBuilder& out,
                     const Model& model) const override;

  void registerService(cursiv::TextBuilder& out,
                       const Service& service) const override;

  [[nodiscard]] std::vector<Include> factoryIncludes() const override;

  void generateArrayHeaderGen(cursiv::TextBuilder& out, const Array& type,
                              bool gen) const override;

  [[nodiscard]] std::vector<Include> headerIncludesArray(
      const Array& type) const override;

  void generateStringHeaderGen(cursiv::TextBuilder& out, const String& type,
                               bool gen) const override;

  [[nodiscard]] std::vector<Include> headerIncludesString(
      const String& type) const override;

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

  [[nodiscard]] std::vector<Include> headerIncludesEntryPoint(
      const EntryPoint& element) const override;

  [[nodiscard]] std::vector<Include> sourceIncludesEntryPoint(
      const EntryPoint& element) const override;

  void initializeEntryPoint(cursiv::TextBuilder& out, const EntryPoint& element,
                            bool gen) const override;

  [[nodiscard]] std::vector<Include> sourceIncludesEventSink(
      const EventSink& element) const override;

  void initializeEventSink(cursiv::TextBuilder& out, const EventSink& element,
                           bool gen) const override;

  void declareEventSourceGen(cursiv::TextBuilder& out,
                             const EventSource& element) const override;

  [[nodiscard]] std::vector<Include> headerIncludesEventSource(
      const EventSource& element) const override;

  void initializeEventSource(cursiv::TextBuilder& out,
                             const EventSource& element) const override;

  void generateStructureHeaderGen(cursiv::TextBuilder& out,
                                  const Structure& type,
                                  bool gen) const override;

  [[nodiscard]] std::vector<Include> headerIncludesStructure(
      const Structure& type) const override;

  [[nodiscard]] std::vector<Include> headerIncludesComponent(
      const Component& type) const override;

  void generateComponentHeader(cursiv::TextBuilder& out,
                               const Component& type) const override;

  void generateComponentSource(cursiv::TextBuilder& out, const Component& type,
                               bool gen) const override;

  void generateComponentHeaderGen(cursiv::TextBuilder& out,
                                  const Component& type,
                                  bool gen) const override;

  [[nodiscard]] std::string componentBase(const Component& type) const override;

  void generateComponentSourceGen(cursiv::TextBuilder& out,
                                  const Component& type,
                                  bool gen) const override;

  // Whether a field of a component is a `TasMdk` field object, not a plain
  // member: an input, an output, or one with a TAS MDK attribute.
  [[nodiscard]] bool isCdkField(const Field& field) const override;

  [[nodiscard]] std::vector<Include> headerIncludesField(
      const Field& field) const override;

  void declareFieldGen(cursiv::TextBuilder& out, const Field& field,
                       bool gen) const override;

  void initializeField(cursiv::TextBuilder& out,
                       const Field& field) const override;

 private:
  // The `_<name>` field type that goes with an array or a structure, or the
  // type itself for a simple type. Empty for no type.
  [[nodiscard]] std::string fieldType(const Type* type) const;

  // `StructureField<...>`, `ArrayField<...>` or `SimpleField<...>`.
  [[nodiscard]] std::string fieldWrapper(const Field& field) const;

  // The qualifiers of the field type of `field`, such as `::transient::input`.
  [[nodiscard]] static std::string fieldQualifiers(const Field& field);

  // The `_<name>` class of a structure, whose fields are objects of the
  // runtime.
  void structureFieldClass(cursiv::TextBuilder& out, const Structure& type,
                           std::span<const Field* const> fields) const;

  // The code that adds the request handlers of an operation or a property,
  // unless one of that name is already there.
  void requestHandlerOperation(cursiv::TextBuilder& out,
                               const Operation& operation) const;

  void requestHandlerProperty(cursiv::TextBuilder& out,
                              const Property& property) const;

  // The local variable of a parameter of a request handler.
  void initParameter(cursiv::TextBuilder& out,
                     const Parameter& parameter) const;

  // What a request handler gives back of a parameter.
  void setParameter(cursiv::TextBuilder& out, const Parameter& parameter) const;
};

}  // namespace xsmp
