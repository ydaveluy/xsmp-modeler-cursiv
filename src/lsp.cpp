#include <cursiv/runtime/LanguageServerMain.hpp>
#include <xsmp/Extension.hpp>
#include <xsmp/LspModule.hpp>
#include <xsmp/SharedLspServices.hpp>

int main(int argc, char** argv) {
  xsmp::loadExtensionLibraries();
  return cursiv::runLanguageServerMain<xsmp::XsmpSharedServices>(
      argc, argv, "xsmp-lsp", xsmp::registerLanguageServer);
}
