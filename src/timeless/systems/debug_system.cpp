#include "timeless/systems/debug_system.hpp"

DebugSystem::DebugSystem() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    shader = std::make_shared<Shader>("Assets/shaders/webgl_debug.vs",
                                      "Assets/shaders/webgl_debug.fs");
}

DebugSystem::~DebugSystem() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

void DebugSystem::register_camera(Entity e) { camera_entity = e; }

void DebugSystem::push_vertex(glm::vec3 p, glm::vec4 c) {
    vertices.insert(vertices.end(), {p.x, p.y, p.z, c.r, c.g, c.b, c.a});
}

void DebugSystem::draw_line(glm::vec3 a, glm::vec3 b, glm::vec4 color) {
    push_vertex(a, color);
    push_vertex(b, color);
}

void DebugSystem::draw_rect(glm::vec3 center, float hw, float hh,
                             glm::vec4 color) {
    float x0 = center.x - hw, x1 = center.x + hw;
    float y0 = center.y - hh, y1 = center.y + hh;
    float z  = center.z;
    draw_line({x0, y0, z}, {x1, y0, z}, color);
    draw_line({x1, y0, z}, {x1, y1, z}, color);
    draw_line({x1, y1, z}, {x0, y1, z}, color);
    draw_line({x0, y1, z}, {x0, y0, z}, color);
}

void DebugSystem::render(ComponentManager &cm, int vp_x, int vp_y,
                          float zoom) {
    if (vertices.empty())
        return;

    auto cam = cm.get_component<Camera>(camera_entity);
    if (!cam) {
        vertices.clear();
        return;
    }

    shader->use();

    GLint pos_loc   = glGetAttribLocation(shader->ID, "aPos");
    GLint color_loc = glGetAttribLocation(shader->ID, "aColor");

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
                 vertices.data(), GL_DYNAMIC_DRAW);

    if (pos_loc >= 0) {
        glVertexAttribPointer(pos_loc, 3, GL_FLOAT, GL_FALSE,
                              7 * sizeof(float), nullptr);
        glEnableVertexAttribArray(pos_loc);
    }
    if (color_loc >= 0) {
        glVertexAttribPointer(color_loc, 4, GL_FLOAT, GL_FALSE,
                              7 * sizeof(float), (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(color_loc);
    }

    glm::mat4 proj = cam->get_projection_matrix(vp_x, vp_y, zoom);
    glm::mat4 view = cam->get_view_matrix();
    glUniformMatrix4fv(glGetUniformLocation(shader->ID, "projection"), 1,
                       GL_FALSE, glm::value_ptr(proj));
    glUniformMatrix4fv(glGetUniformLocation(shader->ID, "view"), 1,
                       GL_FALSE, glm::value_ptr(view));

    glDrawArrays(GL_LINES, 0, (GLsizei)(vertices.size() / 7));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    vertices.clear();
}
