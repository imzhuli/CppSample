#include <zec_ext/Lua/LuaWrap.hpp>
#include <iostream>

using namespace zec;
using namespace std;

int LuaCallbackSubSub(lua_State * LP)
{
	auto W = xLuaStateWrapper(LP);
	auto [p1, p2, p3] = W.Get<int, int, const char *>();
	cout << "p1=" << p1 << ", p2=" << p2 << ", p3=" << p3 << endl;
	return W.Push(1, 2, "Hello R3");
}

int main(int, char *[])
{
	xLuaState LuaState;

	do {
		LuaState.SetGlobal("global_callback_subsub", LuaCallbackSubSub);
		auto [R1,R2,R3] = LuaState.Call<int, int, std::string>("global_callback_subsub", 1, 2, 3);
		cout << "Results: " << R1 << ", " << R2 << ", " << R3 << endl;
	} while(false);

	do {
		LuaState.SetGlobal("global_callback_subsub", LuaCallbackSubSub);
		LuaState.CallN<3>("global_callback_subsub", 1, 2, "ThisIsParamString");
		auto [R1,R2,R3] = LuaState.Get<int, int, const char *>();
		cout << "Results: " << R1 << ", " << R2 << ", " << R3 << endl;

		auto RR3 = LuaState.Pop<std::string>();
		auto RR2 = LuaState.Pop<int>();
		auto RR1 = LuaState.Pop<int>();
		cout << "Results: " << RR1 << ", " << RR2 << ", " << RR3 << endl;
	} while(false);

	return 0;
}
