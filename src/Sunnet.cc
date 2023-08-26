#include <iostream>

#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>

#include "Sunnet.h"
#include "Service.h"

#include "spdlog/spdlog.h"

Sunnet* Sunnet::Inst;
Sunnet::Sunnet() {

    Inst = this;
}

void Sunnet::Start() {
    spdlog::info("HelloSunnet");
    // 忽略sigpipe信号
    signal(SIGPIPE, SIG_IGN);
    // 锁
    pthread_rwlock_init(&m_ServicesLock, NULL);
    pthread_spin_init(&m_GlobalLock, PTHREAD_PROCESS_PRIVATE);
    pthread_mutex_init(&m_SleepMtx, NULL);
    pthread_cond_init(&m_SleepCond, NULL);
    assert(pthread_rwlock_init(&m_ConnsLock, NULL) == 0);
    // 开启worker
    StartWorker();

    // 开启socket worker
    StartSocket();
}

void Sunnet::StartWorker() {
    for(int i = 0; i < m_WorkerNum; ++ i) {
        LOG_INFO("start worker thread: %d", i);
        // 创建线程对象
        Worker* worker = new Worker();
        worker->m_Id = i;
        worker->m_EachNum = 2 << i;
        // 创建线程
        std::thread* wt = new std::thread(*worker);
        // 添加到数组
        m_Workers.push_back(worker);
        m_WorkerThreads.push_back(wt);
    }
}

void Sunnet::Wait() {
    if(m_WorkerThreads.size() > 0) {
        m_WorkerThreads[0]->join();
    }
}

// 新建服务
uint32_t Sunnet::NewService(std::shared_ptr<std::string> type) {
    auto srv = std::make_shared<Service>();
    srv->m_Type = type;
    pthread_rwlock_wrlock(&m_ServicesLock);
    {
        srv->m_Id = m_MaxId;
        ++ m_MaxId;
        m_Services.emplace(srv->m_Id, srv);
    }
    pthread_rwlock_unlock(&m_ServicesLock);
    srv->OnInit();
    return srv->m_Id;
}

// 由id查找服务
std::shared_ptr<Service> Sunnet::GetService(uint32_t id) {
    std::shared_ptr<Service> srv = NULL;
    pthread_rwlock_rdlock(&m_ServicesLock);
    {
        if(m_Services.find(id) != m_Services.end()) {
            return m_Services[id];
        }
    }
    pthread_rwlock_unlock(&m_ServicesLock);
    return srv;
}

// 删除服务
// 只能service自己调自己，因为会调用不加锁的srv->OnExit和srv->m_IsExiting
void Sunnet::KillService(uint32_t id) {
    auto srv = GetService(id);
    if(!srv) {
        return ;
    }

    // 退出前
    srv->OnExit();
    srv->m_IsExiting = true;
    // 删列表
    pthread_rwlock_wrlock(&m_ServicesLock);
    {
        m_Services.erase(id);
    }
    pthread_rwlock_unlock(&m_ServicesLock);
}

// 弹出全局队列
std::shared_ptr<Service> Sunnet::PopGlobalQueue() {
    std::shared_ptr<Service> srv = NULL;
    pthread_spin_lock(&m_GlobalLock);
    {
        if(!m_GlobalQueue.empty()) {
            srv = m_GlobalQueue.front();
            m_GlobalQueue.pop();
            m_GlobalLen --;
        }
    }
    pthread_spin_unlock(&m_GlobalLock);
    return srv;
}

// 插入全局队列
void Sunnet::PushGlobalQueue(std::shared_ptr<Service> srv) {
    pthread_spin_lock(&m_GlobalLock);
    {
        m_GlobalQueue.push(srv);
        ++ m_GlobalLen;
    }
    pthread_spin_unlock(&m_GlobalLock);
}

// 发送消息
void Sunnet::Send(uint32_t toId, std::shared_ptr<BaseMsg> msg) {
    std::shared_ptr<Service> toSrv = GetService(toId);
    if(!toSrv) {
        LOG_ERR("Send Fail, toSrv not exist toId: %d", toId);
        return ;
    }
    // 插入目标服务的消息队列
    toSrv->PushMsg(msg);
    // 检查并放入全局队列
    bool hasPush = false;
    pthread_spin_lock(&toSrv->m_InGlobalLock);
    {
        if(!toSrv->m_InGlobal) {
            PushGlobalQueue(toSrv);
            toSrv->m_InGlobal = true;
            hasPush = true;
        }
    }
    pthread_spin_unlock(&toSrv->m_InGlobalLock);

    // 唤醒进程
    if (hasPush) {
        CheckAndWakeUp();
    }
}

