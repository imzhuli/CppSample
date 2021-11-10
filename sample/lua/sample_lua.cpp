#include "./LuaWrap.hpp"
#include <iostream>

using namespace zec;
using namespace std;

int main(int, char *[])
{
	xLuaState LuaState;

	if (LuaState.LoadString("print('Hello World!')")) {
        if (lua_pcall(LuaState, 0, 0, 0) == LUA_OK) {
            // If it was executed successfuly we 
            // remove the code from the stack
            lua_pop(LuaState, lua_gettop(LuaState));
        }
    }
	
	return 0;
}
