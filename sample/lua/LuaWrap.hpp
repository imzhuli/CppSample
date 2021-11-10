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

private:
    lua_State * _LuaStatePtr = nullptr;
};

