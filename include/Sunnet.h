#pragma once

#include <vector>
#include <thread>
#include <unordered_map>
#include <memory>
#include <queue>

#include "Worker.h"
#include "SocketWorker.h"
#include "Conn.h"
#include "CommonDefs.h"

extern "C" {
    #include "lua.h"
    #include "lauxlib.h"
    #include "lualib.h"
}

class Service;
class BaseMsg;
class Worker;

class Sunnet {
public:
    // 单例
    static Sunnet* Inst;

    // 服务列表
    std::unordered_map<uint32_t, std::shared_ptr<Service>> m_Services;
    uint32_t m_MaxId = 0;               // 最大id         
    pthread_rwlock_t m_ServicesLock;    // 读写锁

public:
    Sunnet();
    void Start();
    void Wait();

    // 增删服务
    uint32_t NewService(std::shared_ptr<std::string>);
    void KillService(uint32_t);

    // 发送消息
    void Send(uint32_t, std::shared_ptr<BaseMsg>);
    // 全局队列操作
    std::shared_ptr<Service> PopGlobalQueue();
    void PushGlobalQueue(std::shared_ptr<Service>);

    // 唤醒工作线程
    void CheckAndWakeUp();
    // 让工作线程等待（仅工作线程调用）
    void WorkerWait();

    // 开启socket线程
    void StartSocket();

    // Conn相关
    int AddConn(int, uint32_t, Conn::Type, int protocolType);
    std::shared_ptr<Conn> GetConn(int);
    bool RemoveConn(int);

    // 网络连接相关接口
    int Listen(lua_State* luaState, int port, uint32_t serviceId, uint32_t protocolType);
    void CloseConn(uint32_t fd);

    // 测试函数
    std::shared_ptr<BaseMsg> MakeMsg(uint32_t, char*, int);
private:
    int m_WorkerNum = 3;        // 工作线程数
    std::vector<Worker*> m_Workers;       // 工作线程
    std::vector<std::thread*> m_WorkerThreads;      // 线程
    std::queue<std::shared_ptr<Service>> m_GlobalQueue;     // 全局队列
    int m_GlobalLen = 0;    // 队列长度
    pthread_spinlock_t m_GlobalLock;        // 锁

    // 休眠和唤醒
    pthread_mutex_t m_SleepMtx;
    pthread_cond_t m_SleepCond;
    int m_SleepCount = 0;

    // socket线程
    SocketWorker* m_SocketWorker;
    std::thread* m_SocketThread;

    // Conn列表
    std::unordered_map<int, std::shared_ptr<Conn>> m_Conns;
    pthread_rwlock_t m_ConnsLock;

private:
    void StartWorker();
    int SetNonBlocking(int fd);
    
    // 获取服务
    std::shared_ptr<Service> GetService(uint32_t);

    // 网络创建
    int HandleTCPCreate(uint32_t port, uint32_t serviceId);
    int HandleUDPCreate(uint32_t port, uint32_t serviceId);
    int HandleKCPCreate(uint32_t port, uint32_t serviceId);
};