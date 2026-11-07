#include <string_view>
#include <xsmp/XsmpNodeKindProvider.hpp>

namespace xsmp {

namespace {

::lsp::CompletionItemKind getCompletionItemKindOf(std::string_view type) {
  if (type == "Array") return ::lsp::CompletionItemKind::Operator;
  if (type == "Assembly" || type == "Configuration" || type == "LinkBase"
      || type == "Schedule")
    return ::lsp::CompletionItemKind::File;
  if (type == "Constant") return ::lsp::CompletionItemKind::Constant;
  if (type == "Enumeration") return ::lsp::CompletionItemKind::Enum;
  if (type == "EnumerationLiteral")
    return ::lsp::CompletionItemKind::EnumMember;
  if (type == "EmitGlobalEvent" || type == "EpochEvent" || type == "EventLink"
      || type == "EventSink" || type == "EventSource" || type == "EventType"
      || type == "GlobalEventHandler" || type == "GlobalEventTriggeredEvent"
      || type == "MissionEvent" || type == "SimulationEvent"
      || type == "Trigger" || type == "ZuluEvent")
    return ::lsp::CompletionItemKind::Event;
  if (type == "CallOperation" || type == "ExecuteTask" || type == "Operation"
      || type == "OperationCall" || type == "Task")
    return ::lsp::CompletionItemKind::Method;
  if (type == "Interface") return ::lsp::CompletionItemKind::Interface;
  if (type == "Integer" || type == "PrimitiveType" || type == "Float")
    return ::lsp::CompletionItemKind::TypeParameter;
  if (type == "Project" || type == "Tool" || type == "Profile"
      || type == "Catalogue")
    return ::lsp::CompletionItemKind::File;
  if (type == "String") return ::lsp::CompletionItemKind::Text;
  if (type == "Structure") return ::lsp::CompletionItemKind::Struct;
  if (type == "AssemblyComponentConfiguration" || type == "AssemblyInstance"
      || type == "Class" || type == "ComponentConfiguration"
      || type == "ComponentLinkBase" || type == "Exception" || type == "Model"
      || type == "ModelInstance" || type == "Service")
    return ::lsp::CompletionItemKind::Class;
  if (type == "ConfigurationUsage") return ::lsp::CompletionItemKind::Module;
  if (type == "PropertyValue" || type == "SetProperty")
    return ::lsp::CompletionItemKind::Property;
  return ::lsp::CompletionItemKind::Field;
}

::lsp::SymbolKind getSymbolKindOf(std::string_view type) {
  if (type == "Assembly" || type == "Configuration" || type == "LinkBase"
      || type == "Schedule")
    return ::lsp::SymbolKind::Package;
  if (type == "Array") return ::lsp::SymbolKind::Array;
  if (type == "CallOperation" || type == "ExecuteTask"
      || type == "OperationCall" || type == "Task" || type == "Operation")
    return ::lsp::SymbolKind::Method;
  if (type == "Constant") return ::lsp::SymbolKind::Constant;
  if (type == "Enumeration") return ::lsp::SymbolKind::Enum;
  if (type == "EnumerationLiteral") return ::lsp::SymbolKind::EnumMember;
  if (type == "EmitGlobalEvent" || type == "EpochEvent" || type == "EventLink"
      || type == "GlobalEventHandler" || type == "GlobalEventTriggeredEvent"
      || type == "MissionEvent" || type == "SimulationEvent"
      || type == "Trigger" || type == "EventSink" || type == "EventSource"
      || type == "EventType" || type == "ZuluEvent")
    return ::lsp::SymbolKind::Event;
  if (type == "AssemblyInstance" || type == "ComponentConfiguration"
      || type == "ComponentLinkBase" || type == "EntryPoint"
      || type == "ModelInstance" || type == "AssemblyComponentConfiguration")
    return ::lsp::SymbolKind::Object;
  if (type == "Interface") return ::lsp::SymbolKind::Interface;
  if (type == "Integer" || type == "PrimitiveType" || type == "Float")
    return ::lsp::SymbolKind::Number;
  if (type == "Project" || type == "Tool" || type == "Profile"
      || type == "Catalogue")
    return ::lsp::SymbolKind::Package;
  if (type == "PropertyValue" || type == "SetProperty" || type == "Property")
    return ::lsp::SymbolKind::Property;
  if (type == "ConfigurationUsage") return ::lsp::SymbolKind::Module;
  if (type == "Parameter") return ::lsp::SymbolKind::Variable;
  if (type == "String") return ::lsp::SymbolKind::String;
  if (type == "Structure") return ::lsp::SymbolKind::Struct;
  if (type == "Namespace") return ::lsp::SymbolKind::Namespace;
  if (type == "Class" || type == "Exception" || type == "Model"
      || type == "Service")
    return ::lsp::SymbolKind::Class;
  return ::lsp::SymbolKind::Field;
}

}  // namespace

::lsp::CompletionItemKind XsmpNodeKindProvider::getCompletionItemKind(
    const cursiv::AstNode& node) const {
  return getCompletionItemKindOf(node.type ? node.type->name
                                           : std::string_view{});
}

::lsp::CompletionItemKind XsmpNodeKindProvider::getCompletionItemKind(
    const cursiv::AstNodeDescription& node) const {
  return getCompletionItemKindOf(node.type ? node.type->name
                                           : std::string_view{});
}

::lsp::SymbolKind XsmpNodeKindProvider::getSymbolKind(
    const cursiv::AstNode& node) const {
  return getSymbolKindOf(node.type ? node.type->name : std::string_view{});
}

::lsp::SymbolKind XsmpNodeKindProvider::getSymbolKind(
    const cursiv::AstNodeDescription& node) const {
  return getSymbolKindOf(node.type ? node.type->name : std::string_view{});
}

}  // namespace xsmp
