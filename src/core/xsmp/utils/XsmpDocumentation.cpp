#include <cctype>
#include <cursiv/documentation/DocComment.hpp>
#include <cursiv/grammar/Grammar.hpp>
#include <cursiv/services/CoreServices.hpp>
#include <cursiv/syntax-tree/AstNode.hpp>
#include <cursiv/workspace/Document.hpp>
#include <cursiv/workspace/TextDocument.hpp>
#include <numeric>
#include <utility>
#include <xsmp/ast.hpp>
#include <xsmp/utils/TreeUtils.hpp>
#include <xsmp/utils/XsmpDocumentation.hpp>

namespace xsmp {

namespace {

std::string_view trim(std::string_view text) {
  constexpr std::string_view spaces = " \t\n\v\f\r";
  const auto first = text.find_first_not_of(spaces);
  if (first == text.npos) return {};
  return text.substr(first, text.find_last_not_of(spaces) - first + 1);
}

// The content of `tag`, laid out as a paragraph: only what stands on a line of
// its own starts a new line.
std::string renderTagContent(const cursiv::DocCommentNode& tag) {
  auto content = tag;
  content.kind = cursiv::DocCommentNode::Kind::Paragraph;
  return content.toString();
}

// The record of the doc comment written before `node`, or `kNoNode`. Other
// comments can stand between them.
cursiv::NodeId commentNode(const cursiv::Document& document,
                           const cursiv::AstNode& node) {
  const auto& cst = document.parseResult.cst();
  if (node.cstNodeId == cursiv::kNoNode || node.cstNodeId >= cst.size()
      || cst.grammar() == nullptr)
    return cursiv::kNoNode;
  const auto start = cst[node.cstNodeId].begin;
  // A root with no visible content has no first token for a comment to precede.
  if (&node == document.parseResult.value && start == cst[node.cstNodeId].end)
    return cursiv::kNoNode;

  // Record ends never decrease: find the last record before the text of `node`.
  cursiv::NodeId low = 0, high = node.cstNodeId;
  while (low < high) {
    const auto middle = std::midpoint(low, high);
    if (cst[middle].end <= start)
      low = middle + 1;
    else
      high = middle;
  }
  for (auto id = low; id > 0;) {
    --id;
    const auto& record = cst[id];
    if (record.begin == record.end) continue;
    if (record.hidden == 0) return cursiv::kNoNode;
    if (cst.grammar()->recordRule(record) == "ML_COMMENT"
        && cursiv::is_doc_comment(cst.text(id)))
      return id;
  }
  return cursiv::kNoNode;
}

// Where `comment` first holds the tag `@name`, in any position, or `npos`.
std::size_t findTag(std::string_view comment, std::string_view name) {
  for (auto at = comment.find(name); at != std::string_view::npos;
       at = comment.find(name, at + 1)) {
    const auto end = at + name.size();
    if (at > 0 && comment[at - 1] == '@'
        && (end == comment.size()
            || std::isalnum(static_cast<unsigned char>(comment[end])) == 0))
      return at - 1;
  }
  return std::string_view::npos;
}

// Where the line of `tag` starts, when it is not the first line of `comment`
// and holds nothing before the tag but its line marker. Nothing before such a
// line changes what the tag holds, so that the parse can start there. Zero
// otherwise.
std::size_t parseFrom(std::string_view comment, std::size_t tag) {
  const auto lineEnd = comment.find_last_of("\r\n", tag);
  if (lineEnd == std::string_view::npos) return 0;
  const auto line = lineEnd + 1;
  auto at = comment.find_first_not_of(" \t", line);
  if (at < tag && comment[at] == '*')
    at = comment.find_first_not_of(" \t", at + 1);
  return at == tag ? line : 0;
}

// The doc comment of `node`, parsed from the first `@tagName` it holds, when it
// holds one, or whole when `tagName` is empty. Looking costs far less than
// parsing, and few comments hold the tag asked for. A plain `/* */` comment
// does not count.
std::optional<cursiv::DocComment> parseComment(const cursiv::AstNode& node,
                                               std::string_view tagName = {}) {
  const auto* document = node.getDocument();
  if (!document) return std::nullopt;
  const auto comment = commentNode(*document, node);
  if (comment == cursiv::kNoNode) return std::nullopt;
  const auto& cst = document->parseResult.cst();
  const auto text = cst.text(comment);
  std::size_t from = 0;
  if (!tagName.empty()) {
    const auto tag = findTag(text, tagName);
    if (tag == std::string_view::npos) return std::nullopt;
    from = parseFrom(text, tag);
  }
  if (!cursiv::is_doc_comment(text)) return std::nullopt;
  // Only offsets are read below, so the line of the comment is not asked for.
  const cursiv::DocCommentPosition start{
      .offset = cst[comment].begin + static_cast<std::uint32_t>(from)};
  static const cursiv::DocCommentParseOptions rest{.start = ""};
  return from == 0 ? cursiv::parse_doc_comment(text, start)
                   : cursiv::parse_doc_comment(text.substr(from), start, rest);
}

// A tag with no content gets an empty range at its end.
DocTagRange toDocTagRange(const cursiv::DocCommentNode& tag) {
  const auto start =
      tag.children.empty() ? tag.range.end : tag.children.front().range.start;
  const auto finish =
      tag.children.empty() ? tag.range.end : tag.children.back().range.end;
  return DocTagRange{.content = std::string(trim(renderTagContent(tag))),
                     .begin = start.offset,
                     .end = finish.offset};
}

}  // namespace

std::optional<cursiv::DocComment> docComment(const cursiv::AstNode& node) {
  return parseComment(node);
}

std::optional<std::string> docTag(const cursiv::AstNode& node,
                                  std::string_view tagName) {
  if (auto range = docTagRange(node, tagName)) return std::move(range->content);
  return std::nullopt;
}

std::optional<DocTagRange> docTagRange(const cursiv::AstNode& node,
                                       std::string_view tagName) {
  const auto parsed = parseComment(node, tagName);
  if (!parsed) return std::nullopt;
  for (const auto* tag : parsed->getTags(tagName))
    if (tag != nullptr && !tag->inlineTag) return toDocTagRange(*tag);
  return std::nullopt;
}

std::vector<DocTagRange> docTagRanges(const cursiv::AstNode& node,
                                      std::string_view tagName) {
  std::vector<DocTagRange> result;
  const auto parsed = parseComment(node, tagName);
  if (!parsed) return result;
  for (const auto* tag : parsed->getTags(tagName))
    if (tag != nullptr && !tag->inlineTag)
      result.push_back(toDocTagRange(*tag));
  return result;
}

std::optional<std::string> docDescription(const cursiv::AstNode& node) {
  // A parameter is described by the `@param name text` of its operation.
  if (const auto* parameter = cursiv::ast_cast<const Parameter>(&node);
      parameter && parameter->parent) {
    for (const auto& tag : docTagRanges(*parameter->parent, "param")) {
      const std::string_view content = tag.content;
      if (content.starts_with(parameter->name)
          && content.size() > parameter->name.size()
          && std::isspace(
              static_cast<unsigned char>(content[parameter->name.size()])))
        return std::string(trim(content.substr(parameter->name.size())));
    }
    return std::nullopt;
  }

  if (cursiv::ast_cast<const ReturnParameter>(&node) && node.parent)
    return docTag(*node.parent, "return");

  // A built-in has no comment to read: its text is fixed here.
  std::string_view builtin;
  if (const auto* value = cursiv::ast_cast<const BuiltInConstant>(&node))
    builtin = value->name;
  if (const auto* value = cursiv::ast_cast<const BuiltInFunction>(&node))
    builtin = value->name;
  if (!builtin.empty()) {
    static constexpr std::pair<std::string_view, std::string_view>
        descriptions[] = {
            {"PI",
             "The double value that is closer than any other to pi, the ratio "
             "of the circumference of a circle to its diameter."},
            {"E",
             "The double value that is closer than any other to e, the base of "
             "the natural logarithms."},
            {"cos", "Returns the trigonometric cosine of an angle."},
            {"sin", "Returns the trigonometric sine of an angle."},
            {"tan", "Returns the trigonometric tangent of an angle."},
            {"acos",
             "Returns the arc cosine of a value; the returned angle is in the "
             "range 0.0 through pi."},
            {"asin",
             "Returns the arc sine of a value; the returned angle is in the "
             "range -pi/2 through pi/2."},
            {"atan",
             "Returns the arc tangent of a value; the returned angle is in the "
             "range -pi/2 through pi/2."},
            {"cosh",
             "Returns the hyperbolic cosine of a double value. The hyperbolic "
             "cosine of x is defined to be (e^x + e^-x)/2 where e is Euler's "
             "number."},
            {"sinh",
             "Returns the hyperbolic sine of a double value. The hyperbolic "
             "sine of x is defined to be (e^x - e^-x)/2 where e is Euler's "
             "number."},
            {"tanh",
             "Returns the hyperbolic tangent of a double value. The hyperbolic "
             "tangent of x is defined to be (e^x - e^-x)/(e^x + e^-x), in "
             "other words, sinh(x)/cosh(x). Note that the absolute value of "
             "the exact tanh is always less than 1."},
            {"exp",
             "Returns Euler's number e raised to the power of a double value."},
            {"log",
             "Returns the natural logarithm (base e) of a double value."},
            {"log10", "Returns the base 10 logarithm of a double value."},
            {"expm1",
             "Returns e^x - 1. Note that for values of x near 0, the exact sum "
             "of expm1(x) + 1 is much closer to the true result of e^x than "
             "exp(x)."},
            {"log1p",
             "Returns the natural logarithm of the sum of the argument and 1. "
             "Note that for small values x, the result of log1p(x) is much "
             "closer to the true result of ln(1 + x) than the floating-point "
             "evaluation of log(1.0+x)."},
            {"sqrt",
             "Returns the correctly rounded positive square root of a double "
             "value."},
            {"ceil",
             "Returns the smallest (closest to negative infinity) double value "
             "that is greater than or equal to the argument and is equal to a "
             "mathematical integer."},
            {"floor",
             "Returns the largest (closest to positive infinity) double value "
             "that is less than or equal to the argument and is equal to a "
             "mathematical integer."},
            {"abs",
             "Returns the absolute value of a double value. If the argument is "
             "not negative, the argument is returned. If the argument is "
             "negative, the negation of the argument is returned."},
            {"cosf", "Returns the trigonometric cosine of an angle."},
            {"sinf", "Returns the trigonometric sine of an angle."},
            {"tanf", "Returns the trigonometric tangent of an angle."},
            {"acosf",
             "Returns the arc cosine of a value; the returned angle is in the "
             "range 0.0 through pi."},
            {"asinf",
             "Returns the arc sine of a value; the returned angle is in the "
             "range -pi/2 through pi/2."},
            {"atanf",
             "Returns the arc tangent of a value; the returned angle is in the "
             "range -pi/2 through pi/2."},
            {"coshf",
             "Returns the hyperbolic cosine of a float value. The hyperbolic "
             "cosine of x is defined to be (e^x + e^-x)/2 where e is Euler's "
             "number."},
            {"sinhf",
             "Returns the hyperbolic sine of a float value. The hyperbolic "
             "sine of x is defined to be (e^x - e^-x)/2 where e is Euler's "
             "number."},
            {"tanhf",
             "Returns the hyperbolic tangent of a float value. The hyperbolic "
             "tangent of x is defined to be (e^x - e^-x)/(e^x + e^-x), in "
             "other words, sinh(x)/cosh(x). Note that the absolute value of "
             "the exact tanh is always less than 1."},
            {"expf",
             "Returns Euler's number e raised to the power of a float value."},
            {"logf",
             "Returns the natural logarithm (base e) of a float value."},
            {"log10f", "Returns the base 10 logarithm of a float value."},
            {"expm1f",
             "Returns e^x - 1. Note that for values of x near 0, the exact sum "
             "of expm1(x) + 1 is much closer to the true result of e^x than "
             "exp(x)."},
            {"log1pf",
             "Returns the natural logarithm of the sum of the argument and 1. "
             "Note that for small values x, the result of log1p(x) is much "
             "closer to the true result of ln(1 + x) than the floating-point "
             "evaluation of log(1.0+x)."},
            {"sqrtf",
             "Returns the correctly rounded positive square root of a float "
             "value."},
            {"ceilf",
             "Returns the smallest (closest to negative infinity) float value "
             "that is greater than or equal to the argument and is equal to a "
             "mathematical integer."},
            {"floorf",
             "Returns the largest (closest to positive infinity) float value "
             "that is less than or equal to the argument and is equal to a "
             "mathematical integer."},
            {"absf",
             "Returns the absolute value of a float value. If the argument is "
             "not negative, the argument is returned. If the argument is "
             "negative, the negation of the argument is returned."},
    };
    for (const auto& [name, description] : descriptions)
      if (name == builtin) return std::string(description);
  }

  // The text before the first block tag.
  const auto parsed = parseComment(node);
  if (!parsed) return std::nullopt;
  std::string rendered;
  for (const auto& element : parsed->elements) {
    if (element.kind == cursiv::DocCommentNode::Kind::Tag && !element.inlineTag)
      break;
    if (!rendered.empty()) rendered += '\n';
    rendered += element.toString();
  }
  const auto text = trim(rendered);
  return text.empty() ? std::nullopt : std::optional(std::string(text));
}

}  // namespace xsmp
