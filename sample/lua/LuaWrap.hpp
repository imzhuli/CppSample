#pragma once
#include "./Common.hpp"
#include <lua.hpp>
#include <tuple>
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

    template<typename tArg>
    inline std::enable_if_t<std::is_integral_v<tArg>, tArg> GetAt(int Index) const { return luaL_checkinteger(_LuaStatePtr, Index); }
    template<typename tArg>
    inline std::enable_if_t<std::is_floating_point_v<tArg>, tArg> GetAt(int Index) const { return luaL_checknumber(_LuaStatePtr, Index); }
    template<typename tArg>
    inline std::enable_if_t<std::is_same_v<tArg, const char *>, tArg> GetAt(int Index) const { return luaL_checkstring(_LuaStatePtr, Index); }
 
    inline auto GetTop() const { return lua_gettop(_LuaStatePtr); }
    inline auto SetTop(int Index) const { lua_settop(_LuaStatePtr, Index); }
    inline auto Pop(int Number) const { lua_pop(_LuaStatePtr, Number); }

    inline void Push(lua_Integer IntValue) const { lua_pushinteger(_LuaStatePtr, IntValue); }
    inline void Push(const char * StrValue) const { lua_pushstring(_LuaStatePtr, StrValue); }
    inline void Push(int (*Func)(lua_State*)) const { lua_pushcfunction(_LuaStatePtr, Func); }
    
    template<typename tArg>
    inline std::enable_if_t<std::is_floating_point_v<tArg>> Push(tArg Number) { lua_pushnumber(_LuaStatePtr, Number); }
    template<typename...Args>
    inline void Push(const char * FmtStr, Args&&...args) const { lua_pushfstring(_LuaStatePtr, FmtStr, std::forward<Args>(args)...); }

    inline int BatchPush() const { return 0; }
    template<typename tFirstArg, typename...tOtherArgs>
    inline int BatchPush(tFirstArg&& FirstArg, tOtherArgs&&...args) const { 
        Push(std::forward<tFirstArg>(FirstArg));
        BatchPush(std::forward<tOtherArgs>(args)...);
        return 1 + (int)sizeof...(args);
    }

    template<typename...Args>
    inline void SetGlobal(const char * name, Args&&...args) {
        Push(std::forward<Args>(args)...);
        lua_setglobal(_LuaStatePtr, name);
    }

    inline lua_Integer PopInt() const {
        auto Top = lua_gettop(_LuaStatePtr); 
        assert (Top);
        auto Ret = lua_tointeger(_LuaStatePtr, Top); 
        lua_pop(_LuaStatePtr, 1); 
        return Ret;
    }

    inline lua_Number PopNumber() const {
        auto Top = lua_gettop(_LuaStatePtr); 
        assert (Top);
        auto Ret = lua_tonumber(_LuaStatePtr, Top);
        lua_pop(_LuaStatePtr, 1); 
        return Ret;
    }

    inline const char * PopStr() const {
        auto Top = lua_gettop(_LuaStatePtr); 
        assert (Top);
        auto Ret = lua_tostring(_LuaStatePtr, Top); 
        lua_pop(_LuaStatePtr, 1); 
        return Ret;
    }

    template<typename tArg>
    inline std::enable_if_t<std::is_integral_v<tArg>, tArg> Pop() const { return (tArg)PopInt(); }
    template<typename tArg>
    inline std::enable_if_t<std::is_floating_point_v<tArg>, tArg> Pop() const { return (tArg)PopNumber(); }
    template<typename tArg>
    inline std::enable_if_t<std::is_same_v<tArg, const char *>, tArg> Pop() const { return PopStr(); }

    template<typename...tArgs>
    inline std::enable_if_t<(sizeof...(tArgs) > 1),std::tuple<tArgs...>> Pop() const {
        auto Top = lua_gettop(_LuaStatePtr); assert (Top);
        std::tuple<tArgs...> Result;
        _FillTuple<0>(Result, Top + 1 - sizeof...(tArgs));
        lua_pop(_LuaStatePtr, (int)(sizeof...(tArgs)));
        return Result;
    }

    inline void Error(const char * Reason) const { lua_pushstring(_LuaStatePtr, Reason); lua_error(_LuaStatePtr); }
    inline bool Execute(const char * CodeStr) const { return LUA_OK == luaL_dostring(_LuaStatePtr, CodeStr); }
    template<size_t ResultNumber = 0, typename...tArgs>
    inline void Call(const char * name, tArgs&&...args) {
        lua_getglobal(_LuaStatePtr, name);        
        BatchPush(std::forward<tArgs>(args)...);
        lua_call(_LuaStatePtr, sizeof...(args), ResultNumber);        
    }

protected:
    lua_State * _LuaStatePtr = nullptr;

private:
    template<size_t MemberIndex, typename tTuple>
    inline void _FillTuple(tTuple & Tuple, int StackOffset) const {
        std::get<MemberIndex>(Tuple) = GetAt<std::remove_reference_t<decltype(std::get<MemberIndex>(Tuple))>>(StackOffset);
        if constexpr(MemberIndex < std::tuple_size<tTuple>() - 1) {            
            _FillTuple<MemberIndex + 1>(Tuple, StackOffset + 1);            
        }
    }
};

class xLuaState
: public xLuaStateWrapper
, xNonCopyable
{
public:
    xLuaState();
    ~xLuaState();
};
