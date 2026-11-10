// Timeless example: a 20x20 isometric grid of tiles that ripple as time ticks
// forward, and ripple back when it is rewound.
//
//   W/A/S/D or arrows   pan the camera        Space    pause / resume
//   mouse drag          pan the camera        Enter    change direction
//   scroll wheel        zoom                  Escape   quit
//
// Windowing and input go through the engine (SDL2 underneath): nothing here
// talks to the backend directly.

#include <chrono>
#include <cmath>
#include <memory>
#include <string>
#include <vector>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

// The whole engine. A game can instead include "timeless/core.hpp" plus only
// the components and systems it uses.
#include "timeless/timeless.hpp"

// The engine loads images with stb_image and leaves its implementation to
// the game.
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace
{
	// the grid
	const int GRID_SIZE = 20;
	const float TILE_WIDTH = 64.0f;
	const float TILE_HEIGHT = 32.0f;

	// the tile spritesheet: 16 x 21 sprites
	const float SHEET_COLUMNS = 16.0f;
	const float SHEET_ROWS = 21.0f;
	const int TILE_SPRITE = 66;

	// time: how long a tick takes, and how the tiles ripple
	const float SECONDS_PER_TICK = 0.25f;
	const float WAVE_HEIGHT = 12.0f;
	const int WAVE_LENGTH = 8; // in tiles

	// how far above its resting place a tile is at a tick
	float wave(int x, int y, int tick)
	{
		const float two_pi = 6.2831853f;
		return WAVE_HEIGHT * std::sin(two_pi * float(x + y + tick) / float(WAVE_LENGTH));
	}
}

class Game
{
private:
	// resources we reuse: shaders, fonts, textures, etc.
	std::shared_ptr<Shader> screen_shader;
	std::shared_ptr<Shader> rewind_shader;
	std::shared_ptr<Shader> sprite_shader;
	std::shared_ptr<Shader> text_shader;
	std::shared_ptr<Font> font;
	std::shared_ptr<Quad> default_quad;
	std::shared_ptr<Texture> tiles_texture;

	bool paused = false;
	bool rewinding = false;

	std::chrono::steady_clock::time_point previous_frame = std::chrono::steady_clock::now();

	void play(bool backward)
	{
		auto ai = TE::get_system<NpcAiSystem>("NpcAiSystem");
		rewinding = backward;
		if (rewinding)
			ai->play_backward(*TE::get_component_manager());
		else
			ai->play_forward(*TE::get_component_manager());
		if (paused)
			ai->pause(*TE::get_component_manager());

		// A framebuffer is drawn to the screen through its screen shader, so
		// swapping that changes the whole picture: rewinding looks like a
		// video tape being wound back.
		TE::get_window_manager()->screen_shaders[0] = rewinding ? rewind_shader : screen_shader;
	}

