
#pragma warning(push, 0)
#define SDL_MAIN_HANDLED
#include "SDL2/SDL.h"
#pragma warning(pop)

#include <emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/fetch.h>

#include "tklib.h"
#include "shared.h"
global s_platform_data g_platform_data;
#include "shared_between_platforms.h"

func void do_one_frame();

int main()
{
	emscripten_set_main_loop(do_one_frame, -1, 0);
	g_platform_data.memory = (u8*)calloc(1, 512 * 1024 * 1024);

	SDL_SetMainReady();
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
	init_common();

	init(&g_platform_data);
	init_after_recompile(&g_platform_data);

	// do_one_frame();

	return 0;
}

func void do_one_frame()
{
	do_game(&g_platform_data);
}
