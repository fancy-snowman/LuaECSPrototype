#include "Scene.hpp"
#include "Components.hpp"
#include "LuaCommon.hpp"

Scene::Scene(lua_State* L) : m_luaState(L)
{
	//
}

Scene::~Scene()
{
	for (auto& s : m_systems)
	{
		delete s;
	}
}

int Scene::GetEntityCount()
{
	return m_registry.alive();
}

int Scene::CreateEntity()
{
	return (int)m_registry.create();
}

bool Scene::IsEntity(int entity)
{
	return m_registry.valid((entt::entity)entity);
}

void Scene::RemoveEntity(int entity)
{
	m_registry.destroy((entt::entity)entity);
}

void Scene::UpdateSystems(float delta)
{
	for (auto it = m_systems.begin(); it != m_systems.end();)
	{
		if ((*it)->OnUpdate(m_registry, delta))
		{
			delete (*it);
			it = m_systems.erase(it);
		}
		else
		{
			it++;
		}
	}
}

void Scene::lua_openscene(lua_State* L, Scene* scene)
{
	lua_newtable(L);
	luaL_Reg sceneMethods[] =
	{
		{ "IsEntity", lua_IsEntity },
		{ "HasComponent", lua_HasComponent },
		{ "GetEntityCount", lua_GetEntityCount },
		{ "CreateEntity", lua_CreateEntity },
		{ "RemoveEntity", lua_RemoveEntity },
		{ "GetComponent", lua_GetComponent },
		{ "SetComponent", lua_SetComponent },
		{ "RemoveComponent", lua_RemoveComponent },
		{ NULL, NULL }
	};
	lua_pushlightuserdata(L, scene);
	luaL_setfuncs(L, sceneMethods, 1);
	lua_setglobal(L, "scene");

	lua_openbehaviour(L, scene);
}

Scene* Scene::lua_GetSceneUpValue(lua_State* L)
{
	Scene* scene = nullptr;
	if (lua_isuserdata(L, lua_upvalueindex(1)))
	{
		scene = (Scene*)lua_touserdata(L, lua_upvalueindex(1));
	}
	return scene;
}

int Scene::RefAndPushBehaviour(lua_State* L, int entity, const char* path)
{
	luaL_dofile(L, path);
	luaL_getmetatable(L, "BehaviourMeta");
	lua_setmetatable(L, -2);

	lua_pushvalue(L, -1);
	int luaTableRef = luaL_ref(L, LUA_REGISTRYINDEX);

	lua_pushinteger(L, entity);
	lua_setfield(L, -2, "ID");

	lua_pushstring(L, path);
	lua_setfield(L, -2, "path");

	lua_getfield(L, -1, "OnCreate");
	lua_pushvalue(L, -2);
	lua_pcall(L, 1, 0, 0);

	return luaTableRef;
}

void Scene::UnrefBehaviour(lua_State* L, int ref)
{
	lua_rawgeti(L, LUA_REGISTRYINDEX, ref);

	luaL_dostring(L, "return function(t) for k,_ in pairs(t) do t[k] = nil end end");
	lua_pushvalue(L, -2);
	if (lua_pcall(L, 1, 0, 0) != LUA_OK)
	{
		DumpError(L);
	}
	luaL_unref(L, LUA_REGISTRYINDEX, ref);
}

int Scene::lua_GetEntityCount(lua_State* L)
{
	Scene* scene = lua_GetSceneUpValue(L);
	int count = scene->GetEntityCount();
	lua_pushinteger(L, count);
	return 1;
}

int Scene::lua_CreateEntity(lua_State* L)
{
	Scene* scene = lua_GetSceneUpValue(L);
	int entity = scene->CreateEntity();

	if (lua_gettop(L) > 0 && lua_istable(L, 1))
	{
		lua_pushnil(L);
		while (lua_next(L, 1) != 0)
		{
			std::string key = lua_tostring(L, -2);

			if (key == "behaviour")
			{
				std::string path = lua_tostring(L, -1);
				int ref = RefAndPushBehaviour(L, entity, path.c_str());
				scene->SetComponent<BehaviourComponent>(entity, path.c_str(), ref);
				lua_pop(L, 2); // Keep key, pop value and behaviour
			}
			else if (key == "transform")
			{
				TransformComponent transform = lua_totransform(L, -1);
				scene->SetComponent<TransformComponent>(entity, transform);
				lua_pop(L, 1); // Keep key, pop value
			}
		}
	}

	lua_pushinteger(L, entity);
	return 1;
}

int Scene::lua_IsEntity(lua_State* L)
{
	Scene* scene = lua_GetSceneUpValue(L);
	int entity = lua_tointeger(L, 1);
	bool alive = scene->IsEntity(entity);
	lua_pushboolean(L, alive);
	return 1;
}

int Scene::lua_RemoveEntity(lua_State* L)
{
	Scene* scene = lua_GetSceneUpValue(L);
	int entity = lua_tointeger(L, 1);
	scene->RemoveEntity(entity);
	return 0;
}

int Scene::lua_HasComponent(lua_State* L)
{
	Scene* scene = lua_GetSceneUpValue(L);
	int entity = lua_tointeger(L, 1);
	std::string type = lua_tostring(L, 2);

	bool hasComponent = true;

	if (type == "behaviour")
		hasComponent = scene->HasComponents<BehaviourComponent>(entity);
	else if (type == "transform")
		hasComponent = scene->HasComponents<TransformComponent>(entity);

	lua_pushboolean(L, hasComponent);
	return 1;
}