	void create_tiles()
	{
		for (int y = 0; y < GRID_SIZE; y++)
		{
			for (int x = 0; x < GRID_SIZE; x++)
			{
				// where the tile rests, in isometric projection
				glm::vec3 rest((x - y) * TILE_WIDTH / 2, (x + y) * TILE_HEIGHT / 2, 0.0f);

				Entity tile = create_entity();

				// a Node puts the tile on the engine's grid
				TE::add_component<Node>(tile, new Node(x, y, 1, TILE_WIDTH, TILE_HEIGHT, x, 0));
				TE::add_component<Quad>(tile, default_quad);
				TE::add_component<Texture>(tile, tiles_texture);
				TE::add_component<Shader>(tile, sprite_shader);
				TE::add_component<Transform>(tile, new Transform(rest, 0.0f, 32.0f, 32.0f));
				// sprites are indexed from bottom-to-top, as per stb_image library
				glm::vec2 sheet_size(tiles_texture->width, tiles_texture->height);
				TE::add_component<Sprite>(tile, new Sprite(TILE_SPRITE, glm::vec4(1.0f), sheet_size, glm::vec2(sheet_size.x / SHEET_COLUMNS, sheet_size.y / SHEET_ROWS), true, false, SHEET_COLUMNS));

				// A Behaviour is a list of Instructions: functions that are run one
				// per tick. Going forward in time, instruction i takes the tile from
				// tick i to tick i + 1; going backward (reverse) it is run again to
				// take it back to tick i. Here that just moves the tile along the
				// wave, by queueing the positions in between on its Transform.
				std::vector<Instruction> instructions;
				for (int i = 0; i < TESettings::MAX_TICKS; i++)
				{
					instructions.push_back(Instruction([x, y, i, rest](Entity entity, bool reverse, float speed)
						{
							auto transform = TE::get_component<Transform>(entity);
							int tick = reverse ? i : i + 1;
							glm::vec3 to(rest.x, rest.y - wave(x, y, tick), rest.z);
							// speed is the length of a tick in seconds
							transform->set_position_frames(transform->peek_position(), to, 1.0f / speed);
						}));
				}
				TE::add_component<Behaviour>(tile, new Behaviour(instructions));

				// register the new tile with the rendering system, so it gets drawn, and
				// with the ai system, so it can get the Behaviour and run it every tick
				TE::get_system<RenderingSystem>("TileRenderingSystem")->register_entity(tile);
				TE::get_system<NpcAiSystem>("NpcAiSystem")->register_entity(tile);
			}
		}

		// finalize the grid by calculating near neighbours for every node
		TE::calculate_grid();
	}

	void create_ui()
	{
		// The ui camera looks at the middle of the screen, so (0, 0) is its
		// centre and y runs down.
		float top = -TESettings::VIEWPORT_Y * 0.5f;

		// the current tick of the ai system, which updates the text itself
		Entity tick_text = create_entity();
		TE::add_component<Shader>(tick_text, text_shader);
		TE::add_component<Font>(tick_text, font);
		TE::add_component<Transform>(tick_text, new Transform(glm::vec3(0.0f, top + 60.0f, 0.0f), 0.0f, 1.0f, 1.0f));
		TE::add_component<Text>(tick_text, new Text(""));
		TE::get_system<RenderingSystem>("UITextRenderingSystem")->register_entity(tick_text);
		TE::get_system<NpcAiSystem>("NpcAiSystem")->attach_text_component(*TE::get_component_manager(), tick_text);

		Entity help_text = create_entity();
		TE::add_component<Shader>(help_text, text_shader);
		TE::add_component<Font>(help_text, font);
		TE::add_component<Transform>(help_text, new Transform(glm::vec3(0.0f, -top - 60.0f, 0.0f), 0.0f, 1.0f, 1.0f));
		TE::add_component<Text>(help_text, new Text("WASD or drag to move    scroll to zoom    space to pause    enter to rewind    esc to quit"));
		TE::get_system<RenderingSystem>("UITextRenderingSystem")->register_entity(help_text);
	}

