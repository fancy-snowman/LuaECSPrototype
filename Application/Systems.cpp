#include "Systems.hpp"
#include "Components.hpp"
#include "LuaCommon.hpp"

// ######################################################################### //
// ############################### BEHAVIOUR ############################### //
// ######################################################################### //

BehaviourSystem::BehaviourSystem(lua_State* L) : m_luaState(L)
{
	//
}

bool BehaviourSystem::OnUpdate(entt::registry& registry, float delta)
{
	/**
	*	Explanation of a behaviour
	*	
	*	- A behaviour is a Lua table with a coroutine called 'OnUpdate'.
	*	- This Lua table can hold all game specific information and/or logic.
	*	- A behaviour can yield a number (float) if it wants to add a cooldown
	*	  before its next update. This means that the coroutine 'OnUpdate' do not
	*	  have to be called each tick in the game.
	*/

	// Query all entities with a behaviour attached
	auto view = registry.view<BehaviourComponent>();

	/**
	*	For each entity with a behaviour :
	* 
	*	1. Decrease cooldown the behaviour
	*	2. If there is no more cooldown:
	*		a) Retrieve the Lua table of the behaviour
	*		b) Initiate or resume the behaviour coroutine
	*		c) Increase cooldown if the coroutine returned a number
	*/
	view.each([&delta](BehaviourComponent& behaviour) {

		// Decrease cooldown if needed
		if (behaviour.UpdateCooldown > 0.f)
		{
			behaviour.UpdateCooldown -= delta;
		}

		// If no more cooldown, update the behaviour
		if (behaviour.UpdateCooldown <= 0.f)
		{
			// Reset the cooldown
			behaviour.UpdateCooldown = 0.0f;

			// Query the Lua table of the behaviour to the Lua stack
			int res = lua_rawgeti(behaviour.ThreadState,
				LUA_REGISTRYINDEX,
				behaviour.BehaviourTableRef);

			// Query the update coroutine from the behaviour table
			lua_getfield(behaviour.ThreadState, -1, "OnUpdate");

			/**
			*	Current layout of the Lua stack (in behaviour.ThreadState):
			* 
			*	2 | function		OnUpdate		| -1
			*	1 | table			behaviour table	| -2
			* 
			*	A coroutine returns its state after it has been resumed. The
			*	value of behaviour.LastYield means the following:
			* 
			*	LUA_OK		The coroutine has not yet be created, or finished
			*				its execution without error during the previous
			*				execution, meaning the coroutine has to be
			*				initialized (again).
			* 
			*	LUA_YEILD	The coroutine yielded during its previous execution.
			*/
			
			int numResults = 0;
			if (behaviour.LastYield == LUA_OK)
			{
				/**
				*	OnUpdate has two parameters :
				* 
				*	1. self		(the behaviour table itself)
				*	2. delta
				*/

				// Push the arguments
				lua_pushvalue(behaviour.ThreadState, -2);
				lua_pushnumber(behaviour.ThreadState, delta);

				// Run the OnUpdate coroutine
				behaviour.LastYield = lua_resume(behaviour.ThreadState,
					nullptr,
					2,
					&numResults);
			}
			else if (behaviour.LastYield == LUA_YIELD)
			{
				// Resume to last yield point with only delta as argument
				lua_pushnumber(behaviour.ThreadState, delta);
				behaviour.LastYield = lua_resume(behaviour.ThreadState,
					nullptr,
					1,
					&numResults);
			}

			// Update the cooldown if, and only if, the coroutine yielded with
			// a number.
			if (behaviour.LastYield == LUA_YIELD
					&& numResults > 0
					&& lua_isnumber(behaviour.ThreadState, 1))
			{
				behaviour.UpdateCooldown += lua_tonumber(behaviour.ThreadState, 1);
			}

			lua_pop(behaviour.ThreadState, lua_gettop(behaviour.ThreadState));
		}
	});

	// Return false to not remove this system from the ECS-system update queue
	return false;
}