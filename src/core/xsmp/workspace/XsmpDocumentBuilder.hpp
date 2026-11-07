#pragma once

#include <cursiv/workspace/DefaultDocumentBuilder.hpp>
#include <span>
#include <vector>

namespace xsmp {

struct XsmpSharedCoreServices;

/// Validates each document with the categories its project selects: `fast` and
/// `built-in`, and the category of each of its profiles and tools. So the checks of
/// a contribution apply to the projects that select it, and no other.
class XsmpDocumentBuilder final : public cursiv::DefaultDocumentBuilder {
 public:
  explicit XsmpDocumentBuilder(XsmpSharedCoreServices& shared);

 protected:
  [[nodiscard]] std::vector<const cursiv::ValidationCategory*>
  findMissingValidationCategories(
      const cursiv::Document& document, const cursiv::CoreServices& language,
      std::span<const cursiv::ValidationCategory* const> requested)
      const override;

 private:
  XsmpSharedCoreServices& _shared;
};

}  // namespace xsmp