	void create_input()
	{
		// Keyboard events go to every entity registered with the
		// KeyboardInputSystem that has a KeyboardInputListener.
		Entity keys = create_entity();
		TE::add_component<KeyboardInputListener>(keys, new KeyboardInputListener([this](KeyboardEvent* event, Entity entity, int data)
			{
				auto ai = TE::get_system<NpcAiSystem>("NpcAiSystem");
				if (event->eventType == "PressSpace")
				{
					paused = !paused;
					if (paused)
						ai->pause(*TE::get_component_manager());
					else
						ai->resume(*TE::get_component_manager());
				}
				if (event->eventType == "PressEnter")
					play(!rewinding);
				if (event->eventType == "PressEscape")
					TE::quit();
			}));
		TE::get_system<KeyboardInputSystem>("KeyboardInputSystem")->register_entity(keys);

		// Mouse events go to every entity with a MouseInputListener; adding the
		// component registers it. Use TE::clicked_on(event, entity) to find out
		// whether an event is on an entity - these are for the whole screen.
		Entity mouse = create_entity();
		TE::add_component<MouseInputListener<MouseEvent>>(mouse, new MouseInputListener<MouseEvent>([](MouseEvent* event, Entity entity, int data)
			{
				auto movement = TE::get_system<MovementSystem>("MovementSystem");
				if (event->eventType == "GlobalLeftMousePress")
					movement->begin_drag(event->raw_position);
				if (event->eventType == "MouseScroll")
					movement->zoom_by(event->y_scroll);
			}));
	}

public:
	Game()
	{
		// call init() to start up the engine: it opens the window and creates
		// the OpenGL context, so it comes before anything that needs those.
		TE::init();

		// The engine's shaders are copied into Assets/shaders by the build.
		screen_shader = std::shared_ptr<Shader>(new Shader("Assets/shaders/webgl_screen.vs", "Assets/shaders/webgl_screen.fs"));
		rewind_shader = std::shared_ptr<Shader>(new Shader("Assets/shaders/webgl_screen.vs", "Assets/shaders/webgl_reverse_screen.fs"));
		sprite_shader = std::shared_ptr<Shader>(new Shader("Assets/shaders/webgl_sprite.vs", "Assets/shaders/webgl_sprite_slice.fs"));
		text_shader = std::shared_ptr<Shader>(new Shader("Assets/shaders/webgl_text.vs", "Assets/shaders/webgl_text.fs"));
		font = std::shared_ptr<Font>(new Font(32));
		default_quad = std::shared_ptr<Quad>(new Quad());
		tiles_texture = std::shared_ptr<Texture>(new Texture("Assets/textures/example_tiles.png", 512, 672));

		// The tiles are drawn into a framebuffer, which is then drawn to the
		// screen through a screen shader - that's where full screen effects go.
		TE::get_window_manager()->add_framebuffer(screen_shader);

		// create systems - you can mix and match different systems, and create, for example, multiple rendering
		// systems so you can handle those individually, which is great for layering etc. In this example, we
		// create 2 rendering systems - one for the tiles, and one for the text on top.
		TE::create_system<RenderingSystem>("TileRenderingSystem", new RenderingSystem());
		TE::create_system<RenderingSystem>("UITextRenderingSystem", new RenderingSystem());
		TE::create_system<MovementSystem>("MovementSystem", new MovementSystem());
		TE::create_system<KeyboardInputSystem>("KeyboardInputSystem", new KeyboardInputSystem());
		TE::create_system<NpcAiSystem>("NpcAiSystem", new NpcAiSystem());

		// create 2 cameras, one for the game objects (tiles, in this example) and another for
		// the ui objects. This way we can move the main camera and keep the ui elements static,
		// i.e. always visible on the screen. Both look straight down the z axis with y running
		// down the screen: a flat, 2D view. The main camera starts over the middle of the grid.
		const glm::vec3 forward(0.0f, 0.0f, -1.0f), up(0.0f, -1.0f, 0.0f);
		const glm::quat no_rotation(1.0f, 0.0f, 0.0f, 0.0f);
		Entity main_camera = create_entity(),
			ui_camera = create_entity();
		TE::add_component<Camera>(main_camera, new Camera(glm::vec3(0.0f, GRID_SIZE * TILE_HEIGHT / 2, 1000.0f), false, forward, up));
		TE::get_component<Camera>(main_camera)->set_rotation(no_rotation);
		// also add a movement controller component to the main camera, so we can control it.
		TE::add_component<MovementController>(main_camera, new MovementController());

		TE::add_component<Camera>(ui_camera, new Camera(glm::vec3(0.0f, 0.0f, 1000.0f), false, forward, up));
		TE::get_component<Camera>(ui_camera)->set_rotation(no_rotation);

		// register the cameras with the systems we created. we need to register them so that
		// those systems know what to move, or what to render from.
		auto movement = TE::get_system<MovementSystem>("MovementSystem");
		movement->register_camera(main_camera);
		movement->turn_keys = false;   // no Q/E turning: this is a flat view
		movement->edge_margin = 0.0f;  // no panning when the cursor is at the window's edge
		TE::get_system<RenderingSystem>("TileRenderingSystem")->register_camera(main_camera);
		// also register the main camera with the mouse input system, so mouse input can be read
		TE::get_mouse_input_system()->register_camera(main_camera);

		TE::get_system<RenderingSystem>("UITextRenderingSystem")->register_camera(ui_camera);

		create_tiles();
		create_ui();
		create_input();

		// set how long a tick takes, and start time running forward.
		TE::get_system<NpcAiSystem>("NpcAiSystem")->change_timer(SECONDS_PER_TICK);
		play(false);
	}

