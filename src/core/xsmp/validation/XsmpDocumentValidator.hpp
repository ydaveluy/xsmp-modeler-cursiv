#pragma once

#include <cursiv/validation/DefaultDocumentValidator.hpp>
#include <cursiv/validation/ValidationAcceptor.hpp>
#include <xsmp/ast.hpp>

namespace xsmp {

// A byte that is not UTF-8 says nothing of what it stands for. In a literal the
// parser refuses it, since the value is unknown: `\xE9` writes a byte. Anywhere
// else it can only be in a comment, which no value depends on, so a warning
// once per file is enough, and each output writes it `\xHH`.
void checkEncoding(const Document& root, cursiv::ValidationAcceptor& accept);

// An escape that C++ does not know stands for its character alone, as in GCC,
// but is most likely a mistake: `"C:\dir"` holds `C:dir`. Only the text of a
// literal shows it, not its value.
void checkEscapes(const Document& root, cursiv::ValidationAcceptor& accept);

// The tool `smp` writes a document to `smdl-gen` under its file name, wherever
// it stands: two documents of a project with one file name would write the same
// file.
void checkOutputFile(const Document& root, cursiv::ValidationAcceptor& accept);

// The framework's linking diagnostics, with two XSMP additions: a name that
// exists but is not of the kind its site accepts says so, and a name defined in
// another project says which dependency to add.
class XsmpDocumentValidator : public cursiv::DefaultDocumentValidator {
 public:
  using DefaultDocumentValidator::DefaultDocumentValidator;

 protected:
  void processLinkingErrors(const cursiv::Document&,
                            std::vector<cursiv::Diagnostic>&,
                            const cursiv::CancellationToken&) const override;
};

}  // namespace xsmp
