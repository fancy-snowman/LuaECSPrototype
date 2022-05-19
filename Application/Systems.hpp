#pragma once
#include "entt/entt.hpp"
#include "LuaCommon.hpp"

// ######################################################################### //
// ############################### BASE CLASS ############################## //
// ######################################################################### //

class System
{
public:
	virtual bool OnUpdate(entt::registry& registry, float delta) = 0;
};



// ######################################################################### //
// ############################### BEHAVIOUR ############################### //
// ######################################################################### //

class BehaviourSystem : public System
{
	lua_State* m_luaState;
public:
	BehaviourSystem(lua_State* L);
	bool OnUpdate(entt::registry& registry, float delta) final;
};