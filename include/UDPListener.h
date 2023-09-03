#pragma once

class UDPListener {
public:
    void Init(int fd);
    void operator()();

private:
    void SetSysteUDPBufferSize(int fd);
    void SetUDPDontFrag(int fd);

private:
    int m_Fd;
};

