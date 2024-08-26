#pragma once
#include "../entity.hpp"
#include "../components/transform.hpp"
#include "../components/shader.hpp"
#include "../components/sprite.hpp"
#include "../components/texture.hpp"
#include "../components/mouse_input_listener.hpp"
#include "../components/event_listener.hpp"
#include "../components/movement_controller.hpp"
#include "../components/keyboard_input_listener.hpp"
#include "../components/camera.hpp"
#include "../components/collider.hpp"
#include "../components/node.hpp"
#include "../components/font.hpp"
#include "../components/text.hpp"
#include "../components/behaviour.hpp"
#include "../components/line.hpp"
#include "../components/animation.hpp"

class ComponentManager
{
public:
    std::unordered_map<Entity, std::shared_ptr<Quad>> quads;
    std::unordered_map<Entity, std::shared_ptr<Texture>> textures;
    std::unordered_map<Entity, std::shared_ptr<Shader>> shaders;
    std::unordered_map<Entity, std::shared_ptr<Sprite>> sprites;
    std::unordered_map<Entity, std::shared_ptr<Transform>> transforms;
    std::unordered_map<Entity, std::shared_ptr<MouseInputListener>> mouse_input_listeners;
    std::unordered_map<Entity, std::shared_ptr<EventListener<Event>>> event_listeners;
    std::unordered_map<Entity, std::shared_ptr<EventListener<PositionEvent>>> position_event_listeners;
    std::unordered_map<Entity, std::shared_ptr<KeyboardInputListener>> keyboard_input_listeners;
    std::unordered_map<Entity, std::shared_ptr<Camera>> cameras;
    std::unordered_map<Entity, std::shared_ptr<Collider>> colliders;
    std::unordered_map<Entity, std::shared_ptr<MovementController>> movements;
    std::unordered_map<Entity, std::shared_ptr<Node>> nodes;
    std::unordered_map<Entity, std::shared_ptr<Font>> fonts;
    std::unordered_map<Entity, std::shared_ptr<Text>> texts;
    std::unordered_map<Entity, std::shared_ptr<Behaviour>> behaviours;
    std::unordered_map<Entity, std::shared_ptr<Line>> geometry;
    std::unordered_map<Entity, std::shared_ptr<Animation>> animations;

    /** These separately defined methods are perhaps not very DRY.
     * It could have been done with templates and a bitfield type signature
     * in the entity id's, but I opted for clarity - sometimes it's better
     * to write out everything instead of trying to keep things DRY, when
     * developing something new!
     */
    void add_component(Entity entity, Quad *quad)
    {
        quads.insert({entity, std::shared_ptr<Quad>(quad)});
    }
    void add_component(Entity entity, std::shared_ptr<Quad> quad)
    {
        quads.insert({entity, quad});
    }
    void add_component(Entity entity, Texture *texture)
    {
        textures.insert({entity, std::shared_ptr<Texture>(texture)});
    }
    void add_component(Entity entity, std::shared_ptr<Texture> texture)
    {
        textures.insert({entity, texture});
    }
    void add_component(Entity entity, Shader *shader)
    {
        shaders.insert({entity, std::shared_ptr<Shader>(shader)});
    }
    void add_component(Entity entity, std::shared_ptr<Shader> shader)
    {
        shaders.insert({entity, shader});
    }
    void add_component(Entity entity, Sprite *sprite)
    {
        sprites.insert({entity, std::shared_ptr<Sprite>(sprite)});
    }
    void add_component(Entity entity, Transform *transform)
    {
        transforms.insert({entity, std::shared_ptr<Transform>(transform)});
    }
    void add_component(Entity entity, MouseInputListener *mouse_input)
    {
        mouse_input_listeners.insert({entity, std::shared_ptr<MouseInputListener>(mouse_input)});
    }
    void add_component(Entity entity, EventListener<Event> *event_listener)
    {
        event_listeners.insert({entity, std::shared_ptr<EventListener<Event>>(event_listener)});
    }
    void add_component(Entity entity, EventListener<PositionEvent> *event_listener)
    {
        position_event_listeners.insert({entity, std::shared_ptr<EventListener<PositionEvent>>(event_listener)});
    }
    void add_component(Entity entity, KeyboardInputListener *keyboard_input)
    {
        keyboard_input_listeners.insert({entity, std::shared_ptr<KeyboardInputListener>(keyboard_input)});
    }
    void add_component(Entity entity, Camera *camera)
    {
        cameras.insert({entity, std::shared_ptr<Camera>(camera)});
    }
    void add_component(Entity entity, std::shared_ptr<Camera> camera)
    {
        cameras.insert({entity, camera});
    }
    void add_component(Entity entity, MovementController *movement)
    {
        movements.insert({entity, std::shared_ptr<MovementController>(movement)});
    }
    void add_component(Entity entity, Collider *collider)
    {
        colliders.insert({entity, std::shared_ptr<Collider>(collider)});
    }
    void add_component(Entity entity, Node *node)
    {
        nodes.insert({entity, std::shared_ptr<Node>(node)});
    }
    void add_component(Entity entity, std::shared_ptr<Node> node)
    {
        nodes.insert({entity, node});
    }
    void add_component(Entity entity, Font *font)
    {
        fonts.insert({entity, std::shared_ptr<Font>(font)});
    }
    void add_component(Entity entity, std::shared_ptr<Font> font)
    {
        fonts.insert({entity, font});
    }
    void add_component(Entity entity, Text *text)
    {
        texts.insert({entity, std::shared_ptr<Text>(text)});
    }
    void add_component(Entity entity, Behaviour *behaviour)
    {
        behaviours.insert({entity, std::shared_ptr<Behaviour>(behaviour)});
    }
    void add_component(Entity entity, Line *line)
    {
        geometry.insert({entity, std::shared_ptr<Line>(line)});
    }
    void add_component(Entity entity, Animation *animation)
    {
        animations.insert({entity, std::shared_ptr<Animation>(animation)});
    }

