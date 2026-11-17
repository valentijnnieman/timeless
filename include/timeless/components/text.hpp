#pragma once
#include "timeless/timer.hpp"
#include "timeless/components/component.hpp"
#include "timeless/components/font.hpp"
#include "timeless/components/shader.hpp"
#include <functional>
#include <glm/glm.hpp>
#include <memory>
#include <queue>
#include <string>
#include <vector>

class Text : public Component {
private:
  // When typing started (TE::now_seconds()); characters appear on this clock,
  // one per type_speed seconds, however many frames that spans.
  double type_start = 0.0;
  std::function<void()> typing_callback;
  std::queue<float> opacities;

public:
  std::string text;
  std::string printed;
  std::vector<glm::vec4> color_vector;

  glm::vec4 color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
  bool center;
  bool hidden = false;

  // Seconds per typed character; 0 shows the text at once. The typing
  // callback runs once per frame that adds characters (not per character).
  float type_speed = 0.0f;
  bool completed = false;
  int print_length = 0;

  Text(
      std::string text, bool center = true,
      glm::vec3 color = glm::vec3(0.0f, 0.0f, 0.0f), float type_speed = 0.0f,
      std::function<void()> typing_callback = []() {});

  void set_default_color_vector();

  void set_opacity_frames(float from, float to, float speed = 1.0);

  void append_opacity_frames(float from, float to, float speed = 1.0);

  void reset_timer();

  void render(std::shared_ptr<Font> font, int x, int y, float height,
              std::shared_ptr<Shader> shader);
};
