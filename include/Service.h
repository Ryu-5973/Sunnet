#pragma once
#include <queue>
#include <thread>
#include <string>

#include <stdint.h>

#include "Msg.h"

extern "C" {
    #include "lua.h"
    #include "lauxlib.h"
    #include "lualib.h"
}

class Service {
public:
    uint32_t m_Id;
    std::shared_ptr<std::string> m_Type;
    bool m_IsExiting;
    std::queue<std::shared_ptr<BaseMsg>> m_MsgQueue;
    pthread_spinlock_t m_QueueLock;
    // 标记是否在全局队列，true：表示在队列中，或正在处理
    bool m_InGlobal = false;
    pthread_spinlock_t m_InGlobalLock;
public:
    Service();
    ~Service();

    void OnInit();
    void OnMsg(std::shared_ptr<BaseMsg>);
    void OnExit();

    void PushMsg(std::shared_ptr<BaseMsg>);

    bool ProcessMsg();
    void ProcessMsgs(int);

    // 线程安全地设计m_InGlobal
    void SetInGlobal(bool);

    // 消息处理方法
    void OnServiceMsg(std::shared_ptr<ServiceMsg> msg);
    void OnAcceptMsg(std::shared_ptr<SocketAcceptMsg> m);
    void OnRWMsg(std::shared_ptr<SocketRWMsg> msg);
    void OnSocketData(int fd, const char* buff, int len);
    void OnSocketWritable(int fd);
    void OnSocketClose(int fd);
private:
    std::shared_ptr<BaseMsg> PopMsg();

    // lua虚拟机
    lua_State *m_LuaState;
};
