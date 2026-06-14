#include "timeless/managers/window_manager.hpp"
#include <SDL.h>

// Opaque platform handle (declared in the header). Holds the SDL window and
// GL context; SDL types appear only inside this translation unit.
struct PlatformWindow {
  SDL_Window *window = nullptr;
  SDL_GLContext gl_context = nullptr;
  SDL_Cursor *cursor = nullptr;
};

namespace {
// Map engine keys to SDL scancodes (used by is_key_pressed / SDL_GetKeyboardState).
SDL_Scancode to_scancode(te::Key key) {
  using K = te::Key;
  switch (key) {
    case K::A: return SDL_SCANCODE_A; case K::B: return SDL_SCANCODE_B;
    case K::C: return SDL_SCANCODE_C; case K::D: return SDL_SCANCODE_D;
    case K::E: return SDL_SCANCODE_E; case K::F: return SDL_SCANCODE_F;
    case K::G: return SDL_SCANCODE_G; case K::H: return SDL_SCANCODE_H;
    case K::I: return SDL_SCANCODE_I; case K::J: return SDL_SCANCODE_J;
    case K::K: return SDL_SCANCODE_K; case K::L: return SDL_SCANCODE_L;
    case K::M: return SDL_SCANCODE_M; case K::N: return SDL_SCANCODE_N;
    case K::O: return SDL_SCANCODE_O; case K::P: return SDL_SCANCODE_P;
    case K::Q: return SDL_SCANCODE_Q; case K::R: return SDL_SCANCODE_R;
    case K::S: return SDL_SCANCODE_S; case K::T: return SDL_SCANCODE_T;
    case K::U: return SDL_SCANCODE_U; case K::V: return SDL_SCANCODE_V;
    case K::W: return SDL_SCANCODE_W; case K::X: return SDL_SCANCODE_X;
    case K::Y: return SDL_SCANCODE_Y; case K::Z: return SDL_SCANCODE_Z;
    case K::Num0: return SDL_SCANCODE_0; case K::Num1: return SDL_SCANCODE_1;
    case K::Num2: return SDL_SCANCODE_2; case K::Num3: return SDL_SCANCODE_3;
    case K::Num4: return SDL_SCANCODE_4; case K::Num5: return SDL_SCANCODE_5;
    case K::Num6: return SDL_SCANCODE_6; case K::Num7: return SDL_SCANCODE_7;
    case K::Num8: return SDL_SCANCODE_8; case K::Num9: return SDL_SCANCODE_9;
    case K::Space: return SDL_SCANCODE_SPACE;
    case K::Enter: return SDL_SCANCODE_RETURN;
    case K::Escape: return SDL_SCANCODE_ESCAPE;
    case K::Tab: return SDL_SCANCODE_TAB;
    case K::Backspace: return SDL_SCANCODE_BACKSPACE;
    case K::Delete: return SDL_SCANCODE_DELETE;
    case K::Left: return SDL_SCANCODE_LEFT;
    case K::Right: return SDL_SCANCODE_RIGHT;
    case K::Up: return SDL_SCANCODE_UP;
    case K::Down: return SDL_SCANCODE_DOWN;
    case K::LeftControl: return SDL_SCANCODE_LCTRL;
    case K::RightControl: return SDL_SCANCODE_RCTRL;
    case K::LeftShift: return SDL_SCANCODE_LSHIFT;
    case K::RightShift: return SDL_SCANCODE_RSHIFT;
    case K::LeftAlt: return SDL_SCANCODE_LALT;
    case K::RightAlt: return SDL_SCANCODE_RALT;
    default: return SDL_SCANCODE_UNKNOWN;
  }
}
} // namespace

double te::now_seconds() {
  static const Uint64 start = SDL_GetPerformanceCounter();
  static const double freq = static_cast<double>(SDL_GetPerformanceFrequency());
  return static_cast<double>(SDL_GetPerformanceCounter() - start) / freq;
}

WindowManager::WindowManager(std::shared_ptr<ComponentManager> cm,
                             std::shared_ptr<MouseInputSystem> mis)
    : cm(cm), mis(mis) {
  platform = std::make_unique<PlatformWindow>();

  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
    std::cout << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
  }

