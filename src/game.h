
#include "gen_meta/game.h.enums"

#if defined(__EMSCRIPTEN__)

#define m_gl_funcs \
X(PFNGLBUFFERDATAPROC, glBufferData) \
X(PFNGLBUFFERSUBDATAPROC, glBufferSubData) \
X(PFNGLGENVERTEXARRAYSPROC, glGenVertexArrays) \
X(PFNGLBINDVERTEXARRAYPROC, glBindVertexArray) \
X(PFNGLGENBUFFERSPROC, glGenBuffers) \
X(PFNGLBINDBUFFERPROC, glBindBuffer) \
X(PFNGLVERTEXATTRIBPOINTERPROC, glVertexAttribPointer) \
X(PFNGLVERTEXATTRIBIPOINTERPROC, glVertexAttribIPointer) \
X(PFNGLENABLEVERTEXATTRIBARRAYPROC, glEnableVertexAttribArray) \
X(PFNGLCREATESHADERPROC, glCreateShader) \
X(PFNGLSHADERSOURCEPROC, glShaderSource) \
X(PFNGLCREATEPROGRAMPROC, glCreateProgram) \
X(PFNGLATTACHSHADERPROC, glAttachShader) \
X(PFNGLLINKPROGRAMPROC, glLinkProgram) \
X(PFNGLCOMPILESHADERPROC, glCompileShader) \
X(PFNGLVERTEXATTRIBDIVISORPROC, glVertexAttribDivisor) \
X(PFNGLDRAWARRAYSINSTANCEDPROC, glDrawArraysInstanced) \
X(PFNGLDRAWELEMENTSINSTANCEDPROC, glDrawElementsInstanced) \
X(PFNGLUNIFORM1FVPROC, glUniform1fv) \
X(PFNGLUNIFORM2FVPROC, glUniform2fv) \
X(PFNGLUNIFORM3FVPROC, glUniform3fv) \
X(PFNGLGETUNIFORMLOCATIONPROC, glGetUniformLocation) \
X(PFNGLUSEPROGRAMPROC, glUseProgram) \
X(PFNGLGETSHADERIVPROC, glGetShaderiv) \
X(PFNGLGETSHADERINFOLOGPROC, glGetShaderInfoLog) \
X(PFNGLGENFRAMEBUFFERSPROC, glGenFramebuffers) \
X(PFNGLBINDFRAMEBUFFERPROC, glBindFramebuffer) \
X(PFNGLFRAMEBUFFERTEXTURE2DPROC, glFramebufferTexture2D) \
X(PFNGLCHECKFRAMEBUFFERSTATUSPROC, glCheckFramebufferStatus) \
X(PFNGLDELETEPROGRAMPROC, glDeleteProgram) \
X(PFNGLDELETESHADERPROC, glDeleteShader) \
X(PFNGLUNIFORM1IPROC, glUniform1i) \
X(PFNGLUNIFORM1FPROC, glUniform1f) \
X(PFNGLDETACHSHADERPROC, glDetachShader) \
X(PFNGLGETPROGRAMIVPROC, glGetProgramiv) \
X(PFNGLGETPROGRAMINFOLOGPROC, glGetProgramInfoLog) \
X(PFNGLDELETEFRAMEBUFFERSPROC, glDeleteFramebuffers) \
X(PFNGLUNIFORMMATRIX4FVPROC, glUniformMatrix4fv) \
X(PFNGLBLENDFUNCSEPARATEPROC, glBlendFuncSeparate) \
X(PFNGLGENERATEMIPMAPPROC, glGenerateMipmap) \
X(PFNGLBINDBUFFERBASEPROC, glBindBufferBase) \

