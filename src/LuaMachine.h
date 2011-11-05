#ifndef LUA_H_
#define LUA_H_

// Lua entry points
#include "Lua/lua_Algorithm.h"
#include "Lua/lua_Audio.h"
#include "Lua/lua_Input.h"
#include "Lua/lua_Platform.h"

/// Embeds Lua scripting engine.
///
/// Features a simple C++-wrapper, adapted from lua-users.org
///
/// \see http://www.lua.org/manual/5.1/
/// \see http://lua-users.org/wiki/LunaWrapper
///
/// Copyright 2011 Bifrost Games. All rights reserved.
/// \author Tommy Nguyen
class LuaMachine
{
public:
	template<class T>
	struct Method
	{
		const char *const name;
		int (T::*lua_CFunction)(lua_State *);
	};

	/// Create a Lua wrapped object.
	template<class T>
    static int alloc(lua_State *L);

	/// Dump Lua stack to stdout.
	static void dump_stack(lua_State *L);

	/// Return information about a specific function or function invocation.
	static lua_Debug* getinfo(lua_State *L);

	~LuaMachine();

private:
	template<class T>
	static int dealloc(lua_State *L);

	template<class T>
	static int thunk(lua_State *L);

	/// Return the wrapper of the object on top of the stack.
	/// \return Pointer to wrapper
	template<class T>
	static T* wrapper(lua_State *L);

	lua_State *L;

	LuaMachine();

	/// Intentionally left undefined.
	LuaMachine(const LuaMachine &);

	/// Call a function with no parameters or return value.
	/// \param k  Name of the function to call
	void call(const char *const);

	void err(int);

	/// Load specified Lua script.
	void load(const char *const);

	/// Update world state and call update function in Lua script.
	void update();

	/// Wrap a C++ object and makes its methods available in Lua.
	/// \param ns  Insert object into specified namespace
	template<class T>
	void wrap(const char *const ns = 0);

	/// Intentionally left undefined.
	LuaMachine& operator=(const LuaMachine &);

	friend class Director;
};

template<class T>
int LuaMachine::alloc(lua_State *L)
{
	T *obj = new T(L);
	lua_pop(L, lua_gettop(L));

	lua_createtable(L, 0, 0);
	lua_pushnumber(L, 0);

	T **ptr = static_cast<T **>(lua_newuserdata(L, sizeof(T*)));
	*ptr = obj;

	lua_getfield(L, LUA_REGISTRYINDEX, T::class_name);
	lua_setmetatable(L, -2);
	lua_settable(L, -3);

	for (int i = 0; T::methods[i].name; ++i)
	{
		lua_pushstring(L, T::methods[i].name);
		lua_pushnumber(L, i);
		lua_pushcclosure(L, &LuaMachine::thunk<T>, 1);
		lua_settable(L, -3);
	}

	return 1;
}

inline LuaMachine::~LuaMachine()
{
	lua_close(this->L);
}

template<class T>
int LuaMachine::dealloc(lua_State *L)
{
	T **ptr = static_cast<T **>(luaL_checkudata(L, -1, T::class_name));
	delete *ptr;
	return 0;
}

template<class T>
int LuaMachine::thunk(lua_State *L)
{
#ifndef NDEBUG

	if (lua_type(L, 1) != LUA_TTABLE)
	{
		lua_Debug *d = getinfo(L);
		printf("%s:%i: error: Called a class function using '.' instead of ':'\n", d->source, d->currentline);
		delete d;
		assert(lua_type(L, 1) == LUA_TTABLE);
	}

#endif

	const int i = static_cast<int>(lua_tonumber(L, lua_upvalueindex(1)));
	lua_rawgeti(L, 1, 0);
	lua_remove(L, 1);

	T **ptr = static_cast<T **>(luaL_checkudata(L, -1, T::class_name));
	lua_pop(L, 1);

	return ((*ptr)->*(T::methods[i].lua_CFunction))(L);
}

template<class T>
T* LuaMachine::wrapper(lua_State *L)
{
	// Get user data from table
	lua_rawgeti(L, -1, 0);
	void *ptr = luaL_checkudata(L, -1, T::class_name);
	lua_pop(L, 1);
	return *static_cast<T **>(ptr);
}

template<class T>
void LuaMachine::wrap(const char *const ns)
{
	if (ns && strcmp(ns, ""))
	{
		lua_getfield(this->L, LUA_GLOBALSINDEX, ns);

		if (lua_type(this->L, -1) != LUA_TTABLE)
		{
			lua_pop(this->L, 1);
			lua_createtable(this->L, 0, 0);
			lua_pushvalue(this->L, -1);
			lua_setfield(L, LUA_GLOBALSINDEX, ns);
		}

		lua_pushcclosure(this->L, &LuaMachine::alloc<T>, 0);
		lua_setfield(this->L, -2, T::class_name);
		lua_pop(this->L, 1);
	}
	else
	{
		lua_pushcclosure(this->L, &LuaMachine::alloc<T>, 0);
		lua_setfield(this->L, LUA_GLOBALSINDEX, T::class_name);
	}

	luaL_newmetatable(this->L, T::class_name);
	lua_pushstring(this->L, "__gc");
	lua_pushcclosure(this->L, &LuaMachine::dealloc<T>, 0);
	lua_settable(this->L, -3);
	lua_pop(this->L, 1);
}

// Lua wrappers
#include "Lua/lua_Font.h"
#include "Lua/lua_Physics.h"
#include "Lua/lua_SpriteBatch.h"
#include "Lua/lua_Sprite.h"
#include "Lua/lua_Texture.h"

#endif
