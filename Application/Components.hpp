#pragma once
#include "LuaCommon.hpp"

// ######################################################################### //
// ############################### BEHAVIOUR ############################### //
// ######################################################################### //

struct BehaviourComponent
{
	char ScriptPath[64];
	int BehaviourTableRef = 0;
	int LastYield = LUA_OK;
	float UpdateCooldown = 0.0f;
	lua_State* ThreadState = nullptr;

	BehaviourComponent(const char* path, int behaviourTableRef = -1, lua_State* threadState = nullptr);
};
void lua_pushbehaviour(lua_State* L, const BehaviourComponent& component);
BehaviourComponent lua_tobehaviour(lua_State* L, int i);





// ######################################################################### //
// ############################### TRANSFORM ############################### //
// ######################################################################### //

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