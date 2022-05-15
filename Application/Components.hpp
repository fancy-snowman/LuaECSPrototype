#pragma once

struct Health
{
	float Value;
};

struct Poison
{
	float TickDamage;
};


struct BehaviourComponent
{
	char ScriptPath[64];
	int LuaTableRef;

	BehaviourComponent(const char* path, int luaRef = -1);
};
void lua_pushbehaviour(lua_State* L, const BehaviourComponent& component);
BehaviourComponent lua_tobehaviour(lua_State* L, int i);

#include <cstring>
struct YieldableBehaviour
{
	char ScriptPath[64];
	int LuaTableRef;
	int LastYield;
	float WaitTimer;
	lua_State* ThreadState;

	YieldableBehaviour(const char* path, int luaRef = -1, lua_State* threadState = nullptr) :
		LuaTableRef(luaRef), LastYield(LUA_OK), WaitTimer(0), ThreadState(threadState)
	{
		memset(ScriptPath, '\0', 64);
		strcpy_s(ScriptPath, path);
	}
};

struct TransformComponent
{
	struct PositionData
	{
		float x;
		float y;
		float z;
	} Position;

	struct EulerRotationData
	{
		float x;
		float y;
		float z;
	} Rotation;

	struct ScaleData
	{
		float x;
		float y;
		float z;
	} Scale;
};
void lua_pushtransform(lua_State* L, const TransformComponent& component);
TransformComponent lua_totransform(lua_State* L, int i);