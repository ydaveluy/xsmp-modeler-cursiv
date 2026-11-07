#pragma once

#include <cursiv/validation/ValidationAcceptor.hpp>
#include <xsmp/XsmpcatCoreServices.hpp>

namespace xsmp {

// The rules of the TAS MDK ("Gram") environment, on top of the standard checks:
// name prefixes, required descriptions, and limits on types and visibility.
class TasMdkValidator final {
 public:
  void checkField(const Field&, cursiv::ValidationAcceptor&) const;
  void checkCatalogue(const Catalogue&, cursiv::ValidationAcceptor&) const;
  void checkModel(const Model&, cursiv::ValidationAcceptor&) const;
  void checkService(const Service&, cursiv::ValidationAcceptor&) const;
  void checkEventType(const EventType&, cursiv::ValidationAcceptor&) const;
  void checkEventSource(const EventSource&, cursiv::ValidationAcceptor&) const;
  void checkEventSink(const EventSink&, cursiv::ValidationAcceptor&) const;
  void checkEntryPoint(const EntryPoint&, cursiv::ValidationAcceptor&) const;
  void checkReference(const Reference&, cursiv::ValidationAcceptor&) const;
  void checkOperation(const Operation&, cursiv::ValidationAcceptor&) const;
  void checkClass(const Class&, cursiv::ValidationAcceptor&) const;

 private:
  // Reports a missing description, on the given keyword of the element.
  void description(const NamedElement&, cursiv::ValidationAcceptor&,
                   std::string_view keyword) const;
};

// The checks apply where a document runs `category`.
void registerTasMdkValidationChecks(XsmpcatCoreServices&,
                                    const cursiv::ValidationCategory& category);

}  // namespace xsmp
