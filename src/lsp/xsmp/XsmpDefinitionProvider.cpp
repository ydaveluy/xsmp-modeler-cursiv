#include <algorithm>
#include <cursiv/references/NameProvider.hpp>
#include <cursiv/references/References.hpp>
#include <cursiv/services/SharedCoreServices.hpp>
#include <cursiv/syntax-tree/CstUtils.hpp>
#include <cursiv/text/LspPosition.hpp>
#include <cursiv/workspace/Documents.hpp>
#include <utility>
#include <xsmp/DeploymentSemantics.hpp>
#include <xsmp/XsmpDefinitionProvider.hpp>
#include <xsmp/ast.hpp>

namespace xsmp {

namespace {

using Node = cursiv::AstNode;

const TemplateParameter* parameter(const Assembly* assembly,
                                   std::string_view name) {
  if (assembly)
    for (auto* p : assembly->parameters)
      if (p->name == name) return p;
  return nullptr;
}

// The template parameter `name` seen from `node`, null if none: a task's
// assembly, then the assemblies around a configuration, then the assembly,
// schedule or link base the file declares.
const TemplateParameter* findParameter(const Node* node,
                                       std::string_view name) {
  if (auto* task = cursiv::container_of_type<Task>(node))
    if (auto* found = parameter(task->context.get<Assembly>(), name))
      return found;
  if (auto* configuration =
          cursiv::container_of_type<ComponentConfiguration>(node)) {
    const auto* referencePath = cursiv::container_of_type<Path>(node);
    const auto inDeclarationPath =
        referencePath && referencePath->parent == configuration;
    // The path that declares a configuration is read from the one around it.
    if (inDeclarationPath) {
      if (auto* found = parameter(configuration->context.get<Assembly>(), name))
        return found;
      configuration = cursiv::container_of_type<ComponentConfiguration>(
          configuration->parent);
    }
    for (auto* current = configuration; current;
         current = cursiv::container_of_type<ComponentConfiguration>(
             current->parent)) {
      if (auto* found = parameter(current->context.get<Assembly>(), name))
        return found;
      if (auto* target = pathTarget(current->name)) {
        if (auto* instance =
                cursiv::container_of_type<AssemblyInstance>(target))
          if (auto* found = parameter(instance->assembly.get(), name))
            return found;
        if (auto* found =
                parameter(cursiv::container_of_type<Assembly>(target), name))
          return found;
      }
    }
  }
  if (auto* assembly = cursiv::container_of_type<Assembly>(node))
    return parameter(assembly, name);
  if (auto* schedule = cursiv::container_of_type<Schedule>(node)) {
    for (auto* p : schedule->parameters)
      if (p->name == name) return p;
    return nullptr;
  }
  if (auto* links = cursiv::container_of_type<LinkBase>(node))
    return parameter(links->assembly.get(), name);
  return nullptr;
}

bool isIdentifier(std::string_view text) {
  const auto word = [](char c) {
    return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z')
           || (c >= 'a' && c <= 'z') || c == '_';
  };
  return !text.empty() && !(text.front() >= '0' && text.front() <= '9')
         && std::ranges::all_of(text, word);
}

// Calls `visit(name, begin, end)` for the `{name}` placeholders of a token, one
// by one, with the place of the name, braces excluded, in the document. Stops
// when it answers true, and answers the same.
template <class Visit>
bool visitPlaceholders(std::string_view text, cursiv::TextOffset token,
                       Visit visit) {
  for (auto opening = text.find('{'); opening != text.npos;
       opening = text.find('{', opening + 1)) {
    const auto closing = text.find('}', opening);
    if (closing == text.npos) break;
    const auto name = text.substr(opening + 1, closing - opening - 1);
    if (isIdentifier(name)
        && visit(name, token + static_cast<cursiv::TextOffset>(opening + 1),
                 token + static_cast<cursiv::TextOffset>(closing)))
      return true;
  }
  return false;
}

// What `node` is typed by: itself for a type, the type a member holds, the
// enumeration of a literal. Null for anything else.
const Type* typeOf(const Node* node) {
  if (const auto* type = cursiv::ast_cast<Type>(node)) return type;
  if (const auto* n = cursiv::ast_cast<Field>(node)) return n->type.get();
  if (const auto* n = cursiv::ast_cast<Constant>(node)) return n->type.get();
  if (const auto* n = cursiv::ast_cast<Property>(node)) return n->type.get();
  if (const auto* n = cursiv::ast_cast<Association>(node)) return n->type.get();
  if (const auto* n = cursiv::ast_cast<Parameter>(node)) return n->type.get();
  if (const auto* n = cursiv::ast_cast<ReturnParameter>(node))
    return n->type.get();
  if (const auto* n = cursiv::ast_cast<Container>(node)) return n->type.get();
  if (const auto* n = cursiv::ast_cast<Reference>(node))
    return n->interface_.get();
  if (const auto* n = cursiv::ast_cast<EventSink>(node)) return n->type.get();
  if (const auto* n = cursiv::ast_cast<EventSource>(node)) return n->type.get();
  if (cursiv::ast_cast<EnumerationLiteral>(node))
    return cursiv::ast_cast<Enumeration>(node->parent);
  return nullptr;
}

// A link from `begin`..`end` in `source` to the name of `target`. Nothing for a
// target with no name in a parsed document.
std::optional<::lsp::LocationLink> location(const Node& target,
                                            const cursiv::Document& source,
                                            cursiv::TextOffset begin,
                                            cursiv::TextOffset end) {
  const auto* home = target.getDocument();
  if (!home || !home->language || target.cstNodeId == cursiv::kNoNode)
    return std::nullopt;
  const auto name = home->language->nameProvider->getNameNode(target, home);
  if (!name) return std::nullopt;
  const auto& node = home->parseResult.cst()[target.cstNodeId];
  ::lsp::LocationLink link;
  link.targetUri = ::lsp::Uri::parse(home->uri);
  link.targetRange =
      cursiv::to_lsp(home->textDocument().rangeAt(node.begin, node.end));
  link.targetSelectionRange =
      cursiv::to_lsp(home->textDocument().rangeAt(name->begin(), name->end()));
  link.originSelectionRange =
      cursiv::to_lsp(source.textDocument().rangeAt(begin, end));
  return link;
}

}  // namespace

cursiv::Declaration XsmpDeploymentReferences::findDeclaration(
    const cursiv::Document& document, cursiv::TextOffset offset,
    const cursiv::CancellationToken& token) const {
  auto found = DefaultReferences::findDeclaration(document, offset, token);
  if (found) return found;

  const auto& cst = document.parseResult.cst();
  // From the first leaf that ends at or after the caret.
  for (auto id = offset ? cst.leaf_after(offset - 1) : 0; id < cst.size();
       id = cst.next_leaf(id)) {
    const auto leaf = cst.as_leaf(id);
    if (leaf.begin > offset) break;
    if (leaf.hidden || leaf.recovered) continue;
    const auto* owner = cursiv::owner_of_leaf(cst, leaf.id);
    const auto text = cst.text(leaf.id);
    // A token made only of a placeholder denotes its parameter; a token such as
    // Child{Lane} denotes the child.
    if (text.size() > 2 && text.front() == '{' && text.back() == '}'
        && isIdentifier(text.substr(1, text.size() - 2)))
      if (const auto* target =
              findParameter(owner, text.substr(1, text.size() - 2)))
        return {.span = {.begin = leaf.begin, .end = leaf.end},
                .targets = {target}};

    const auto* segment = cursiv::ast_cast<PatternPathNamedSegment>(owner);
    if (!segment) continue;
    const auto* path = cursiv::container_of_type<Path>(segment);
    if (!path) continue;
    const auto* target = resolveSegment(segment);
    if (!target) continue;
    // The path of a configuration or an include denotes instances only.
    if ((cursiv::ast_cast<ComponentConfiguration>(path->parent)
         || cursiv::ast_cast<ConfigurationUsage>(path->parent))
        && !(cursiv::ast_cast<ModelInstance>(target)
             || cursiv::ast_cast<AssemblyInstance>(target)))
      continue;
    return {.span = {.begin = leaf.begin, .end = leaf.end},
            .targets = {target}};
  }

  return found;
}

std::vector<cursiv::ReferenceDescription>
XsmpDeploymentReferences::findReferences(
    const cursiv::AstNode& target, const cursiv::FindReferencesOptions& options,
    const cursiv::CancellationToken& token) const {
  auto rows = DefaultReferences::findReferences(target, options, token);
  const auto* parameter = cursiv::ast_cast<TemplateParameter>(&target);
  const auto* home = target.getDocument();
  if (!parameter || !home) return rows;

  // A placeholder lies inside a token, where no reference is written: each
  // document that may read the parameter is searched for it.
  for (const auto& document : shared.documents->all()) {
    cursiv::throw_if_cancelled(token);
    if ((options.documentId != cursiv::kNoDocument
         && options.documentId != document->id)
        || !document->hasAst()
        || cursiv::ast_cast<Catalogue>(document->parseResult.value))
      continue;
    const auto& cst = document->parseResult.cst();
    for (cursiv::NodeId id = 0; id < cst.size(); id = cst.next_leaf(id)) {
      const auto leaf = cst.as_leaf(id);
      if (leaf.hidden || leaf.recovered) continue;
      (void)visitPlaceholders(
          cst.text(leaf.id), leaf.begin,
          [&](std::string_view name, cursiv::TextOffset begin,
              cursiv::TextOffset end) {
            if (name == parameter->name
                && findParameter(cursiv::owner_of_leaf(cst, leaf.id), name)
                       == parameter)
              rows.push_back({.sourceDocumentId = document->id,
                              .sourceBegin = begin,
                              .sourceEnd = end,
                              .targetDocumentId = home->id,
                              .targetSymbolId = target.cstNodeId,
                              .local = document->id == home->id});
            return false;
          });
    }
  }

  std::ranges::sort(rows, {}, [](const auto& row) {
    return std::pair{row.sourceDocumentId, row.sourceBegin};
  });
  return rows;
}

std::optional<std::vector<::lsp::LocationLink>>
XsmpDefinitionProvider::getDefinition(
    const cursiv::Document& document, ::lsp::Position position,
    const cursiv::CancellationToken& token) const {
  cursiv::throw_if_cancelled(token);
  const auto offset =
      document.textDocument().offsetAt(cursiv::from_lsp(position));
  const auto& cst = document.parseResult.cst();
  const Node* cursorOwner = nullptr;
  // From the first leaf that ends at or after the caret.
  for (auto id = offset ? cst.leaf_after(offset - 1) : 0; id < cst.size();
       id = cst.next_leaf(id)) {
    const auto leaf = cst.as_leaf(id);
    if (leaf.begin > offset) break;
    if (leaf.hidden || leaf.recovered) continue;
    cursorOwner = cursiv::owner_of_leaf(cst, leaf.id);
    // The `{name}` placeholder at the caret, if it leads somewhere.
    std::optional<::lsp::LocationLink> link;
    if (visitPlaceholders(
            cst.text(leaf.id), leaf.begin,
            [&](std::string_view name, cursiv::TextOffset begin,
                cursiv::TextOffset end) {
              if (offset + 1 < begin || offset > end + 1) return false;
              const auto* target = findParameter(cursorOwner, name);
              if (target)
                link = location(*target, document, begin - 1, end + 1);
              return link.has_value();
            }))
      return std::vector<::lsp::LocationLink>{*link};
  }

  const auto* owner = cursorOwner ? cursorOwner : cursiv::owner_at(cst, offset);

  // An `unsafe` name is not checked, so it leads nowhere.
  if (const auto* segment = cursiv::ast_cast<ConcretePathNamedSegment>(owner);
      segment && segment->unsafe)
    return std::nullopt;
  if (const auto* reference = cursiv::ast_cast<LocalNamedReference>(owner);
      reference && reference->unsafe)
    return std::nullopt;

  // Neither does a container crossed by a configuration or an include path.
  if (const auto* path = cursiv::container_of_type<Path>(owner)) {
    const auto* configuration =
        cursiv::ast_cast<ComponentConfiguration>(path->parent);
    const auto* usage = cursiv::ast_cast<ConfigurationUsage>(path->parent);
    if ((configuration && configuration->name == path)
        || (usage && usage->path == path))
      if (const auto* segment = cursiv::ast_cast<PathSegment>(owner);
          segment && cursiv::ast_cast<Container>(resolveSegment(segment)))
        return std::nullopt;
  }

  if (const auto* segment = cursiv::ast_cast<PatternPathNamedSegment>(owner))
    if (const auto* target = resolveSegment(segment)) {
      const auto& span = cst[segment->cstNodeId];
      if (auto link = location(*target, document, span.begin, span.end))
        return std::vector<::lsp::LocationLink>{*link};
    }

  return DefaultDefinitionProvider::getDefinition(document, position, token);
}

std::optional<std::vector<::lsp::LocationLink>>
XsmpcatTypeDefinitionProvider::getTypeDefinition(
    const cursiv::Document& document, ::lsp::Position position,
    const cursiv::CancellationToken& token) const {
  const auto found = services.references->findDeclaration(
      document, document.textDocument().offsetAt(cursiv::from_lsp(position)),
      token);

  std::vector<::lsp::LocationLink> result;
  for (const auto* target : found.targets) {
    cursiv::throw_if_cancelled(token);
    const auto* type = typeOf(target);
    if (!type) continue;
    if (auto link = location(*type, document, found.span.begin, found.span.end))
      result.push_back(*link);
  }
  if (result.empty()) return std::nullopt;
  return result;
}

}  // namespace xsmp