int Scene::lua_SetComponent(lua_State* L)
{
	Scene* scene = lua_GetSceneUpValue(L);

	int entity = lua_tointeger(L, 1);
	std::string type = lua_tostring(L, 2);

	if (type == "health")
	{
		float value = lua_tonumber(L, 3);
		scene->SetComponent<Health>(entity, value);
	}
	else if (type == "poison")
	{
		float tickDamage = lua_tonumber(L, 3);
		scene->SetComponent<Poison>(entity, tickDamage);
	}
	else if (type == "behaviour")
	{
		if (scene->HasComponents<BehaviourComponent>(entity))
		{
			scene->RemoveComponent<BehaviourComponent>(entity);
		}

		const char* path = lua_tostring(L, 3);
		int ref = RefAndPushBehaviour(L, entity, path);

		lua_State* thread = lua_newthread(L);
		lua_setfield(L, -2, "thread");

		scene->SetComponent<YieldableBehaviour>(entity, path, ref, thread);
		//scene->SetComponent<BehaviourComponent>(entity, path, ref);

		return 1;
	}
	if (type == "transform")
	{
		scene->SetComponent<TransformComponent>(entity);
	}

	return 0;
}

int Scene::lua_GetComponent(lua_State* L)
{
	Scene* scene = lua_GetSceneUpValue(L);

	if (!lua_isinteger(L, 1) || !lua_isstring(L, 2))
	{
		lua_pushnil(L);
		return 1;
	}
	
	int entity = lua_tointeger(L, 1);
	std::string type = lua_tostring(L, 2);

	// Sanity check that the entity exist
	if (!scene->IsEntity(entity))
	{
		lua_pushnil(L);
		return 1;
	}

	if (type == "behaviour" && scene->HasComponents<BehaviourComponent>(entity))
	{
		BehaviourComponent& behaviour = scene->GetComponent<BehaviourComponent>(entity);
		lua_pushbehaviour(L, behaviour);
		return 1;
	}
	else if (type == "transform" && scene->HasComponents<TransformComponent>(entity))
	{
		TransformComponent& transform = scene->GetComponent<TransformComponent>(entity);
		lua_pushtransform(L, transform);
		return 1;
	}

	lua_pushnil(L);
	return 1;
}

int Scene::lua_RemoveComponent(lua_State* L)
{
	Scene* scene = lua_GetSceneUpValue(L);

	int entity = lua_tointeger(L, 1);
	std::string type = lua_tostring(L, 2);

	if (type == "behaviour")
		scene->RemoveComponent<BehaviourComponent>(entity);
	else if (type == "transform")
		scene->RemoveComponent<TransformComponent>(entity);

	return 0;
}

void Scene::lua_openbehaviour(lua_State* L, Scene* scene)
{
	luaL_newmetatable(L, "BehaviourMeta");

	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");

	luaL_Reg behaviourMetaMethods[] =
	{
		{ "GetComponent", lua_BehaviourGetComponent },
		{ "SetComponent", lua_BehaviourSetComponent },
		{ "RemoveComponent", lua_BehaviourRemoveComponent },
		{ NULL, NULL }
	};
	lua_pushlightuserdata(L, scene);
	luaL_setfuncs(L, behaviourMetaMethods, 1);
	lua_setglobal(L, "Behaviour");
}

int Scene::lua_BehaviourGetComponent(lua_State* L)
{
	Scene* scene = lua_GetSceneUpValue(L);

	lua_getfield(L, 1, "ID");
	int entity = lua_tointeger(L, -1);
	lua_pop(L, 1);

	std::string type = lua_tostring(L, 2);

	if (type == "behaviour")
	{
		lua_pushvalue(L, 1);
		return 1;
	}
	else if (type == "transform")
	{
		TransformComponent& transform = scene->GetComponent<TransformComponent>(entity);
		lua_pushtransform(L, transform);
		return 1;
	}
	
	lua_pushnil(L);
	return 1;
}

int Scene::lua_BehaviourSetComponent(lua_State* L)
{
	Scene* scene = lua_GetSceneUpValue(L);

	lua_getfield(L, 1, "ID");
	int entity = lua_tointeger(L, -1);
	lua_pop(L, 1);

	std::string type = lua_tostring(L, 2);

	if (type == "behaviour")
	{
		if (scene->HasComponents<BehaviourComponent>(entity))
		{
			scene->RemoveComponent<BehaviourComponent>(entity);
		}

		const char* path = lua_tostring(L, 3);
		int ref = RefAndPushBehaviour(L, entity, path);
		scene->SetComponent<BehaviourComponent>(entity, path, ref);
	}
	else if (type == "transform")
	{
		scene->SetComponent<TransformComponent>(entity, lua_totransform(L, 3));
	}

	return 0;
}

int Scene::lua_BehaviourRemoveComponent(lua_State* L)
{
	Scene* scene = lua_GetSceneUpValue(L);

	lua_getfield(L, 1, "ID");
	int entity = lua_tointeger(L, -1);
	lua_pop(L, 1);

	std::string type = lua_tostring(L, 2);

	if (type == "behaviour")
	{
		BehaviourComponent& behaviour = scene->GetComponent<BehaviourComponent>(entity);
		UnrefBehaviour(L, behaviour.LuaTableRef);
		scene->RemoveComponent<BehaviourComponent>(entity);
	}
	else if (type == "transform")
	{
		scene->RemoveComponent<TransformComponent>(entity);
	}
	
	return 0;
}
