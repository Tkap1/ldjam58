
#define m_cpu_side 1
#if !defined(__EMSCRIPTEN__)
#define m_winhttp 1
#endif

#pragma comment(lib, "opengl32.lib")

#pragma warning(push, 0)
// #define SDL_MAIN_HANDLED
#include "SDL2/SDL.h"
#pragma warning(pop)

#if defined(__EMSCRIPTEN__)
#include <emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/fetch.h>
#endif // __EMSCRIPTEN__

#if defined(_WIN32)
#pragma warning(push, 0)
#define NOMINMAX
#if !defined(m_winhttp)
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#if defined(m_winhttp)
#include <winhttp.h>
#pragma comment(lib, "winhttp.lib")
#endif
#undef near
#undef far
#pragma warning(pop)
#endif // _WIN32


#include <stdlib.h>

#include <gl/GL.h>
#if !defined(__EMSCRIPTEN__)
#include "external/glcorearb.h"
#endif

#if defined(__EMSCRIPTEN__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-value"
#endif

#pragma warning(push, 0)
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ASSERT
#include "external/stb_image.h"

#define STB_TRUETYPE_IMPLEMENTATION
#define STBTT_assert
#include "external/stb_truetype.h"
#pragma warning(pop)

#if defined(__EMSCRIPTEN__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-value"
#endif

#if defined(__EMSCRIPTEN__)
#define m_gl_single_channel GL_LUMINANCE
#else // __EMSCRIPTEN__
#define m_gl_single_channel GL_RED
#endif // __EMSCRIPTEN__


#if defined(m_debug)
#define gl(...) __VA_ARGS__; {int error = glGetError(); if(error != 0) { on_gl_error(#__VA_ARGS__, __FILE__, __LINE__, error); }}
#else // m_debug
#define gl(...) __VA_ARGS__
#endif // m_debug

#include "tklib.h"
#include "shared.h"

#include "config.h"
#include "leaderboard.h"
#include "engine.h"
#include "game.h"
#include "shader_shared.h"


#if defined(__EMSCRIPTEN__)
global constexpr b8 c_on_web = true;
#else
global constexpr b8 c_on_web = false;
#endif


global s_platform_data* g_platform_data;
global s_game* game;
global s_v2 g_mouse;
global b8 g_left_click;
global b8 g_right_click;

#include "json.cpp"

#if defined(__EMSCRIPTEN__) || defined(m_winhttp)
#include "leaderboard.cpp"
#endif

#include "engine.cpp"