#ifdef __EMSCRIPTEN__
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#else
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
#endif
  // Request an alpha channel in the framebuffer so the canvas can be composited
  // transparently over the HTML page (emscripten maps this to WebGL alpha:true).
  SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);

  int width = TESettings::WINDOW_X;
  int height = TESettings::WINDOW_Y;

  if (TESettings::NATIVE_RESOLUTION) {
    SDL_DisplayMode mode;
    if (SDL_GetCurrentDisplayMode(0, &mode) == 0) {
      width = mode.w;
      height = mode.h;
      TESettings::WINDOW_X = width;
      TESettings::WINDOW_Y = height;
    }
  }

  Uint32 flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
  if (TESettings::FULLSCREEN)
    flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;

  platform->window =
      SDL_CreateWindow("Timeless", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                       width, height, flags);

  if (platform->window == nullptr) {
    std::cout << "Failed to create SDL window: " << SDL_GetError() << std::endl;
    SDL_Quit();
  }

  platform->gl_context = SDL_GL_CreateContext(platform->window);
  if (platform->gl_context == nullptr) {
    std::cout << "Failed to create GL context: " << SDL_GetError() << std::endl;
  }
  SDL_GL_MakeCurrent(platform->window, platform->gl_context);

  SDL_GL_SetSwapInterval(1); // vsync

  platform->cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
  SDL_SetCursor(platform->cursor);

#ifdef __EMSCRIPTEN__
#else
  if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
    std::cout << "Failed to initialize GLAD" << std::endl;
  }
#endif
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  std::cout << "OpenGL Initialized!" << std::endl;

  glGenVertexArrays(1, &ScreenVAO);
  glGenBuffers(1, &ScreenVBO);
  glBindVertexArray(ScreenVAO);
  glBindBuffer(GL_ARRAY_BUFFER, ScreenVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quad_verts), &quad_verts, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

WindowManager::~WindowManager() = default;

void WindowManager::cleanup() {
  std::cout << "Destroying WindowManager..." << std::endl;
  glDeleteVertexArrays(1, &ScreenVAO);
  glDeleteBuffers(1, &ScreenVBO);
  for (auto fbo : framebuffers) glDeleteFramebuffers(1, &fbo);
  for (auto tex : textures) glDeleteTextures(1, &tex);
  for (auto rbo : rbos) glDeleteRenderbuffers(1, &rbo);

  if (platform) {
    if (platform->cursor) SDL_FreeCursor(platform->cursor);
    if (platform->gl_context) SDL_GL_DeleteContext(platform->gl_context);
    if (platform->window) SDL_DestroyWindow(platform->window);
    platform->cursor = nullptr;
    platform->gl_context = nullptr;
    platform->window = nullptr;
  }
  SDL_Quit();
}

void WindowManager::add_framebuffer(std::shared_ptr<Shader> shader, int width, int height, bool add_screen_shader) {
  unsigned int fbo, tex, rbo;
  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);

  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glBindTexture(GL_TEXTURE_2D, 0);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

  glGenRenderbuffers(1, &rbo);
  glBindRenderbuffer(GL_RENDERBUFFER, rbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    std::cout << "Could not create Framebuffer!" << std::endl;
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  framebuffers.push_back(fbo);
  textures.push_back(tex);
  rbos.push_back(rbo);
  if (add_screen_shader)
    screen_shaders.push_back(shader);
}

void WindowManager::select_framebuffer(size_t idx, bool clear) {
  if (idx < framebuffers.size()) {
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[idx]);
    glViewport(0, 0, TESettings::VIEWPORT_X, TESettings::VIEWPORT_Y);
    if (clear) {
      glClearColor(TESettings::SCREEN_COLOR.r, TESettings::SCREEN_COLOR.g,
                   TESettings::SCREEN_COLOR.b, TESettings::SCREEN_COLOR.a);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }
  }
}

void WindowManager::resize_framebuffers(int new_width, int new_height) {
  for (size_t i = 0; i < framebuffers.size(); ++i) {
    glDeleteFramebuffers(1, &framebuffers[i]);
    glDeleteTextures(1, &textures[i]);
    glDeleteRenderbuffers(1, &rbos[i]);
  }
  framebuffers.clear();
  textures.clear();
  rbos.clear();

  for (size_t i = 0; i < screen_shaders.size(); ++i) {
    add_framebuffer(screen_shaders[i], new_width, new_height, false);
  }
}

