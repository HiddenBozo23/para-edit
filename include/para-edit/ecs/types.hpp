#pragma once

#include <bitset>
#include <cstdint>

namespace para {
// ecs types and constexprs
using Entity = std::uint16_t;
constexpr Entity INVALID_ENTITY = UINT16_MAX;
constexpr std::size_t MAX_ENTITIES = 10000;  // includes entity 0 - max valid value is 9999

using ComponentIndex = std::uint8_t;  // max 255 components
constexpr ComponentIndex INVALID_COMPONENT = UINT8_MAX;
constexpr std::size_t MAX_COMPONENTS = 32;
using Signature = std::bitset<MAX_COMPONENTS>;

constexpr std::size_t INVALID_INDEX = SIZE_MAX;

// types
}  // namespace para