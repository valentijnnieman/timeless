#include "timeless/components/particle.hpp"
#include "glm/gtc/type_ptr.hpp"

namespace {
std::mt19937 rng(std::random_device{}());
float randf(float a, float b) {
    std::uniform_real_distribution<float> dist(a, b);
    return dist(rng);
}
glm::vec4 randomColor() {
    return glm::vec4(randf(0.5f, 1.0f), randf(0.5f, 1.0f), randf(0.5f, 1.0f), 1.0f);
}
glm::vec3 randomDirection(float minAngle, float maxAngle) {
    float angle = randf(minAngle, maxAngle);
    return glm::vec3(cos(angle), sin(angle), 0.0f);
}
}

ParticleEmitter::ParticleEmitter(std::shared_ptr<Transform> transform) : position(transform->get_position()), rotation(transform->getRotationQuat())  
{
}

void ParticleEmitter::emit(int count) {
    for (int i = 0; i < count; ++i) {
        Particle p;
        p.position = position;
        float speed = randf(1000.0f, 2500.0f);
        float angle = randf(-glm::pi<float>()/4, glm::pi<float>()/4); // spread

        // Create a base direction vector with spread in local space
        glm::vec3 baseDir = glm::vec3(cos(angle), sin(angle), 0.0f);

        // Rotate baseDir by the emitter's rotation quaternion
        glm::vec3 dir = rotation * baseDir;

        p.velocity = dir * speed;
        p.color = randomColor();
        p.lifetime = randf(2.8f, 4.5f);
        p.age = 0.0f;
        p.size = randf(6.0f, 12.0f);
        p.active = true;
        particles.push_back(p);
    }
}

void ParticleEmitter::update(float dt) {
    for (auto& p : particles) {
        p.position += p.velocity * dt;
        p.age += dt;
    }
    // Remove dead particles
    particles.erase(
        std::remove_if(particles.begin(), particles.end(),
            [](const Particle& p) { return p.age > p.lifetime; }),
        particles.end()
    );
}

void ParticleEmitter::render(std::shared_ptr<Quad> quad, std::shared_ptr<Shader> shader) {
    for (const auto& p : particles) {
        quad->render();
        float alpha = 1.0f - (p.age / p.lifetime);
        glUniform4fv(glGetUniformLocation(shader->ID, "color"), 1,
                    glm::value_ptr(glm::vec4(p.color.r, p.color.g, p.color.b, p.color.a * alpha)));
          
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, p.position);
        model = glm::scale(model, glm::vec3(p.size));
        glUniformMatrix4fv(glGetUniformLocation(shader->ID, "model"), 1, GL_FALSE,
                          glm::value_ptr(model));

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        GLint err = glGetError();
        if(err != GL_NO_ERROR) {
            std::cerr << "OpenGL error: " << err << std::endl;
        }
    }
}
