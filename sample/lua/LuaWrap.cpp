#include "./LuaWrap.hpp"
#include <new>

xLuaState::xLuaState()
{
    _LuaStatePtr = luaL_newstate();
    if (!_LuaStatePtr) {
        throw std::bad_alloc();
    }
    luaL_openlibs(_LuaStatePtr);
}

xLuaState::~xLuaState()
{
    lua_close(Steal(_LuaStatePtr));
}
