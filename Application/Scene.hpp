#pragma once

#include "lua.hpp"
#include "entt\entt.hpp"
#include <vector>


class System
{
public:
	virtual bool OnUpdate(entt::registry& registry, float delta) = 0;
};

class Scene
{
	lua_State* m_luaState;
	entt::registry m_registry;
	std::vector<System*> m_systems;

public:

	Scene(lua_State* L);
	~Scene();

public:

	// Entities

	int GetEntityCount();

	int CreateEntity();
	bool IsEntity(int entity);
	void RemoveEntity(int entity);

	// Components

	template <typename...Args>
	bool HasComponents(int entity);

	template <typename T>
	T& GetComponent(int entity);

	template <typename T>
	void SetComponent(int entity, const T&);

	template <typename T, typename...Args>
	void SetComponent(int entity, Args... args);

	template <typename T>
	void RemoveComponent(int entity);

	// Systems

	template <typename T, typename...Args>
	void CreateSystem(Args... args);
	
	void UpdateSystems(float delta);

public:

	static void lua_openscene(lua_State* L, Scene* scene);

private:

	static Scene* lua_GetSceneUpValue(lua_State* L);
	static int RefAndPushBehaviour(lua_State* L, int entity, const char* path);
	static void UnrefBehaviour(lua_State* L, int ref);

	static int lua_GetEntityCount(lua_State* L);
	static int lua_CreateEntity(lua_State* L);
	static int lua_IsEntity(lua_State* L);
	static int lua_RemoveEntity(lua_State* L);

	static int lua_HasComponent(lua_State* L);
	static int lua_GetComponent(lua_State* L);
	static int lua_SetComponent(lua_State* L);
	static int lua_RemoveComponent(lua_State* L);

	static void lua_openbehaviour(lua_State* L, Scene* scene);

	static int lua_BehaviourGetComponent(lua_State* L);
	static int lua_BehaviourSetComponent(lua_State* L);
	static int lua_BehaviourRemoveComponent(lua_State* L);
};

// ------------------

template<typename ... Args>
inline bool Scene::HasComponents(int entity)
{
	return m_registry.all_of<Args...>((entt::entity)entity);
}

template<typename T>
inline T& Scene::GetComponent(int entity)
{
	return m_registry.get<T>((entt::entity)entity);
}

template<typename T>
inline void Scene::SetComponent(int entity, const T& component)
{
	m_registry.emplace_or_replace<T>((entt::entity)entity, component);
}

template<typename T, typename ...Args>
inline void Scene::SetComponent(int entity, Args... args)
{
	T& comp = m_registry.emplace_or_replace<T>((entt::entity)entity, args...);
}

template<typename T>
inline void Scene::RemoveComponent(int entity)
{
	m_registry.remove<T>((entt::entity)entity);
}

template<typename T, typename ...Args>
inline void Scene::CreateSystem(Args ...args)
{
	m_systems.emplace_back(new T(args...));
}