    std::shared_ptr<Quad> get_quad(Entity entity)
    {
        return quads.at(entity);
    }
    std::shared_ptr<Texture> get_texture(Entity entity)
    {
		return textures.at(entity);
    }
    std::shared_ptr<Shader> get_shader(Entity entity)
    {
        return shaders.at(entity);
    }
    std::shared_ptr<Transform> get_transform(Entity entity)
    {
        return transforms.at(entity);
    }
    std::shared_ptr<Sprite> get_sprite(Entity entity)
    {
        return sprites.at(entity);
    }
    std::shared_ptr<Camera> get_camera(Entity entity)
    {
        return cameras.at(entity);
    }
    std::shared_ptr<Font> get_font(Entity entity)
    {
        return fonts.at(entity);
    }
    std::shared_ptr<Behaviour> get_behaviour(Entity entity)
    {
        return behaviours.at(entity);
    }
    std::shared_ptr<Text> get_text(Entity entity)
    {
        return texts.at(entity);
    }
    std::shared_ptr<MouseInputListener> get_mouse_input_listener(Entity entity)
    {
        return mouse_input_listeners.at(entity);
    }
    std::shared_ptr<EventListener<Event>> get_event_listener(Entity entity)
    {
        return event_listeners.at(entity);
    }
    std::shared_ptr<EventListener<PositionEvent>> get_position_event_listener(Entity entity)
    {
        return position_event_listeners.at(entity);
    }
    std::shared_ptr<KeyboardInputListener> get_keyboard_input_listener(Entity entity)
    {
        return keyboard_input_listeners.at(entity);
    }
    std::shared_ptr<Animation> get_animation(Entity entity)
    {
        return animations.at(entity);
    }

    void remove_component(Entity entity, Texture *texture)
    {
        textures.erase(entity);
    }
    void remove_component(Entity entity, std::shared_ptr<Texture> texture)
    {
        textures.erase(entity);
    }
    void remove_component(Entity entity, Quad *quad)
    {
        quads.erase(entity);
    }
    void remove_component(Entity entity, std::shared_ptr<Quad> quad)
    {
        quads.erase(entity);
    }
    void remove_component(Entity entity, Shader *shader)
    {
        shaders.erase(entity);
    }
    void remove_component(Entity entity, std::shared_ptr<Shader> shader)
    {
        shaders.erase(entity);
    }
    void remove_component(Entity entity, Transform *transform)
    {
        transforms.erase(entity);
    }
    void remove_component(Entity entity, std::shared_ptr<Transform> transform)
    {
        transforms.erase(entity);
    }

    /** this method completely removes all components for an entity */
    void remove_entity(Entity entity)
    {
        quads.erase(entity);
        textures.erase(entity);
        shaders.erase(entity);
        sprites.erase(entity);
        transforms.erase(entity);
        mouse_input_listeners.erase(entity);
        // cameras.erase(entity);
        colliders.erase(entity);
        movements.erase(entity);
        nodes.erase(entity);
        fonts.erase(entity);
        texts.erase(entity);
        behaviours.erase(entity);
    }
};