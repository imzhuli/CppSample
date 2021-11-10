#pragma once
#include "./Common.hpp"
#include <lua.hpp>
#include <string>

class xLuaState
: xNonCopyable
{
public:
    xLuaState();
    ~xLuaState();

    inline operator lua_State * () const { return _LuaStatePtr; }

    inline bool LoadString(const char * CodeStr) {
        return LUA_OK == luaL_loadstring(_LuaStatePtr, CodeStr);
    }
    inline bool LoadString(const std::string & CodeStr) {
        return LUA_OK == luaL_loadstring(_LuaStatePtr, CodeStr.c_str());
    }
    inline auto GetTop() const { return lua_gettop(_LuaStatePtr); }
    inline auto SetTop(int Index) const { lua_settop(_LuaStatePtr, Index); }
    inline auto Pop(int Index) const { lua_pop(_LuaStatePtr, Index); }

    inline bool Execute(const char * CodeStr) const { return LUA_OK == luaL_dostring(_LuaStatePtr, CodeStr); }

private:
    lua_State * _LuaStatePtr = nullptr;
};

