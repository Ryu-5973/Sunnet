#include <memory>
#include <string>
#include <cstring>
#include <iostream>
#include <string>

#include <stdint.h>
#include <unistd.h>

#include "LuaAPI.h"
#include "Sunnet.h"
#include "Msg.h"
#include "CommonDefs.h"


void LuaAPI::Register(lua_State* luaState) {
    // _g_Sunnnet
    static luaL_Reg lualibs_sunnet[] ={
        {"NewService", NewService},
        {"KillService", KillService},
        {"Send", Send},
        {"Listen", Listen},
        {"CloseConn", CloseConn},
        {"Write", Write},
        {NULL, NULL}
    };
    luaL_newlib(luaState, lualibs_sunnet);
    lua_setglobal(luaState, "sunnet");

    // _g_Log
    static luaL_Reg lualibs_log[] = {
        {"LogInfo", LogInfo},
        {"LogDebug", LogDebug},
        {"LogWarn", LogWarn},
        {"LogErr", LogErr},
        {NULL, NULL}
    };

    luaL_newlib(luaState, lualibs_log);
    lua_setglobal(luaState, "_g_Log");
}

int LuaAPI::NewService(lua_State* luaState) {
    // 参数个数
    int num = lua_gettop(luaState);
    // 参数1: 服务类型
    if(lua_isstring(luaState, 1) == 0) {
        lua_pushinteger(luaState, -1);
        return 0;
    }
    size_t len = 0;
    const char* type = lua_tolstring(luaState, 1, &len);
    char* newStr = new char[len + 1];
    newStr[len] = '\0';
    memcpy(newStr, type, len);
    auto t = std::make_shared<std::string>(newStr);
    uint32_t id = Sunnet::Inst->NewService(t);
    lua_pushinteger(luaState, id);

    return 1;
} 

int LuaAPI::KillService(lua_State* luaState) {
    int argNum = lua_gettop(luaState);
    if(argNum != 1 || lua_isinteger(luaState, 1) == 0) {
        lua_pushinteger(luaState, -1);
        return 0;
    }

    uint32_t id = lua_tointeger(luaState, 1);
    Sunnet::Inst->KillService(id);
    
    return 1;
}

int LuaAPI::Send(lua_State* luaState) {
    int argNum = lua_gettop(luaState);
    if(argNum != 3) {
        LOG_ERR("Send fail, argNum err");
        return 0;
    }
    if(lua_isinteger(luaState, 1) == 0) {
        LOG_ERR("Send fail, arg1 err");
        return 0;
    }
    int source = lua_tointeger(luaState, 1);
    if(lua_isinteger(luaState, 2) == 0) {
        LOG_ERR("Send fail, arg2 err");
        return 0;
    }
    int toId = lua_tointeger(luaState, 2);
    if(lua_isstring(luaState, 3) == 0) {
        LOG_ERR("Send fail, arg3 err");
        return 0;
    }
    size_t len = 0;
    const char* buff = lua_tolstring(luaState, 3, &len);
    char* newStr = new char[len];
    memcpy(newStr, buff, len);

    auto msg = std::make_shared<ServiceMsg>();
    msg->m_Type = BaseMsg::Type::SERVICE;
    msg->m_Source = source;
    msg->m_Buff = std::shared_ptr<char>(newStr);
    msg->m_Size = len;
    Sunnet::Inst->Send(toId, msg);

    return 1;
}

int LuaAPI::Listen(lua_State* luaState) {
    int argNum = lua_gettop(luaState);
    if(argNum != 3) {
        LOG_ERR("Listen Fail, argNum = %d", argNum);
        return 0;
    }
    if(lua_isinteger(luaState, 1) == 0) {
        LOG_ERR("Listen Fail, arg1 err");
        return 0;
    }
    uint32_t port = lua_tointeger(luaState, 1);
    if(lua_isinteger(luaState, 2) == 0) {
        LOG_ERR("Listen Fail, arg2 err");
        return 0;
    }
    uint32_t serviceId = lua_tointeger(luaState, 2);
    if(lua_isinteger(luaState, 3) == 0) {
        LOG_ERR("Listen Fail, arg3 err");
        return 0;
    }
    uint32_t protocolType = lua_tointeger(luaState, 3);
    int ret = Sunnet::Inst->Listen(luaState, port, serviceId, protocolType);
    lua_pushinteger(luaState, ret);

    return 1;
}

