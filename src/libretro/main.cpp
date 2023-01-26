/**
 * Copyright (c) 2006-2023 LOVE Development Team
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 **/

#include "libretro/global.h"

#include "common/version.h"
#include "common/runtime.h"
#include "modules/love/love.h"
#include "graphics/opengl/OpenGL.h"

// Lua
extern "C" {
	#include <lua.h>
	#include <lualib.h>
	#include <lauxlib.h>
}

static lua_State *L = nullptr;

retro_environment_t g_retro_set_env = nullptr;
retro_video_refresh_t g_retro_video = nullptr;
retro_hw_get_current_framebuffer_t g_retro_get_current_framebuffer = nullptr;

RETRO_API void retro_set_environment(retro_environment_t env) {
    g_retro_set_env = env;
}

RETRO_API void retro_set_video_refresh(retro_video_refresh_t sendFrame)
{
    g_retro_video = sendFrame;
}

RETRO_API void retro_set_audio_sample(retro_audio_sample_t sendAudioSample)
{
}

RETRO_API void retro_set_audio_sample_batch(retro_audio_sample_batch_t audioSampleBatch)
{
}

RETRO_API void retro_set_input_poll(retro_input_poll_t pollInput)
{
}

RETRO_API void retro_set_input_state(retro_input_state_t getInputState)
{
}

RETRO_API void retro_set_controller_port_device(unsigned port, unsigned device)
{
}

RETRO_API void retro_get_system_info(struct retro_system_info *info)
{
    info->library_name = "Love";
    info->library_version = love_version();
    info->need_fullpath = false;
    info->valid_extensions = "lua";
    info->block_extract = false;
}

RETRO_API void retro_get_system_av_info(struct retro_system_av_info *info)
{
    info->geometry.base_width = 800;
    info->geometry.base_height = 600;
    info->geometry.max_width = 1920;
    info->geometry.max_height = 1080;
    info->timing.fps = 60.0;
    info->timing.sample_rate = 48000;
}

RETRO_API size_t retro_get_memory_size(unsigned id)
{
    return 0;
}

RETRO_API void *retro_get_memory_data(unsigned id)
{
    return nullptr;
}

RETRO_API unsigned retro_api_version(void) {
    return 0;
}

RETRO_API void retro_reset()
{
}

static int love_preload(lua_State *L, lua_CFunction f, const char *name)
{
	lua_getglobal(L, "package");
	lua_getfield(L, -1, "preload");
	lua_pushcfunction(L, f);
	lua_setfield(L, -2, name);
	lua_pop(L, 2);
	return 0;
}

RETRO_API void retro_init() {
	// Create the virtual machine.
	L = luaL_newstate();
	luaL_openlibs(L);

	// LuaJIT-specific setup needs to be done as early as possible - before
	// get_app_arguments because that loads external library code. This is also
	// loaded inside require("love"). Note that it doesn't use the love table.
	love_preload(L, luaopen_love_jitsetup, "love.jitsetup");
	lua_getglobal(L, "require");
	lua_pushstring(L, "love.jitsetup");
	lua_call(L, 1, 0);
}

RETRO_API void retro_deinit() {
    lua_close(L);
    L = nullptr;
}

static void hw_context_reset()
{
    fprintf(stderr, "hw_context_reset not implemented\n");
}

static void hw_context_destroy()
{
    fprintf(stderr, "hw_context_destroy not implemented\n");
}

static void hw_context_setup()
{
    retro_pixel_format pf = RETRO_PIXEL_FORMAT_XRGB8888;
    if (!g_retro_set_env(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &pf)) {
        fprintf(stderr, "failed to set pixel format\n");
    }

    retro_hw_render_callback hw_render_callback = {};
    hw_render_callback.context_reset = hw_context_reset;
    hw_render_callback.context_destroy = hw_context_destroy;

    hw_render_callback.bottom_left_origin = true;
    hw_render_callback.stencil = true;
    hw_render_callback.depth = true;
    //hw_render_callback.context_type = RETRO_HW_CONTEXT_OPENGLES_VERSION;
    hw_render_callback.context_type = RETRO_HW_CONTEXT_OPENGL_CORE;
    hw_render_callback.version_major = 4;
    hw_render_callback.version_minor = 5;
    hw_render_callback.debug_context = true;

    int res = g_retro_set_env(RETRO_ENVIRONMENT_SET_HW_RENDER, &hw_render_callback);
    fprintf(stderr, "SET_HW_RENDER:%d\n", res);
    if (res) {
        g_retro_get_current_framebuffer = hw_render_callback.get_current_framebuffer;
    }
}

RETRO_API bool retro_load_game(const struct retro_game_info *game)
{
    // Not sure this is the best place
    hw_context_setup();

	int argc = 2;
	const char *argv[2] = { "love", game->path };

	// Add love to package.preload for easy requiring.
	love_preload(L, luaopen_love, "love");

	// Add command line arguments to global arg (like stand-alone Lua).
	{
		lua_newtable(L);

		if (argc > 0)
		{
			lua_pushstring(L, argv[0]);
			lua_rawseti(L, -2, -2);
		}

		lua_pushstring(L, "embedded boot.lua");
		lua_rawseti(L, -2, -1);

		for (int i = 1; i < argc; i++)
		{
			lua_pushstring(L, argv[i]);
			lua_rawseti(L, -2, i);
		}

		lua_setglobal(L, "arg");
	}

	// require "love"
	lua_getglobal(L, "require");
	lua_pushstring(L, "love");
	lua_call(L, 1, 1); // leave the returned table on the stack.

	// Add love._exe = true.
	// This indicates that we're running the standalone version of love, and not
	// the library version.
	{
		lua_pushboolean(L, 1);
		lua_setfield(L, -2, "_exe");
	}

	// Pop the love table returned by require "love".
	lua_pop(L, 1);

	// require "love.boot" (preloaded when love was required.)
	lua_getglobal(L, "require");
	lua_pushstring(L, "love.boot");
	lua_call(L, 1, 1);

	// Turn the returned boot function into a coroutine and call it until done.
	lua_newthread(L);
	lua_pushvalue(L, -2);

    return true;
}

RETRO_API void retro_unload_game()
{
}

RETRO_API void retro_run() {
    if (!L) {
        return;
    }

	// Restore GL state of the game
	love::graphics::opengl::gl.restoreState();

	int stackpos = lua_gettop(L);
	int nres;
	love::luax_resume(L, 0, &nres);
#if LUA_VERSION_NUM >= 504
    lua_pop(L, nres);
#else
    lua_pop(L, lua_gettop(L) - stackpos);
#endif
}
