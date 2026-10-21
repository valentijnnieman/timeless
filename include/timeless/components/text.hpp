#pragma once
#include "timeless/timer.hpp"
#include "timeless/components/component.hpp"
#include "timeless/components/font.hpp"
#include "timeless/components/shader.hpp"
#include <memory>
#include <vector>

class Text : public Component {
private:
  std::unique_ptr<Timer> type_timer;
  std::function<void()> typing_callback;
  std::queue<float> opacities;

public:
  std::string text;
  std::string printed;
  std::vector<glm::vec4> color_vector;

  glm::vec4 color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
  bool center;
  bool hidden = false;

  float type_speed = 0.0f;
  bool completed = false;
  int print_length = 0;

  Text(
      std::string text, bool center = true,
      glm::vec3 color = glm::vec3(0.0f, 0.0f, 0.0f), float type_speed = 0.0f,
      std::function<void()> typing_callback = []() {})
      : text(text), center(center),
        color(glm::vec4(color.x, color.y, color.z, 1.0f)),
        type_speed(type_speed), typing_callback(typing_callback) {
    set_default_color_vector();
    if (type_speed > 0.0) {
      type_timer = std::unique_ptr<Timer>(new Timer(type_speed));
      printed = "";
    } else {
      printed = text;
    }
  }

  void set_default_color_vector() {
    for (int i = 0; i < text.size(); i++) {
      color_vector.push_back(color);
    }
  }

  void set_opacity_frames(float from, float to, float speed = 1.0) {
    while (!opacities.empty())
      opacities.pop();

    for (double i = 0.0; i <= 1.0; i += speed / 60.0) {
      double x_l = std::lerp(from, to, i);
      opacities.push(x_l);
    }
  }

  void append_opacity_frames(float from, float to, float speed = 1.0) {
    for (double i = 0.0; i <= 1.0; i += speed / 60.0) {
      double x_l = std::lerp(from, to, i);
      opacities.push(x_l);
    }
  }

  void reset_timer() {
    type_timer = std::unique_ptr<Timer>(new Timer(type_speed));
    print_length = 0;
    completed = false;
  }

  void render(std::shared_ptr<Font> font, int x, int y, float height,
              std::shared_ptr<Shader> shader) {
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(font->VAO);
    if (!opacities.empty()) {
      float o = opacities.front();
      color.a = o;
      for (auto &c : color_vector) {
        c.a = o;
      }
      opacities.pop();
    }
    if (type_timer != nullptr) {
      if (print_length < text.length() && type_timer->pollTime()) {
        print_length += 1;

        typing_callback();
      } else if (!completed && print_length >= text.length()) {
        completed = true;
      }
      printed = text.substr(0, print_length);
    }

    if (printed.size() > color_vector.size())
      set_default_color_vector();

    // Every glyph's quad in one buffer, then one draw per run of glyphs that
    // share a colour (usually the whole string): per-glyph draws cost a lot
    // of CPU in the driver, on iOS especially.
    std::vector<float> vertices;
    std::vector<glm::vec4> run_colors;
    std::vector<int> run_starts;
    vertices.reserve(printed.size() * 24);
    for (size_t i = 0; i < printed.size(); i++) {
      auto it = font->glyphs.find(printed[i]);
      if (it == font->glyphs.end()) {
        std::cout << "Character " << printed[i] << " not found in font glyphs!"
                  << std::endl;
        continue;
      }
      const Glyph &glyph = it->second;
      float xpos = x + glyph.bearing.x;
      float ypos = y - glyph.bearing.y + height;
      // advance is in 1/64 pixels
      x += (glyph.advance >> 6);
      // Glyphs without pixels (whitespace, control characters) only advance.
      if (glyph.size.x == 0 || glyph.size.y == 0)
        continue;

      float w = glyph.size.x;
      float h = glyph.size.y;
      float u0 = glyph.uv.x, v0 = glyph.uv.y, u1 = glyph.uv.z, v1 = glyph.uv.w;
      float quad[24] = {xpos,     ypos + h, u0, v1, xpos,     ypos,     u0, v0,
                        xpos + w, ypos,     u1, v0, xpos,     ypos + h, u0, v1,
                        xpos + w, ypos,     u1, v0, xpos + w, ypos + h, u1, v1};
      if (run_colors.empty() || run_colors.back() != color_vector[i]) {
        run_colors.push_back(color_vector[i]);
        run_starts.push_back(int(vertices.size() / 4));
      }
      vertices.insert(vertices.end(), quad, quad + 24);
    }

    if (!vertices.empty()) {
      glBindTexture(GL_TEXTURE_2D, font->atlas);
      glBindBuffer(GL_ARRAY_BUFFER, font->VBO);
      // Fresh storage every time (orphaning), so the driver never waits for
      // the previous string's draw to finish with it.
      glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
                   vertices.data(), GL_STREAM_DRAW);
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      GLint color_loc = glGetUniformLocation(shader->ID, "textColor");
      int total = int(vertices.size() / 4);
      for (size_t r = 0; r < run_colors.size(); r++) {
        int end = r + 1 < run_starts.size() ? run_starts[r + 1] : total;
        const glm::vec4 &col = run_colors[r];
        glUniform4f(color_loc, col.r, col.g, col.b, col.a);
        glDrawArrays(GL_TRIANGLES, run_starts[r], end - run_starts[r]);
      }
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
  }
};
