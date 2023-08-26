#include <iostream>
#include <memory>

#include <unistd.h>

#include "Worker.h"
#include "Service.h"
#include "Sunnet.h"

// 线程函数
void Worker::operator()() {
    while (true) {
        std::shared_ptr<Service> srv = Sunnet::Inst->PopGlobalQueue();
        if(!srv) {
            Sunnet::Inst->WorkerWait();
        }else {
            srv->ProcessMsgs(m_EachNum);
            CheckAndPutGlobal(srv);
        }
    }
}


// 那些调Sunnet的通过传参数解决
// 状态不在队列中, global = true
void Worker::CheckAndPutGlobal(std::shared_ptr<Service> srv) {
    // 退出中(服务的退出方式只能它自己调用，这样isExiting才不会产生线程冲突)
    if(srv->m_IsExiting) {
        return ;
    }
    pthread_spin_lock(&srv->m_QueueLock);
    {
        // 重新放回全局队列
        if(!srv->m_MsgQueue.empty()) {
            // 此时srv->m_InGlobal一定是true
            Sunnet::Inst->PushGlobalQueue(srv);
        }else { // 不在队列中，重设m_InGlobal
            srv->SetInGlobal(false);
        }
    }
    pthread_spin_unlock(&srv->m_QueueLock);
}