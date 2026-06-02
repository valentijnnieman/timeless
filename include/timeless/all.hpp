#pragma once

// ===========================================================================
// timeless/all.hpp — convenience umbrella that pulls in the whole engine.
//
// This is the "include one thing and get everything" header. It is the facade
// (timeless/core.hpp) plus every public component and system. Prefer including
// timeless/core.hpp and only the modules you actually use; reach for this when
// you want the old all-in-one behaviour.
// ===========================================================================

#include "timeless/core.hpp"

// --- components ---
#include "timeless/components/quad.hpp"
#include "timeless/components/texture.hpp"
#include "timeless/components/shader.hpp"
#include "timeless/components/font.hpp"
#include "timeless/components/particle.hpp"
#include "timeless/components/sprite.hpp"
#include "timeless/components/model.hpp"
#include "timeless/components/mesh.hpp"
#include "timeless/components/text.hpp"
#include "timeless/components/line.hpp"
#include "timeless/components/skeletal_animation.hpp"
#include "timeless/components/node_animation.hpp"
#include "timeless/components/behaviour.hpp"
#include "timeless/components/collider.hpp"
#include "timeless/components/node.hpp"
#include "timeless/components/event_listener.hpp"
#include "timeless/components/movement_controller.hpp"
#include "timeless/components/keyboard_input_listener.hpp"

// --- systems ---
#include "timeless/systems/rendering_system.hpp"
#include "timeless/systems/movement_system.hpp"
#include "timeless/systems/keyboard_input_system.hpp"
#include "timeless/systems/npc_ai_system.hpp"
#include "timeless/systems/inventory_system.hpp"
#include "timeless/systems/event_system.hpp"
#include "timeless/systems/animation_system.hpp"
#include "timeless/systems/camera_system.hpp"
#include "timeless/systems/sound_system.hpp"
#include "timeless/systems/debug_system.hpp"
