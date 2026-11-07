#pragma once

#include <xsmp/cpp/GapPatternCppGenerator.hpp>

namespace xsmp {

/// The C++ generator of the "esa-cdk" profile. It implements what depends on
/// the ESA SMP CDK runtime library.
class EsaCdkGenerator : public GapPatternCppGenerator {
 public:
  using GapPatternCppGenerator::GapPatternCppGenerator;

 protected:
  [[nodiscard]] unsigned cxxStandard() const override { return 11; }

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

  // Each member is a pointer to a runtime object: the initializer creates it
  // with `new`, the constructor adds it to its owner and the destructor deletes
  // it.
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

  [[nodiscard]] std::vector<Include> headerIncludesComponent(
      const Component& type) const override;

  [[nodiscard]] std::vector<Include> sourceIncludesComponent(
      const Component& type) const override;

  void generateComponentHeader(cursiv::TextBuilder& out,
                               const Component& type) const override;

  [[nodiscard]] std::string componentBase(const Component& type) const override;

  [[nodiscard]] std::vector<std::string> componentBases(
      const Component& type) const override;

  // Beyond the base rule, an Operation needs a `@View` attribute, not to be
  // static and only `in` parameters whose type is not a String.
  [[nodiscard]] std::optional<std::string> notInvokableReason(
      const Operation& operation) const override;

  void generateComponentHeaderGen(cursiv::TextBuilder& out,
                                  const Component& type,
                                  bool gen) const override;

  void generateComponentSourceGen(cursiv::TextBuilder& out,
                                  const Component& type,
                                  bool gen) const override;

 private:
  // What the constructor does with each of its members.
  void constructMembers(cursiv::TextBuilder& out,
                        std::span<NamedElement* const> elements) const;

  // Whether the component handles requests: only operations do.
  [[nodiscard]] bool hasInvokableMembers(const Component& type) const;

  // Writes the `AddIfMissing` statement of `PopulateRequestHandlers` for an
  // operation.
  void requestHandlerOperation(cursiv::TextBuilder& out,
                               const Operation& operation, bool gen) const;
};

}  // namespace xsmp
