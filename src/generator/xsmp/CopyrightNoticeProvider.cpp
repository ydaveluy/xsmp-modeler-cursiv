#include <algorithm>
#include <cctype>
#include <ctime>
#include <cursiv/documentation/CommentProvider.hpp>
#include <cursiv/documentation/DocComment.hpp>
#include <iomanip>
#include <sstream>
#include <vector>
#include <xsmp/CopyrightNoticeProvider.hpp>
#include <xsmp/utils/GenerationStamp.hpp>
#include <xsmp/utils/TreeUtils.hpp>
#include <xsmp/utils/Utf8.hpp>

namespace xsmp {

namespace {

bool isSpace(char c) {
  return std::isspace(static_cast<unsigned char>(c)) != 0;
}

std::string_view trim(std::string_view text) {
  while (!text.empty() && isSpace(text.front())) text.remove_prefix(1);
  while (!text.empty() && isSpace(text.back())) text.remove_suffix(1);
  return text;
}

// Removes the `/*` and `*/` delimiters (with the extra `*`s and one blank next
// to them) and the leading `*` of every following line from a block comment.
std::string stripBlockComment(std::string_view comment) {
  std::vector<std::string> lines(1);
  for (std::size_t i = 0; i < comment.size(); ++i) {
    if (comment[i] == '\r') {
      lines.emplace_back();
      if (i + 1 < comment.size() && comment[i + 1] == '\n') ++i;
    } else if (comment[i] == '\n') {
      lines.emplace_back();
    } else {
      lines.back() += comment[i];
    }
  }

  auto& first = lines.front();
  if (first.starts_with("/*")) {
    std::size_t index = 2;
    while (index < first.size() && first[index] == '*') ++index;
    if (index < first.size() && first[index] == ' ') ++index;
    first.erase(0, index);
  }
  auto& last = lines.back();
  if (last.ends_with("*/")) {
    auto end = last.size() - 2;
    while (end > 0 && last[end - 1] == '*') --end;
    if (end > 0 && last[end - 1] == ' ') --end;
    last.erase(end);
  }
  for (std::size_t i = 1; i < lines.size(); ++i) {
    auto& line = lines[i];
    auto index = line.find_first_not_of(' ');
    if (index == std::string::npos) index = line.size();
    if (index < line.size() && line[index] == '*') {
      ++index;
      if (index < line.size() && line[index] == ' ') ++index;
    }
    line.erase(0, index);
  }

  std::string result;
  for (std::size_t i = 0; i < lines.size(); ++i) {
    if (i > 0) result += '\n';
    result += lines[i];
  }
  return result;
}

// Whether a block comment comes first in `text`, line comments aside.
bool startsWithBlockComment(std::string_view text) {
  while (true) {
    text = trim(text);
    if (text.starts_with("/*")) return true;
    if (!text.starts_with("//")) return false;
    text.remove_prefix(std::min(text.find_first_of("\r\n"), text.size()));
  }
}

// Replaces ${year}, ${user}, ${date} and ${time}. The date is formatted as
// `2026-01-31` and the time as `13:45:09`, both in local time, or in UTC for a
// reproducible build.
std::string processVariables(std::string text) {
  const auto pinned = reproducibleTime();
  const auto calendar =
      pinned ? utcCalendar(*pinned) : localCalendar(std::time(nullptr));
  const auto format = [&](const char* pattern) {
    std::ostringstream out;
    out << std::put_time(&calendar, pattern);
    return out.str();
  };

  const std::pair<std::string_view, std::string> values[] = {
      {"${year}", format("%Y")},
      {"${user}", generationUser()},
      {"${date}", format("%Y-%m-%d")},
      {"${time}", format("%H:%M:%S")}};

  for (const auto& [key, value] : values)
    for (std::size_t at = 0; (at = text.find(key, at)) != std::string::npos;
         at += value.size())
      text.replace(at, key.size(), value);
  return text;
}

}  // namespace

std::optional<std::string> getCopyrightNotice(
    const cursiv::CommentProvider& commentProvider,
    const cursiv::AstNode& catalogueNode, std::string_view linePrefix) {
  std::string notice;
  if (const auto* document = catalogueNode.getDocument()) {
    // The banner is the first comment of the file, wherever the node's own
    // comment is.
    auto text = document->textDocument().getText();
    // Skip a UTF-8 byte order mark.
    if (text.starts_with("\xEF\xBB\xBF")) text.remove_prefix(3);
    const auto skipSpaces = [&] {
      while (!text.empty() && isSpace(text.front())) text.remove_prefix(1);
    };
    skipSpaces();
    if (text.starts_with("/*")) {
      const auto end = text.find("*/", 2);
      if (end == std::string_view::npos) return std::nullopt;
      // A doc comment documents what follows it, unless another block comment
      // is closer to that.
      if (text.starts_with("/**")
          && !startsWithBlockComment(text.substr(end + 2)))
        return std::nullopt;
      notice = std::string(trim(stripBlockComment(text.substr(0, end + 2))));
    } else if (text.starts_with("//")) {
      // Consecutive `//` lines make one notice, even with blank lines between
      // them.
      while (text.starts_with("//")) {
        text.remove_prefix(2);
        if (text.starts_with(' ')) text.remove_prefix(1);
        const auto end = text.find_first_of("\r\n");
        if (!notice.empty()) notice += '\n';
        notice += text.substr(0, end);
        if (end == std::string_view::npos) break;
        text.remove_prefix(end);
        skipSpaces();
      }
      notice = std::string(trim(notice));
    }
  } else {
    // Without a document, the node's own comment is all there is.
    const auto comment = commentProvider.getComment(catalogueNode);
    if (!comment || cursiv::is_doc_comment(*comment)) return std::nullopt;
    notice = std::string(trim(stripBlockComment(*comment)));
  }

  if (notice.empty()) return std::nullopt;
  notice = utf8Text(processVariables(std::move(notice)));
  if (linePrefix.empty()) return notice;

  std::string result(linePrefix);
  for (const auto c : notice) {
    result += c;
    if (c == '\n') result += linePrefix;
  }
  return result;
}

}  // namespace xsmp
