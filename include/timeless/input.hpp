#pragma once

// ===========================================================================
// timeless/input.hpp — backend-agnostic input + timing abstraction.
//
// The engine no longer leaks the windowing backend (GLFW/SDL) into its public
// API. Game code and engine systems refer to keys/buttons via these
// engine-owned enums and query state through WindowManager::is_key_pressed().
// Translation to the underlying backend's scancodes lives in the platform .cpp.
// ===========================================================================

namespace te {

// Engine key identifiers. Values are arbitrary (the backend mapping is internal);
// the contiguous A..Z and Num0..Num9 runs are guaranteed so callers can index
// with arithmetic, e.g. key_digit(n) or Key(int(Key::A) + i).
enum class Key {
  Unknown = 0,

  // Letters
  A, B, C, D, E, F, G, H, I, J, K, L, M,
  N, O, P, Q, R, S, T, U, V, W, X, Y, Z,

  // Top-row digits
  Num0, Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9,

  // Editing / whitespace
  Space, Enter, Escape, Tab, Backspace, Delete,

  // Arrows
  Left, Right, Up, Down,

  // Modifiers
  LeftControl, RightControl, LeftShift, RightShift, LeftAlt, RightAlt,
};

enum class MouseButton {
  Left,
  Right,
  Middle,
};

// Map a 0-based digit index to its Num key (key_digit(0) == Key::Num0).
inline Key key_digit(int n) {
  return static_cast<Key>(static_cast<int>(Key::Num0) + n);
}

// Monotonic time in seconds since engine/platform init. Replaces glfwGetTime().
// Defined in src/timeless/managers/window_manager.cpp via the SDL backend.
double now_seconds();

} // namespace te
