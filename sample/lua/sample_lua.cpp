#include "./LuaWrap.hpp"
#include <iostream>

using namespace zec;
using namespace std;


int LuaCallback(lua_State * LP)
{
	auto W = xLuaStateWrapper(LP);
	W.Execute("print('LuaCallback')");
	return 1;
}

int LuaCallbackSub(lua_State * LP)
{
	auto W = xLuaStateWrapper(LP);
	auto [Param1, Param2, Param3] = W.Pop<int, int, const char *>();
	return W.BatchPush(Param1 + Param2, Param1 - Param2, Param3);
}

int LuaCallbackSubSub(lua_State * LP)
{
	auto W = xLuaStateWrapper(LP);
	return W.BatchPush(1, 2, "Hello R3");
}

int main(int, char *[])
{
	xLuaState LuaState;

	auto Code1 = "print('Hello World!')11";
	if (!LuaState.Execute(Code1)) {
		cout << LuaState.PopStr() << endl;
	}

	auto Code2 = "print('Hello World!')";
	if (!LuaState.Execute(Code2)) {
		cout << LuaState.PopStr() << endl;
	}
	
	LuaState.SetGlobal("global_int", 1024);
	LuaState.SetGlobal("global_str", "Hello string!");
	LuaState.SetGlobal("global_fstr", "Hello the int value is %d!", 1024);
	LuaState.SetGlobal("global_fstr1", "Hello the int value is %f!", 123.456);
	LuaState.SetGlobal("global_callback", LuaCallback);
	LuaState.SetGlobal("global_callback_sub", LuaCallbackSub);
	LuaState.SetGlobal("global_callback_subsub", LuaCallbackSubSub);

	LuaState.Execute("print(global_int)");
	LuaState.Execute("print(global_str)");
	LuaState.Execute("print(global_fstr)");
	LuaState.Execute("print(global_fstr1)");
	LuaState.Execute("global_callback()");

	LuaState.Execute("print(global_callback_sub(1000, 112, 'hello world!'))");

	do {
		LuaState.Call<3>("global_callback_subsub");	
		auto [R1,R2,R3] = LuaState.Pop<int, int, const char *>();
		cout << "Results: " << R1 << ", " << R2 << ", " << R3 << endl;
	} while(false);

	cout << "Top: " << lua_gettop(LuaState) << endl;

	return 0;
}