void WindowManager::render_framebuffer_as_quad(size_t idx, bool clear, int tick, bool to_screen) {
  if (idx >= screen_shaders.size() || idx >= textures.size()) return;
  glEnable(GL_BLEND);
  // Premultiplied "over": the offscreen FBO holds premultiplied colour, so this
  // lets the page background show through transparent regions of the composite.
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
  int vp_w = to_screen ? TESettings::WINDOW_X : TESettings::VIEWPORT_X;
  int vp_h = to_screen ? TESettings::WINDOW_Y : TESettings::VIEWPORT_Y;
  glViewport(0, 0, vp_w, vp_h);
  if (clear) {
    glClearColor(TESettings::SCREEN_COLOR.r, TESettings::SCREEN_COLOR.g,
                 TESettings::SCREEN_COLOR.b, TESettings::SCREEN_COLOR.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  }
  screen_shaders[idx]->use();

  glBindVertexArray(ScreenVAO);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, textures[idx]);

  glUniform1f(glGetUniformLocation(screen_shaders[idx]->ID, "width"), TESettings::VIEWPORT_X);
  glUniform1f(glGetUniformLocation(screen_shaders[idx]->ID, "height"), TESettings::VIEWPORT_Y);
  // Normalise raw mouse position by actual window size so shader effects
  // (e.g. search highlight) track the cursor correctly regardless of resolution.
  glUniform2fv(
      glGetUniformLocation(screen_shaders[idx]->ID, "mousePosition"), 1,
      glm::value_ptr(glm::vec2(shader_mouse_position.x / TESettings::WINDOW_X,
                               shader_mouse_position.y / TESettings::WINDOW_Y)));

  set_shader_time(screen_shaders[idx]);

  glDrawArrays(GL_TRIANGLES, 0, 6);
  glBindVertexArray(0);
  glBindTexture(GL_TEXTURE_2D, 0);
}

void WindowManager::set_shader_time(std::shared_ptr<Shader> shader) {
  if (!shader)
    return;

  GLint timeLoc = glGetUniformLocation(shader->ID, "time");
  if (timeLoc != -1)
    glUniform1f(timeLoc, static_cast<float>(te::now_seconds()));

  if (screen_shaders.size() > 0) {
    GLint resLoc = glGetUniformLocation(screen_shaders[0]->ID, "resolution");
    if (resLoc != -1)
      glUniform2fv(resLoc, 1,
                   glm::value_ptr(glm::vec2(TESettings::SCREEN_X, TESettings::SCREEN_Y)));
  }
}

void WindowManager::set_shader_mouse_position(glm::vec2 mouse_pos) {
  shader_mouse_position = mouse_pos;
}

void WindowManager::handle_resize(int width, int height) {
  TESettings::rescale_window(width, height);
  glm::vec2 new_size(width, height);
  if (es != nullptr) {
    es->create_event<glm::vec2>(*cm, "ResizeWindow", &new_size);
  }
}

void WindowManager::mouse_move_handler(MouseMoveEvent *event) {
  mouse_position = event->screen_position;
  mis->mouse_move_handler(*cm, event);
}

void WindowManager::mouse_click_handler(MouseEvent *event) {
  mis->mouse_click_handler(*cm, event);
}

void WindowManager::mouse_release_handler(MouseEvent *event) {
  mis->mouse_release_handler(*cm, event);
}

void WindowManager::mouse_scroll_handler(MouseEvent *event) {
  mis->mouse_scroll_handler(*cm, event);
}

// --- Coordinate mapping helpers (preserve the original GLFW-callback math) ---

// Cursor-move mapping: window pixels -> design-resolution world coords,
// scaling by VIEWPORT/WINDOW then centering on the viewport.
static glm::vec2 map_move_to_world(double xpos, double ypos) {
  double scale_x = double(TESettings::VIEWPORT_X) / TESettings::WINDOW_X;
  double scale_y = double(TESettings::VIEWPORT_Y) / TESettings::WINDOW_Y;
  double fb_x = xpos * scale_x - TESettings::VIEWPORT_X * 0.5;
  double fb_y = ypos * scale_y - TESettings::VIEWPORT_Y * 0.5;
  double world_x = (fb_x / TESettings::VIEWPORT_X) * TESettings::VIEWPORT_X;
  double world_y = (fb_y / TESettings::VIEWPORT_Y) * TESettings::VIEWPORT_Y;
  return glm::vec2(world_x, world_y);
}