#else
#define m_gl_funcs \
X(PFNGLBUFFERDATAPROC, glBufferData) \
X(PFNGLBUFFERSUBDATAPROC, glBufferSubData) \
X(PFNGLGENVERTEXARRAYSPROC, glGenVertexArrays) \
X(PFNGLBINDVERTEXARRAYPROC, glBindVertexArray) \
X(PFNGLGENBUFFERSPROC, glGenBuffers) \
X(PFNGLBINDBUFFERPROC, glBindBuffer) \
X(PFNGLVERTEXATTRIBPOINTERPROC, glVertexAttribPointer) \
X(PFNGLVERTEXATTRIBIPOINTERPROC, glVertexAttribIPointer) \
X(PFNGLENABLEVERTEXATTRIBARRAYPROC, glEnableVertexAttribArray) \
X(PFNGLCREATESHADERPROC, glCreateShader) \
X(PFNGLSHADERSOURCEPROC, glShaderSource) \
X(PFNGLCREATEPROGRAMPROC, glCreateProgram) \
X(PFNGLATTACHSHADERPROC, glAttachShader) \
X(PFNGLLINKPROGRAMPROC, glLinkProgram) \
X(PFNGLCOMPILESHADERPROC, glCompileShader) \
X(PFNGLVERTEXATTRIBDIVISORPROC, glVertexAttribDivisor) \
X(PFNGLDRAWARRAYSINSTANCEDPROC, glDrawArraysInstanced) \
X(PFNGLDRAWELEMENTSINSTANCEDPROC, glDrawElementsInstanced) \
X(PFNGLUNIFORM1FVPROC, glUniform1fv) \
X(PFNGLUNIFORM2FVPROC, glUniform2fv) \
X(PFNGLUNIFORM3FVPROC, glUniform3fv) \
X(PFNGLGETUNIFORMLOCATIONPROC, glGetUniformLocation) \
X(PFNGLUSEPROGRAMPROC, glUseProgram) \
X(PFNGLGETSHADERIVPROC, glGetShaderiv) \
X(PFNGLGETSHADERINFOLOGPROC, glGetShaderInfoLog) \
X(PFNGLGENFRAMEBUFFERSPROC, glGenFramebuffers) \
X(PFNGLBINDFRAMEBUFFERPROC, glBindFramebuffer) \
X(PFNGLFRAMEBUFFERTEXTURE2DPROC, glFramebufferTexture2D) \
X(PFNGLCHECKFRAMEBUFFERSTATUSPROC, glCheckFramebufferStatus) \
X(PFNGLACTIVETEXTUREPROC, glActiveTexture) \
X(PFNGLDELETEPROGRAMPROC, glDeleteProgram) \
X(PFNGLDELETESHADERPROC, glDeleteShader) \
X(PFNGLUNIFORM1IPROC, glUniform1i) \
X(PFNGLUNIFORM1FPROC, glUniform1f) \
X(PFNGLDETACHSHADERPROC, glDetachShader) \
X(PFNGLGETPROGRAMIVPROC, glGetProgramiv) \
X(PFNGLGETPROGRAMINFOLOGPROC, glGetProgramInfoLog) \
X(PFNGLDELETEFRAMEBUFFERSPROC, glDeleteFramebuffers) \
X(PFNGLUNIFORMMATRIX4FVPROC, glUniformMatrix4fv) \
X(PFNGLBLENDFUNCSEPARATEPROC, glBlendFuncSeparate) \
X(PFNGLGENERATEMIPMAPPROC, glGenerateMipmap) \
X(PFNGLBINDBUFFERBASEPROC, glBindBufferBase) \

#endif

#define X(type, name) static type name = NULL;
m_gl_funcs
#undef X

#define invalid_default_case default: { assert(!"Invalid default case"); }
#define invalid_else else { assert(!"Invalid else"); }

#if defined(_WIN32)
#define m_dll_export extern "C" __declspec(dllexport)
#else // _WIN32
#define m_dll_export
#endif

enum e_game_state1
{
	e_game_state1_default,
	e_game_state1_defeat,
};

struct s_lerpable
{
	float curr;
	float target;
};

enum e_game_state0
{
	e_game_state0_main_menu,
	e_game_state0_leaderboard,
	e_game_state0_win_leaderboard,
	e_game_state0_options,
	e_game_state0_pause,
	e_game_state0_play,
	e_game_state0_input_name,
};

