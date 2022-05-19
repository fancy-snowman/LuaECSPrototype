#include "entt/entt.hpp"
#include "lua.hpp"

#include "Scene.hpp"
#include "Components.hpp"
#include "Systems.hpp"

#include <stdlib.h>
#include <time.h>
#include <chrono>

#include <iostream>
#include <thread>
#include <string>
#include <Windows.h>

#include "LuaCommon.hpp"

void ConsoleThreadFunction(lua_State* L)
{
	std::string input;
	while (GetConsoleWindow())
	{
		std::cout << "> ";
		std::getline(std::cin, input);

		if (luaL_dostring(L, input.c_str()) != LUA_OK)
		{
			DumpError(L);
		}
	}
}

std::string GetValueString(lua_State* L, int i)
{
	switch (lua_type(L, i))
	{
	case LUA_TNIL: return "nil";
	case LUA_TBOOLEAN: return lua_toboolean(L, i) ? "true" : "false";
	case LUA_TNUMBER: return std::to_string(lua_tonumber(L, i));
	case LUA_TSTRING: return lua_tostring(L, i);
	default: return "";
	}
}

int main()
{
	/**
	*	Set up the Lua environment
	*/

	lua_State* L = luaL_newstate();
	luaL_openlibs(L);

	Scene scene(L);
	Scene::lua_openscene(L, &scene);

	std::thread consoleThread(ConsoleThreadFunction, L);



	/**
	*	Set up necessary ECS-systems
	*/

	scene.CreateSystem<BehaviourSystem>(L);

	if (luaL_dofile(L, "Setup.lua") != LUA_OK)
	{
		DumpError(L);
	}



	/**
	*	Set up the main loop
	*/

	using Clock = std::chrono::high_resolution_clock;

	const Clock::duration TARGET_DELTA = std::chrono::seconds(1);
	Clock::time_point lastTick = Clock::now();

	// All systems are updated each tick.
	// Configure the duration, in seconds, the duration between updates
	// should be.
	float TICK_DURATION = 1.0f / 60.f; // 60 ticks per second



	/**
	*	Run the main loop
	*/

	float deltaSum = 0.f;
	while (true) // TODO: Make this "close-able"
	{
		Clock::time_point thisTick = Clock::now();
		Clock::duration delta = thisTick - lastTick;
		lastTick = thisTick;

		deltaSum += std::chrono::duration_cast<std::chrono::duration<float>>(delta).count();

		if (deltaSum >= TICK_DURATION)
		{
			scene.UpdateSystems(deltaSum);
			deltaSum -= TICK_DURATION;
		}
	}



	/**
	*	Cleanup
	*/

	consoleThread.join();

	return 0;
}