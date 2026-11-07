#include <algorithm>
#include <cstddef>
#include <iterator>
#include <xsmp/XsmpcatCoreServices.hpp>
#include <xsmp/ast.hpp>
#include <xsmp/references/XsmpTypeProvider.hpp>
#include <xsmp/utils/XsmpUtils.hpp>

namespace xsmp {

XsmpTypeProvider::XsmpTypeProvider(const XsmpcatCoreServices& services)
    : services(services), _typeCache(services.shared) {}

const Type* XsmpTypeProvider::getType(const cursiv::AstNode& node) const {
  // Completion's temporary expressions have no stable document identity.
  if (node.cstNodeId == cursiv::kNoNode) return doGetType(node);
  return _typeCache.get(&node, [this, &node] { return doGetType(node); });
}

const Type* XsmpTypeProvider::doGetType(const cursiv::AstNode& node) const {
  const auto* container = node.parent;
  if (container == nullptr) {
    return nullptr;
  }
  if (const auto* field = cursiv::ast_cast<const Field>(container)) {
    return field->type.get();
  }
  if (const auto* constant = cursiv::ast_cast<const Constant>(container)) {
    return constant->type.get();
  }
  if (const auto* property = cursiv::ast_cast<const Property>(container)) {
    return property->type.get();
  }
  if (const auto* parameter = cursiv::ast_cast<const Parameter>(container)) {
    return parameter->type.get();
  }
  if (const auto* returnParameter =
          cursiv::ast_cast<const ReturnParameter>(container)) {
    return returnParameter->type.get();
  }
  // Checked BEFORE the generic Expression fallback below, since
  // `CollectionLiteral : Expression`.
  if (const auto* collection =
          cursiv::ast_cast<const CollectionLiteral>(container)) {
    const auto* outer = getType(*collection);
    if (const auto* arrayType = cursiv::ast_cast<const Array>(outer)) {
      return arrayType->itemType.get();
    }
    if (const auto* structure = cursiv::ast_cast<const Structure>(outer)) {
      // By position, also for a designated initializer: validation requires
      // them in field order.
      const auto fields = getAllFields(*structure);
      const auto& elements = collection->elements;
      const auto it = std::ranges::find(elements, &node);
      if (it == elements.end()) {
        return nullptr;
      }
      const auto index =
          static_cast<std::size_t>(std::distance(elements.begin(), it));
      return index < fields.size() ? fields[index]->type.get() : nullptr;
    }
    return nullptr;
  }
  // A generic Expression container (e.g. ParenthesizedExpression,
  // DesignatedInitializer) passes through to ITS OWN container's type.
  if (cursiv::ast_cast<const Expression>(container) != nullptr) {
    return getType(*container);
  }
  if (const auto* attribute = cursiv::ast_cast<const Attribute>(container)) {
    const auto* attributeType =
        cursiv::ast_cast<const AttributeType>(attribute->type.get());
    return attributeType != nullptr ? attributeType->type.get() : nullptr;
  }
  if (const auto* attributeType =
          cursiv::ast_cast<const AttributeType>(container)) {
    return attributeType->type.get();
  }
  return nullptr;
}

}  // namespace xsmp
