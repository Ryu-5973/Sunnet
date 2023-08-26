#pragma once

extern "C" {
    #include "lua.h"
    #include "lauxlib.h"
}

class LuaAPI {
public:
    static void Register(lua_State* luaState);

    // sunnet相关接口
    static int NewService(lua_State* luaState);
    static int KillService(lua_State* luaState);
    static int Send(lua_State* luaState);
    static int Listen(lua_State* luaState);
    static int CloseConn(lua_State* luaState);
    static int Write(lua_State* luaState);

    // log相关接口
    static int LogInfo(lua_State* luaState);
    static int LogDebug(lua_State* luaState);
    static int LogWarn(lua_State* luaState);
    static int LogErr(lua_State* luaState);


};