struct s_key_event
{
	b8 went_down;
	int modifiers;
	int key;
};

struct s_key_state
{
	b8 is_down;
	int half_transition_count;
};

template <int n>
struct s_input_str
{
	b8 visual_pos_initialized;
	s_v2 cursor_visual_pos;
	float last_edit_time;
	float last_action_time;
	s_maybe<int> cursor;
	s_str_builder<n> str;
};

struct s_input_name_state
{
	s_input_str<16> name;
	s_str_builder<64> error_str;
};

struct s_timed_msg
{
	float spawn_timestamp;
	s_v2 pos;
	s_str_builder<64> builder;
};

enum e_enemy_movement
{
	e_enemy_movement_normal,
	e_enemy_movement_zig_zag,
	e_enemy_movement_spiral,
};

data_enum(e_enemy,

	s_enemy_type_data
	g_enemy_type_data

	basic {
		.spawn_weight = 1000,
		.health_multi = 1.0f,
		.gold_reward = 1,
		.knockback_resistance = 0.0f,
		.size = {32, 32},
		.atlas_index = {125, 25},
	}

	b {
		.prev_enemy_required_kill_count = 10,
		.spawn_weight = 3000,
		.health_multi = 2.0f,
		.gold_reward = 2,
		.knockback_resistance = 0.0f,
		.size = {32, 32},
		.atlas_index = {125, 26},
	}

	c {
		.prev_enemy_required_kill_count = 10,
		.spawn_weight = 5000,
		.health_multi = 3.0f,
		.gold_reward = 3,
		.knockback_resistance = 0.0f,
		.size = {32, 32},
		.atlas_index = {125, 27},
	}

	fast {
		.prev_enemy_required_kill_count = 10,
		.spawn_weight = 7000,
		.health_multi = 1.0f,
		.gold_reward = 4,
		.speed_multi = 2,
		.knockback_resistance = -1.0f,
		.size = {32, 32},
		.atlas_index = {125, 28},
	}

	big {
		.prev_enemy_required_kill_count = 20,
		.spawn_weight = 9000,
		.health_multi = 20.0f,
		.gold_reward = 10,
		.speed_multi = 1,
		.knockback_resistance = 0.75f,
		.movement_type = e_enemy_movement_spiral,
		.size = {64, 64},
		.atlas_index = {113, 16},
	}

	fast_zig_zag {
		.prev_enemy_required_kill_count = 5,
		.spawn_weight = 11000,
		.health_multi = 2.0f,
		.gold_reward = 8,
		.speed_multi = 4,
		.knockback_resistance = -1.0f,
		.movement_type = e_enemy_movement_zig_zag,
		.size = {32, 32},
		.atlas_index = {104, 14},
	}

	d {
		.prev_enemy_required_kill_count = 20,
		.spawn_weight = 13000,
		.health_multi = 10.0f,
		.gold_reward = 15,
		.knockback_resistance = 0.5f,
		.size = {48, 48},
		.atlas_index = {115, 40},
	}

	e {
		.prev_enemy_required_kill_count = 10,
		.spawn_weight = 15000,
		.health_multi = 20.0f,
		.gold_reward = 30,
		.speed_multi = 1.25f,
		.knockback_resistance = 0.5f,
		.size = {48, 48},
		.atlas_index = {113, 37},
	}

	spiral2 {
		.prev_enemy_required_kill_count = 10,
		.spawn_weight = 17000,
		.health_multi = 50.0f,
		.gold_reward = 50,
		.speed_multi = 3.0f,
		.knockback_resistance = 0.75f,
		.movement_type = e_enemy_movement_spiral,
		.size = {48, 48},
		.atlas_index = {107, 39},
	}

	f {
		.prev_enemy_required_kill_count = 10,
		.spawn_weight = 19000,
		.health_multi = 200.0f,
		.gold_reward = 100,
		.speed_multi = 1.0f,
		.knockback_resistance = 0.9f,
		.size = {48, 48},
		.atlas_index = {110, 41},
	}

	boss {
		.prev_enemy_required_kill_count = 10,
		.spawn_weight = 1000000,
		.health_multi = 3000.0f,
		.gold_reward = 100,
		.speed_multi = 0.5f,
		.knockback_resistance = 0.99f,
		.size = {128, 128},
		.atlas_index = {125, 29},
	}
)

