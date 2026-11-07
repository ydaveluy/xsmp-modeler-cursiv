#pragma once

#include <filesystem>
#include <xsmp/XsmpcatCoreServices.hpp>
#include <xsmp/utils/XsmpDocumentation.hpp>

namespace xsmp {

struct Assembly;

// Writes a model document as AsciiDoc, with Mermaid diagrams for a catalogue.
class ADocGenerator {
 public:
  /// The AsciiDoc of a document of any kind, or an empty string for a node
  /// that is not a document.
  [[nodiscard]] std::string render(const cursiv::AstNode& document) const;

  /// Writes the AsciiDoc of `document` to `adoc-gen/<name>-gen.adoc` under
  /// `destination`.
  void generate(const cursiv::AstNode& document,
                const std::filesystem::path& destination) const;

 private:
};

}  // namespace xsmp