std::shared_ptr<BaseMsg> Sunnet::MakeMsg(uint32_t source, char* buff, int len) {
    auto msg =std::make_shared<ServiceMsg>();
    msg->m_Type = BaseMsg::Type::SERVICE;
    msg->m_Source = source;
    //基本类型的对象没有析构函数，
    //所以用delete 或 delete[]都可以销毁基本类型数组；
    //智能指针默认使用delete销毁对象，
    //所以无须重写智能指针的销毁方法
    msg->m_Buff = std::shared_ptr<char>(buff);
    msg->m_Size = len;
    return msg;
}

// Worker线程调用，进入休眠
void Sunnet::WorkerWait() {
    pthread_mutex_lock(&m_SleepMtx);
    m_SleepCount ++;
    pthread_cond_wait(&m_SleepCond, &m_SleepMtx);
    m_SleepCount --;
    pthread_mutex_unlock(&m_SleepMtx);
}

// 检查并唤醒线程
void Sunnet::CheckAndWakeUp() {
    // unsafe
    if(m_SleepCount == 0) {
        return;
    }

    if(m_WorkerNum - m_SleepCount <= m_GlobalLen) {
        LOG_DEBUG("wakeup");
        pthread_cond_signal(&m_SleepCond);
    }
}

// 开启socket线程
void Sunnet::StartSocket() {
    // 创建线程对象
    m_SocketWorker = new SocketWorker();
    // 初始化
    m_SocketWorker->Init();
    // 创建线程
    m_SocketThread = new std::thread(*m_SocketWorker);
}

int Sunnet::AddConn(int fd, uint32_t id, Conn::Type type) {
    auto conn = std::make_shared<Conn>();
    conn->m_Fd = fd;
    conn->m_ServiceId = id;
    conn->m_Type = type;
    pthread_rwlock_wrlock(&m_ConnsLock);
    {
        m_Conns.emplace(fd, conn);
    }
    pthread_rwlock_unlock(&m_ConnsLock);
    return fd;
}

std::shared_ptr<Conn> Sunnet::GetConn(int fd) {
    std::shared_ptr<Conn> ret = nullptr;
    pthread_rwlock_rdlock(&m_ConnsLock);
    {
        if(m_Conns.find(fd) != m_Conns.end()) {
            ret = m_Conns[fd];
        }
    }
    pthread_rwlock_unlock(&m_ConnsLock);
    return ret;
}

bool Sunnet::RemoveConn(int fd) {
    int ret = 0;
    pthread_rwlock_wrlock(&m_ConnsLock);
    {
        ret = m_Conns.erase(fd);
    }
    pthread_rwlock_unlock(&m_ConnsLock);
    return ret == 1;
}

int Sunnet::Listen(uint32_t port, uint32_t serviceId) {
    int listenFd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenFd < 0) {
        LOG_ERR("listen error, listenFd <= 0");
        return -1;
    }

    fcntl(listenFd, F_SETFL, O_NONBLOCK);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    int ret = bind(listenFd, (struct sockaddr*)&addr, sizeof(addr));
    if(ret == -1) {
        LOG_ERR("listen error, bind fail port = %d", port);
        return -1;
    }

    ret = listen(listenFd, 64);
    if(ret < 0) {
        return -1;
    }

    AddConn(listenFd, serviceId, Conn::Type::LISTEN);

    m_SocketWorker->AddEvent(listenFd);

    LOG_INFO("[Sunnet] Port = %d Service = %d Listening...", port, serviceId);
    // LOG_INFO("%d %d %d", addr.sin_family, addr.sin_port, addr.sin_addr.s_addr); todo: 找时间把这玩意转成看得懂的。。。
    return listenFd;
}

void Sunnet::CloseConn(uint32_t fd) {
    bool succ = RemoveConn(fd);

    close(fd);

    if(succ) {
        m_SocketWorker->RemoveEvent(fd);
    }
}



