#include "entt/entt.hpp"
#include "lua.hpp"

#include "Scene.hpp"
#include "Components.hpp"

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
		//std::cout << "> ";
		//std::getline(std::cin, input);

		//if (luaL_dostring(L, input.c_str()) != LUA_OK)
		//{
		//	DumpError(L);
		//}
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

void YieldTest(lua_State* L)
{
	Scene scene(L);
	Scene::lua_openscene(L, &scene);

	class YieldableScriptSystem : public System
	{
		lua_State* L;
	public:
		YieldableScriptSystem(lua_State* L) : L(L) {}
		bool OnUpdate(entt::registry& registry, float delta) final
		{
			auto view = registry.view<YieldableBehaviour>();

			if (view.size() > 0)
			{
				view.each([&](YieldableBehaviour& behaviour) {

					behaviour.WaitTimer -= delta;

					if (behaviour.WaitTimer <= 0.f)
					{
						behaviour.WaitTimer = 0.f;

						int res = lua_rawgeti(behaviour.ThreadState, LUA_REGISTRYINDEX, behaviour.LuaTableRef);
						lua_getfield(behaviour.ThreadState, -1, "OnUpdate");

						int numResults;
						if (behaviour.LastYield == LUA_OK)
						{
							// Call OnUpdate from start with arguments
							lua_pushvalue(behaviour.ThreadState, -2);
							lua_pushnumber(behaviour.ThreadState, delta);
							behaviour.LastYield = lua_resume(behaviour.ThreadState, nullptr, 2, &numResults);			
						}
						else if (behaviour.LastYield == LUA_YIELD)
						{
							// Resume to last yield point with no arguments
							behaviour.LastYield = lua_resume(behaviour.ThreadState, nullptr, 0, &numResults);
						}

						if (numResults > 0 && lua_isnumber(behaviour.ThreadState, 1))
						{
							behaviour.WaitTimer += lua_tonumber(behaviour.ThreadState, 1);
							lua_pop(behaviour.ThreadState, lua_gettop(behaviour.ThreadState));
						}

						/*
							#define LUA_OK		0
							#define LUA_YIELD	1
							#define LUA_ERRRUN	2
							#define LUA_ERRSYNTAX	3
							#define LUA_ERRMEM	4
							#define LUA_ERRERR	5
						*/
						int post = lua_gettop(behaviour.ThreadState);
						lua_pop(behaviour.ThreadState, 1);
					}
				});
			}
			return false;
		}
	};

	scene.CreateSystem<YieldableScriptSystem>(L);
	
	if (luaL_dofile(L, "Setup.lua") != LUA_OK)
	{
		DumpError(L);
	}

	using Clock = std::chrono::high_resolution_clock;

	const Clock::duration TARGET_DELTA = std::chrono::seconds(1);
	Clock::duration deltaSum = std::chrono::seconds(0);
	Clock::time_point lastTick = Clock::now();

	while (true)
	{
		Clock::time_point thisTick = Clock::now();
		Clock::duration delta = thisTick - lastTick;
		deltaSum += thisTick - lastTick;
		lastTick = thisTick;

		if (true || deltaSum >= TARGET_DELTA)
		{
			scene.UpdateSystems(std::chrono::duration_cast<std::chrono::duration<float>>(delta).count());
			deltaSum -= TARGET_DELTA;
			deltaSum = std::chrono::seconds(0);
		}
	}
}

