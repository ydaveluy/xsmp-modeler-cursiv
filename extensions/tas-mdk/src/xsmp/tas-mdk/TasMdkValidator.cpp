#include <algorithm>
#include <cursiv/validation/ValidationRegistry.hpp>
#include <filesystem>
#include <unordered_set>
#include <xsmp/tas-mdk/TasMdkValidator.hpp>
#include <xsmp/utils/XsmpUtils.hpp>

namespace xsmp {

namespace {

// The top of the chain of bases. Null when a link is unresolved or the chain is
// cyclic.
const Component* rootBase(const Component& component) {
  const auto* node = &component;
  std::unordered_set<const Component*> visited;
  while (node->base) {
    if (!visited.insert(node).second) return nullptr;
    const auto* base = cursiv::ast_cast<Component>(node->base->get());
    if (!base) return nullptr;
    node = base;
  }
  return node;
}

}  // namespace

void TasMdkValidator::description(const NamedElement& element,
                                  cursiv::ValidationAcceptor& accept,
                                  std::string_view keyword) const {
  if (const auto text = docDescription(element); !text || text->empty())
    accept
        .error(element, "The " + std::string(element.type->name)
                            + " description is missing.")
        .keyword(keyword);
}

void TasMdkValidator::checkField(const Field& field,
                                 cursiv::ValidationAcceptor& accept) const {
  if (isString8(field.type.get()))
    accept.error(field, "String8 type is forbidden for fields.")
        .property<&Field::type>();
  if (!cursiv::ast_cast<Component>(field.parent)) return;
  if (getRealVisibility(field) == Visibility::Public)
    accept.error(field, "A field cannot be public in Gram environment.")
        .property<&Field::modifiers>();

  const auto input = std::ranges::find(field.modifiers, "input")
                     != field.modifiers.end(),
             output = std::ranges::find(field.modifiers, "output")
                      != field.modifiers.end();
  if (input && output) {
    accept.error(field, "A field cannot be both an input and an output.")
        .property<&Field::modifiers>();
    return;
  }

  if (!field.name.empty()) {
    if (input && !field.name.starts_with("inp_"))
      accept.error(field, "The name of an input field must start with 'inp_'.")
          .property<&Field::name>();
    else if (output && !field.name.starts_with("out_"))
      accept.error(field, "The name of an output field must start with 'out_'.")
          .property<&Field::name>();
    else if (!input && !output && !field.name.starts_with("fea_")
             && !field.name.starts_with("sta_"))
      accept
          .error(field,
                 "The name of a feature field must start with 'fea_' and a "
                 "state must start with 'sta_'.")
          .property<&Field::name>();
  }
  description(field, accept, "field");
}

void TasMdkValidator::checkCatalogue(const Catalogue& catalogue,
                                     cursiv::ValidationAcceptor& accept) const {
  const auto* document = catalogue.getDocument();
  if (!document) return;
  const auto slash = document->uri.find_last_of('/');
  const auto filename =
      std::filesystem::path(
          document->uri.substr(slash == std::string::npos ? 0 : slash + 1))
          .stem()
          .string();
  if (filename != catalogue.name)
    accept
        .error(catalogue,
               "The Catalogue name '" + std::string(catalogue.name)
                   + "' must match the file name '" + filename
                   + "'. Rename the file or the Catalogue name accordingly.")
        .property<&Catalogue::name>();
}

void TasMdkValidator::checkModel(const Model& model,
                                 cursiv::ValidationAcceptor& accept) const {
  const auto* base = rootBase(model);
  if (base && qualifiedName(*base) != "TasMdk.Model")
    accept
        .error(model, "The Model " + std::string(model.name)
                          + " must extend 'TasMdk.Model' or one of its "
                            "subclasses.")
        .keyword("model");
}

void TasMdkValidator::checkService(const Service& service,
                                   cursiv::ValidationAcceptor& accept) const {
  const auto* base = rootBase(service);
  if (base && qualifiedName(*base) != "TasMdk.Service")
    accept
        .error(service, "The Service " + std::string(service.name)
                            + " must extend 'TasMdk.Service' or one of its "
                              "subclasses.")
        .keyword("service");
}

void TasMdkValidator::checkEventType(const EventType& event,
                                     cursiv::ValidationAcceptor& accept) const {
  if (event.eventArgs && event.eventArgs->get())
    accept
        .error(event,
               "In Gram environment, an Event shall only be of type void.")
        .keyword("event");
}

void TasMdkValidator::checkEventSource(
    const EventSource& source, cursiv::ValidationAcceptor& accept) const {
  if (!source.name.starts_with("eso_"))
    accept.error(source, "The name of an EventSource must start with 'eso_'.")
        .property<&EventSource::name>();
  if (const auto* event = cursiv::ast_cast<EventType>(source.type.get());
      event && event->eventArgs && event->eventArgs->get())
    accept.error(source, "An EventSource must be of type void.")
        .property<&EventSource::type>();
  description(source, accept, "eventsource");
}

void TasMdkValidator::checkEventSink(const EventSink& sink,
                                     cursiv::ValidationAcceptor& accept) const {
  if (!sink.name.starts_with("esi_"))
    accept.error(sink, "The name of an EventSink must start with 'esi_'.")
        .property<&EventSink::name>();
  if (const auto* event = cursiv::ast_cast<EventType>(sink.type.get());
      event && event->eventArgs && event->eventArgs->get())
    accept.error(sink, "An EventSink must be of type void.")
        .property<&EventSink::type>();
  description(sink, accept, "eventsink");
}

void TasMdkValidator::checkEntryPoint(
    const EntryPoint& entry, cursiv::ValidationAcceptor& accept) const {
  if (!entry.name.starts_with("ept_"))
    accept.error(entry, "The name of an EntryPoint must start with 'ept_'.")
        .property<&EntryPoint::name>();
  description(entry, accept, "entrypoint");
}

void TasMdkValidator::checkReference(const Reference& reference,
                                     cursiv::ValidationAcceptor& accept) const {
  if (!reference.name.starts_with("ref_"))
    accept.error(reference, "The name of a Reference must start with 'ref_'.")
        .property<&Reference::name>();
}

void TasMdkValidator::checkOperation(const Operation& operation,
                                     cursiv::ValidationAcceptor& accept) const {
  if (!cursiv::ast_cast<Component>(operation.parent)) return;
  if (getRealVisibility(operation) == Visibility::Public)
    accept.error(operation, "An operation cannot be public in Gram environment.")
        .property<&Operation::modifiers>();
  if (!operation.name.starts_with("ope_"))
    accept.error(operation, "The name of an operation must start with 'ope_'.")
        .property<&Operation::name>();
  // An unresolved type is already reported as such.
  const auto notPublicable = [](const auto* type) {
    return type && !cursiv::ast_cast<SimpleType>(type);
  };
  for (const auto* parameter : operation.parameter)
    if (notPublicable(parameter->type.get()))
      accept
          .error(*parameter,
                 "A parameter of type "
                     + std::string(parameter->type.get()->type->name)
                     + " is not publicable.")
          .property<&Parameter::type>();
  if (const auto* result = operation.returnParameter;
      result && notPublicable(result->type.get()))
    accept
        .error(*result, "A parameter of type "
                            + std::string(result->type.get()->type->name)
                            + " is not publicable.")
        .property<&ReturnParameter::type>();
  description(operation, accept, "def");
}

void TasMdkValidator::checkClass(const Class& clazz,
                                 cursiv::ValidationAcceptor& accept) const {
  accept.error(clazz, "Classes are not allowed in the Gram environment.")
      .keyword("class");
}

void registerTasMdkValidationChecks(
    XsmpcatCoreServices& services, const cursiv::ValidationCategory& category) {
  using Registry = cursiv::ValidationRegistry;
  // It holds nothing, so one serves every workspace.
  static const TasMdkValidator validator;
  services.validationRegistry->registerChecks(
      {Registry::makeValidationCheck<&TasMdkValidator::checkField>(validator),
       Registry::makeValidationCheck<&TasMdkValidator::checkCatalogue>(validator),
       Registry::makeValidationCheck<&TasMdkValidator::checkModel>(validator),
       Registry::makeValidationCheck<&TasMdkValidator::checkService>(validator),
       Registry::makeValidationCheck<&TasMdkValidator::checkEventType>(validator),
       Registry::makeValidationCheck<&TasMdkValidator::checkEventSource>(validator),
       Registry::makeValidationCheck<&TasMdkValidator::checkEventSink>(validator),
       Registry::makeValidationCheck<&TasMdkValidator::checkEntryPoint>(validator),
       Registry::makeValidationCheck<&TasMdkValidator::checkReference>(validator),
       Registry::makeValidationCheck<&TasMdkValidator::checkOperation>(validator),
       Registry::makeValidationCheck<&TasMdkValidator::checkClass>(validator)},
      &category);
}

}  // namespace xsmp