int LuaAPI::CloseConn(lua_State* luaState) {
    int argNum = lua_gettop(luaState);
    if(argNum != 1) {
        LOG_ERR("Close fail, argNum = %d", argNum);
        return 0;
    }

    if(lua_isinteger(luaState, 1) == 0) {
        LOG_ERR("CloseConn fail, arg1 err");
        return 0;
    }
    int fd = lua_tointeger(luaState, 1);
    Sunnet::Inst->CloseConn(fd);

    return 1;
}

int LuaAPI::Write(lua_State* luaState) {
    int argNum = lua_gettop(luaState);
    if(argNum != 2) {
        LOG_ERR("Write fail, argNum = %d", argNum);
        return 0;
    }
    if(lua_isinteger(luaState, 1) == 0) {
        LOG_ERR("Write fail, arg1 err");
        return 0;
    }
    int fd = lua_tointeger(luaState, 1);

    if (lua_isstring(luaState, 2) == 0) {
        LOG_ERR("Write fail, arg2 err");
        return 0;
    }
    size_t len = 0;
    const char* buff = lua_tolstring(luaState, 2, &len);

    int ret = write(fd, buff, len);
    lua_pushinteger(luaState, ret);
    
    return 1;
}


int LuaAPI::LogInfo(lua_State* luaState) {
    int argNum = lua_gettop(luaState);
    if(argNum != 1) {
        LOG_ERR("LogInfo fail, argNum = %d", argNum);
        return 0;
    }

    if (lua_isstring(luaState, 1) == 0) {
        LOG_ERR("LogInfo fail, arg1 err");
        return 0;
    }
    size_t len = 0;
    const char* buff = lua_tolstring(luaState, 1, &len);

    std::string str(buff, len);
    spdlog::info(str);
    
    return 1;
}

int LuaAPI::LogDebug(lua_State* luaState) {
    int argNum = lua_gettop(luaState);
    if(argNum != 1) {
        LOG_ERR("LogDebug fail, argNum = %d", argNum);
        return 0;
    }

    if (lua_isstring(luaState, 1) == 0) {
        LOG_ERR("LogDebug fail, arg1 err");
        return 0;
    }
    size_t len = 0;
    const char* buff = lua_tolstring(luaState, 1, &len);

    std::string str(buff, len);
    spdlog::debug(str);
    
    return 1;
}

int LuaAPI::LogWarn(lua_State* luaState) {
    int argNum = lua_gettop(luaState);
    if(argNum != 1) {
        LOG_ERR("LogWarn fail, argNum = %d", argNum);
        return 0;
    }

    if (lua_isstring(luaState, 1) == 0) {
        LOG_ERR("LogWarn fail, arg1 err");
        return 0;
    }
    size_t len = 0;
    const char* buff = lua_tolstring(luaState, 1, &len);

    std::string str(buff, len);
    spdlog::warn(str);
    
    return 1;
}

int LuaAPI::LogErr(lua_State* luaState) {
    int argNum = lua_gettop(luaState);
    if(argNum != 1) {
        LOG_ERR("LogErr fail, argNum = %d", argNum);
        return 0;
    }

    if (lua_isstring(luaState, 1) == 0) {
        LOG_ERR("LogErr fail, arg1 err");
        return 0;
    }
    size_t len = 0;
    const char* buff = lua_tolstring(luaState, 1, &len);

    std::string str(buff, len);
    spdlog::error(str);
    
    return 1;
}

int LuaAPI::GetEnumConfig(std::string enumName, std::string key) {
    static lua_State* luaState = NULL;
    if(luaState == NULL) {
        luaState = luaL_newstate();
        luaL_openlibs(luaState);
        // 执行lua文件
        int isOk = luaL_dofile(luaState, "../service/ExportToCPP.lua");
        if(isOk == 1) {     // success:0, fail: 1
            LOG_ERR("run lua fail: %s", lua_tostring(luaState, - 1));
            return -1;
        }
    }
    
    lua_getglobal(luaState, enumName.c_str());
    lua_getfield(luaState, -1, key.c_str());
    if(lua_isinteger(luaState, -1) == 0) {
        LOG_ERR("Invalide enum");
        return -1;
    }
    int val = lua_tointeger(luaState, -1);
    return val;
}