	~Game()
	{
		// let go of our own resources while there still is an OpenGL context,
		// then shut down the engine.
		screen_shader.reset();
		rewind_shader.reset();
		sprite_shader.reset();
		text_shader.reset();
		font.reset();
		default_quad.reset();
		tiles_texture.reset();
		TE::cleanup();
	}

	// one frame
	void loop()
	{
		auto now = std::chrono::steady_clock::now();
		float delta_time = std::chrono::duration<float>(now - previous_frame).count();
		previous_frame = now;

		// TE::loop() takes in a lambda function that gets the window manager and the
		// component manager. here we define how and when to update our systems, and
		// call rendering functions.
		TE::loop(
			[&](WindowManager& wm, ComponentManager& cm)
			{
				auto ai = TE::get_system<NpcAiSystem>("NpcAiSystem");

				TE::get_system<MovementSystem>("MovementSystem")->update(cm, wm, delta_time);
				TE::get_system<KeyboardInputSystem>("KeyboardInputSystem")->update(cm, wm);
				// the ai system runs the next Instruction of every Behaviour once per tick,
				// on its own timer, so ticks don't depend on the frame rate.
				ai->update(cm);

				// at either end of time, turn around
				if (!rewinding && ai->main_index >= TESettings::MAX_TICKS)
					play(true);
				else if (rewinding && ai->main_index <= 0)
					play(false);

				// render the tiles into the framebuffer...
				wm.select_framebuffer(0, true);
				TE::get_system<RenderingSystem>("TileRenderingSystem")->render(cm, TESettings::VIEWPORT_X, TESettings::VIEWPORT_Y, TESettings::ZOOM, ai->main_index, delta_time);
				// ...then the framebuffer to the screen, through its screen shader. The
				// screen has to be selected first: a framebuffer is drawn as a quad
				// into whatever is bound, so that they can be chained.
				glBindFramebuffer(GL_FRAMEBUFFER, TE::screen_framebuffer);
				wm.render_framebuffer_as_quad(0, true, ai->main_index, true);

				// render the ui on top, unaffected by the screen shader or the zoom
				TE::get_system<RenderingSystem>("UITextRenderingSystem")->render(cm, TESettings::VIEWPORT_X, TESettings::VIEWPORT_Y);

				wm.swap_buffers();
				wm.poll_events();
			});
	}
};

#ifdef __EMSCRIPTEN__
static void browser_frame(void* game)
{
	static_cast<Game*>(game)->loop();
}
#endif

int main()
{
	Game game;
#ifdef __EMSCRIPTEN__
	// the browser calls us once per frame
	emscripten_set_canvas_element_size("canvas", TESettings::SCREEN_X, TESettings::SCREEN_Y);
	emscripten_set_main_loop_arg(browser_frame, &game, 0, 1);
#else
	auto wm = TE::get_window_manager();
	// until the window is closed, or TE::quit() is called
	while (!wm->should_close())
	{
		game.loop();
	}
#endif

	return 0;
}
