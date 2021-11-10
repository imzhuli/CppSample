#include "./LuaWrap.hpp"
#include <new>

xLuaState::xLuaState()
{
#if LUA_VERSION_NUM >= 503
    _LuaStatePtr = luaL_newstate();
#else
    _LuaStatePtr = lua_open();
#endif

    if (!_LuaStatePtr) {
        throw std::bad_alloc();
    }
    luaL_openlibs(_LuaStatePtr);
}

xLuaState::~xLuaState()
{
    lua_close(Steal(_LuaStatePtr));
}
