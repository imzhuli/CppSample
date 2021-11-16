#include <zec_ext/Lua/LuaWrap.hpp>
#include <iostream>

using namespace zec;
using namespace std;

int LuaCallbackSubSub(lua_State * LP)
{
	auto W = xLuaStateWrapper(LP);
	auto [p1, p2, p3] = W.Pop<int, int, int>();
	cout << "p1=" << p1 << ", p2=" << p2 << ", p3=" << p3 << endl;
	return W.Push(1, 2, "Hello R3");
}

int main(int, char *[])
{
	xLuaState LuaState;

	LuaState.SetGlobal("global_callback_subsub", LuaCallbackSubSub);
	auto [R1,R2,R3] = LuaState.Call<int, int, std::string>("global_callback_subsub", 1, 2, 3);
	cout << "Results: " << R1 << ", " << R2 << ", " << R3 << endl;

	return 0;
}
