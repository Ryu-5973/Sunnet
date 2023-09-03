#pragma once
#include <memory>
#include <string>

#include <stdint.h>

#include <CommonDefs.h>

// 消息基类
class BaseMsg {
public:
    enum Type {
        SERVICE = 1,
        SOCKET_ACCEPT = 2,
        SOCKET_RW = 3,
        SOCKET_UDP = 4,
    };
    uint8_t m_Type;
    char m_Load[DEFAULT_LOAD_SIZE]{};
    virtual ~BaseMsg();

};

// 服务间消息
class ServiceMsg : public BaseMsg {
public:
    uint32_t m_Source;      // 消息发送方
    std::shared_ptr<char> m_Buff;   // 消息内容
    size_t m_Size;          // 消息内容大小
};

// 有新链接
class SocketAcceptMsg : public BaseMsg {
public:
    int m_ListenFd;
    int m_WriteFd;
};

class SocketRWMsg : public BaseMsg {
public:
    int m_Fd;
    bool m_IsRead = false;
    bool m_IsWrite = false;
};

class UDPMsg: public BaseMsg {
public:
    ssize_t m_Len;
};

