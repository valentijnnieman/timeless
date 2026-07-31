#pragma once
#include <string>
#include <unordered_map>

#include <glm/glm.hpp>

#include "timeless/components/component.hpp"

// Per-entity shader uniform overrides, uploaded by RenderingSystem just before
// the entity's model draws.
//
// Entities often SHARE a Model and Shader (e.g. every beachbod uses one cached
// Model + the same program), so per-entity shader params can't live on those
// shared objects — they'd clobber. This component keeps that data on the entity
// itself instead. The engine stays generic: it uploads whatever name->value
// pairs it's handed and never needs to know what they mean. Being a component,
// it's destroyed with the entity, so a recycled entity id can't inherit stale
// values.
//
// Note on uniform bleed: uniform values are program-global GL state, so an
// entity that DOESN'T set a uniform inherits whatever the previous draw left.
// Callers that share a program should therefore seed defaults (e.g. amounts at
// 0, alpha at 1) so every draw fully specifies its own state.
class ShaderUniforms : public Component {
public:
  std::unordered_map<std::string, float> floats;
  std::unordered_map<std::string, glm::vec3> vec3s;

  // When true, RenderingSystem drops depth writes for this entity's draw so a
  // translucent model doesn't occlude geometry behind it (depth test stays on,
  // so nearer opaque geometry still hides it).
  bool translucent = false;
};
