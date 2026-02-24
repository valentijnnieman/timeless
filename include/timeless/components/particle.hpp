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

    void emit(int count);
    void update(float dt);
    void render(std::shared_ptr<Quad> quad, std::shared_ptr<Shader> shader);
};
