#include "LuaCommon.hpp"
#include "Components.hpp"

#include <cstring>

BehaviourComponent::BehaviourComponent(const char* path, int luaRef) : LuaTableRef(luaRef)
{
	memset(ScriptPath, '\0', 64);
	strcpy_s(ScriptPath, path);
}

void lua_pushbehaviour(lua_State* L, const BehaviourComponent& component)
{
	lua_rawgeti(L, LUA_REGISTRYINDEX, component.LuaTableRef);
}

BehaviourComponent lua_tobehaviour(lua_State* L, int i)
{
	lua_getfield(L, i, "path");
	const char* path = lua_tostring(L, -1);
	lua_pop(L, 1);

	return BehaviourComponent(path);
}

void lua_pushtransform(lua_State* L, const TransformComponent& component)
{
	// Transform
	lua_newtable(L);

	// Position
	lua_newtable(L);
	lua_pushnumber(L, component.Position.x); lua_setfield(L, -2, "x");
	lua_pushnumber(L, component.Position.y); lua_setfield(L, -2, "y");
	lua_pushnumber(L, component.Position.z); lua_setfield(L, -2, "z");
	lua_setfield(L, -2, "position");

	// Position
	lua_newtable(L);
	lua_pushnumber(L, component.Rotation.x); lua_setfield(L, -2, "x");
	lua_pushnumber(L, component.Rotation.y); lua_setfield(L, -2, "y");
	lua_pushnumber(L, component.Rotation.z); lua_setfield(L, -2, "z");
	lua_setfield(L, -2, "rotation");

	// Position
	lua_newtable(L);
	lua_pushnumber(L, component.Scale.x); lua_setfield(L, -2, "x");
	lua_pushnumber(L, component.Scale.y); lua_setfield(L, -2, "y");
	lua_pushnumber(L, component.Scale.z); lua_setfield(L, -2, "z");
	lua_setfield(L, -2, "scale");
}

TransformComponent lua_totransform(lua_State* L, int i)
{
	TransformComponent component;

	lua_getfield(L, i, "position");
	lua_getfield(L, -1, "x"); component.Position.x = lua_tonumber(L, -1); lua_pop(L, 1);
	lua_getfield(L, -1, "y"); component.Position.y = lua_tonumber(L, -1); lua_pop(L, 1);
	lua_getfield(L, -1, "z"); component.Position.z = lua_tonumber(L, -1); lua_pop(L, 1);
	lua_pop(L, 1); // Pop position

	lua_getfield(L, i, "rotation");
	lua_getfield(L, -1, "x"); component.Rotation.x = lua_tonumber(L, -1); lua_pop(L, 1);
	lua_getfield(L, -1, "y"); component.Rotation.y = lua_tonumber(L, -1); lua_pop(L, 1);
	lua_getfield(L, -1, "z"); component.Rotation.z = lua_tonumber(L, -1); lua_pop(L, 1);
	lua_pop(L, 1); // Pop rotation

	lua_getfield(L, i, "scale");
	lua_getfield(L, -1, "x"); component.Scale.x = lua_tonumber(L, -1); lua_pop(L, 1);
	lua_getfield(L, -1, "y"); component.Scale.y = lua_tonumber(L, -1); lua_pop(L, 1);
	lua_getfield(L, -1, "z"); component.Scale.z = lua_tonumber(L, -1); lua_pop(L, 1);
	lua_pop(L, 1); // Pop scale

	return component;
}