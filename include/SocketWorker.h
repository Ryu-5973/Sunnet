#pragma once
#include <memory>

#include <sys/epoll.h>

#include "Conn.h"

class SocketWorker {
public:
    void Init();
    void operator()();

    void AddEvent(int fd);
    void RemoveEvent(int fd);
    void ModifyEvent(int fd, bool epollOut);

private:
    void OnEvent(epoll_event ev);
    void OnAccept(std::shared_ptr<Conn> conn);
    void OnRW(std::shared_ptr<Conn> conn, bool r, bool w);
    void OnError(std::shared_ptr<Conn> conn);
private:
    // epoll 描述符
    int m_EpollFd;
};