// Button mapping: window pixels -> design-resolution world coords, centering
// on the window then scaling to the viewport.
static glm::vec2 map_click_to_world(double xpos, double ypos) {
  double cx = xpos - TESettings::WINDOW_X * 0.5;
  double cy = ypos - TESettings::WINDOW_Y * 0.5;
  double world_x = (cx / TESettings::WINDOW_X) * TESettings::VIEWPORT_X;
  double world_y = (cy / TESettings::WINDOW_Y) * TESettings::VIEWPORT_Y;
  return glm::vec2(world_x, world_y);
}

void WindowManager::poll_events() {
  SDL_Event e;
  while (SDL_PollEvent(&e)) {
    switch (e.type) {
      case SDL_QUIT:
        quit_requested = true;
        break;

      case SDL_WINDOWEVENT:
        if (e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED ||
            e.window.event == SDL_WINDOWEVENT_RESIZED) {
          handle_resize(e.window.data1, e.window.data2);
        } else if (e.window.event == SDL_WINDOWEVENT_CLOSE) {
          quit_requested = true;
        }
        break;

      case SDL_MOUSEMOTION: {
        set_shader_mouse_position(glm::vec2(e.motion.x, e.motion.y));
        glm::vec2 world = map_move_to_world(e.motion.x, e.motion.y);
        mouse_move_handler(new MouseMoveEvent(
            "MouseMove", world, glm::vec2(e.motion.x, e.motion.y)));
        break;
      }

      case SDL_MOUSEWHEEL: {
        float xoff = e.wheel.preciseX;
        float yoff = e.wheel.preciseY;
        mouse_scroll_handler(new MouseEvent(
            "MouseScroll", mouse_position, raw_mouse_position, xoff, yoff));
        break;
      }

      case SDL_MOUSEBUTTONDOWN:
      case SDL_MOUSEBUTTONUP: {
        glm::vec2 raw(e.button.x, e.button.y);
        glm::vec2 world = map_click_to_world(e.button.x, e.button.y);
        bool press = (e.type == SDL_MOUSEBUTTONDOWN);
        const char *name = nullptr;
        if (e.button.button == SDL_BUTTON_LEFT)
          name = press ? "LeftMousePress" : "LeftMouseRelease";
        else if (e.button.button == SDL_BUTTON_RIGHT)
          name = press ? "RightMousePress" : "RightMouseRelease";
        else if (e.button.button == SDL_BUTTON_MIDDLE)
          name = press ? "MiddleMousePress" : "MiddleMouseRelease";
        if (name) {
          MouseEvent *ev = new MouseEvent(name, world, raw);
          if (press)
            mouse_click_handler(ev);
          else
            mouse_release_handler(ev);
        }
        break;
      }

      default:
        break;
    }
  }
}

void WindowManager::swap_buffers() {
  SDL_GL_SwapWindow(platform->window);
}

bool WindowManager::should_close() {
  return quit_requested || !running;
}

bool WindowManager::is_key_pressed(te::Key key) {
  const Uint8 *state = SDL_GetKeyboardState(nullptr);
  SDL_Scancode sc = to_scancode(key);
  if (sc == SDL_SCANCODE_UNKNOWN) return false;
  return state[sc] != 0;
}

bool WindowManager::is_mouse_button_pressed(te::MouseButton button) {
  Uint32 state = SDL_GetMouseState(nullptr, nullptr);
  switch (button) {
    case te::MouseButton::Left:   return (state & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;
    case te::MouseButton::Right:  return (state & SDL_BUTTON(SDL_BUTTON_RIGHT)) != 0;
    case te::MouseButton::Middle: return (state & SDL_BUTTON(SDL_BUTTON_MIDDLE)) != 0;
  }
  return false;
}

glm::vec2 WindowManager::get_cursor_position() {
  int x = 0, y = 0;
  SDL_GetMouseState(&x, &y);
  return glm::vec2(x, y);
}

void WindowManager::set_cursor_visible(bool visible) {
  SDL_ShowCursor(visible ? SDL_ENABLE : SDL_DISABLE);
}
