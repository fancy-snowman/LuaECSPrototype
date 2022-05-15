#include "LuaCommon.hpp"
#include <string>
#include <iostream>
#include <iomanip>

void DumpError(lua_State* L)
{
	if (lua_gettop(L) && lua_isstring(L, -1))
	{
		std::cout << "Lua error: " << lua_tostring(L, -1) << std::endl;
		lua_pop(L, 1);
	}
}

void DumpStack(lua_State* L)
{
	int size = lua_gettop(L);

	std::cout << " " << std::string(15, '-');
	std::cout << " STACK BEGIN ";
	std::cout << std::string(15, '-') << std::endl;

	for (int i = size; i > 0; i--)
	{
		int type = lua_type(L, i);
		std::string typeName = lua_typename(L, type);
		std::string value;

		if (type == LUA_TSTRING)
		{
			value = "\"";
			value += lua_tostring(L, i);

			if (value.size() > 18)
			{
				value.resize(16);
				value += "..";
			}

			value += "\"";
		}
		else if (type == LUA_TBOOLEAN)
		{
			value = lua_toboolean(L, i) ? "true" : "false";
		}
		else if (type == LUA_TNIL)
		{
			value = "nil";
		}
		else if (lua_isnumber(L, i))
		{
			value = std::to_string(lua_tonumber(L, i));
		}
		else if (lua_isstring(L, i))
		{
			value = lua_tostring(L, i);
		}

		std::cout << std::setw(3) << i << " | ";
		std::cout << typeName << std::setw(31 - typeName.size()) << value;
		std::cout << std::setw(5 - typeName.size() - value.size()) << " | ";
		std::cout << std::setw(2) << -(size - i + 1);
		std::cout << std::endl;
	}

	std::cout << " " << std::string(16, '-');
	std::cout << " STACK END ";
	std::cout << std::string(16, '-') << std::endl;
}
