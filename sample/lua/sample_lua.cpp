#include "./LuaWrap.hpp"
#include <iostream>

using namespace zec;
using namespace std;

int main(int, char *[])
{
	xLuaState LuaState;
	auto Code = "print('Hello World!')";

    if (LuaState.Execute(Code)) {
		// If it was executed successfuly we 
		// remove the code from the stack
		cout << "LuaStackTop = " << LuaState.GetTop() << endl;
		lua_pop(LuaState, lua_gettop(LuaState));
    }
	
	return 0;
}