m_dll_export void init(s_platform_data* platform_data)
{
	g_platform_data = platform_data;
	game = (s_game*)platform_data->memory;
	unpoison_memory(game, sizeof(s_game));
	game->speed_index = 5;
	game->rng = make_rng(1234);
	game->reload_shaders = true;
	game->speed = 0;
	game->music_speed = {1, 1};

	if(!c_on_web) {
		game->disable_music = true;
	}

	SDL_StartTextInput();

	set_state(&game->state0, e_game_state0_main_menu);
	game->do_hard_reset = true;
	set_state(&game->hard_data.state1, e_game_state1_default);

	u8* cursor = platform_data->memory + sizeof(s_game);

	{
		game->arena = make_arena_from_memory(cursor, 10 * c_mb);
		cursor += 10 * c_mb;
	}
	{
		game->update_frame_arena = make_arena_from_memory(cursor, 10 * c_mb);
		cursor += 10 * c_mb;
	}
	{
		game->render_frame_arena = make_arena_from_memory(cursor, 400 * c_mb);
		cursor += 400 * c_mb;
	}
	{
		game->circular_arena = make_circular_arena_from_memory(cursor, 10 * c_mb);
		cursor += 10 * c_mb;
	}

	platform_data->cycle_frequency = SDL_GetPerformanceFrequency();
	platform_data->start_cycles = SDL_GetPerformanceCounter();

	platform_data->window_size.x = (int)c_world_size.x;
	platform_data->window_size.y = (int)c_world_size.y;

	g_platform_data->window = SDL_CreateWindow(
		"Loop Fighter", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		(int)c_world_size.x, (int)c_world_size.y, SDL_WINDOW_HIDDEN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
	);
	SDL_SetWindowPosition(g_platform_data->window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
	SDL_ShowWindow(g_platform_data->window);

	#if defined(__EMSCRIPTEN__)
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
	#else
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	#endif

	g_platform_data->gl_context = SDL_GL_CreateContext(g_platform_data->window);
	SDL_GL_SetSwapInterval(1);

	#define X(type, name) name = (type)SDL_GL_GetProcAddress(#name); assert(name);
		m_gl_funcs
	#undef X

	{
		gl(glGenBuffers(1, &game->ubo));
		gl(glBindBuffer(GL_UNIFORM_BUFFER, game->ubo));
		gl(glBufferData(GL_UNIFORM_BUFFER, sizeof(s_uniform_data), NULL, GL_DYNAMIC_DRAW));
		gl(glBindBufferBase(GL_UNIFORM_BUFFER, 0, game->ubo));
	}

	{
		constexpr float c_size = 0.5f;
		s_vertex vertex_arr[] = {
			{v3(-c_size, -c_size, 0), v3(0, -1, 0), make_rrr(1), v2(0, 1)},
			{v3(c_size, -c_size, 0), v3(0, -1, 0), make_rrr(1), v2(1, 1)},
			{v3(c_size, c_size, 0), v3(0, -1, 0), make_rrr(1), v2(1, 0)},
			{v3(-c_size, -c_size, 0), v3(0, -1, 0), make_rrr(1), v2(0, 1)},
			{v3(c_size, c_size, 0), v3(0, -1, 0), make_rrr(1), v2(1, 0)},
			{v3(-c_size, c_size, 0), v3(0, -1, 0), make_rrr(1), v2(0, 0)},
		};
		game->mesh_arr[e_mesh_quad] = make_mesh_from_vertices(vertex_arr, array_count(vertex_arr));
	}

	{
		game->mesh_arr[e_mesh_cube] = make_mesh_from_obj_file("assets/cube.obj", &game->render_frame_arena);
		game->mesh_arr[e_mesh_sphere] = make_mesh_from_obj_file("assets/sphere.obj", &game->render_frame_arena);
	}

	{
		s_mesh* mesh = &game->mesh_arr[e_mesh_line];
		mesh->vertex_count = 6;
		gl(glGenVertexArrays(1, &mesh->vao));
		gl(glBindVertexArray(mesh->vao));

		gl(glGenBuffers(1, &mesh->instance_vbo.id));
		gl(glBindBuffer(GL_ARRAY_BUFFER, mesh->instance_vbo.id));

		u8* offset = 0;
		constexpr int stride = sizeof(float) * 9;
		int attrib_index = 0;

		gl(glVertexAttribPointer(attrib_index, 2, GL_FLOAT, GL_FALSE, stride, offset)); // line start
		gl(glEnableVertexAttribArray(attrib_index));
		gl(glVertexAttribDivisor(attrib_index, 1));
		attrib_index += 1;
		offset += sizeof(float) * 2;

		gl(glVertexAttribPointer(attrib_index, 2, GL_FLOAT, GL_FALSE, stride, offset)); // line end
		gl(glEnableVertexAttribArray(attrib_index));
		gl(glVertexAttribDivisor(attrib_index, 1));
		attrib_index += 1;
		offset += sizeof(float) * 2;

		gl(glVertexAttribPointer(attrib_index, 1, GL_FLOAT, GL_FALSE, stride, offset)); // line width
		gl(glEnableVertexAttribArray(attrib_index));
		gl(glVertexAttribDivisor(attrib_index, 1));
		attrib_index += 1;
		offset += sizeof(float) * 1;

		gl(glVertexAttribPointer(attrib_index, 4, GL_FLOAT, GL_FALSE, stride, offset)); // instance color
		gl(glEnableVertexAttribArray(attrib_index));
		gl(glVertexAttribDivisor(attrib_index, 1));
		attrib_index += 1;
		offset += sizeof(float) * 4;
	}

	for(int i = 0; i < e_sound_count; i += 1) {
		platform_data->sound_arr[i] = platform_data->load_sound_from_file(c_sound_data_arr[i].path);
	}

	for(int i = 0; i < e_texture_count; i += 1) {
		char* path = c_texture_path_arr[i];
		if(strlen(path) > 0) {
			game->texture_arr[i] = load_texture_from_file(path, GL_NEAREST);
		}
	}

	game->font = load_font_from_file("assets/Inconsolata-Regular.ttf", 128, &game->render_frame_arena);

	{
		u32* texture = &game->texture_arr[e_texture_light].id;
		game->light_fbo.size.x = (int)c_world_size.x;
		game->light_fbo.size.y = (int)c_world_size.y;
		gl(glGenFramebuffers(1, &game->light_fbo.id));
		bind_framebuffer(game->light_fbo.id);
		gl(glGenTextures(1, texture));
		gl(glBindTexture(GL_TEXTURE_2D, *texture));
		gl(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, game->light_fbo.size.x, game->light_fbo.size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL));
		gl(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		gl(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
		gl(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, *texture, 0));
		assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
		bind_framebuffer(0);
	}

	#if defined(m_winhttp)
	// @TODO(tkap, 08/08/2025): This might break with hot reloading
	game->session = WinHttpOpen(L"A WinHTTP POST Example/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
	assert(game->session);
	#endif

	#if defined(__EMSCRIPTEN__) || defined(m_winhttp)
	load_or_create_leaderboard_id();
	#endif

	play_sound(e_sound_music, {.loop = true, .speed = game->music_speed.curr});
}

m_dll_export void init_after_recompile(s_platform_data* platform_data)
{
	g_platform_data = platform_data;
	game = (s_game*)platform_data->memory;

	#define X(type, name) name = (type)SDL_GL_GetProcAddress(#name); assert(name);
		m_gl_funcs
	#undef X
}

#if defined(__EMSCRIPTEN__)
EM_JS(int, browser_get_width, (), {
	const { width, height } = canvas.getBoundingClientRect();
	return width;
});

EM_JS(int, browser_get_height, (), {
	const { width, height } = canvas.getBoundingClientRect();
	return height;
});
#endif // __EMSCRIPTEN__

m_dll_export void do_game(s_platform_data* platform_data)
{
	g_platform_data = platform_data;
	game = (s_game*)platform_data->memory;

	f64 delta64 = get_seconds() - game->time_before;
	delta64 = at_most(1.0/20.0, delta64);
	game->time_before = get_seconds();

	{
		#if defined(__EMSCRIPTEN__)
		int width = browser_get_width();
		int height = browser_get_height();
		g_platform_data->window_size.x = width;
		g_platform_data->window_size.y = height;
		if(g_platform_data->prev_window_size.x != width || g_platform_data->prev_window_size.y != height) {
			set_window_size(width, height);
			g_platform_data->window_resized = true;
		}
		g_platform_data->prev_window_size.x = width;
		g_platform_data->prev_window_size.y = height;
		#endif // __EMSCRIPTEN__
	}

	// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		handle state start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
	{
		b8 state_changed = handle_state(&game->state0, game->render_time);
		if(state_changed) {
			game->accumulator += c_update_delay;
		}
	}
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		handle state end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

	input();

	float game_speed = c_game_speed_arr[game->speed_index] * game->speed;
	game->accumulator += delta64 * game_speed;
	f64 clamped_accumulator = at_most(c_update_delay * 20, game->accumulator);
	while(clamped_accumulator >= c_update_delay) {
		game->accumulator -= c_update_delay;
		clamped_accumulator -= c_update_delay;
		update();
	}
	float interp_dt = (float)(clamped_accumulator / c_update_delay);
	render(interp_dt, (float)delta64);
}

func void input()
{
	game->char_events.count = 0;
	game->key_events.count = 0;

	// u8* keyboard_state = (u8*)SDL_GetKeyboardState(null);
	// game->input.left = game->input.left || keyboard_state[SDL_SCANCODE_A];
	// game->input.right = game->input.right || keyboard_state[SDL_SCANCODE_D];

	for(int i = 0; i < c_max_keys; i += 1) {
		game->input_arr[i].half_transition_count = 0;
	}

	g_left_click = false;
	g_right_click = false;
	{
		int x;
		int y;
		SDL_GetMouseState(&x, &y);
		g_mouse = v2(x, y);
		s_rect letterbox = do_letterbox(v2(g_platform_data->window_size), c_world_size);
		g_mouse.x = range_lerp(g_mouse.x, letterbox.x, letterbox.x + letterbox.size.x, 0, c_world_size.x);
		g_mouse.y = range_lerp(g_mouse.y, letterbox.y, letterbox.y + letterbox.size.y, 0, c_world_size.y);
	}

	s_hard_game_data* hard_data = &game->hard_data;
	s_soft_game_data* soft_data = &game->soft_data;

	e_game_state0 state0 = (e_game_state0)get_state(&game->state0);
	e_game_state1 state1 = (e_game_state1)get_state(&hard_data->state1);

	SDL_Event event;
	while(SDL_PollEvent(&event) != 0) {
		switch(event.type) {
			case SDL_QUIT: {
				g_platform_data->quit = true;
			} break;

			case SDL_WINDOWEVENT: {
				if(event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
					int width = event.window.data1;
					int height = event.window.data2;
					g_platform_data->window_size.x = width;
					g_platform_data->window_size.y = height;
					g_platform_data->window_resized = true;
				}
			} break;

			case SDL_KEYDOWN:
			case SDL_KEYUP: {
				int key = event.key.keysym.sym;
				SDL_Scancode scancode = event.key.keysym.scancode;
				if(key == SDLK_LSHIFT) {
					key = c_left_shift;
				}
				else if(key == SDLK_LCTRL) {
					key = c_left_ctrl;
				}
				b8 is_down = event.type == SDL_KEYDOWN;
				handle_key_event(key, is_down, event.key.repeat != 0);

				if(key < c_max_keys) {
					game->input_arr[key].is_down = is_down;
					game->input_arr[key].half_transition_count += 1;
				}
				if(event.type == SDL_KEYDOWN) {
					if(key == SDLK_r && event.key.repeat == 0) {
						if(event.key.keysym.mod & KMOD_LCTRL) {
							game->do_hard_reset = true;
						}
						else if(state1 == e_game_state1_defeat) {
							game->do_hard_reset = true;
						}
					}
					else if(key == SDLK_SPACE && event.key.repeat == 0) {
					}
					else if(scancode == SDL_SCANCODE_A) {
					}
					else if(scancode == SDL_SCANCODE_S && event.key.repeat == 0) {
					}
					else if((key == SDLK_ESCAPE && event.key.repeat == 0) || (key == SDLK_o && event.key.repeat == 0) || (key == SDLK_p && event.key.repeat == 0)) {
						if(state0 == e_game_state0_play && state1 == e_game_state1_default) {
							add_state(&game->state0, e_game_state0_pause);
						}
						else if(state0 == e_game_state0_pause) {
							pop_state_transition(&game->state0, game->render_time, c_transition_time);
						}
					}
					#if defined(m_debug)
					else if(key == SDLK_s && event.key.repeat == 0 && event.key.keysym.mod & KMOD_LCTRL) {
					}
					else if(key == SDLK_l && event.key.repeat == 0 && event.key.keysym.mod & KMOD_LCTRL) {
					}
					else if(key == SDLK_KP_PLUS) {
						game->speed_index = circular_index(game->speed_index + 1, array_count(c_game_speed_arr));
						printf("Game speed: %f\n", c_game_speed_arr[game->speed_index]);
					}
					else if(key == SDLK_KP_MINUS) {
						game->speed_index = circular_index(game->speed_index - 1, array_count(c_game_speed_arr));
						printf("Game speed: %f\n", c_game_speed_arr[game->speed_index]);
					}
					else if(key == SDLK_F1) {
						game->cheat_menu_enabled = !game->cheat_menu_enabled;
					}
					else if(key == SDLK_j && event.key.repeat == 0) {
					}
					else if(key == SDLK_h && event.key.repeat == 0) {
					}
					#endif // m_debug

				}

				// @Note(tkap, 28/06/2025): Key up
				else {
					if(key == SDLK_SPACE) {
					}
				}
			} break;

			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
			{
				if(event.type == SDL_MOUSEBUTTONDOWN && event.button.button == 1) {
					g_left_click = true;
				}
				if(event.type == SDL_MOUSEBUTTONDOWN && event.button.button == 3) {
					g_right_click = true;
				}
				// int key = sdl_key_to_windows_key(event.button.button);
				// b8 is_down = event.type == SDL_MOUSEBUTTONDOWN;
				// handle_key_event(key, is_down, false);

				if(event.button.button == 1) {
					int key = c_left_button;
					game->input_arr[key].is_down = event.type == SDL_MOUSEBUTTONDOWN;
					game->input_arr[key].half_transition_count += 1;
				}

				else if(event.button.button == 3) {
					int key = c_right_button;
					game->input_arr[key].is_down = event.type == SDL_MOUSEBUTTONDOWN;
					game->input_arr[key].half_transition_count += 1;
				}

			} break;

			case SDL_TEXTINPUT: {
				char c = event.text.text[0];
				game->char_events.add((char)c);
			} break;

			case SDL_MOUSEWHEEL: {
			} break;
		}
	}
}

func void update()
{
	reset_linear_arena(&game->update_frame_arena);

	e_game_state0 state0 = (e_game_state0)get_state(&game->state0);

	s_hard_game_data* hard_data = &game->hard_data;
	s_soft_game_data* soft_data = &game->soft_data;
	auto entity_arr = &soft_data->entity_arr;

	float delta = (float)c_update_delay;

	if(game->do_hard_reset) {
		game->do_hard_reset = false;
		memset(hard_data, 0, sizeof(*hard_data));
		memset(soft_data, 0, sizeof(*soft_data));
		game->update_time = 0;
		set_state(&game->hard_data.state1, e_game_state1_default);
		reset_linear_arena(&game->arena);
		for_enum(type_i, e_entity) {
			entity_manager_reset(entity_arr, type_i);
		}

		game->music_speed.target = 1;
	}
	if(game->do_soft_reset) {
	}

	b8 do_game = false;
	switch(state0) {
		case e_game_state0_play: {
			e_game_state1 state1 = (e_game_state1)get_state(&hard_data->state1);
			switch(state1) {
				xcase e_game_state1_default: {
					do_game = true;
				}
				xcase e_game_state1_defeat: {
				}
			}
		} break;

		default: {}
	}

	for(int i = 0; i < c_max_entities; i += 1) {
		if(entity_arr->active[i]) {
			entity_arr->data[i].prev_pos = entity_arr->data[i].pos;
		}
	}

	if(soft_data->frames_to_freeze > 0) {
		soft_data->frames_to_freeze -= 1;
		return;
	}

	if(do_game) {
		game->update_time += (float)c_update_delay;
		hard_data->update_count += 1;
		soft_data->update_count += 1;
	}

	if(0) {
		// soft_data->boss_defeated_timestamp = 0;
		if(game->leaderboard_nice_name.count <= 0 && c_on_web) {
			add_temporary_state_transition(&game->state0, e_game_state0_input_name, game->render_time, c_transition_time);
		}
		else if(!soft_data->tried_to_submit_score) {
			soft_data->tried_to_submit_score = true;
			add_state_transition(&game->state0, e_game_state0_win_leaderboard, game->render_time, c_transition_time);
			game->update_count_at_win_time = hard_data->update_count;

			#if defined(__EMSCRIPTEN__)
			submit_leaderboard_score(hard_data->update_count, c_leaderboard_id);
			#elif defined(m_winhttp)
			get_leaderboard(c_leaderboard_id);
			#endif
		}
	}
	memset(&soft_data->frame_data, 0, sizeof(soft_data->frame_data));
}

func void render(float interp_dt, float delta)
{
	reset_linear_arena(&game->render_frame_arena);

	#if defined(_WIN32)
	while(g_platform_data->hot_read_index[1] < g_platform_data->hot_write_index) {
		char* path = g_platform_data->hot_file_arr[g_platform_data->hot_read_index[1] % c_max_hot_files];
		if(strstr(path, ".shader")) {
			game->reload_shaders = true;
		}
		g_platform_data->hot_read_index[1] += 1;
	}
	#endif // _WIN32

	if(game->reload_shaders) {
		game->reload_shaders = false;

		for(int i = 0; i < e_shader_count; i += 1) {
			s_shader shader = load_shader_from_file(c_shader_path_arr[i], &game->render_frame_arena);
			if(shader.id > 0) {
				if(game->shader_arr[i].id > 0) {
					gl(glDeleteProgram(game->shader_arr[i].id));
				}
				game->shader_arr[i] = shader;

				#if defined(m_debug)
				printf("Loaded %s\n", c_shader_path_arr[i]);
				#endif // m_debug
			}
		}
	}

	for(int render_pass_i = 0; render_pass_i < c_render_pass_count; render_pass_i += 1) {
		for(int i = 0; i < e_shader_count; i += 1) {
			for(int j = 0; j < e_texture_count; j += 1) {
				for(int k = 0; k < e_mesh_count; k += 1) {
					game->render_pass_arr[render_pass_i].render_group_index_arr[i][j][k] = -1;
				}
			}
		}
		game->render_pass_arr[render_pass_i].render_group_arr.count = 0;
		memset(game->render_pass_arr[render_pass_i].render_instance_count, 0, sizeof(game->render_pass_arr[render_pass_i].render_instance_count));
	}

	s_hard_game_data* hard_data = &game->hard_data;
	s_soft_game_data* soft_data = &game->soft_data;

	s_m4 ortho = make_orthographic(0, c_world_size.x, c_world_size.y, 0, -100, 100);

	bind_framebuffer(0);
	clear_framebuffer_depth(0);
	clear_framebuffer_color(0, v4(0.0f, 0, 0, 0));

	e_game_state0 state0 = (e_game_state0)get_state(&game->state0);

	b8 do_game = false;
	b8 do_defeat = false;

	float wanted_speed = get_wanted_game_speed(interp_dt);

	{
		if(game->disable_music) {
			game->music_volume.target = 0;
		}
		else {
			game->music_volume.target = 1;
		}
		do_lerpable_snap(&game->music_volume, delta * 2, 0.01f);
		s_active_sound* music = find_playing_sound(e_sound_music);
		assert(music);
		if(music) {
			music->data.volume = game->music_volume.curr;
		}
	}

	switch(state0) {

		// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		main menu start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
		case e_game_state0_main_menu: {
			game->speed = 0;
			game->music_speed.target = 1;

			if(do_button(S("Play"), wxy(0.5f, 0.5f), true) == e_button_result_left_click || is_key_pressed(SDLK_RETURN, true)) {
				add_state_transition(&game->state0, e_game_state0_play, game->render_time, c_transition_time);
				game->do_hard_reset = true;
			}

			if(do_button(S("Leaderboard"), wxy(0.5f, 0.6f), true) == e_button_result_left_click) {
				#if defined(__EMSCRIPTEN__) || defined(m_winhttp)
				get_leaderboard(c_leaderboard_id);
				#endif
				add_state_transition(&game->state0, e_game_state0_leaderboard, game->render_time, c_transition_time);
			}

			if(do_button(S("Options"), wxy(0.5f, 0.7f), true) == e_button_result_left_click) {
				add_state_transition(&game->state0, e_game_state0_options, game->render_time, c_transition_time);
			}

			draw_text(c_game_name, wxy(0.5f, 0.2f), 128, make_rrr(1), true, &game->font, zero, 0);
			draw_text(S("www.twitch.tv/Tkap1"), wxy(0.5f, 0.3f), 32, make_rrr(0.6f), true, &game->font, zero, 0);

			if(c_on_web) {
				s_v4 color = hsv_to_rgb(game->render_time * 360, 1, 1);
				draw_text(S("Go fullscreen!\n             V"), wxy(0.9f, 0.93f), sin_range(32, 40, game->render_time * 8), color, true, &game->font, zero, 0);
			}

			{
				s_render_flush_data data = make_render_flush_data(zero, zero);
				data.projection = ortho;
				data.blend_mode = e_blend_mode_normal;
				data.depth_mode = e_depth_mode_no_read_yes_write;
				render_flush(data, true, 0);
			}

		} break;
		// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		main menu end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

		// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		pause menu start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
		case e_game_state0_pause: {
			game->speed = 0;
			game->music_speed.target = 1;


			if(do_button(S("Resume"), wxy(0.5f, 0.5f), true) == e_button_result_left_click || is_key_pressed(SDLK_RETURN, true)) {
				pop_state_transition(&game->state0, game->render_time, c_transition_time);
			}

			if(do_button(S("Leaderboard"), wxy(0.5f, 0.6f), true) == e_button_result_left_click) {
				#if defined(__EMSCRIPTEN__)
				get_leaderboard(c_leaderboard_id);
				#endif
				add_state_transition(&game->state0, e_game_state0_leaderboard, game->render_time, c_transition_time);
			}

			if(do_button(S("Options"), wxy(0.5f, 0.7f), true) == e_button_result_left_click) {
				add_state_transition(&game->state0, e_game_state0_options, game->render_time, c_transition_time);
			}

			draw_text(c_game_name, wxy(0.5f, 0.2f), 128, make_rrr(1), true, &game->font, zero, 0);
			draw_text(S("www.twitch.tv/Tkap1"), wxy(0.5f, 0.3f), 32, make_rrr(0.6f), true, &game->font, zero, 0);

			{
				s_render_flush_data data = make_render_flush_data(zero, zero);
				data.projection = ortho;
				data.blend_mode = e_blend_mode_normal;
				data.depth_mode = e_depth_mode_no_read_yes_write;
				render_flush(data, true, 0);
			}

		} break;
		// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		pause menu end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

		// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		leaderboard start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
		case e_game_state0_leaderboard: {
			game->speed = 0;
			game->music_speed.target = 1;
			do_leaderboard();

			{
				s_render_flush_data data = make_render_flush_data(zero, zero);
				data.projection = ortho;
				data.blend_mode = e_blend_mode_normal;
				data.depth_mode = e_depth_mode_no_read_yes_write;
				render_flush(data, true, 0);
			}

		} break;

		case e_game_state0_win_leaderboard: {
			game->speed = 0;
			game->music_speed.target = 1;
			do_leaderboard();

			{
				s_time_format data = update_count_to_time_format(game->update_count_at_win_time);
				s_len_str text = format_text("%02i:%02i.%03i", data.minutes, data.seconds, data.milliseconds);
				draw_text(text, c_world_center * v2(1.0f, 0.2f), 64, make_rrr(1), true, &game->font, zero, 0);

				draw_text(S("Press R to restart..."), c_world_center * v2(1.0f, 0.4f), sin_range(48, 60, game->render_time * 8.0f), make_rrr(0.66f), true, &game->font, zero, 0);
			}


			b8 want_to_reset = is_key_pressed(SDLK_r, true);
			if(
				do_button(S("Restart"), c_world_size * v2(0.87f, 0.82f), true) == e_button_result_left_click
				|| is_key_pressed(SDLK_ESCAPE, true) || want_to_reset
			) {
				pop_state_transition(&game->state0, game->render_time, c_transition_time);
				game->do_hard_reset = true;
			}

			{
				s_render_flush_data data = make_render_flush_data(zero, zero);
				data.projection = ortho;
				data.blend_mode = e_blend_mode_normal;
				data.depth_mode = e_depth_mode_no_read_yes_write;
				render_flush(data, true, 0);
			}

		} break;
		// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		leaderboard end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

		// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		options start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
		case e_game_state0_options: {
			game->speed = 0;
			game->music_speed.target = 1;

			s_v2 pos = wxy(0.5f, 0.12f);
			s_v2 button_size = v2(600, 48);

			{
				s_len_str text = format_text("Sound: %s", game->turn_off_all_sounds ? "Off" : "On");
				do_bool_button_ex(text, pos, button_size, true, &game->turn_off_all_sounds);
				pos.y += 80;
			}

			{
				s_len_str text = format_text("Music: %s", game->disable_music ? "Off" : "On");
				do_bool_button_ex(text, pos, button_size, true, &game->disable_music);
				pos.y += 80;
			}

			{
				s_len_str text = format_text("Timer: %s", game->hide_timer ? "Off" : "On");
				do_bool_button_ex(text, pos, button_size, true, &game->hide_timer);
				pos.y += 80;
			}

			{
				s_len_str text = format_text("Lights: %s", game->disable_lights ? "Off" : "On");
				do_bool_button_ex(text, pos, button_size, true, &game->disable_lights);
				pos.y += 80;
			}

			{
				s_len_str text = format_text("Damage numbers: %s", game->disable_damage_numbers ? "Off" : "On");
				do_bool_button_ex(text, pos, button_size, true, &game->disable_damage_numbers);
				pos.y += 80;
			}

			{
				s_len_str text = format_text("Gold numbers: %s", game->disable_gold_numbers ? "Off" : "On");
				do_bool_button_ex(text, pos, button_size, true, &game->disable_gold_numbers);
				pos.y += 80;
			}

			{
				s_len_str text = format_text("Auto dash when 0 cooldown: %s", game->disable_auto_dash_when_no_cooldown ? "Off" : "On");
				do_bool_button_ex(text, pos, button_size, true, &game->disable_auto_dash_when_no_cooldown);
				pos.y += 80;
			}

			b8 escape = is_key_pressed(SDLK_ESCAPE, true);
			if(do_button(S("Back"), wxy(0.87f, 0.92f), true) == e_button_result_left_click || escape) {
				pop_state_transition(&game->state0, game->render_time, c_transition_time);
			}

			{
				s_render_flush_data data = make_render_flush_data(zero, zero);
				data.projection = ortho;
				data.blend_mode = e_blend_mode_normal;
				data.depth_mode = e_depth_mode_no_read_yes_write;
				render_flush(data, true, 0);
			}
		} break;
		// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		options end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

		// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		play start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
		case e_game_state0_play: {
			game->speed = wanted_speed;
			game->music_speed.target = 1;

			handle_state(&hard_data->state1, game->render_time);

			e_game_state1 state1 = (e_game_state1)get_state(&hard_data->state1);
			switch(state1) {
				xcase e_game_state1_default: {
					do_game = true;
				}
				xcase e_game_state1_defeat: {
					do_game = true;
					do_defeat = true;
					game->music_speed.target = 0.5f;
				}
			}

		} break;
		// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		play end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

		case e_game_state0_input_name: {
			game->speed = 0;
			game->music_speed.target = 1;
			s_input_name_state* state = &game->input_name_state;
			float font_size = 36;
			s_v2 pos = c_world_size * v2(0.5f, 0.4f);

			int count_before = state->name.str.count;
			b8 submitted = handle_string_input(&state->name, game->render_time);
			int count_after = state->name.str.count;
			if(count_before != count_after) {
				play_sound(e_sound_key, zero);
			}
			if(submitted) {
				b8 can_submit = true;
				if(state->name.str.count < 2) {
					can_submit = false;
					cstr_into_builder(&state->error_str, "Name must have at least 2 characters!");
				}
				if(can_submit) {
					state->error_str.count = 0;
					#if defined(__EMSCRIPTEN__)
					set_leaderboard_name(builder_to_len_str(&state->name.str));
					#endif
					{
						s_len_str temp = builder_to_len_str(&state->name.str);
						game->leaderboard_nice_name.count = 0;
						builder_add(&game->leaderboard_nice_name, "%.*s", expand_str(temp));
					}
				}
			}

			draw_text(S("Victory!"), c_world_size * v2(0.5f, 0.1f), font_size, make_rrr(1), true, &game->font, zero, 0);
			draw_text(S("Enter your name"), c_world_size * v2(0.5f, 0.2f), font_size, make_rrr(1), true, &game->font, zero, 0);
			if(state->error_str.count > 0) {
				draw_text(builder_to_len_str(&state->error_str), c_world_size * v2(0.5f, 0.3f), font_size, hex_to_rgb(0xD77870), true, &game->font, zero, 0);
			}

			if(state->name.str.count > 0) {
				draw_text(builder_to_len_str(&state->name.str), pos, font_size, make_rrr(1), true, &game->font, zero, 0);
			}

			s_v2 full_text_size = get_text_size(builder_to_len_str(&state->name.str), &game->font, font_size);
			s_v2 partial_text_size = get_text_size_with_count(builder_to_len_str(&state->name.str), &game->font, font_size, state->name.cursor.value, 0);
			s_v2 cursor_pos = v2(
				-full_text_size.x * 0.5f + pos.x + partial_text_size.x,
				pos.y - font_size * 0.5f
			);

			s_v2 cursor_size = v2(15.0f, font_size);
			float t = game->render_time - max(state->name.last_action_time, state->name.last_edit_time);
			b8 blink = false;
			constexpr float c_blink_rate = 0.75f;
			if(t > 0.75f && fmodf(t, c_blink_rate) >= c_blink_rate / 2) {
				blink = true;
			}
			float t2 = clamp(game->render_time - state->name.last_edit_time, 0.0f, 1.0f);
			s_v4 color = lerp_color(hex_to_rgb(0xffdddd), multiply_rgb_clamped(hex_to_rgb(0xABC28F), 0.8f), 1 - powf(1 - t2, 3));
			float extra_height = ease_out_elastic2_advanced(t2, 0, 0.75f, 20, 0);
			cursor_size.y += extra_height;

			if(!state->name.visual_pos_initialized) {
				state->name.visual_pos_initialized = true;
				state->name.cursor_visual_pos = cursor_pos;
			}
			else {
				state->name.cursor_visual_pos = lerp_snap(state->name.cursor_visual_pos, cursor_pos, delta * 20);
			}

			if(!blink) {
				draw_rect_topleft(state->name.cursor_visual_pos - v2(0.0f, extra_height / 2), cursor_size, color, 0);
			}

			s_render_flush_data data = make_render_flush_data(zero, zero);
			data.projection = ortho;
			data.blend_mode = e_blend_mode_normal;
			data.depth_mode = e_depth_mode_no_read_no_write;
			render_flush(data, true, 0);

		} break;
	}

	if(do_game) {
		b8 do_game_ui = true;
		auto entity_arr = &soft_data->entity_arr;

		update_particles(delta, true, 0);

		{
			s_render_flush_data data = make_render_flush_data(zero, zero);
			data.projection = ortho;
			data.blend_mode = e_blend_mode_additive;
			data.depth_mode = e_depth_mode_no_read_no_write;
			render_flush(data, true, 0);
		}

		if(do_game_ui) {

			s_rect rect = {
				c_game_area.x, 176.0f, c_world_size.x - c_game_area.x, c_world_size.y
			};
			s_v2 button_size = v2(320, 48);
			s_container container = make_down_center_x_container(rect, button_size, 10);

			{
				s_render_flush_data data = make_render_flush_data(zero, zero);
				data.projection = ortho;
				data.blend_mode = e_blend_mode_normal;
				data.depth_mode = e_depth_mode_read_and_write;
				render_flush(data, true, 0);
			}

			{
				s_render_flush_data data = make_render_flush_data(zero, zero);
				data.projection = ortho;
				data.blend_mode = e_blend_mode_normal;
				data.depth_mode = e_depth_mode_read_and_write;
				render_flush(data, true, 0);
			}

			if(game->tooltip.count > 0) {
				float font_size = 32;
				s_v2 size = get_text_size(game->tooltip, &game->font, font_size);
				s_v2 rect_pos = zero;
				rect_pos -= v2(8);
				size += v2(16);
				rect_pos = topleft_to_bottomleft_mouse(rect_pos, size, g_mouse);
				rect_pos = prevent_offscreen(rect_pos, size);
				s_v2 text_pos = rect_pos + v2(8);
				draw_rect_topleft(rect_pos, size, make_ra(0.0f, 0.95f), 0);
				draw_text(game->tooltip, text_pos, font_size, make_rrr(1), false, &game->font, zero, 0);
				{
					s_render_flush_data data = make_render_flush_data(zero, zero);
					data.projection = ortho;
					data.blend_mode = e_blend_mode_normal;
					data.depth_mode = e_depth_mode_no_read_no_write;
					render_flush(data, true, 0);
				}
			}
			game->tooltip = zero;
		}

		// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		cheat menu start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
		#if defined(m_debug)
		if(game->cheat_menu_enabled) {
			s_rect rect = {
				10, 10, 350, c_world_size.y
			};
			s_v2 size = v2(320, 48);
			s_container container = make_down_center_x_container(rect, size, 10);
		}
		#endif
		// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		cheat menu end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	}
	else {
		update_particles(delta, false, 0);
	}

	if(do_defeat) {

		draw_rect_topleft(v2(0), c_world_size, make_ra(0, 0.75f), 0);
		draw_text(S("Defeat"), wxy(0.5f, 0.4f) + rand_v2_11(&game->rng) * 8, 128, make_rgb(1, 0.2f, 0.2f), true, &game->font, zero, 0);
		draw_text(S("Press R to try again"), wxy(0.5f, 0.55f), sin_range(48, 64, game->render_time * 8), make_rrr(1), true, &game->font, zero, 0);

		{
			s_render_flush_data data = make_render_flush_data(zero, zero);
			data.projection = ortho;
			data.blend_mode = e_blend_mode_normal;
			data.depth_mode = e_depth_mode_no_read_no_write;
			render_flush(data, true, 0);
		}
	}

	s_state_transition transition = get_state_transition(&game->state0, game->render_time);
	if(transition.active) {
		{
			float alpha = 0;
			if(transition.time_data.percent <= 0.5f) {
				alpha = transition.time_data.percent * 2;
			}
			else {
				alpha = transition.time_data.inv_percent * 2;
			}
			s_instance_data data = zero;
			data.model = fullscreen_m4();
			data.color = make_rgba(0.0f, 0, 0, alpha);
			add_to_render_group(data, e_shader_flat, e_texture_white, e_mesh_quad, 0);
		}

		{
			s_render_flush_data data = make_render_flush_data(zero, zero);
			data.projection = ortho;
			data.blend_mode = e_blend_mode_normal;
			data.depth_mode = e_depth_mode_no_read_no_write;
			render_flush(data, true, 0);
		}
	}

	{
		do_lerpable_snap(&game->music_speed, delta * 2.0f, 0.01f);
		s_active_sound* music = find_playing_sound(e_sound_music);
		if(music) {
			music->data.speed = game->music_speed.curr;
		}
	}

	SDL_GL_SwapWindow(g_platform_data->window);

	game->render_time += delta;
}

func f64 get_seconds()
{
	u64 now =	SDL_GetPerformanceCounter();
	return (now - g_platform_data->start_cycles) / (f64)g_platform_data->cycle_frequency;
}

func void on_gl_error(const char* expr, char* file, int line, int error)
{
	#define m_gl_errors \
	X(GL_INVALID_ENUM, "GL_INVALID_ENUM") \
	X(GL_INVALID_VALUE, "GL_INVALID_VALUE") \
	X(GL_INVALID_OPERATION, "GL_INVALID_OPERATION") \
	X(GL_STACK_OVERFLOW, "GL_STACK_OVERFLOW") \
	X(GL_STACK_UNDERFLOW, "GL_STACK_UNDERFLOW") \
	X(GL_OUT_OF_MEMORY, "GL_STACK_OUT_OF_MEMORY") \
	X(GL_INVALID_FRAMEBUFFER_OPERATION, "GL_STACK_INVALID_FRAME_BUFFER_OPERATION")

	const char* error_str;
	#define X(a, b) case a: { error_str = b; } break;
	switch(error) {
		m_gl_errors
		default: {
			error_str = "unknown error";
		} break;
	}
	#undef X
	#undef m_gl_errors

	printf("GL ERROR: %s - %i (%s)\n", expr, error, error_str);
	printf("  %s(%i)\n", file, line);
	printf("--------\n");

	#if defined(_WIN32)
	__debugbreak();
	#else
	__builtin_trap();
	#endif
}

func void draw_rect(s_v2 pos, s_v2 size, s_v4 color, int render_pass_index)
{
	s_instance_data data = zero;
	data.model = m4_translate(v3(pos, 0));
	data.model = m4_multiply(data.model, m4_scale(v3(size, 1)));
	data.color = color;

	add_to_render_group(data, e_shader_flat, e_texture_white, e_mesh_quad, render_pass_index);
}

func void draw_rect_topleft(s_v2 pos, s_v2 size, s_v4 color, int render_pass_index)
{
	pos += size * 0.5f;
	draw_rect(pos, size, color, render_pass_index);
}

func void draw_texture_screen(s_v2 pos, s_v2 size, s_v4 color, e_texture texture_id, e_shader shader_id, s_v2 uv_min, s_v2 uv_max, s_draw_data draw_data, int render_pass_index)
{
	s_instance_data data = zero;
	data.model = m4_translate(v3(pos, draw_data.z));
	data.model = m4_multiply(data.model, m4_scale(v3(size, 1)));
	data.color = color;
	data.uv_min = uv_min;
	data.uv_max = uv_max;

	add_to_render_group(data, shader_id, texture_id, e_mesh_quad, render_pass_index);
}

func void draw_mesh(e_mesh mesh_id, s_m4 model, s_v4 color, e_shader shader_id, int render_pass_index)
{
	s_instance_data data = zero;
	data.model = model;
	data.color = color;
	add_to_render_group(data, shader_id, e_texture_white, mesh_id, render_pass_index);
}

func void draw_mesh(e_mesh mesh_id, s_v3 pos, s_v3 size, s_v4 color, e_shader shader_id, int render_pass_index)
{
	s_m4 model = m4_translate(pos);
	model = m4_multiply(model, m4_scale(size));
	draw_mesh(mesh_id, model, color, shader_id, render_pass_index);
}

func void bind_framebuffer(u32 fbo)
{
	if(game->curr_fbo != fbo) {
		gl(glBindFramebuffer(GL_FRAMEBUFFER, fbo));
		game->curr_fbo = fbo;
	}
}

func void clear_framebuffer_color(u32 fbo, s_v4 color)
{
	bind_framebuffer(fbo);
	glClearColor(color.x, color.y, color.z, color.w);
	glClear(GL_COLOR_BUFFER_BIT);
}

func void clear_framebuffer_depth(u32 fbo)
{
	bind_framebuffer(fbo);
	set_depth_mode(e_depth_mode_read_and_write);
	glClear(GL_DEPTH_BUFFER_BIT);
}

func void render_flush(s_render_flush_data data, b8 reset_render_count, int render_pass_index)
{
	bind_framebuffer(data.fbo.id);

	assert(render_pass_index >= 0);
	assert(render_pass_index < c_render_pass_count);
	s_render_pass* render_pass = &game->render_pass_arr[render_pass_index];

	if(data.fbo.id == 0) {
		s_rect letterbox = do_letterbox(v2(g_platform_data->window_size), c_world_size);
		glViewport((int)letterbox.x, (int)letterbox.y, (int)letterbox.w, (int)letterbox.h);
	}
	else {
		glViewport(0, 0, data.fbo.size.x, data.fbo.size.y);
	}

	set_cull_mode(data.cull_mode);
	set_depth_mode(data.depth_mode);
	set_blend_mode(data.blend_mode);

	{
		gl(glBindBuffer(GL_UNIFORM_BUFFER, game->ubo));
		s_uniform_data uniform_data = zero;
		uniform_data.view = data.view;
		uniform_data.projection = data.projection;
		uniform_data.light_view = data.light_view;
		uniform_data.light_projection = data.light_projection;
		uniform_data.render_time = game->render_time;
		uniform_data.cam_pos = data.cam_pos;
		uniform_data.mouse = g_mouse;
		uniform_data.player_pos = data.player_pos;
		uniform_data.window_size.x = (float)g_platform_data->window_size.x;
		uniform_data.window_size.y = (float)g_platform_data->window_size.y;
		gl(glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(s_uniform_data), &uniform_data));
	}

	foreach_val(group_i, group, render_pass->render_group_arr) {
		s_mesh* mesh = &game->mesh_arr[group.mesh_id];
		int* instance_count = &render_pass->render_instance_count[group.shader_id][group.texture_id][group.mesh_id];
		assert(*instance_count > 0);
		void* instance_data = render_pass->render_instance_arr[group.shader_id][group.texture_id][group.mesh_id];

		gl(glUseProgram(game->shader_arr[group.shader_id].id));

		int in_texture_loc = glGetUniformLocation(game->shader_arr[group.shader_id].id, "in_texture");
		int noise_loc = glGetUniformLocation(game->shader_arr[group.shader_id].id, "noise");
		if(in_texture_loc >= 0) {
			glUniform1i(in_texture_loc, 0);
			glActiveTexture(GL_TEXTURE0);
			gl(glBindTexture(GL_TEXTURE_2D, game->texture_arr[group.texture_id].id));
		}
		if(noise_loc >= 0) {
			glUniform1i(noise_loc, 2);
			glActiveTexture(GL_TEXTURE2);
			gl(glBindTexture(GL_TEXTURE_2D, game->texture_arr[e_texture_noise].id));
		}

		gl(glBindVertexArray(mesh->vao));
		gl(glBindBuffer(GL_ARRAY_BUFFER, mesh->instance_vbo.id));
		gl(glBufferSubData(GL_ARRAY_BUFFER, 0, group.element_size * *instance_count, instance_data));
		gl(glDrawArraysInstanced(GL_TRIANGLES, 0, mesh->vertex_count, *instance_count));
		if(reset_render_count) {
			render_pass->render_group_arr.remove_and_swap(group_i);
			render_pass->render_group_index_arr[group.shader_id][group.texture_id][group.mesh_id] = -1;
			group_i -= 1;
			*instance_count = 0;
		}
	}
}

template <typename t>
func void add_to_render_group(t data, e_shader shader_id, e_texture texture_id, e_mesh mesh_id, int render_pass_index)
{
	assert(render_pass_index >= 0);
	assert(render_pass_index < c_render_pass_count);
	s_render_pass* render_pass = &game->render_pass_arr[render_pass_index];

	s_render_group render_group = zero;
	render_group.shader_id = shader_id;
	render_group.texture_id = texture_id;
	render_group.mesh_id = mesh_id;
	render_group.element_size = sizeof(t);

	s_mesh* mesh = &game->mesh_arr[render_group.mesh_id];

	int* render_group_index = &render_pass->render_group_index_arr[render_group.shader_id][render_group.texture_id][render_group.mesh_id];
	if(*render_group_index < 0) {
		render_pass->render_group_arr.add(render_group);
		*render_group_index = render_pass->render_group_arr.count - 1;
	}
	int* count = &render_pass->render_instance_count[render_group.shader_id][render_group.texture_id][render_group.mesh_id];
	int* max_elements = &render_pass->render_instance_max_elements[render_group.shader_id][render_group.texture_id][render_group.mesh_id];
	t* ptr = (t*)render_pass->render_instance_arr[render_group.shader_id][render_group.texture_id][render_group.mesh_id];
	b8 expand = *max_elements <= *count;
	b8 get_new_ptr = *count <= 0 || expand;
	int new_max_elements = *max_elements;
	if(expand) {
		if(new_max_elements <= 0) {
			new_max_elements = 64;
		}
		else {
			new_max_elements *= 2;
		}
		if(new_max_elements > mesh->instance_vbo.max_elements) {
			gl(glBindBuffer(GL_ARRAY_BUFFER, mesh->instance_vbo.id));
			gl(glBufferData(GL_ARRAY_BUFFER, sizeof(t) * new_max_elements, null, GL_DYNAMIC_DRAW));
			mesh->instance_vbo.max_elements = new_max_elements;
		}
	}
	if(get_new_ptr) {
		t* temp = (t*)arena_alloc(&game->render_frame_arena, sizeof(t) * new_max_elements);
		if(*count > 0) {
			memcpy(temp, ptr, *count * sizeof(t));
		}
		render_pass->render_instance_arr[render_group.shader_id][render_group.texture_id][render_group.mesh_id] = (void*)temp;
		ptr = temp;
	}
	*max_elements = new_max_elements;
	*(ptr + *count) = data;
	*count += 1;
}

func s_shader load_shader_from_file(char* file, s_linear_arena* arena)
{
	b8 delete_shaders[2] = {true, true};
	char* src = (char*)read_file(file, arena, null);
	assert(src);

	u32 shader_arr[] = {glCreateShader(GL_VERTEX_SHADER), glCreateShader(GL_FRAGMENT_SHADER)};

	#if defined(__EMSCRIPTEN__)
	const char* header = "#version 300 es\nprecision highp float;\n";
	#else
	const char* header = "#version 330 core\n";
	#endif

	char* shared_src = (char*)try_really_hard_to_read_file("src/shader_shared.h", arena);
	assert(shared_src);

	for(int i = 0; i < 2; i += 1) {
		const char* src_arr[] = {header, "", "", shared_src, src};
		if(i == 0) {
			src_arr[1] = "#define m_vertex 1\n";
			src_arr[2] = "#define shared_var out\n";
		}
		else {
			src_arr[1] = "#define m_fragment 1\n";
			src_arr[2] = "#define shared_var in\n";
		}
		gl(glShaderSource(shader_arr[i], array_count(src_arr), (const GLchar * const *)src_arr, null));
		gl(glCompileShader(shader_arr[i]));

		int compile_success;
		char info_log[1024];
		gl(glGetShaderiv(shader_arr[i], GL_COMPILE_STATUS, &compile_success));

		if(!compile_success) {
			gl(glGetShaderInfoLog(shader_arr[i], sizeof(info_log), null, info_log));
			printf("Failed to compile shader: %s\n%s", file, info_log);
			delete_shaders[i] = false;
		}
	}

	b8 detach_shaders = delete_shaders[0] && delete_shaders[1];

	u32 program = 0;
	if(delete_shaders[0] && delete_shaders[1]) {
		program = gl(glCreateProgram());
		gl(glAttachShader(program, shader_arr[0]));
		gl(glAttachShader(program, shader_arr[1]));
		gl(glLinkProgram(program));

		int linked = 0;
		gl(glGetProgramiv(program, GL_LINK_STATUS, &linked));
		if(!linked) {
			char info_log[1024] = zero;
			gl(glGetProgramInfoLog(program, sizeof(info_log), null, info_log));
			printf("FAILED TO LINK: %s\n", info_log);
		}
	}

	if(detach_shaders) {
		gl(glDetachShader(program, shader_arr[0]));
		gl(glDetachShader(program, shader_arr[1]));
	}

	if(delete_shaders[0]) {
		gl(glDeleteShader(shader_arr[0]));
	}
	if(delete_shaders[1]) {
		gl(glDeleteShader(shader_arr[1]));
	}

	s_shader result = zero;
	result.id = program;
	return result;
}

func e_button_result do_button(s_len_str text, s_v2 pos, b8 centered)
{
	s_v2 size = v2(256, 48);
	e_button_result result = do_button_ex(text, pos, size, centered, zero);
	return result;
}

func e_button_result do_button_ex(s_len_str text, s_v2 pos, s_v2 size, b8 centered, s_button_data optional)
{
	e_button_result result = e_button_result_none;
	if(!centered) {
		pos += size * 0.5f;
	}

	b8 do_tooltip = false;
	b8 hovered = mouse_vs_rect_center(g_mouse, pos, size);
	s_v4 color = optional.button_color;
	s_v4 text_color = make_rrr(1);
	if(optional.disabled) {
		color = multiply_rgb(color, 0.6f);
		text_color = make_rrr(0.7f);
	}
	if(hovered) {
		do_tooltip = true;
		if(!optional.disabled) {
			size += v2(8);
			if(!centered) {
				pos -= v2(4) * 0.5f;
			}
			color = multiply_rgb(color, 2);
			if(g_left_click) {
				result = e_button_result_left_click;
				if(!optional.mute_click_sound) {
					play_sound(e_sound_click, zero);
				}
			}
		}
		if(g_right_click) {
			result = e_button_result_right_click;
		}
	}

	{
		s_instance_data data = zero;
		data.model = m4_translate(v3(pos, 0));
		data.model = m4_multiply(data.model, m4_scale(v3(size, 1)));
		data.color = color;
		add_to_render_group(data, e_shader_button, e_texture_white, e_mesh_quad, optional.render_pass_index);
	}

	draw_text(text, pos, optional.font_size, text_color, true, &game->font, {.z = 1}, optional.render_pass_index);

	if(do_tooltip && optional.tooltip.count > 0) {
		game->tooltip = optional.tooltip;
	}

	return result;
}

func b8 do_bool_button(s_len_str text, s_v2 pos, b8 centered, b8* out)
{
	s_v2 size = v2(256, 48);
	b8 result = do_bool_button_ex(text, pos, size, centered, out);
	return result;
}

func b8 do_bool_button_ex(s_len_str text, s_v2 pos, s_v2 size, b8 centered, b8* out)
{
	assert(out);
	b8 result = false;
	if(do_button_ex(text, pos, size, centered, zero) == e_button_result_left_click) {
		result = true;
		*out = !(*out);
	}
	return result;
}

func b8 is_key_pressed(int key, b8 consume)
{
	b8 result = false;
	if(key < c_max_keys) {
		result = game->input_arr[key].half_transition_count > 1 || (game->input_arr[key].half_transition_count > 0 && game->input_arr[key].is_down);
	}
	if(result && consume) {
		game->input_arr[key].half_transition_count = 0;
		game->input_arr[key].is_down = false;
	}
	return result;
}

func b8 is_key_down(int key)
{
	b8 result = false;
	if(key < c_max_keys) {
		result = game->input_arr[key].half_transition_count > 1 || game->input_arr[key].is_down;
	}
	return result;
}

template <int n>
func void cstr_into_builder(s_str_builder<n>* builder, char* str)
{
	assert(str);

	int len = (int)strlen(str);
	assert(len <= n);
	memcpy(builder->str, str, len);
	builder->count = len;
}

template <int n>
func s_len_str builder_to_len_str(s_str_builder<n>* builder)
{
	s_len_str result = zero;
	result.str = builder->str;
	result.count = builder->count;
	return result;
}

template <int n>
func b8 handle_string_input(s_input_str<n>* str, float time)
{
	b8 result = false;
	if(!str->cursor.valid) {
		str->cursor = maybe(0);
	}
	foreach_val(c_i, c, game->char_events) {
		if(is_alpha_numeric(c) || c == '_') {
			if(!is_builder_full(&str->str)) {
				builder_insert_char(&str->str, str->cursor.value, c);
				str->cursor.value += 1;
				str->last_edit_time = time;
				str->last_action_time = str->last_edit_time;
			}
		}
	}

	foreach_val(event_i, event, game->key_events) {
		if(!event.went_down) { continue; }
		if(event.key == SDLK_RETURN) {
			result = true;
			str->last_action_time = time;
		}
		else if(event.key == SDLK_ESCAPE) {
			str->cursor.value = 0;
			str->str.count = 0;
			str->str.str[0] = 0;
			str->last_edit_time = time;
			str->last_action_time = str->last_edit_time;
		}
		else if(event.key == SDLK_BACKSPACE) {
			if(str->cursor.value > 0) {
				str->cursor.value -= 1;
				builder_remove_char_at(&str->str, str->cursor.value);
				str->last_edit_time = time;
				str->last_action_time = str->last_edit_time;
			}
		}
	}
	return result;
}

func void handle_key_event(int key, b8 is_down, b8 is_repeat)
{
	if(is_down) {
		game->any_key_pressed = true;
	}
	if(key < c_max_keys) {
		if(!is_repeat) {
			game->any_key_pressed = true;
		}

		{
			s_key_event key_event = {};
			key_event.went_down = is_down;
			key_event.key = key;
			// key_event.modifiers |= e_input_modifier_ctrl * is_key_down(&g_platform_data.input, c_key_left_ctrl);
			game->key_events.add(key_event);
		}
	}
}

func void do_leaderboard()
{
	b8 escape = is_key_pressed(SDLK_ESCAPE, true);
	if(do_button(S("Back"), wxy(0.87f, 0.92f), true) == e_button_result_left_click || escape) {
		s_maybe<int> prev = get_previous_non_temporary_state(&game->state0);
		if(prev.valid && prev.value == e_game_state0_pause) {
			pop_state_transition(&game->state0, game->render_time, c_transition_time);
		}
		else {
			add_state_transition(&game->state0, e_game_state0_main_menu, game->render_time, c_transition_time);
			clear_state(&game->state0);
		}
	}

	{
		if(!game->leaderboard_received) {
			draw_text(S("Getting leaderboard..."), c_world_center, 48, make_rrr(0.66f), true, &game->font, zero, 0);
		}
		else if(game->leaderboard_arr.count <= 0) {
			draw_text(S("No scores yet :("), c_world_center, 48, make_rrr(0.66f), true, &game->font, zero, 0);
		}

		s_v2 pos = c_world_center * v2(1.0f, 0.7f);
		int num_pages = ceilfi(game->leaderboard_arr.count / (float)c_leaderboard_visible_entries_per_page);
		b8 is_last_page = game->curr_leaderboard_page >= num_pages - 1;
		int start = (c_leaderboard_visible_entries_per_page * game->curr_leaderboard_page);
		int end = start + c_leaderboard_visible_entries_per_page;
		if(end > game->leaderboard_arr.count) {
			end = game->leaderboard_arr.count;
		}
		for(int entry_i = start; entry_i < end; entry_i += 1) {
			s_leaderboard_entry entry = game->leaderboard_arr[entry_i];
			s_time_format data = update_count_to_time_format(entry.time);
			s_v4 color = make_rrr(0.8f);
			if(builder_equals(&game->leaderboard_public_uid, &entry.internal_name)) {
				color = hex_to_rgb(0xD3A861);
			}
			char* name = entry.internal_name.str;
			if(entry.nice_name.count > 0) {
				name = entry.nice_name.str;
			}
			constexpr float font_size = 28;
			draw_text(format_text("%2i %s", entry_i + 1, name), v2(c_world_size.x * 0.1f, pos.y - font_size * 0.75f), font_size, color, false, &game->font, zero, 0);
			s_len_str text = format_text("%02i:%02i.%03i", data.minutes, data.seconds, data.milliseconds);
			draw_text(text, v2(c_world_size.x * 0.5f, pos.y - font_size * 0.75f), font_size, color, false, &game->font, zero, 0);
			pos.y += font_size * 1.5f;
		}
		{
			s_v2 button_size = v2(256, 48);
			{
				s_button_data optional = zero;
				if(game->curr_leaderboard_page <= 0) {
					optional.disabled = true;
				}
				if(do_button_ex(S("Previous"), wxy(0.25f, 0.92f), button_size, true, optional) == e_button_result_left_click) {
					game->curr_leaderboard_page -= 1;
				}
			}
			{
				s_button_data optional = zero;
				if(is_last_page) {
					optional.disabled = true;
				}
				if(do_button_ex(S("Next"), wxy(0.45f, 0.92f), button_size, true, optional) == e_button_result_left_click) {
					game->curr_leaderboard_page += 1;
				}
			}
		}
	}
}

func s_v2 get_rect_normal_of_closest_edge(s_v2 p, s_v2 center, s_v2 size)
{
	s_v2 edge_arr[] = {
		v2(center.x - size.x * 0.5f, center.y),
		v2(center.x + size.x * 0.5f, center.y),
		v2(center.x, center.y - size.y * 0.5f),
		v2(center.x, center.y + size.y * 0.5f),
	};
	s_v2 normal_arr[] = {
		v2(-1, 0),
		v2(1, 0),
		v2(0, -1),
		v2(0, 1),
	};
	float closest_dist[2] = {9999999, 9999999};
	int closest_index[2] = {0, 0};

	for(int i = 0; i < 4; i += 1) {
		float dist;
		if(i <= 1) {
			dist = fabsf(p.x - edge_arr[i].x);
		}
		else {
			dist = fabsf(p.y - edge_arr[i].y);
		}
		if(dist < closest_dist[0]) {
			closest_dist[0] = dist;
			closest_index[0] = i;
		}
		else if(dist < closest_dist[1]) {
			closest_dist[1] = dist;
			closest_index[1] = i;
		}
	}
	s_v2 result = normal_arr[closest_index[0]];

	// @Note(tkap, 19/04/2025): Probably breaks for very thin rectangles
	if(fabsf(closest_dist[0] - closest_dist[1]) <= 0.01f) {
		result += normal_arr[closest_index[1]];
		result = v2_normalized(result);
	}
	return result;
}

func b8 is_valid_2d_index(s_v2i index, int x_count, int y_count)
{
	b8 result = index.x >= 0 && index.x < x_count && index.y >= 0 && index.y < y_count;
	return result;
}

func b8 check_action(float curr_time, float timestamp, float grace)
{
	float passed = curr_time - timestamp;
	b8 result = passed <= grace && timestamp > 0;
	return result;
}

func void draw_atlas(s_v2 pos, s_v2 size, s_v2i index, s_v4 color, int render_pass_index)
{
	draw_atlas_ex(pos, size, index, color, 0, zero, render_pass_index);
}

func void draw_atlas_ex(s_v2 pos, s_v2 size, s_v2i index, s_v4 color, float rotation, s_draw_data draw_data, int render_pass_index)
{
	s_instance_data data = zero;
	data.model = m4_translate(v3(pos, draw_data.z));
	if(rotation != 0) {
		data.model *= m4_rotate(rotation, v3(0, 0, 1));
	}
	data.model *= m4_scale(v3(size, 1));
	data.color = color;
	int x = index.x * c_atlas_sprite_size + c_atlas_padding;
	data.uv_min.x = x / (float)c_atlas_size_v.x;
	data.uv_max.x = data.uv_min.x + (c_atlas_sprite_size - c_atlas_padding) / (float)c_atlas_size_v.x;
	int y = index.y * c_atlas_sprite_size + c_atlas_padding;
	data.uv_min.y = y / (float)(c_atlas_size_v.y);
	data.uv_max.y = data.uv_min.y + (c_atlas_sprite_size - c_atlas_padding) / (float)c_atlas_size_v.y;
	data.mix_weight = draw_data.mix_weight;
	data.mix_color = draw_data.mix_color;

	if(draw_data.flip_x) {
		swap(&data.uv_min.x, &data.uv_max.x);
	}

	add_to_render_group(data, e_shader_flat_remove_black, e_texture_atlas, e_mesh_quad, render_pass_index);
}

func void draw_atlas_topleft(s_v2 pos, s_v2 size, s_v2i index, s_v4 color, int render_pass_index)
{
	pos += size * 0.5f;
	draw_atlas(pos, size, index, color, render_pass_index);
}

func void draw_circle(s_v2 pos, float radius, s_v4 color, int render_pass_index)
{
	s_instance_data data = zero;
	data.model = m4_translate(v3(pos, 0));
	data.model = m4_multiply(data.model, m4_scale(v3(radius * 2, radius * 2, 1)));
	data.color = color;

	add_to_render_group(data, e_shader_circle, e_texture_white, e_mesh_quad, render_pass_index);
}

func void draw_light(s_v2 pos, float radius, s_v4 color, float smoothness, int render_pass_index)
{
	s_instance_data data = zero;
	data.model = m4_translate(v3(pos, 0));
	data.model = m4_multiply(data.model, m4_scale(v3(radius * 2, radius * 2, 1)));
	data.color = color;
	data.mix_weight = smoothness;

	add_to_render_group(data, e_shader_light, e_texture_white, e_mesh_quad, render_pass_index);
}

func void do_screen_shake(float intensity)
{
	s_soft_game_data* soft_data = &game->soft_data;
	soft_data->start_screen_shake_timestamp = game->render_time;
	soft_data->shake_intensity = intensity;
}

func void teleport_entity(s_entity* entity, s_v2 pos)
{
	entity->pos = pos;
	entity->prev_pos = pos;
}

func s_particle_emitter_a make_emitter_a()
{
	s_particle_emitter_a result = zero;
	result.particle_duration = 1;
	result.radius = 16;
	result.speed = 128;
	{
		s_particle_color color = zero;
		color.color = make_rrr(1);
		result.color_arr.add(color);
	}
	{
		s_particle_color color = zero;
		color.color = make_rrr(0.0f);
		color.percent = 1;
		result.color_arr.add(color);
	}
	return result;
}

func s_particle_emitter_b make_emitter_b()
{
	s_particle_emitter_b result = zero;
	result.particles_per_second = 1;
	result.particle_count = 1;
	return result;
}

func int add_emitter(s_entity emitter)
{
	s_soft_game_data* soft_data = &game->soft_data;
	emitter.emitter_b.creation_timestamp = game->render_time;
	emitter.emitter_b.last_emit_timestamp = game->render_time - 1.0f / emitter.emitter_b.particles_per_second;
	int index = entity_manager_add_if_not_full(&soft_data->entity_arr, e_entity_emitter, emitter);
	return index;
}

func s_v2 gxy(float x, float y)
{
	s_v2 result = {x * c_game_area.x, y * c_game_area.y};
	return result;
}

func s_v2 gxy(float x)
{
	s_v2 result = gxy(x, x);
	return result;
}


func s_container make_center_x_container(s_rect rect, s_v2 element_size, float padding, int element_count)
{
	assert(element_count > 0);

	s_container result = zero;
	float space_used = (element_size.x * element_count) + (padding * (element_count - 1));
	float space_left = rect.size.x - space_used;
	result.advance.x = element_size.x + padding;
	result.curr_pos.x = rect.pos.x + space_left * 0.5f;
	result.curr_pos.y = rect.pos.y + rect.size.y * 0.5f - element_size.y * 0.5f;
	return result;
}

func s_container make_forward_container(s_rect rect, s_v2 element_size, float padding)
{
	s_container result = zero;
	result.advance.x = element_size.x + padding;
	result.curr_pos.x = rect.pos.x + padding;
	result.curr_pos.y = rect.pos.y + rect.size.y * 0.5f - element_size.y * 0.5f;
	return result;
}

func s_container make_down_center_x_container(s_rect rect, s_v2 element_size, float padding)
{
	s_container result = zero;
	result.advance.y = element_size.y + padding;
	result.curr_pos.x = rect.pos.x + rect.size.x * 0.5f - element_size.x * 0.5f;
	result.curr_pos.y = rect.pos.y + padding;
	return result;
}

func s_container make_forward_align_right_container(s_rect rect, s_v2 element_size, float padding, float edge_padding, int element_count)
{
	s_container result = zero;
	float space_used = (element_size.x * element_count) + (padding * (element_count - 1));
	result.advance.x = element_size.x + padding;
	result.curr_pos.x = rect.size.x - space_used - edge_padding;
	result.curr_pos.y = rect.pos.y + rect.size.y * 0.5f - element_size.y * 0.5f;
	return result;
}

func s_v2 container_get_pos_and_advance(s_container* c)
{
	s_v2 result = c->curr_pos;
	c->curr_pos += c->advance;
	return result;
}

func s_v2 topleft_to_bottomleft_mouse(s_v2 pos, s_v2 size, s_v2 mouse)
{
	s_v2 result = pos;
	result += mouse;
	result.x -= pos.x;
	result.y -= pos.y;
	result.y -= size.y;
	return result;
}

func s_v2 prevent_offscreen(s_v2 pos, s_v2 size)
{
	s_v2 result = pos;
	float* ptr[4] = {
		&result.x, &result.x, &result.y, &result.y
	};
	// left, right, up, down
	float diff[4] = {
		pos.x - 0,
		c_world_size.x - (pos.x + size.x),
		pos.y - 0,
		c_world_size.y - (pos.y + size.y),
	};
	for(int i = 0; i < 4; i += 1) {
		if(diff[i] < 0) {
			float x = i % 2 == 0 ? -1.0f : 1.0f;
			*ptr[i] += diff[i] * x;
		}
	}
	return result;
}

func char* handle_plural(float x)
{
	char* result = "s";
	if(fabsf(x) == 1.0f) {
		result = "";
	}
	return result;
}

template <typename t, typename F>
func void radix_sort_32(t* source, u32 count, F get_radix, s_linear_arena* arena)
{
	if(count == 0) { return; }

	t* destination = (t*)arena_alloc(arena, sizeof(t) * count);

	for(u32 index_i = 0; index_i < 32; index_i += 8) {
		u32 offsets[256] = zero;

		// @Note(tkap, 02/07/2024): Count how many of each value between 0-255 we have
		// [0, 1, 1, 1, 2, 2] == [1, 3, 2]
		for(u32 i = 0; i < count; i += 1) {
			u32 val = get_radix(source[i]);
			u32 mask = 0xff << index_i;
			u32 radix_val = (val & mask) >> index_i;
			offsets[radix_val] += 1;
		}

		// @Note(tkap, 02/07/2024): Turn the offsets array from an array of counts into an array of offsets
		u32 total = 0;
		for(u32 i = 0; i < 256; i += 1) {
			u32 temp_count = offsets[i];
			offsets[i] = total;
			total += temp_count;
		}

		// @Note(tkap, 02/07/2024): Modify the new list
		for(u32 i = 0; i < count; i += 1) {
			u32 val = get_radix(source[i]);
			u32 mask = 0xff << index_i;
			u32 radix_val = (val & mask) >> index_i;

			u32 destination_index = offsets[radix_val];
			offsets[radix_val] += 1;
			destination[destination_index] = source[i];
		}

		t* swap_temp = destination;
		destination = source;
		source = swap_temp;
	}
}

func void draw_keycap(char c, s_v2 pos, s_v2 size, float alpha, int render_pass_index)
{
	pos += size * 0.5f;
	draw_atlas_ex(pos, size, v2i(124, 42), make_ra(1, alpha), 0, zero, render_pass_index);
	s_len_str str = format_text("%c", to_upper_case(c));
	pos.x -= size.x * 0.025f;
	pos.y -= size.x * 0.05f;
	s_v4 color = set_alpha(c_key_color, alpha);
	draw_text(str, pos, size.x, color, true, &game->font, {.z = 1}, render_pass_index);
}

func void add_multiplicative_light(s_v2 pos, float radius, s_v4 color, float smoothness)
{
	s_light light = zero;
	light.pos = pos;
	light.radius = radius;
	light.color = color;
	light.smoothness = smoothness;
	if(!game->multiplicative_light_arr.is_full()) {
		game->multiplicative_light_arr.add(light);
	}
}

func void add_additive_light(s_v2 pos, float radius, s_v4 color, float smoothness)
{
	s_light light = zero;
	light.pos = pos;
	light.radius = radius;
	light.color = color;
	light.smoothness = smoothness;
	if(!game->additive_light_arr.is_full()) {
		game->additive_light_arr.add(light);
	}
}

func s_entity make_entity()
{
	s_entity entity = zero;
	game->next_entity_id += 1;
	entity.id = game->next_entity_id;
	return entity;
}

func s_entity* get_entity(s_entity_ref ref)
{
	s_entity* result = null;
	if(ref.id > 0) {
		if(ref.index >= 0 && ref.index < c_max_entities) {
			if(game->soft_data.entity_arr.active[ref.index]) {
				if(game->soft_data.entity_arr.data[ref.index].id == ref.id) {
					result = &game->soft_data.entity_arr.data[ref.index];
				}
			}
		}
	}
	return result;
}

func float get_wanted_game_speed(float interp_dt)
{
	(void)interp_dt;
	float result = 1;
	// if(game->soft_data.hit_enemy_while_dashing_timestamp > 0) {
	// 	s_time_data time_data = get_time_data(update_time_to_render_time(game->update_time, interp_dt), game->soft_data.hit_enemy_while_dashing_timestamp, 0.04f);
	// 	time_data.percent = at_most(1.0f, time_data.percent);
	// 	result = ease_out_expo_advanced(time_data.percent, 0.75f, 1, 0.1f, 1);
	// 	if(time_data.percent >= 1) {
	// 		game->soft_data.hit_enemy_while_dashing_timestamp = 0;
	// 	}
	// }
	return result;
}

func s_active_sound* find_playing_sound(e_sound id)
{
	foreach_ptr(sound_i, sound, g_platform_data->active_sound_arr) {
		if(sound->loaded_sound_id == id) {
			return sound;
		}
	}
	return null;
}

func void do_lerpable_snap(s_lerpable* lerpable, float dt, float max_diff)
{
	lerpable->curr = lerp_snap(lerpable->curr, lerpable->target, dt, max_diff);
}
