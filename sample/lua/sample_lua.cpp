#include <zec_ext/Lua/LuaWrap.hpp>
#include <iostream>

using namespace zec;
using namespace std;

int LuaCallbackSubSub(lua_State * LP)
{
	auto W = xLuaStateWrapper(LP);
	return W.Push(1, 2, "Hello R3");
}

int main(int, char *[])
{
	xLuaState LuaState;

	LuaState.SetGlobal("global_callback_subsub", LuaCallbackSubSub);
	LuaState.Call<3>("global_callback_subsub");	
	auto [R1,R2,R3] = LuaState.Pop<int, int, std::string>();
	cout << "Results: " << R1 << ", " << R2 << ", " << R3 << endl;

	return 0;
}
