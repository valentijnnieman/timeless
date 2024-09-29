#include <functional>
#include <memory>

#include "timeless/timeless.hpp"
#include "timeless/entity.hpp"
#include "timeless/algorithm/graph.hpp"

#include "timeless/components/behaviour.hpp"
#include "timeless/components/node.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

class Game
{
private:
	std::shared_ptr<Font> font_12;
	std::shared_ptr<Font> font_16;

	std::shared_ptr<Shader> sprite_shader;
	std::shared_ptr<Shader> text_shader;
	std::shared_ptr<Shader> color_shader;

	std::shared_ptr<Quad> default_quad;

	std::shared_ptr<Texture> tiles_texture;

	bool running = true;
public:
	Game()
	{
		// call init() to start up the engine
		TE::init();

		// create some resources that we can reuse, such as shaders, fonts, textures, etc.
		std::shared_ptr<Shader> screen_shader = std::shared_ptr<Shader>(new Shader("Assets/shaders/screen.vs", "Assets/shaders/screen.fs"));
		font_12 = std::shared_ptr<Font>(new Font(12));
		font_16 = std::shared_ptr<Font>(new Font(16));
		sprite_shader = std::shared_ptr<Shader>(new Shader("Assets/shaders/sprite.vs", "Assets/shaders/sprite_slice.fs"));
		text_shader = std::shared_ptr<Shader>(new Shader("Assets/shaders/text.vs", "Assets/shaders/text.fs"));
		default_quad = std::shared_ptr<Quad>(new Quad());
		tiles_texture = std::shared_ptr<Texture>(new Texture("Assets/textures/example_tiles.png", 512, 672));

		// a window manager is always created, since it is always needed to run the engine.
		// with get_window_manager() we get a pointer to it, and enable_screen_shader() sets the shader 
		// to be used for the entire screen. This shader can be used to create full screen effects, such as the 
		// video rewind effect in this example.
		TE::get_window_manager()->enable_screen_shader(screen_shader, screen_shader);

		// create systems - you can mix and match different systems, and create, for example, multiple rendering
		// systems so you can handle those individually, which is great for layering etc. In this example, we create 2 rendering systems - one for the tiles, and one for all the ui components.
		TE::create_system<RenderingSystem>("TileRenderingSystem", new RenderingSystem());
		TE::create_system<RenderingSystem>("UIRenderingSystem", new RenderingSystem());
		TE::create_system<TextRenderingSystem>("TextRenderingSystem", new TextRenderingSystem());
		TE::create_system<TextRenderingSystem>("UITextRenderingSystem", new TextRenderingSystem());
		TE::create_system<MovementSystem>("MovementSystem", new MovementSystem());
		TE::create_system<KeyboardInputSystem>("KeyboardInputSystem", new KeyboardInputSystem());
		TE::create_system<NpcAiSystem>("NpcAiSystem", new NpcAiSystem());
		TE::create_system<AnimationSystem>("AnimationSystem", new AnimationSystem());

		// create 2 cameras, one for the game objects (tiles, in this example) and another for 
		// the ui objects. This way we can move the main camera and keep the ui elements static, i.e. always visible on the screen.
		Entity main_camera = create_entity(),
			ui_camera = create_entity();
		TE::add_component(main_camera, new Camera(glm::vec3(-100.0f, 0.0f, 40.0f)));
		// also add a movement controller component to the main camera, so we can control
		// it using the keyboard (W,A,S,D).
		TE::add_component(main_camera, new MovementController());

		TE::add_component(ui_camera, new Camera(glm::vec3(0.0f, 0.0f, 10.0f)));

		// register the cameras with the multiple systems we created. we need to register them
		// so that those systems can respond to when a camera moves etc, and knows what to render based on
		// the camera.
		TE::get_system<MovementSystem>("MovementSystem")->register_camera(main_camera);
		TE::get_system<RenderingSystem>("TileRenderingSystem")->register_camera(main_camera);
		TE::get_system<TextRenderingSystem>("TextRenderingSystem")->register_camera(main_camera);
		// also register the main camera with the mouse input system, so mouse input can be read
		TE::get_mouse_input_system()->register_camera(main_camera);

		TE::get_system<RenderingSystem>("UIRenderingSystem")->register_camera(ui_camera);
		TE::get_system<TextRenderingSystem>("UITextRenderingSystem")->register_camera(ui_camera);

		// define some variables for our tiles, based on how the tiles in the 
		// sprite sheet are layed out (see the texture png file used above).
		float t_width = 64.0f;
		float t_height = 32.0f;
		float columns = 16.0f;
		float rows = 21.0f;

		// keep track of the index of our sprites used
		int index = 0;
		for (int y = 0; y < 20; y++)
		{
			for (int x = 0; x < 20; x++)
			{
				double x_pos = (x - y) * t_width / 2;
				double y_pos = (y + x) * t_height / 2;

				Entity tile = create_entity();

				// new nodes created here are automatically added to the default grid
				// that's created upon starting the engine.
				TE::add_component(tile, new Node(x, y, 1, t_width, t_height, x, 0));
				TE::add_component<Quad>(tile, default_quad);
				TE::add_component(tile, tiles_texture);
				TE::add_component<Shader>(tile, sprite_shader);
				TE::add_component<Transform>(tile, new Transform(glm::vec3(x_pos, y_pos, 1.0f), 0.0f, 32.0f, 32.0f));
				// sprites are indexed from bottom-to-top, as per stb_image library
				TE::add_component(tile, new Sprite(66, glm::vec4(1.0f), glm::vec2(tiles_texture->width, tiles_texture->height), glm::vec2(tiles_texture->width / columns, tiles_texture->height / rows), true, false, columns));
				TE::add_component(tile, new Animation());
				// lastly, we add an mouse input listener component - although we leave the handler function 
				// blank for now.
				TE::add_component(tile, new MouseInputListener([&](MouseEvent* event, Entity entity, int data)
					{
						// handle mouse input events here
					}
				));

				// create a set of Instructions - functions that are fired per tick. 
				std::vector<Instruction> instructions;

				glm::vec3 pos = TE::get_transform(tile)->position;
				for (int i = 0; i < 200; i++)
				{
					Instruction new_instruction = Instruction([x, y, i, tile, pos](Entity entity, bool reverse, float speed)
						{
							auto animation = TE::get_animation(tile);

							// animate tiles up and down based on even/odd positioning
							if ((x + y + i) % 2 == 0)
							{
								TE::get_animation(tile)->set_position_frames(pos, glm::vec3(pos.x, pos.y + 8.0f, 0.0f), 5.0);
							}
							else
							{
								TE::get_animation(tile)->set_position_frames(glm::vec3(pos.x, pos.y + 8.0f, 0.0f), pos, 5.0);
							}
						}
					);
					instructions.push_back(new_instruction);
				}

				TE::add_component(tile, new Behaviour(instructions));

				// register the new tile with the rendering system and animation system
				TE::get_system<RenderingSystem>("TileRenderingSystem")->register_entity(tile);
				TE::get_system<AnimationSystem>("AnimationSystem")->register_entity(tile);

				// register the tile with the ai system, so it can get the Behaviour and run it every tick
				TE::get_system<NpcAiSystem>("NpcAiSystem")->register_entity(tile);

				index++;
			}
		}

		// finalize the grid by calculating near neighbours for every node
		TE::calculate_grid();

		// create entity for displaying the current "tick" of the ai system
		Entity index_text = create_entity();
		TE::add_component(index_text, text_shader);
		TE::add_component(index_text, new Transform(glm::vec3(1900.0f, 200.0f, 0.0f), 0.0f, 1.0f, 1.0f));
		TE::add_component(index_text, font_16);
		TE::add_component(index_text, new Text(std::to_string(0)));

		// register the above entity with the ai system, which updates the text component with the current
		// tick information.
		TE::get_system<NpcAiSystem>("NpcAiSystem")->attach_text_component(*TE::get_component_manager(), index_text);

	}
	void loop()
	{
		// TE::loop() takes in a lambda function that'll be called every frame. 
		// here we can define how and when to update our systems, and call rendering functions.
		// we can also define some boilerplate frame limiting stuff (more to come soon)
		TE::loop(
			[&](GLFWwindow *window, ComponentManager &cm, WindowManager &wm)
			{
				double t = 0.0;
				double dt = 1.0 / 60.0;

				double current_time = glfwGetTime();
				while (!glfwWindowShouldClose(window) && running)
				{
					double new_time = glfwGetTime();
					double frame_time = new_time - current_time;
					current_time = new_time;

					while (frame_time > 0.0)
					{
						float delta_time = std::min(frame_time, dt);

						// update certain systems here, so the framerate for these are smoother.
						// for the AI system, this is especially important, because we don't want the 
						// ticks to update variably based on a users system. 
						TE::get_system<MovementSystem>("MovementSystem")->update(cm, window);
						TE::get_system<KeyboardInputSystem>("KeyboardInputSystem")->update(cm, window);
						TE::get_system<NpcAiSystem>("NpcAiSystem")->update(cm);
						TE::get_system<AnimationSystem>("AnimationSystem")->update(cm);
						frame_time -= delta_time;
						t += delta_time;
					}

					auto ai = TE::get_system<NpcAiSystem>("NpcAiSystem");

					/** render scene into framebuffer */
					wm.select_framebuffer(wm.screen_shader, true);

					TE::get_system<TextRenderingSystem>("TextRenderingSystem")->render(cm, TESettings::VIEWPORT_X, TESettings::VIEWPORT_Y);

					wm.render_framebuffer_as_quad(wm.screen_shader, true);

					wm.select_framebuffer(wm.tile_shader, false);
					TE::get_system<RenderingSystem>("TileRenderingSystem")->render(cm, TESettings::VIEWPORT_X, TESettings::VIEWPORT_Y, TESettings::ZOOM, ai->main_index);
					wm.render_framebuffer_as_quad(wm.tile_shader, false);

					// render ui and other parts not affected by screen shader
					TE::get_system<RenderingSystem>("UIRenderingSystem")->render(cm, TESettings::VIEWPORT_X, TESettings::VIEWPORT_Y);
					TE::get_system<TextRenderingSystem>("UITextRenderingSystem")->render(cm, TESettings::VIEWPORT_X, TESettings::VIEWPORT_Y);

					glfwSwapBuffers(window);
					glfwPollEvents();
				}
			});
	}
};

int main()
{
	Game game = Game();

	game.loop();

	glfwTerminate();

	return 0;
}
