#include <algorithm>
#include <cursiv/completion/FuzzyMatcher.hpp>
#include <cursiv/documentation/DocumentationProvider.hpp>
#include <cursiv/grammar/Grammar.hpp>
#include <cursiv/parser/Parser.hpp>
#include <cursiv/references/NameProvider.hpp>
#include <cursiv/references/ScopeProvider.hpp>
#include <cursiv/symbols/NodeKindProvider.hpp>
#include <cursiv/syntax-tree/AstReflection.hpp>
#include <cursiv/syntax-tree/CstUtils.hpp>
#include <cursiv/text/LspPosition.hpp>
#include <iomanip>
#include <set>
#include <sstream>
#include <xsmp/XsmpUuid.hpp>
#include <xsmp/XsmpcatCompletionProvider.hpp>
#include <xsmp/solver/Solver.hpp>
#include <xsmp/utils/AttributeUsage.hpp>
#include <xsmp/utils/XsmpUtils.hpp>

namespace xsmp {

namespace {

// Whether the last token before the caret closes a type or a namespace.
bool afterClosedBlock(const cursiv::CompletionContext& context) {
  const auto& cst = context.document.parseResult.cst();
  for (auto id = cst.leaf_after(context.offset); id > 0;) {
    const auto leaf = cst.as_leaf(id = cst.leaf_before(id));
    if (leaf.hidden || leaf.recovered) continue;
    const auto* owner = cursiv::owner_of_leaf(cst, leaf.id);
    return cst.text(leaf.id) == "}"
           && (cursiv::ast_cast<Type>(owner)
               || cursiv::ast_cast<Namespace>(owner));
  }
  return false;
}

// The first token written at or after `offset`, empty if none.
std::string_view tokenAfter(const cursiv::Cst& cst, cursiv::TextOffset offset) {
  for (auto id = cst.leaf_after(offset); id < cst.size();
       id = cst.next_leaf(id))
    if (const auto leaf = cst.as_leaf(id); !leaf.hidden && !leaf.recovered)
      return cst.text(leaf.id);
  return {};
}

// Where the declaration whose keyword stands at the caret starts: at the
// attributes and modifiers before the keyword, else at the keyword.
cursiv::TextOffset declarationStart(const cursiv::CompletionContext& context) {
  const auto& cst = context.document.parseResult.cst();
  auto start = context.tokenOffset;
  for (auto id = cst.leaf_after(start); id > 0;) {
    const auto leaf = cst.as_leaf(id = cst.leaf_before(id));
    if (leaf.hidden || leaf.recovered) continue;
    if (cst.grammar()->recordFeature(cst[leaf.id]) != "modifiers"
        && !cursiv::container_of_type<Attribute>(
            cursiv::owner_of_leaf(cst, leaf.id)))
      break;
    start = leaf.begin;
  }
  return start;
}

// The literal a value of this type starts from, `{...}` for an array or a
// structure, spelled out two levels deep at most and only while it stays short.
// Empty for a type with no such literal.
std::string defaultValue(const Type* type, unsigned depth = 0) {
  constexpr std::size_t maxSize = 4096;
  if (!type) return {};

  if (const auto* array = cursiv::ast_cast<Array>(type)) {
    const auto value = getValue(array->size);
    const auto size = value ? integralValueOf(*value) : std::nullopt;
    if (depth >= 2 || !size || *size < 0) return "{}";
    const auto item = defaultValue(array->itemType.get(), depth + 1);
    if (static_cast<std::uint64_t>(*size) * (item.size() + 2) > maxSize)
      return "{}";
    std::string text = "{";
    for (std::int64_t i = 0; i < *size; ++i) text += (i ? ", " : "") + item;
    return text + "}";
  }

  if (const auto* structure = cursiv::ast_cast<Structure>(type)) {
    if (depth >= 2) return "{}";
    std::string text = "{";
    for (const auto* field : getAllFields(*structure)) {
      if (text.size() > 1) text += ", ";
      text += "." + std::string(field->name) + " = "
              + defaultValue(field->type.get(), depth + 1);
      if (text.size() > maxSize) return "{}";
    }
    return text + "}";
  }

  if (const auto* enumeration = cursiv::ast_cast<Enumeration>(type))
    return enumeration->literal.empty()
               ? "0"
               : qualifiedName(*enumeration->literal[0]);

  switch (getPTK(type)) {
    case PTK::Bool:
      return "false";
    case PTK::Float32:
      return "0.0f";
    case PTK::Float64:
      return "0.0";
    case PTK::Int8:
    case PTK::Int16:
    case PTK::Int32:
      return "0";
    case PTK::Int64:
      return "0L";
    case PTK::UInt8:
    case PTK::UInt16:
    case PTK::UInt32:
      return "0U";
    case PTK::UInt64:
      return "0UL";
    case PTK::Char8:
      return "'\\0'";
    case PTK::String8:
      return "\"\"";
    case PTK::DateTime:
      return "\"2000-01-01T12:00:00Z\"";
    case PTK::Duration:
      return "\"PT0S\"";
    default:
      return cursiv::ast_cast<String>(type) ? "\"\"" : "";
  }
}

// How a keyword that starts a definition is offered. An empty `body` is written
// at the caret from the visible types; a `type` also gets a `@uuid` comment.
struct Spec {
  std::string_view keyword, label, detail, body;
  bool type = false;
};

constexpr Spec specifications[] = {
    {.keyword = "catalogue",
     .label = "Catalogue",
     .detail = "Catalogue Definition",
     .body = "catalogue ${1:Catalogue}\n$0"},
    {.keyword = "namespace",
     .label = "Namespace",
     .detail = "Namespace Definition",
     .body = "namespace ${1:Namespace}\n{\n\t$0\n}"},
    {.keyword = "struct",
     .label = "Structure",
     .detail = "Structure Definition",
     .body = "struct ${1:Name}\n{\n\t$0\n}",
     .type = true},
    {.keyword = "class",
     .label = "Class",
     .detail = "Class Definition",
     .body = "class ${1:Name}\n{\n\t$0\n}",
     .type = true},
    {.keyword = "exception",
     .label = "Exception",
     .detail = "Exception Definition",
     .body = "exception ${1:Name}\n{\n\t$0\n}",
     .type = true},
    {.keyword = "interface",
     .label = "Interface",
     .detail = "Interface Definition",
     .body = "interface ${1:Name}\n{\n\t$0\n}",
     .type = true},
    {.keyword = "model",
     .label = "Model",
     .detail = "Model Definition",
     .body = "model ${1:Name}\n{\n\t$0\n}",
     .type = true},
    {.keyword = "service",
     .label = "Service",
     .detail = "Service Definition",
     .body = "service ${1:Name}\n{\n\t$0\n}",
     .type = true},
    {.keyword = "array",
     .label = "Array",
     .detail = "Array Definition",
     .body = "",
     .type = true},
    {.keyword = "using",
     .label = "Value Reference",
     .detail = "Value Reference Definition",
     .body = "",
     .type = true},
    {.keyword = "integer",
     .label = "Integer",
     .detail = "Integer Definition",
     .body = "integer ${1:Name} extends ${2:Smp.Int32}",
     .type = true},
    {.keyword = "float",
     .label = "Float",
     .detail = "Float Definition",
     .body = "float ${1:Name} extends ${2:Smp.Float64}",
     .type = true},
    {.keyword = "event",
     .label = "Event Type",
     .detail = "Event Type Definition",
     .body = "event ${1:Name}",
     .type = true},
    {.keyword = "string",
     .label = "String",
     .detail = "String Definition",
     .body = "string ${1:Name}[${2:32}]",
     .type = true},
    {.keyword = "primitive",
     .label = "Primitive",
     .detail = "Primitive Type Definition",
     .body = "primitive ${1:Name}",
     .type = true},
    {.keyword = "native",
     .label = "Native",
     .detail = "Native Type Definition",
     .body = "native ${1:Name}",
     .type = true},
    {.keyword = "attribute",
     .label = "Attribute Type",
     .detail = "Attribute Type Definition",
     .body = "",
     .type = true},
    {.keyword = "enum",
     .label = "Enumeration",
     .detail = "Enumeration Definition",
     .body = "enum ${1:Name}\n{\n\t${2:Literal} = ${3:0}\n}",
     .type = true},
    {.keyword = "constant",
     .label = "Constant",
     .detail = "Constant Definition",
     .body = ""},
    {.keyword = "field",
     .label = "Field",
     .detail = "Field Definition",
     .body = ""},
    {.keyword = "property",
     .label = "Property",
     .detail = "Property Definition",
     .body = ""},
    {.keyword = "def",
     .label = "Operation",
     .detail = "Operation Definition",
     .body = "def void ${1:name}($0)"},
    {.keyword = "association",
     .label = "Association",
     .detail = "Association Definition",
     .body = ""},
    {.keyword = "container",
     .label = "Container",
     .detail = "Container Definition",
     .body = ""},
    {.keyword = "reference",
     .label = "Reference",
     .detail = "Reference Definition",
     .body = ""},
    {.keyword = "entrypoint",
     .label = "Entry Point",
     .detail = "Entry Point Definition",
     .body = "entrypoint ${1:name}"},
    {.keyword = "eventsink",
     .label = "Event Sink",
     .detail = "Event Sink Definition",
     .body = ""},
    {.keyword = "eventsource",
     .label = "Event Source",
     .detail = "Event Source Definition",
     .body = ""}};

}  // namespace

bool XsmpcatCompletionProvider::acceptCandidate(
    const cursiv::AstNode* owner, std::string_view property,
    const cursiv::AstNodeDescription& description) const {
  const auto* node = description.node;
  const auto* type = cursiv::ast_cast<Type>(node);
  if (!owner || !node) return true;
  if (type && !isTypeVisibleFrom(*owner, *type)) return false;

  if (property == "base") {
    if (const auto* component = cursiv::ast_cast<Component>(owner))
      return !isBaseOfComponent(component, type);
    if (const auto* interface = cursiv::ast_cast<Interface>(owner))
      return !isBaseOfInterface(interface, type)
             && std::ranges::none_of(interface->base, [&](const auto& base) {
                  return base.get() == type;
                });
    if (const auto* clazz = cursiv::ast_cast<Class>(owner))
      return !isBaseOfClass(clazz, type);
  }

  if (property == "interface_") {
    if (const auto* component = cursiv::ast_cast<Component>(owner))
      return std::ranges::none_of(
          component->interface_,
          [&](const auto& reference) { return reference.get() == type; });
  }

  if (property == "itemType")
    return !isRecursiveType(cursiv::ast_cast<Array>(owner), type);

  if (property == "primitiveType")
    return cursiv::ast_cast<Integer>(owner)
               ? isIntegralType(getPTK(type)) && getPTK(type) != PTK::DateTime
                     && getPTK(type) != PTK::Duration
               : isFloatingType(getPTK(type));

  if (property == "type") {
    if (const auto* field = cursiv::ast_cast<Field>(owner)) {
      // Only a Model or Service forbids what the grammar otherwise accepts.
      const auto compatible = !cursiv::ast_cast<Component>(field->parent)
                              || cursiv::ast_cast<ValueType>(type) != nullptr;
      return compatible
             && !isRecursiveType(cursiv::ast_cast<Type>(field->parent), type);
    }
    if (const auto* attribute = cursiv::ast_cast<Attribute>(owner)) {
      // `@usage` says where an attribute type applies, and it applies once
      // unless it says `@allowMultiple`.
      const auto* candidate = cursiv::ast_cast<AttributeType>(type);
      if (!candidate) return false;
      const auto usages = docTagRanges(*candidate, "usage");
      if (std::ranges::none_of(usages, [&](const auto& usage) {
            return attribute->parent
                   && matchesUsageCategory(*attribute->parent, usage.content);
          }))
        return false;
      if (const auto* parent =
              cursiv::ast_cast<NamedElement>(attribute->parent))
        if (!docTag(*candidate, "allowMultiple"))
          for (const auto* existing : parent->attributes)
            if (existing != attribute && existing->type.get() == candidate)
              return false;
      return true;
    }
  }

  if (property == "value" && cursiv::ast_cast<NamedElementReference>(owner)) {
    const auto* expression = cursiv::ast_cast<Expression>(owner);
    const auto* expected = core.typeProvider->getType(*owner);
    if (const auto* literal = cursiv::ast_cast<EnumerationLiteral>(node))
      return !expected || literal->parent == expected;
    const auto* constant = cursiv::ast_cast<Constant>(node);
    return constant && isConstantVisibleFrom(*expression, *constant)
           && (!expected || constant->type.get() == expected);
  }

  if (property == "raisedException" || property == "getRaises"
      || property == "setRaises") {
    if (const auto* operation = cursiv::ast_cast<Operation>(owner))
      return std::ranges::none_of(
          operation->raisedException,
          [&](const auto& reference) { return reference.get() == type; });
    if (const auto* member = cursiv::ast_cast<Property>(owner)) {
      const auto& references =
          property == "getRaises" ? member->getRaises : member->setRaises;
      return std::ranges::none_of(references, [&](const auto& reference) {
        return reference.get() == type;
      });
    }
    return true;
  }

  if (property == "attachedField" || property == "input"
      || property == "output") {
    const auto* field = cursiv::ast_cast<Field>(node);
    return field
           && (field->parent == owner->parent || !isEffectivelyPrivate(*field))
           && (property != "input"
               || std::ranges::find(field->modifiers, "input")
                      != field->modifiers.end())
           && (property != "output"
               || std::ranges::find(field->modifiers, "output")
                      != field->modifiers.end());
  }

  return true;
}

void XsmpcatCompletionProvider::visitReferenceCandidates(
    const cursiv::ReferenceInfo& reference, const cursiv::CompletionContext&,
    cursiv::function_ref<bool(const cursiv::AstNodeDescription&)> visitor)
    const {
  // The node the member belongs to, where the caret stands inside one.
  const auto* owner = reference.container;
  while (owner
         && (!owner->type
             || core.astReflection->referenceIndexOf(owner->type,
                                                     reference.property)
                    != reference.reference))
    owner = owner->parent;

  (void)core.scopeProvider->visitScopeEntries(
      reference, [&](const cursiv::AstNodeDescription& candidate) {
        if (!acceptCandidate(owner, reference.property, candidate)) return true;
        if (!visitor(candidate)) return false;
        // A library's types are also offered by their short name.
        if (!candidate.name.starts_with("Smp.")
            && !candidate.name.starts_with("Attributes."))
          return true;
        auto alias = candidate;
        alias.name = candidate.name.substr(candidate.name.find('.') + 1);
        return visitor(alias);
      });
}

cursiv::CompletionValueItem
XsmpcatCompletionProvider::createReferenceCompletionItem(
    const cursiv::AstNodeDescription& candidate,
    const cursiv::ReferenceInfo& reference,
    const cursiv::CompletionContext& context) const {
  auto item = DefaultCompletionProvider::createReferenceCompletionItem(
      candidate, reference, context);
  const std::string name(candidate.name);
  item.filterText = name;
  // Primitive types first, then the names with the fewest qualifiers.
  std::ostringstream depth;
  depth << std::setw(4) << std::setfill('0')
        << (std::ranges::count(name, '.') + 1);
  const auto primitive = candidate.type == &cursiv::ast_type_tag<PrimitiveType>;
  item.sortText =
      std::string(primitive ? "0000_" : "1000_") + depth.str() + "_" + name;

  if (candidate.node && docTag(*candidate.node, "deprecated")) {
    item.tags.emplace();
    item.tags->emplace_back(::lsp::CompletionItemTag::Deprecated);
  }

  // Inside a definition the item finishes what the type starts.
  const auto is = [&]<class T>(std::string_view property) {
    return reference.reference
           == core.astReflection->referenceIndexOf(&cursiv::ast_type_tag<T>,
                                                   property);
  };

  // A multiplicity, or the rest of the type, may be written already.
  const auto* member =
      cursiv::ast_cast<NamedElementWithMultiplicity>(reference.container);
  const auto star =
      member && (member->multiplicity || member->optional_) ? "" : "*";
  const auto next =
      tokenAfter(context.document.parseResult.cst(), context.tokenEndOffset);

  auto insertion = name;
  if (is.template operator()<Field>("type"))
    item.detail = "Field Definition";
  else if (is.template operator()<Constant>("type"))
    item.detail = "Constant Definition";
  else if (is.template operator()<Property>("type"))
    item.detail = "Property Definition";
  else if (is.template operator()<Association>("type"))
    item.detail = "Association Definition";
  else if (is.template operator()<Container>("type")) {
    item.detail = "Container Definition";
    insertion += star;
  } else if (is.template operator()<Reference>("interface_")) {
    item.detail = "Reference Definition";
    insertion += star;
  } else if (is.template operator()<EventSink>("type"))
    item.detail = "Event Sink Definition";
  else if (is.template operator()<EventSource>("type"))
    item.detail = "Event Source Definition";
  else if (is.template operator()<ValueReference>("type")) {
    item.detail = "Value reference definition.";
    if (next != "*") insertion += '*';
  } else if (is.template operator()<Array>("itemType")) {
    item.detail = "Array definition.";
    if (next != "[") insertion += "[${1:1}]";
  }

  if (insertion.find("${") != std::string::npos)
    item.insertTextFormat = ::lsp::InsertTextFormat::Snippet;
  // A type replaces the name it stands on, also when the caret is before it.
  item.textEdit =
      ::lsp::TextEdit{.range = cursiv::to_lsp(context.textDocument.rangeAt(
                          context.tokenOffset, context.tokenEndOffset)),
                      .newText = insertion};
  return item;
}

cursiv::CompletionValueItem XsmpcatCompletionProvider::createSnippet(
    std::string label, std::string body, std::string detail) const {
  cursiv::CompletionValueItem item;
  item.filterText = label;
  item.sortText = "3000_" + label;
  item.label = std::move(label);
  item.detail = std::move(detail);
  item.kind = ::lsp::CompletionItemKind::Snippet;
  item.insertTextFormat = ::lsp::InsertTextFormat::Snippet;
  item.insertText = std::move(body);
  return item;
}

std::string XsmpcatCompletionProvider::typeChoice(
    const cursiv::CompletionContext& context, const cursiv::AstTypeTag* type,
    std::string_view fallback, unsigned index) const {
  const cursiv::AstNode* container = context.node();
  if (!container) container = context.document.parseResult.value;
  std::vector<std::string> names, localNames;
  std::set<std::string> seen;
  const auto append = [&](std::string_view name, bool local) {
    if (seen.emplace(name).second)
      (local ? localNames : names).emplace_back(name);
  };

  const cursiv::ReferenceInfo scope{.container = container,
                                    .document = &context.document};
  (void)core.scopeProvider->visitScopeEntries(
      scope, [&](const cursiv::AstNodeDescription& description) {
        // A `Component` or `Interface` fallback asks for a reference type.
        if (description.type && description.type->derivesFrom(type))
          if (const auto* candidate = cursiv::ast_cast<Type>(description.node);
              candidate && isTypeVisibleFrom(*container, *candidate)
              && ((fallback != "Component" && fallback != "Interface")
                  || cursiv::ast_cast<ReferenceType>(candidate))) {
            const auto local = description.documentId == context.document.id;
            append(description.name, local);
            if (description.name.starts_with("Smp.")
                || description.name.starts_with("Attributes."))
              append(description.name.substr(description.name.find('.') + 1),
                     local);
          }
        return true;
      });

  // Symbols exported from the current document come sorted by name, before
  // those of imported documents in their declaration order.
  std::ranges::sort(localNames);
  names.insert(names.begin(), localNames.begin(), localNames.end());

  const auto marker = "${" + std::to_string(index);
  if (names.empty()) return marker + ":" + std::string(fallback) + "}";
  auto out = marker + "|";
  for (const auto& name : names) {
    if (out.back() != '|') out += ',';
    for (auto c : name) {
      if (c == ',' || c == '|' || c == '\\') out += '\\';
      out += c;
    }
  }
  return out + "|}";
}

void XsmpcatCompletionProvider::completionFor(
    const cursiv::CompletionContext& context, const cursiv::NextFeature& next,
    cursiv::CompletionAcceptor accept) const {
  // A keyword the grammar lets a name spell says nothing more of the name.
  const auto named =
      next.property == "name"
      || (next.type
          && core.astReflection->referenceIndexOf(next.type, next.property)
                 != cursiv::kNoReference);
  if (named
      && !lsp.parser->getGrammar().terminalSpellings(next.feature).empty())
    return;

  DefaultCompletionProvider::completionFor(context, next, accept);
  if (next.entered && next.property == "name"
      && cursiv::container_of_type<Enumeration>(context.node()))
    (void)accept(context,
                 createSnippet("Enumeration Literal", "${1:Literal} = ${2:0}",
                               "Enumeration Literal"));
}

void XsmpcatCompletionProvider::completionForKeyword(
    const cursiv::CompletionContext& context, const cursiv::NextFeature& next,
    std::string_view keyword, cursiv::CompletionAcceptor accept) const {
  const auto* spec = std::ranges::find_if(
      specifications, [&](const auto& it) { return it.keyword == keyword; });
  if (spec == std::end(specifications)) {
    // A new line after a closed block offers definitions, not loose keywords.
    if (afterClosedBlock(context)) return;
    DefaultCompletionProvider::completionForKeyword(
        context, next, keyword,
        [&](const cursiv::CompletionContext& at,
            cursiv::CompletionValueItem&& item) {
          item.filterText = item.label;
          item.sortText = "4000_" + item.label;
          return accept(at, std::move(item));
        });
    return;
  }

  // The grammar allows the keyword here: it is offered as the snippet that
  // writes the whole statement.
  const auto& matcher = *lsp.lspShared.fuzzyMatcher;
  if (!matcher.match(context.identifier, spec->keyword)
      && !matcher.match(context.identifier, spec->label))
    return;

  std::string body(spec->body);
  if (keyword == "array" || keyword == "using") {
    body = std::string(keyword) + " ${1:Name} = "
           + typeChoice(context, &cursiv::ast_type_tag<ValueType>,
                        "Smp.Float64", 2)
           + (keyword == "array" ? "[${3:1}]" : "*");
  } else if (body.empty()) {
    const auto* tag = &cursiv::ast_type_tag<LanguageType>;
    std::string_view fallback = "Smp.Int32";
    if (keyword == "constant") tag = &cursiv::ast_type_tag<SimpleType>;
    if (keyword == "attribute") {
      tag = &cursiv::ast_type_tag<ValueType>;
      fallback = "Smp.Bool";
    }
    // A field of a model or a service holds a value type.
    if (keyword == "field")
      for (const auto* at = context.node(); at; at = at->parent)
        if (cursiv::ast_cast<Component>(at))
          tag = &cursiv::ast_type_tag<ValueType>;
    if (keyword == "container" || keyword == "reference")
      fallback = keyword == "container" ? "Component" : "Interface";
    if (keyword == "eventsink" || keyword == "eventsource") {
      tag = &cursiv::ast_type_tag<EventType>;
      fallback = "EventType";
    }
    body =
        std::string(keyword) + " " + typeChoice(context, tag, fallback, 1)
        + (keyword == "attribute" ? " ${2:Name} = ${3:false}" : " ${2:name}");
    if (keyword == "constant") body += " = ${3:0}";
    if (keyword == "container" || keyword == "reference")
      body.insert(body.find(" ${2:name}"), "*");
  }

  // The `@uuid` of a type goes before all that is already written of it, and
  // one comment holds it once.
  std::vector<::lsp::TextEdit> uuid;
  if (spec->type) {
    const auto start = declarationStart(context);
    const auto comment = docCommentBefore(context.document, start);
    const auto tagged =
        comment
        && context.document.parseResult.cst().text(comment->id).find("@uuid")
               != std::string_view::npos;
    if (!comment && start == context.tokenOffset)
      body = "/** @uuid " + newUuid() + " */\n" + body;
    else if (!tagged)
      uuid.push_back(uuidEdit(context.document, start));
  }

  auto item = createSnippet(std::string(spec->label), std::move(body),
                            std::string(spec->detail));
  if (!uuid.empty()) item.additionalTextEdits = std::move(uuid);
  // Matched above, on the keyword or the label.
  item.matched = true;
  (void)accept(context, std::move(item));
}

std::vector<::lsp::CompletionItem> XsmpcatCompletionProvider::deduplicateItems(
    std::vector<::lsp::CompletionItem> items) const {
  // Two items are the same when label, detail and inserted text agree. A
  // snippet holds a fresh UUID, so only its label and detail count.
  std::set<std::string> seen;
  std::erase_if(items, [&](const ::lsp::CompletionItem& item) {
    auto key = item.label + '\0' + item.detail.value_or("");
    if (item.kind != ::lsp::CompletionItemKind::Snippet)
      if (item.textEdit)
        std::visit([&](const auto& edit) { key += '\0' + edit.newText; },
                   *item.textEdit);
    return !seen.insert(std::move(key)).second;
  });
  return items;
}

::lsp::CompletionList XsmpcatCompletionProvider::getCompletion(
    const cursiv::Document& document, const ::lsp::CompletionParams& params,
    const cursiv::CancellationToken& token) const {
  auto list = DefaultCompletionProvider::getCompletion(document, params, token);
  std::ranges::stable_sort(list.items, {}, [](const auto& item) {
    return item.sortText.value_or(item.label);
  });

  const auto offset = std::min(
      document.textDocument().offsetAt(cursiv::from_lsp(params.position)),
      static_cast<cursiv::TextOffset>(
          document.textDocument().getText().size()));

  // Right after the `=` of a field or a constant, wherever the caret is on the
  // lines after, a value is offered. Inside a token there is none to offer.
  const auto& cst = document.parseResult.cst();
  if (const auto id = cst.leaf_after(offset);
      id < cst.size() && cst.as_leaf(id).begin < offset)
    return list;
  const cursiv::AstNode* valueOwner = nullptr;
  cursiv::TextOffset equalsEnd = 0;
  for (auto id = cst.leaf_after(offset); id > 0;) {
    const auto leaf = cst.as_leaf(id = cst.leaf_before(id));
    if (leaf.hidden || leaf.recovered) continue;
    if (cst.text(leaf.id) == "=") {
      valueOwner = cursiv::owner_of_leaf(cst, leaf.id);
      equalsEnd = leaf.end;
    }
    break;
  }

  const Type* expected = nullptr;
  if (const auto* field = cursiv::ast_cast<Field>(valueOwner))
    expected = field->type.get();
  else if (const auto* constant = cursiv::ast_cast<Constant>(valueOwner))
    expected = constant->type.get();
  if (!expected) return list;

  const auto range =
      cursiv::to_lsp(document.textDocument().rangeAt(offset, offset));
  const auto makeItem = [&](std::string label, std::string detail) {
    ::lsp::CompletionItem item;
    item.filterText = label;
    item.detail = std::move(detail);
    item.kind = ::lsp::CompletionItemKind::Value;
    // `=` and a value read apart.
    item.textEdit = ::lsp::TextEdit{
        .range = range, .newText = (offset == equalsEnd ? " " : "") + label};
    item.label = std::move(label);
    return item;
  };

  std::vector<::lsp::CompletionItem> values;
  std::set<std::string> labels;
  const auto add = [&](std::string value) {
    if (!value.empty() && labels.insert(value).second)
      values.push_back(makeItem(value, "Value"));
  };

  if (const auto* enumeration = cursiv::ast_cast<Enumeration>(expected))
    for (const auto* literal : enumeration->literal)
      add(qualifiedName(*literal));
  else if (getPTK(expected) == PTK::Bool) {
    add("false");
    add("true");
  } else if (getPTK(expected) == PTK::Float32
             || getPTK(expected) == PTK::Float64) {
    add(getPTK(expected) == PTK::Float32 ? "0.f" : "0.");
    add("$PI");
    add("$E");
  } else
    add(defaultValue(expected));
  if (cursiv::ast_cast<ReferenceType>(expected)
      || cursiv::ast_cast<ValueReference>(expected))
    add("nullptr");

  // A throwaway expression here asks the scope for the constants it can read.
  NamedElementReference expression;
  expression.type = &cursiv::ast_type_tag<NamedElementReference>;
  expression.parent = const_cast<cursiv::AstNode*>(valueOwner);
  cursiv::ReferenceInfo valueScope{
      .container = &expression, .document = &document, .property = "value"};
  (void)core.scopeProvider->visitScopeEntries(
      valueScope, [&](const cursiv::AstNodeDescription& description) {
        cursiv::throw_if_cancelled(token);
        if (const auto* constant = cursiv::ast_cast<Constant>(description.node);
            constant && constant->type.get() == expected
            && (constant->parent == valueOwner->parent
                || !isEffectivelyPrivate(*constant)))
          if (labels.insert(std::string(description.name)).second) {
            auto item = makeItem(std::string(description.name), "Constant");
            item.kind = lsp.lspShared.nodeKindProvider->getCompletionItemKind(
                description);
            std::ostringstream depth;
            depth << std::setw(4) << std::setfill('0')
                  << (std::ranges::count(item.label, '.') + 1);
            item.sortText = "1000_" + depth.str() + "_" + item.label;
            if (const auto documentation =
                    core.documentationProvider->getDocumentation(*constant))
              item.documentation = ::lsp::MarkupContent{
                  .kind = ::lsp::MarkupKind::Markdown, .value = *documentation};
            values.push_back(std::move(item));
          }
        return true;
      });

  if (values.empty()) return list;
  values[0].preselect = true;
  for (auto& item : values)
    if (item.detail == "Value") item.sortText = "0500_" + item.label;
  // The plain values first, in their order, then the constants.
  std::ranges::stable_sort(values, [](const auto& a, const auto& b) {
    if (a.detail == "Value" || b.detail == "Value")
      return a.detail == "Value" && b.detail != "Value";
    return a.sortText < b.sortText;
  });
  // Right after `=` only a value makes sense: these replace the other items.
  list.items = std::move(values);
  return list;
}

}  // namespace xsmp
