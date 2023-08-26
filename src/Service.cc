#include <iostream>
#include <cstring>

#include <unistd.h>

#include "Service.h"
#include "Sunnet.h"
#include "LuaAPI.h"

// 构造函数
Service::Service() {
    // 初始化锁
    pthread_spin_init(&m_QueueLock, PTHREAD_PROCESS_PRIVATE);
    pthread_spin_init(&m_InGlobalLock, PTHREAD_PROCESS_PRIVATE);
}

// 析构函数
Service::~Service() {
    pthread_spin_destroy(&m_QueueLock);
    pthread_spin_destroy(&m_InGlobalLock);
}

// 插入消息
void Service::PushMsg(std::shared_ptr<BaseMsg> msg) {
    pthread_spin_lock(&m_QueueLock);
    {
        m_MsgQueue.push(msg);
    }
    pthread_spin_unlock(&m_QueueLock);
}

// 取出消息
std::shared_ptr<BaseMsg> Service::PopMsg() {
    std::shared_ptr<BaseMsg> msg = nullptr;
    pthread_spin_lock(&m_QueueLock);
    {
        if(!m_MsgQueue.empty()) {
            msg = m_MsgQueue.front();
            m_MsgQueue.pop();
        }
    }
    pthread_spin_unlock(&m_QueueLock);
    return msg;
}

// 创建服务后触发
void Service::OnInit() {
    LOG_INFO("[%d] OnInit", m_Id);
    // 新建lua虚拟机
    m_LuaState = luaL_newstate();
    luaL_openlibs(m_LuaState);
    // 注册Sunnet系统API
    LuaAPI::Register(m_LuaState);
    // 执行lua文件
    std::string filename = "../service/" + *m_Type + "/init.lua";
    int isOk = luaL_dofile(m_LuaState, filename.data());
    if(isOk == 1) {     // success:0, fail: 1
        LOG_ERR("run lua fail: %s", lua_tostring(m_LuaState, - 1));
        return ;
    }
    // 调用lua函数
    lua_getglobal(m_LuaState, "OnInit");
    lua_pushinteger(m_LuaState, m_Id);
    isOk = lua_pcall(m_LuaState, 1, 0, 0);
    if(isOk != 0) {
        LOG_ERR("call lua OnInit fail %s", lua_tostring(m_LuaState, -1));
    }
}

// 收到消息时触发
void Service::OnMsg(std::shared_ptr<BaseMsg> msg) {
    // 测试用
    if(msg->m_Type == BaseMsg::Type::SERVICE) {
        auto m = std::dynamic_pointer_cast<ServiceMsg>(msg);
        OnServiceMsg(m);
    }else if(msg->m_Type == BaseMsg::Type::SOCKET_ACCEPT) {
        auto m = std::dynamic_pointer_cast<SocketAcceptMsg>(msg);
        OnAcceptMsg(m);
    }else if(msg->m_Type == BaseMsg::Type::SOCKET_RW) {
        auto m = std::dynamic_pointer_cast<SocketRWMsg>(msg);
        OnRWMsg(m);
    }else {
        LOG_INFO("[%d] OnMsg", m_Id);
    }
}

// 服务退出后触发
void Service::OnExit() {
    LOG_INFO("[%d] OnExit", m_Id);

    // 调用lua onexit
    lua_getglobal(m_LuaState, "OnExit");
    int isOk = lua_pcall(m_LuaState, 0, 0, 0);
    if(isOk != 0) {
        LOG_ERR("call lua OnExit fail %s", lua_tostring(m_LuaState, -1));
        return ;
    }
    // 关闭lua虚拟机
    lua_close(m_LuaState);
}

// 处理一条信息，返回值代表是否处理
bool Service::ProcessMsg() {
    std::shared_ptr<BaseMsg> msg = PopMsg();
    if(msg) {
        OnMsg(msg);
        return true;
    }else {
        return false;   // 返回false表队列为空
    }
}

// 处理多条信息
void Service::ProcessMsgs(int cnt) {
    for(int i = 0; i < cnt; ++ i) {
        bool succ = ProcessMsg();
        if(!succ) {
            break;
        }
    }
}

void Service::SetInGlobal(bool inGlobal) {
    pthread_spin_lock(&m_InGlobalLock);
    {
        m_InGlobal = inGlobal;
    }
    pthread_spin_unlock(&m_InGlobalLock);
}

void Service::OnServiceMsg(std::shared_ptr<ServiceMsg> msg) {
    lua_getglobal(m_LuaState, "OnServiceMsg");
    lua_pushinteger(m_LuaState, msg->m_Source);
    lua_pushlstring(m_LuaState, msg->m_Buff.get(), msg->m_Size);
    int isOk = lua_pcall(m_LuaState, 2, 0, 0);
    if(isOk != 0) {
        LOG_ERR("call lua OnServiceMsg fail %s", lua_tostring(m_LuaState, -1));
        return ;
    }
}
void Service::OnAcceptMsg(std::shared_ptr<SocketAcceptMsg> msg) {
    lua_getglobal(m_LuaState, "OnAcceptMsg");
    lua_pushinteger(m_LuaState, msg->m_ListenFd);
    lua_pushinteger(m_LuaState, msg->m_WriteFd);
    int isOk = lua_pcall(m_LuaState, 2, 0, 0);
    if(isOk == 0) {
        LOG_ERR("call lua OnAccept fail %s", lua_tostring(m_LuaState, -1));
        return ;
    }
}
void Service::OnRWMsg(std::shared_ptr<SocketRWMsg> msg) {
    int fd = msg->m_Fd;
    // 可读
    if (msg->m_IsRead) {
        const int BUFFSIZE = 512;
        char buff[BUFFSIZE];

        int len = 0;
        do {
            len = read(fd, &buff, BUFFSIZE);
            if(len > 0) {
                OnSocketData(fd, buff, len);
            }
        }while(len == BUFFSIZE);

        if(len < 0 && errno != EAGAIN) {
            if(Sunnet::Inst->GetConn(fd)) {
                OnSocketClose(fd);
                Sunnet::Inst->CloseConn(fd);
            }
        }
    }

    // 可写
    if(msg->m_IsWrite) {
        if(Sunnet::Inst->GetConn(fd)) {
        OnSocketWritable(fd);
        }
    }
}

void Service::OnSocketData(int fd, const char* buff, int len) {
    lua_getglobal(m_LuaState, "OnSocketData");
    lua_pushinteger(m_LuaState, fd);
    lua_pushlstring(m_LuaState, buff, len);
    int isOk = lua_pcall(m_LuaState, 2, 0, 0);
    if(isOk != 0) {
        LOG_ERR("call lua OnSocketData fail %s", lua_tostring(m_LuaState, -1));
        return ;
    }
}

void Service::OnSocketWritable(int fd) {
    LOG_INFO("OnSocketWritable %d", fd);
}

void Service::OnSocketClose(int fd) {
    LOG_INFO("OnSocketClose %d", fd);
}

