#pragma once

#include <stdint.h>

class Conn{
public:
    enum Type {
        LISTEN = 1,
        CLINET = 2,
    };

    uint8_t m_Type;
    int m_Fd;
    uint32_t m_ServiceId;
    int m_ProtocolType;
};

