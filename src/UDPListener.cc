
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "UDPListener.h"
#include "CommonDefs.h"
#include "Msg.h"
#include "Sunnet.h"

void UDPListener::Init(int fd) {
    assert(fd > 0);
    m_Fd = fd;
    SetSysteUDPBufferSize(fd);
    SetUDPDontFrag(fd);
    // Sunnet::Inst->AddConn
    LOG_INFO("UDPListener Init, Fd = %d", fd);
}

void UDPListener::operator()() {
    sockaddr_in clientAddr;
    socklen_t clientAddrLen;
    auto conn = Sunnet::Inst->GetConn(m_Fd);
    if(conn == nullptr) {
        LOG_ERR("OnEvent Error, Conn == nullptr");
        return ;
    }

    while (true) {
        auto msg = std::make_shared<UDPMsg>();
        msg->m_Type = BaseMsg::SOCKET_UDP;
        ssize_t len = recvfrom(m_Fd, msg->m_Load, MAX_UDP_PACKAGE_SIZE, 0, (sockaddr*)&clientAddr, &clientAddrLen);
        if (len == -1) {
            LOG_WARN("Recvfrom failed");
            continue;
        }
        msg->m_Len = len;
        Sunnet::Inst->Send(conn->m_ServiceId, msg);
    }
    
}

void UDPListener::SetSysteUDPBufferSize(int fd) {
    int defUdpRecvBufSiz = -1;
    socklen_t optLen = sizeof(defUdpRecvBufSiz);
    if (getsockopt(fd, SOL_SOCKET, SO_RCVBUF, &defUdpRecvBufSiz, &optLen) < 0) {
        LOG_WARN("Get Default UDP Receive Buffer Size Error");
        return ;
    }
    LOG_INFO("System Default UDP Socket Receive Buffer Size = %d", defUdpRecvBufSiz);
    int recvBufferSiz = MAX_UDP_PACKAGE_SIZE;
    optLen = sizeof(recvBufferSiz);

    if (setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &recvBufferSiz, optLen) < 0) {
        LOG_WARN("Set UDP Receive Buffer Size Error");
        return ;
    }
    LOG_INFO("Set System UDP Socket Receive Buffer Size = %d", recvBufferSiz);
}

void UDPListener::SetUDPDontFrag(int fd) {
    // todo: 后面看看有没有必要弄禁止分包
}
