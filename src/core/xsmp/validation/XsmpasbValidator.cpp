#include <cstdint>
#include <map>
#include <string>
#include <unordered_set>
#include <vector>
#include <xsmp/DeploymentSemantics.hpp>
#include <xsmp/utils/TreeUtils.hpp>
#include <xsmp/utils/XsmpUtils.hpp>
#include <xsmp/validation/XsmpasbValidator.hpp>

namespace xsmp {

void XsmpasbValidator::checkAssembly(const Assembly& node,
                                     cursiv::ValidationAcceptor& accept) const {
  checkUnusedParameters(node.parameters, usedParameters(node), accept);
  checkReferenceBounds(&node, nullptr, accept);
  checkDocument(node, accept);
  checkDuplicatedParameters(node.parameters, accept);
}

namespace {

// An instance is an SMP object at run time.
void checkObjectName(const NamedElement& node,
                     cursiv::ValidationAcceptor& accept) {
  if (isCppKeyword(node.name))
    accept.warning(node, "A name shall not be an ISO/ANSI C++ keyword.")
        .range(rangeForFeature(node, "name"));
}

// The arguments of `node` may give a child of the assembly it instantiates a
// name that is a C++ keyword.
void checkInstantiatedChildNames(const AssemblyInstance& node,
                                 cursiv::ValidationAcceptor& accept) {
  const auto* assembly = node.assembly.get();
  if (!assembly || !assembly->model || node.arguments.empty()) return;
  Bindings bindings;
  bindInstance(bindings, node);
  for (const auto* child : descendantsOf(*assembly->model)) {
    if (!cursiv::ast_cast<ModelInstance>(child)
        && !cursiv::ast_cast<AssemblyInstance>(child))
      continue;
    const std::string_view name = nameOf(child);
    if (name.find('{') == std::string_view::npos) continue;
    if (const auto concrete = expanded(name, bindings);
        concrete && isCppKeyword(*concrete))
      accept
          .warning(node, "The expanded child instance name '" + *concrete
                             + "' shall not be an ISO/ANSI C++ keyword.")
          .property<&AssemblyInstance::arguments>();
  }
}

// A link of an assembly stays inside the current Model Instance.
void checkRelativePaths(const Link& node, cursiv::ValidationAcceptor& accept) {
  if (node.ownerPath && !node.ownerPath->unsafe && node.ownerPath->absolute)
    accept
        .error(node,
               "The Owner Path shall refer to the current Model Instance or "
               "one of its children.")
        .property<&Link::ownerPath>();
  if (node.clientPath && !node.clientPath->unsafe && node.clientPath->absolute)
    accept
        .error(node,
               "The Client Path shall refer to the current Model Instance or "
               "one of its children.")
        .property<&Link::clientPath>();
}

}  // namespace

void XsmpasbValidator::checkEventLink(
    const EventLink& node, cursiv::ValidationAcceptor& accept) const {
  checkRelativePaths(node, accept);
  checkEventConnection(node, accept);
}

void XsmpasbValidator::checkFieldLink(
    const FieldLink& node, cursiv::ValidationAcceptor& accept) const {
  checkRelativePaths(node, accept);
  checkFieldConnection(node, node.ownerPath, node.clientPath, accept);
}

void XsmpasbValidator::checkInterfaceLink(
    const InterfaceLink& node, cursiv::ValidationAcceptor& accept) const {
  checkInterface(node, accept);
}

void XsmpasbValidator::checkModelInstance(
    const ModelInstance& node, cursiv::ValidationAcceptor& accept) const {
  checkInstanceName(node, "enclosing Assembly", accept);
  checkObjectName(node, accept);

  if (const auto* implementation = node.implementation.get();
      implementation && hasModifier(*implementation, "abstract"))
    accept
        .error(node,
               "A Model Instance shall not be an instance of an abstract "
               "Component.")
        .property<&ModelInstance::implementation>();

  if (node.implementation.refText.empty() && !node.strImplementation
      && !holdsReportedLiteral(node, "strImplementation")) {
    if (auto* sub = cursiv::ast_cast<SubInstance>(node.parent)) {
      if (sub->container && !sub->container->unsafe)
        if (auto* container =
                cursiv::ast_cast<Container>(sub->container->reference.get());
            container
            && (!container->defaultComponent
                || !container->defaultComponent->get()))
          accept
              .error(node,
                     "A sub-instance without an explicit implementation "
                     "shall target a Container with a default component.")
              .property<&ModelInstance::name>();
    } else
      accept
          .error(node, "A root Model Instance shall declare an implementation.")
          .property<&ModelInstance::name>();
  }

  std::unordered_set<std::string_view> names;
  std::map<const Container*, std::size_t> counts;
  for (auto* child : node.elements)
    if (auto* sub = cursiv::ast_cast<SubInstance>(child);
        sub && sub->instance) {
      if (!names.insert(sub->instance->name).second)
        accept
            .error(*sub->instance,
                   "Child Model Instance and Assembly Instance names shall "
                   "be unique at the same hierarchy level.")
            .range(rangeForFeature(*sub->instance, "name"));
      if (sub->container && !sub->container->unsafe)
        if (auto* container =
                cursiv::ast_cast<Container>(sub->container->reference.get())) {
          if (const auto upper = getUpper(*container);
              upper && *upper >= 0
              && ++counts[container] > static_cast<std::uint64_t>(*upper))
            accept
                .error(*sub, "The Container '" + std::string(container->name)
                                 + "' shall not contain more than "
                                 + std::to_string(*upper) + " sub-instance(s).")
                .property<&SubInstance::container>();
        }
    }
}

void XsmpasbValidator::checkAssemblyInstance(
    const AssemblyInstance& node, cursiv::ValidationAcceptor& accept) const {
  checkInstanceName(node, "enclosing Assembly", accept);
  checkObjectName(node, accept);
  checkInstantiatedChildNames(node, accept);
  checkTemplateArguments(node.arguments, accept);
  const auto* containing = cursiv::container_of_type<Assembly>(&node);

  // Follows the assemblies this one instantiates, however deep, looking for the
  // one it sits in.
  std::unordered_set<const Assembly*> seen;
  std::vector<const Assembly*> queue;
  if (node.assembly.get()) queue.push_back(node.assembly.get());
  for (std::size_t at = 0; at < queue.size(); ++at) {
    const auto* assembly = queue[at];
    if (assembly == containing) {
      accept.error(node, "Recursive Assembly instantiation is not allowed.")
          .property<&AssemblyInstance::assembly>();
      break;
    }
    if (!seen.insert(assembly).second || !assembly->getDocument()) continue;
    for (auto* child : assembly->getDocument()->parseResult.nodes())
      if (auto* nested = cursiv::ast_cast<AssemblyInstance>(child);
          nested && cursiv::container_of_type<Assembly>(nested) == assembly
          && nested->assembly.get())
        queue.push_back(nested->assembly.get());
  }
}

void XsmpasbValidator::checkSubInstance(
    const SubInstance& node, cursiv::ValidationAcceptor& accept) const {
  // Without a known owner type the reference has nothing to be checked against.
  if (!node.container || node.container->unsafe
      || !targetType(localContext(node.container)))
    return;

  const auto* container =
      cursiv::ast_cast<Container>(node.container->reference.get());
  if (!container)
    accept
        .error(node,
               "The selected container shall resolve to a Container of the "
               "current Component.")
        .property<&SubInstance::container>();
  else if (const auto* actual = cursiv::ast_cast<Component>(
               targetType(targetType(node.instance)));
           actual && container->type.get()
           && !isBaseOfReferenceType(container->type.get(),
                                     qualifiedName(*container->type.get()),
                                     actual))
    accept
        .error(node,
               "The type of the sub-instance shall be compatible with the "
               "selected Container.")
        .property<&SubInstance::container>();
}

void XsmpasbValidator::checkGlobalEventHandler(
    const GlobalEventHandler& node, cursiv::ValidationAcceptor& accept) const {
  if (node.entryPoint && !node.entryPoint->unsafe
      && targetType(localContext(node.entryPoint))
      && !cursiv::ast_cast<EntryPoint>(node.entryPoint->reference.get()))
    accept
        .error(node,
               "The selected entry point shall resolve to an EntryPoint of "
               "the current Component.")
        .property<&GlobalEventHandler::entryPoint>();
}

void XsmpasbValidator::checkOperationCall(
    const OperationCall& node, cursiv::ValidationAcceptor& accept) const {
  if (node.operation && !node.operation->unsafe
      && targetType(localContext(node.operation))) {
    const auto* operation =
        cursiv::ast_cast<Operation>(node.operation->reference.get());
    if (!operation)
      accept
          .error(node,
                 "The selected operation shall resolve to an Operation of "
                 "the current Component.")
          .property<&OperationCall::operation>();
    else
      checkOperationParameters(node, operation, node.parameters, accept);
  }
  checkDuplicatedCallParameters(node.parameters, accept);
}

void XsmpasbValidator::checkPropertyValue(
    const PropertyValue& node, cursiv::ValidationAcceptor& accept) const {
  if (!node.property || node.property->unsafe
      || !targetType(localContext(node.property)))
    return;
  const auto* target =
      cursiv::ast_cast<Property>(node.property->reference.get());
  if (!target)
    accept
        .error(node,
               "The selected property shall resolve to a Property of the "
               "current Component.")
        .property<&PropertyValue::property>();
  else
    checkPropertyAssignment(node, "property", target, node.value, accept);
}

}  // namespace xsmp