struct s_enemy_type_data
{
	int prev_enemy_required_kill_count;
	u32 spawn_weight;
	float health_multi;
	int gold_reward;
	float speed_multi = 1;
	float knockback_resistance;
	e_enemy_movement movement_type;
	s_v2 size;
	s_v2i atlas_index;
};

struct s_container
{
	s_v2 curr_pos;
	s_v2 advance;
};

struct s_button_data
{
	b8 disabled;
	b8 mute_click_sound;
	float font_size = 32;
	s_len_str tooltip;
	s_v4 button_color = {0.25f, 0.25f, 0.25f, 1.0f};
};

enum e_button_result
{
	e_button_result_none,
	e_button_result_left_click,
	e_button_result_right_click,
};

data_enum(e_upgrade,
	s_upgrade_data
	g_upgrade_data

	damage {
		.name = S("Damage"),
		.cost = 10,
		.extra_cost_per_level = 5,
		.stat_boost = 50,
	}

	speed {
		.name = S("Speed"),
		.cost = 10,
		.extra_cost_per_level = 5,
		.stat_boost = 20,
		.max_upgrades = 5,
	}

	knockback {
		.name = S("Knockback"),
		.cost = 10,
		.extra_cost_per_level = 5,
		.stat_boost = 20,
	}

	range {
		.name = S("Attack range"),
		.cost = 20,
		.extra_cost_per_level = 5,
		.stat_boost = 20,
	}

	dash_cooldown {
		.name = S("Dash cooldown"),
		.cost = 30,
		.extra_cost_per_level = 20,
		.stat_boost = 20,
		.max_upgrades = 5,
	}

	max_lives {
		.name = S("Lives"),
		.cost = 30,
		.extra_cost_per_level = 5,
		.stat_boost = 5,
		.max_upgrades = 4,
	}

	lightning_bolt {
		.name = S("Lightning bolt"),
		.cost = 50,
		.extra_cost_per_level = 30,
		.stat_boost = 50,
	}

	more_hits_per_attack {
		.name = S("Multistrike"),
		.cost = 100,
		.extra_cost_per_level = 200,
		.stat_boost = 1,
		.max_upgrades = 4,
	}

	auto_attack {
		.name = S("Auto attack"),
		.cost = 200,
		.extra_cost_per_level = 100,
		.stat_boost = 25,
	}

)

struct s_upgrade_data
{
	s_len_str name;
	int cost;
	int extra_cost_per_level;
	float stat_boost;
	int max_upgrades;
};


struct s_entity
{
	int id;
	float timer;
	s_v2 pos;
	s_v2 prev_pos;
	float spawn_timestamp;
	float duration;
	union {

		// @Note(tkap, 31/07/2025): Player
		struct {
			float did_attack_enemy_timestamp;
			float fist_wobble_time;
			s_v2 attacked_enemy_pos;
			int range_emitter;
			float fist_rotation[2];
			float stamina;
		};

		// @Note(tkap, 31/07/2025): Emitter
		struct {
			s_particle_emitter_a emitter_a;
			s_particle_emitter_b emitter_b;
		};

		// @Note(tkap, 31/07/2025): Enemy
		struct {
			float remove_soon_timestamp;
			e_enemy enemy_type;
			s_maybe<float> knockback;
			float damage_taken;
			float hit_timestamp;
		};

		// @Note(tkap, 31/07/2025): fct
		struct {
			b8 shake;
			int fct_type;
			float font_size;
			s_str_builder<16> builder;
			s_v2 vel;
		};

		// @Note(tkap, 31/07/2025): visual effect
		struct {
			s_v2 effect_size;
		};
	};
};

struct s_frame_data
{
	int lives_to_lose;
};