int main()
{
	lua_State* L = luaL_newstate();
	luaL_openlibs(L);

	std::thread consoleThread(ConsoleThreadFunction, L);

	//luaL_dofile(L, "Script.lua");

	//lua_close(L);

	//entt::registry registry;

	//struct Health
	//{
	//	float Value;
	//};

	//struct Poison
	//{
	//	float TickDamage;
	//};

	//Scene scene(L);
	//Scene::lua_openscene(L, &scene);
	//scene.CreateSystem<PoisonSystem>(3);
	//scene.CreateSystem<CleanupSystem>();

	//// requires include <stdlib.h> and <time.h>
	//srand(time(NULL));

	Scene scene(L);
	Scene::lua_openscene(L, &scene);

	for (int i = 0; i < 100; i++)
	{
		// Create a new entity.
		auto entity = scene.CreateEntity();

		// Each entity starts out with 100 health points.
		scene.SetComponent<Health>(entity, 100.f);

		// Each entity is poisoned, where the damage per
		// tick is random for each entity.
		float tickDamage = rand() % 10 + 1; // random [1;10]
		scene.SetComponent<Poison>(entity, tickDamage);
	}

	class BehaviourSystem : public System
	{
		lua_State* L;
	public:
		BehaviourSystem(lua_State* L) : L(L) {}

		bool OnUpdate(entt::registry& registry, float delta) final
		{
			auto view = registry.view<BehaviourComponent>();

			if (view.size() > 0)
			{
				view.each([&](BehaviourComponent& script) {
					lua_rawgeti(L, LUA_REGISTRYINDEX, script.LuaTableRef);
					lua_getfield(L, -1, "OnUpdate");
					lua_pushvalue(L, -2);

					if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
						DumpError(L);
					}

					lua_pop(L, 1);
					});
			}
			return false;
		}
	};

	//scene.CreateSystem<ScriptSystem>(L);

	//// Iterate until there are no more entitetes

	//using Clock = std::chrono::high_resolution_clock;

	//const Clock::duration TARGET_DELTA = std::chrono::seconds(1);
	//Clock::duration deltaSum = std::chrono::seconds(0);
	//Clock::time_point lastTick = Clock::now();

	//int iterations = 0;
	//while (true)
	//{
	//	Clock::time_point thisTick = Clock::now();
	//	deltaSum += thisTick - lastTick;
	//	lastTick = thisTick;

	//	if (deltaSum >= TARGET_DELTA)
	//	{
	//		deltaSum -= TARGET_DELTA;
	//		scene.UpdateSystems(1);
	//	}

	//	//iterations++;
	//	//std::cout << "Iteration " << iterations
	//	//	<< ", entities alive: " << scene.GetEntityCount()
	//	//	<< std::endl;
	//}








	class PoisonSystem : public System
	{
		int m_lifetime;
	public:
		PoisonSystem(int lifetime) : m_lifetime(lifetime) {}
		bool OnUpdate(entt::registry& registry, float delta) final
		{
			auto view = registry.view<Health, Poison>();
			view.each([](Health& health, const Poison& poison) {
				health.Value -= poison.TickDamage;
			});

			return (--m_lifetime) <= 0;
		}
	};

	//class CleanupSystem : public System
	//{
	//public:
	//	bool OnUpdate(entt::registry& registry, float delta) final
	//	{
	//		auto view = registry.view<Health>();
	//		view.each([&](entt::entity entity, const Health& health) {
	//			if (health.Value <= 0.f)
	//			{
	//				registry.destroy(entity);
	//			}
	//		});
	//		return false;
	//	}
	//};

	//class InfoSystem : public System
	//{
	//	int m_updateCounter = 0;
	//public:
	//	InfoSystem() = default;
	//	bool OnUpdate(entt::registry& registry, float delta) final
	//	{
	//		int count = registry.alive();
	//		auto healthView = registry.view<Health>();
	//		auto poisonView = registry.view<Poison>();
	//		printf("\n-- Update %i --\n", ++m_updateCounter);
	//		printf(" Living entities:\t%i\n", healthView.size());
	//		printf(" Poisoned entities:\t%i\n", poisonView.size());
	//		return false;
	//	}
	//};

	float delta = 1.f;

	luaL_dofile(L, "demo.lua");

	scene.CreateSystem<PoisonSystem>(5); // Lifetime of 5 ticks
	// ...
	scene.UpdateSystems(delta);
	//scene.CreateSystem<CleanupSystem>();
	//scene.CreateSystem<InfoSystem>(); // For printing info per tick
	//luaL_dofile(L, "sceneDemo.lua"); // Creates entities

	//for (int i = 0; i < 10; i++)
	//{
	//	scene.UpdateSystems(1); // delta = 1 second
	//}

	YieldTest(L);

	consoleThread.join();

	return 0;
}