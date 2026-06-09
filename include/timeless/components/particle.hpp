#pragma once

#include <glm/glm.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <random>
#include <algorithm>
#include <memory>
#include <vector>
#include "timeless/components/quad.hpp"
#include "timeless/components/transform.hpp"
#include "timeless/components/shader.hpp"

struct Particle {
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec4 color;
    float lifetime;
    float age;
    float size;
    bool active;
};

class ParticleEmitter {
public:
    ParticleEmitter(std::shared_ptr<Transform> transform);
    glm::vec3 position;
    glm::quat rotation;

    std::vector<Particle> particles;
    float emissionRate; // particles per second

    float speed_min    = 1000.0f;
    float speed_max    = 2500.0f;
    float lifetime_min = 2.8f;
    float lifetime_max = 4.5f;
    float size_min     = 6.0f;
    float size_max     = 12.0f;

    // Emit particles in a cone around `direction` (default straight down) with
    // `spread` jitter and a constant `color` (alpha fades over life), and apply
    // `gravity` each update — lets emitters do things like dripping lotion.
    glm::vec3 gravity   = glm::vec3(0.0f);
    glm::vec3 direction = glm::vec3(0.0f, 0.0f, -1.0f);
    float     spread    = 0.3f;
    glm::vec4 color     = glm::vec4(1.0f);

    void emit(int count);
    void update(float dt);
    void render(std::shared_ptr<Quad> quad, std::shared_ptr<Shader> shader);
};
