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
	W.Execute("print('LuaCallbackSub')");

	auto Param1 = W.GetIntParam(1);
	auto Param2 = W.GetIntParam(2);
	auto Param3 = W.GetStrParam(3);

	cout << "Callback inner result: " << (int)(Param1 - Param2) << endl;
	cout << "Callback sub param3: " << Param3 << endl;

	W.Push(Param1 - Param2);
	return 1;
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

	LuaState.Execute("print(global_int)");
	LuaState.Execute("print(global_str)");
	LuaState.Execute("print(global_fstr)");
	LuaState.Execute("print(global_fstr1)");
	LuaState.Execute("global_callback()");

	LuaState.Execute("print(global_callback_sub(1000, 112, 'hello world!'))");

	LuaState.Call<1>("global_callback_sub", 1000, 112, "hello world!");
	auto CallResult = LuaState.PopInt();
	cout << "CallResult: " << CallResult << endl;

	return 0;
}
