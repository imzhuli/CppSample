#pragma once
#include <zec/Common.hpp>
#include <lua.hpp>
#include <tuple>
#include <string>

ZEC_NS
{

    class xLuaStateWrapper
    {
    public:
        ZEC_INLINE xLuaStateWrapper() = default;
        ZEC_INLINE xLuaStateWrapper(lua_State * LuaStatePtr) : _LuaStatePtr(LuaStatePtr) {}
        ZEC_INLINE ~xLuaStateWrapper() = default;

        ZEC_INLINE operator lua_State * () const { return _LuaStatePtr; }

        ZEC_INLINE bool LoadStr(const char * CodeStr) {
            return LUA_OK == luaL_loadstring(_LuaStatePtr, CodeStr);
        }
        ZEC_INLINE bool LoadStr(const std::string & CodeStr) {
            return LUA_OK == luaL_loadstring(_LuaStatePtr, CodeStr.c_str());
        }

        template<typename tArg>
        ZEC_INLINE std::enable_if_t<std::is_integral_v<tArg>, tArg> GetAt(int Index) const { return luaL_checkinteger(_LuaStatePtr, Index); }
        template<typename tArg>
        ZEC_INLINE std::enable_if_t<std::is_floating_point_v<tArg>, tArg> GetAt(int Index) const { return luaL_checknumber(_LuaStatePtr, Index); }
        template<typename tArg>
        ZEC_INLINE std::enable_if_t<std::is_same_v<tArg, const char *>, tArg> GetAt(int Index) const { return luaL_checkstring(_LuaStatePtr, Index); }
        template<typename tArg>
        ZEC_INLINE std::enable_if_t<std::is_same_v<tArg, std::string>, tArg> GetAt(int Index) const { return luaL_checkstring(_LuaStatePtr, Index); }

        ZEC_INLINE auto GetTop() const { return lua_gettop(_LuaStatePtr); }
        ZEC_INLINE auto SetTop(int Index) const { lua_settop(_LuaStatePtr, Index); }
        ZEC_INLINE auto Pop(int Number) const { lua_pop(_LuaStatePtr, Number); }

        ZEC_INLINE void Push() const {}
        ZEC_INLINE void Push(lua_Integer IntValue) const { lua_pushinteger(_LuaStatePtr, IntValue); }
        ZEC_INLINE void Push(const char * StrValue) const { lua_pushstring(_LuaStatePtr, StrValue); }
        ZEC_INLINE void Push(const std::string& StrValue) const { lua_pushstring(_LuaStatePtr, StrValue.c_str()); }
        ZEC_INLINE void Push(int (*Func)(lua_State*)) const { lua_pushcfunction(_LuaStatePtr, Func); }

        template<typename tArg>
        ZEC_INLINE std::enable_if_t<std::is_floating_point_v<tArg>> Push(tArg Number) { lua_pushnumber(_LuaStatePtr, Number); }
        template<typename...Args>
        ZEC_INLINE void PushF(const char * FmtStr, Args&&...args) const { lua_pushfstring(_LuaStatePtr, FmtStr, std::forward<Args>(args)...); }

        template<typename tFirstArg, typename...tOtherArgs>
        ZEC_INLINE std::enable_if_t<sizeof...(tOtherArgs), int> Push(tFirstArg&& FirstArg, tOtherArgs&&...args) const {
            Push(std::forward<tFirstArg>(FirstArg));
            Push(std::forward<tOtherArgs>(args)...);
            return 1 + (int)sizeof...(args);
        }

        template<typename...Args>
        ZEC_INLINE void SetGlobal(const char * name, Args&&...args) {
            Push(std::forward<Args>(args)...);
            lua_setglobal(_LuaStatePtr, name);
        }

        ZEC_INLINE lua_Integer PopInt() const {
            auto Top = lua_gettop(_LuaStatePtr);
            assert (Top);
            auto Ret = lua_tointeger(_LuaStatePtr, Top);
            lua_pop(_LuaStatePtr, 1);
            return Ret;
        }

        ZEC_INLINE lua_Number PopNumber() const {
            auto Top = lua_gettop(_LuaStatePtr);
            assert (Top);
            auto Ret = lua_tonumber(_LuaStatePtr, Top);
            lua_pop(_LuaStatePtr, 1);
            return Ret;
        }

        ZEC_INLINE std::string PopStr() const {
            auto Top = lua_gettop(_LuaStatePtr);
            assert (Top);
            auto Ret = lua_tostring(_LuaStatePtr, Top);
            lua_pop(_LuaStatePtr, 1);
            return Ret;
        }

        template<typename tArg>
        ZEC_INLINE std::enable_if_t<std::is_integral_v<tArg>, tArg> Pop() const { return (tArg)PopInt(); }
        template<typename tArg>
        ZEC_INLINE std::enable_if_t<std::is_floating_point_v<tArg>, tArg> Pop() const { return (tArg)PopNumber(); }
        template<typename tArg>
        ZEC_INLINE std::enable_if_t<std::is_same_v<tArg, std::string>, tArg> Pop() const { return PopStr(); }

        template<typename...tArgs>
        ZEC_INLINE std::enable_if_t<(sizeof...(tArgs) > 1),std::tuple<tArgs...>> Pop() const {
            auto Top = lua_gettop(_LuaStatePtr); assert (Top);
            std::tuple<tArgs...> Result;
            _FillTuple<0>(Result, Top + 1 - sizeof...(tArgs));
            lua_pop(_LuaStatePtr, (int)(sizeof...(tArgs)));
            return Result;
        }

        ZEC_INLINE void Error(const char * Reason) const { lua_pushstring(_LuaStatePtr, Reason); lua_error(_LuaStatePtr); }
        ZEC_INLINE bool Execute(const char * CodeStr) const { return LUA_OK == luaL_dostring(_LuaStatePtr, CodeStr); }
        ZEC_INLINE bool ExecuteFile(const char * Filename) const { return LUA_OK == luaL_dofile(_LuaStatePtr, Filename); }
        template<size_t ResultNumber = 0, typename...tArgs>
        ZEC_INLINE void CallN(const char * name, tArgs&&...args) const {
            lua_getglobal(_LuaStatePtr, name);
            Push(std::forward<tArgs>(args)...);
            lua_call(_LuaStatePtr, sizeof...(args), ResultNumber);
        }

        template<typename...tRetVars, typename...tArgs>
        ZEC_INLINE auto Call(const char * name, tArgs&&...args) const {
            CallN<sizeof...(tRetVars)>(name, std::forward<tArgs>(args)...);
            return Pop<tRetVars...>();
        }

    protected:
        lua_State * _LuaStatePtr = nullptr;

    public:
        template<typename...tRetVars>
        ZEC_INLINE auto Get() const { return xGet<tRetVars...>{this}; }

    private:
        template<size_t MemberIndex, typename tTuple>
        ZEC_INLINE void _FillTuple(tTuple & Tuple, int StackOffset) const {
            static_assert(!std::is_same_v<std::remove_reference_t<decltype(std::get<MemberIndex>(Tuple))>, const char *>);
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
        ZEC_API_MEMBER xLuaState();
        ZEC_API_MEMBER ~xLuaState();
    };

}
