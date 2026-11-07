// The XSMP benchmark: it times parsing, document builds and editor features on
// the given files. Run it with `--help` to list the options.

#include <cursiv/LanguageBench.hpp>
#include <xsmp/LspModule.hpp>
#include <xsmp/SharedLspServices.hpp>

int main(int argc, char** argv) {
  return cursiv::bench::run_language<xsmp::XsmpSharedServices,
                                     xsmp::registerLanguageServer>(argc, argv);
}
