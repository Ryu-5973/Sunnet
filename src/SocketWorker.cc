#include <iostream>
#include <string>
#include <cstring>

#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/socket.h>

#include "SocketWorker.h"
#include "CommonDefs.h"
#include "Sunnet.h"
#include "Conn.h"
#include "Msg.h"

// 初始化
void SocketWorker::Init() {
    LOG_INFO("SocketWorker Init");
    // 创建epoll对象
    m_EpollFd = epoll_create(1024);
    assert(m_EpollFd > 0);
    LOG_INFO("ScoketWorker Epoll Create EpollFd = %d", m_EpollFd);
}

void SocketWorker::operator()() {
    while (true) {
        struct epoll_event events[EVENT_SIZE];
        int eventCount = epoll_wait(m_EpollFd, events, EVENT_SIZE, -1);

        for(int i = 0; i < eventCount; ++ i) {
            epoll_event ev = events[i];
            OnEvent(ev);
        }
    }      
}

void SocketWorker::AddEvent(int fd, uint32_t event) {
    LOG_INFO("AddEvent Fd = %d", fd);

    struct epoll_event ev;
    ev.events = event;
    ev.data.fd = fd;
    if(epoll_ctl(m_EpollFd, EPOLL_CTL_ADD, fd, &ev) == -1) {
        LOG_INFO("AddEvent epoll_ctl fail: %s", strerror(errno));
    }
}

void SocketWorker::RemoveEvent(int fd) {
    LOG_INFO("RemoveEvent Fd = %d", fd);
    epoll_ctl(m_EpollFd, EPOLL_CTL_DEL, fd, NULL);
}

void SocketWorker::ModifyEvent(int fd, bool epollOut) {
    LOG_INFO("ModifyEvent Fd = %d", fd, " EpollOut = ", epollOut);

    struct epoll_event ev;
    ev.data.fd = fd;
    ev.events = EPOLLIN | EPOLLET;
    if(epollOut) {
        ev.events = ev.events | EPOLLOUT;
    }

    epoll_ctl(m_EpollFd, EPOLL_CTL_MOD, fd, &ev);
}

void SocketWorker::OnEvent(epoll_event ev) {
    int fd = ev.data.fd;
    auto conn = Sunnet::Inst->GetConn(fd);
    if(conn == nullptr) {
        LOG_ERR("OnEvent Error, Conn == nullptr");
        return ;
    }

    bool isRead = ev.events & EPOLLIN;
    bool isWrite = ev.events & EPOLLOUT;
    bool isError = ev.events & EPOLLERR;

    // 监听socket
    if(conn->m_Type == Conn::Type::LISTEN) {
        if(isRead) {
            OnAccept(conn);
        }
    }else {
        if(isRead || isWrite) {
            OnRW(conn, isRead, isWrite);
        }
        if(isError) {
            OnError(conn);
        }
    }
}

void SocketWorker::OnAccept(std::shared_ptr<Conn> conn) {
    LOG_INFO("OnAccept fd = %d", conn->m_Fd);

    int clientFd = accept(conn->m_Fd, NULL, NULL);
    if(clientFd < 0) {
        LOG_ERR("accept error");
        return ;
    }

    fcntl(clientFd, F_SETFL, O_NONBLOCK);

    // 写缓冲区大小
    unsigned long buffSize = WRITE_BUFFSIZE;
    if(setsockopt(clientFd, SOL_SOCKET, SO_SNDBUFFORCE, &buffSize, sizeof(buffSize)) < 0) {
        LOG_ERR("OnAccept setsockopt Fail %s", strerror(errno));
        return ;
    }

    Sunnet::Inst->AddConn(clientFd, conn->m_ServiceId, Conn::Type::CLINET, -1);
    AddEvent(clientFd);

    auto msg = std::make_shared<SocketAcceptMsg>();
    msg->m_Type = BaseMsg::Type::SOCKET_ACCEPT;
    msg->m_ListenFd = conn->m_Fd;
    msg->m_WriteFd =clientFd;
    Sunnet::Inst->Send(conn->m_ServiceId, msg);
}

void SocketWorker::OnRW(std::shared_ptr<Conn> conn, bool r, bool w) {
    LOG_INFO("OnRW fd = %d", conn->m_Fd);
    auto msg = std::make_shared<SocketRWMsg>();
    msg->m_Fd = conn->m_Fd;
    msg->m_IsRead = r;
    msg->m_IsWrite = w;
    msg->m_Type = BaseMsg::Type::SOCKET_RW;
    Sunnet::Inst->Send(conn->m_ServiceId, msg);
}

void SocketWorker::OnError(std::shared_ptr<Conn> conn) {
    LOG_INFO("OnError fd = %d", conn->m_Fd);
}
