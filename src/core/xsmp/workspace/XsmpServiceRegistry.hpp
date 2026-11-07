#pragma once

#include <cursiv/services/DefaultServiceRegistry.hpp>

namespace xsmp {

// The languages by extension, and for the XSMP form of an SMP XML file, by the
// kind of that file: `xsmp-smp:/a/b.smpcat` is a catalogue. A mirror URI of an
// earlier version, which ended with the native extension, has no language: an
// editor that still holds one opens the new one.
class XsmpServiceRegistry final : public cursiv::DefaultServiceRegistry {
 public:
  using DefaultServiceRegistry::DefaultServiceRegistry;

  [[nodiscard]] const cursiv::CoreServices& getServices(
      std::string_view uri, std::string_view languageId = {}) const override;
  [[nodiscard]] const cursiv::CoreServices* findServices(
      std::string_view uri, std::string_view languageId = {}) const override;
};

}  // namespace xsmp
