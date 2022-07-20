#include <zec_ext/Lua/LuaWrap.hpp>
#include <iostream>

using namespace zec;
using namespace std;

int LuaCallbackSubSub(lua_State * LP)
{
	auto W = xLuaStateWrapper(LP);
	auto [p1, p2, p3] = W.Get<int, double, const char *>();
	cout << "p1=" << p1 << ", p2=" << p2 << ", p3=" << p3 << endl;
	return W.Return(1, 2.2, "Hello R3");
}

int main(int, char *[])
{
	xLuaState LuaState;

	do {
		LuaState.SetGlobal("global_callback_subsub", LuaCallbackSubSub);
		do { // normal calling
			int p1 = 1;
			float p2 = 2.1f;
			const char * p3 = "Param3";
            lua_getglobal(LuaState, "global_callback_subsub");
			lua_pushinteger(LuaState, p1);
			lua_pushnumber(LuaState, p2);
			lua_pushstring(LuaState, p3);
            lua_call(LuaState, 3, 3);
			int top = lua_gettop(LuaState);
			std::string r3 = luaL_checkstring(LuaState, top);
			auto r2 = luaL_checknumber(LuaState, top - 1);
			auto r1 = luaL_checkinteger(LuaState, top - 2);
			lua_pop(LuaState, 3);
			cout << "Results: " << r1 << ", " << r2 << ", " << r3 << endl;
		} while(false);
		do { // shortcut
			auto [R1,R2,R3] = LuaState.Call<int, float, std::string>("global_callback_subsub", 1, 2.0, "Param3");
			cout << "Results: " << R1 << ", " << R2 << ", " << R3 << endl;
		} while(false);
	} while(false);

	do {
		LuaState.Push(1, 2,3,4,5,6,7,8,9,10); // for RR0 check
		LuaState.SetGlobal("global_callback_subsub", LuaCallbackSubSub);
		LuaState.CallN<3>("global_callback_subsub", 1, 2.1, "ThisIsParamString");
		auto [R0, R1,R2,R3] = LuaState.Get<int, int, float, const char *>();
		cout << "Results: " << R1 << ", " << R2 << ", " << R3 << ", " << R0 << endl;

		cout << "Top: " << LuaState.GetTop() << endl;

		auto [RR3] = LuaState.Pop<std::string>();
		auto [RR2] = LuaState.Pop<float>();
		auto [RR1] = LuaState.Pop<int>();
		auto [RR0] = LuaState.Pop<int>();
		cout << "Results: " << RR1 << ", " << RR2 << ", " << RR3 << ", " << RR0 << endl;
	} while(false);

	do {
		LuaState.Call("print", 1, 2.5, "hello world!");
	} while(false);

	return 0;
}