struct s_queued_upgrade
{
	e_upgrade id;
	int count;
};

struct s_soft_game_data
{
	float progression_timestamp_arr[e_enemy_count];
	s_list<s_queued_upgrade, 64> queued_upgrade_arr;
	float boss_defeated_timestamp;
	int frames_to_freeze;
	s_entity_ref boss_ref;
	b8 boss_spawned;
	s_frame_data frame_data;
	int lives_lost;
	int gold;
	float spawn_timer;
	float life_change_timestamp;
	float gold_change_timestamp;
	s_timer dash_timer;
	s_timer lightning_bolt_timer;
	s_timer attack_timer;
	b8 tried_to_submit_score;
	int update_count;
	float shake_intensity;
	float start_screen_shake_timestamp;
	float start_restart_timestamp;
	s_list<s_particle, 65536> particle_arr;

	s_entity_manager<s_entity, c_max_entities> entity_arr;

	s_list<s_timed_msg, 8> timed_msg_arr;

	int upgrade_count[e_upgrade_count];

	int enemy_type_kill_count_arr[e_enemy_count];
};

struct s_hard_game_data
{
	s_state state1;
	int update_count;
};

struct s_light
{
	s_v2 pos;
	float radius;
	float smoothness;
	s_v4 color;
};

struct s_game
{
	b8 disable_auto_dash_when_no_cooldown;
	b8 disable_hover_over_upgrade_to_pause;
	b8 disable_damage_numbers;
	b8 disable_gold_numbers;
	b8 disable_lights;
	float completed_dash_tutorial_timestamp;
	s_lerpable music_speed;
	s_lerpable music_volume;
	int next_entity_id;
	#if defined(m_debug)
	b8 cheat_menu_enabled;
	#endif
	b8 reload_shaders;
	b8 any_key_pressed;
	b8 purchased_at_least_one_upgrade;
	s_linear_arena arena;
	s_linear_arena update_frame_arena;
	s_linear_arena render_frame_arena;
	s_circular_arena circular_arena;
	u32 ubo;
	s_texture texture_arr[e_texture_count];
	s_mesh mesh_arr[e_mesh_count];
	s_shader shader_arr[e_shader_count];
	float render_time;
	float update_time;
	f64 accumulator;
	f64 time_before;
	u32 curr_fbo;
	int speed_index;
	s_font font;
	s_rng rng;
	float speed;
	s_input_name_state input_name_state;
	int num_times_we_attacked_an_enemy;
	int num_times_we_dashed;

	#if defined(m_winhttp)
	HINTERNET session;
	#endif

	s_len_str tooltip;

	s_fbo light_fbo;

	s_list<s_light, 256> multiplicative_light_arr;
	s_list<s_light, 256> additive_light_arr;

	b8 disable_music;
	b8 hide_timer;
	b8 turn_off_all_sounds;

	int update_count_at_win_time;

	s_state state0;

	s_list<s_leaderboard_entry, c_max_leaderboard_entries> leaderboard_arr;
	b8 leaderboard_received;
	int curr_leaderboard_page;

	s_str_builder<256> leaderboard_session_token;
	s_str_builder<256> leaderboard_public_uid;
	s_str_builder<256> leaderboard_nice_name;
	s_str_builder<256> leaderboard_player_identifier;
	int leaderboard_player_id;

	s_key_state input_arr[c_max_keys];
	s_list<s_key_event, 128> key_events;
	s_list<char, 128> char_events;

	b8 do_soft_reset;
	b8 do_hard_reset;

	s_hard_game_data hard_data;
	s_soft_game_data soft_data;

	int render_instance_count[e_shader_count][e_texture_count][e_mesh_count];
	int render_instance_max_elements[e_shader_count][e_texture_count][e_mesh_count];
	int render_group_index_arr[e_shader_count][e_texture_count][e_mesh_count];
	void* render_instance_arr[e_shader_count][e_texture_count][e_mesh_count];
	s_list<s_render_group, 128> render_group_arr;
};


#include "generated/generated_game.cpp"
#include "gen_meta/game.h.globals"