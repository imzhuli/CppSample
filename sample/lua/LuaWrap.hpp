#pragma once
#include "./Common.hpp"
#include <lua.hpp>
#include <string>

class xLuaStateWrapper
{
public:
    inline xLuaStateWrapper() = default;
    inline xLuaStateWrapper(lua_State * LuaStatePtr) : _LuaStatePtr(LuaStatePtr) {}
    inline ~xLuaStateWrapper() = default;

    inline operator lua_State * () const { return _LuaStatePtr; }

    inline bool LoadStr(const char * CodeStr) {
        return LUA_OK == luaL_loadstring(_LuaStatePtr, CodeStr);
    }
    inline bool LoadStr(const std::string & CodeStr) {
        return LUA_OK == luaL_loadstring(_LuaStatePtr, CodeStr.c_str());
    }

    inline auto GetTop() const { return lua_gettop(_LuaStatePtr); }
    // inline auto SetTop(int Index) const { lua_settop(_LuaStatePtr, Index); }
    // inline auto Pop(int Index) const { lua_pop(_LuaStatePtr, Index); }

    inline void Push(lua_Integer IntValue) const { lua_pushinteger(_LuaStatePtr, IntValue); }
    inline void Push(const char * StrValue) const { lua_pushstring(_LuaStatePtr, StrValue); }
    inline void Push(int (*Func)(lua_State*)) const { lua_pushcfunction(_LuaStatePtr, Func); }
    
    template<typename tArg>
    inline std::enable_if_t<std::is_floating_point_v<tArg>> Push(tArg Number) { lua_pushnumber(_LuaStatePtr, Number); }
    template<typename...Args>
    inline void Push(const char * FmtStr, Args&&...args) const { lua_pushfstring(_LuaStatePtr, FmtStr, std::forward<Args>(args)...); }

    inline void BatchPush() const {}
    template<typename tFirstArg, typename...tOtherArgs>
    inline void BatchPush(tFirstArg&& FirstArg, tOtherArgs&&...args) const { 
        Push(std::forward<tFirstArg>(FirstArg));
        BatchPush(std::forward<tOtherArgs>(args)...);
    }

    template<typename...Args>
    inline void SetGlobal(const char * name, Args&&...args) {
        Push(std::forward<Args>(args)...);
        lua_setglobal(_LuaStatePtr, name);
    }

    lua_Integer PopInt() const {
        auto Top = lua_gettop(_LuaStatePtr); 
        assert (Top);
        auto Ret = lua_tointeger(_LuaStatePtr, Top); 
        lua_pop(_LuaStatePtr, Top); 
        return Ret;
    }

    lua_Number PopNumber() const {
        auto Top = lua_gettop(_LuaStatePtr); 
        assert (Top);
        auto Ret = lua_tonumber(_LuaStatePtr, Top);
        lua_pop(_LuaStatePtr, Top); 
        return Ret;
    }

    const char * PopStr() const {
        auto Top = lua_gettop(_LuaStatePtr); 
        assert (Top);
        auto Ret = lua_tostring(_LuaStatePtr, Top); 
        lua_pop(_LuaStatePtr, Top); 
        return Ret;
    }

    inline lua_Integer   GetIntParam(int Index) { assert(Index); return luaL_checkinteger(_LuaStatePtr, Index); }
    inline lua_Number    GetNumberParam(int Index) { assert(Index); return luaL_checknumber(_LuaStatePtr, Index); }
    inline const char *  GetStrParam(int Index) { assert(Index); return luaL_checkstring(_LuaStatePtr, Index); }
 
    inline bool Execute(const char * CodeStr) const { return LUA_OK == luaL_dostring(_LuaStatePtr, CodeStr); }
    template<size_t ResultNumber = 0, typename...tArgs>
    inline void Call(const char * name, tArgs&&...args) {
        lua_getglobal(_LuaStatePtr, name);        
        BatchPush(std::forward<tArgs>(args)...);
        lua_call(_LuaStatePtr, sizeof...(args), ResultNumber);
    }

protected:
    lua_State * _LuaStatePtr = nullptr;
};

class xLuaState
: public xLuaStateWrapper
, xNonCopyable
{
public:
    xLuaState();
    ~xLuaState();
};
