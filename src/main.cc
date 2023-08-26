#include <unistd.h>

#include "Sunnet.h"

void test() {
    auto pingType = std::make_shared<std::string>("ping");
    uint32_t ping1 = Sunnet::Inst->NewService(pingType);
    uint32_t ping2 = Sunnet::Inst->NewService(pingType);
    uint32_t pong = Sunnet::Inst->NewService(pingType);

    auto msg1 = Sunnet::Inst->MakeMsg(ping1, new char[3]{'h', 'i', '\0'}, 3);
    auto msg2 = Sunnet::Inst->MakeMsg(ping2, 
        new char[6]{'h', 'e', 'l', 'l', 'o', '\0'}, 6);
    
    Sunnet::Inst->Send(pong, msg1);
    Sunnet::Inst->Send(pong, msg2);
}

void TestSocketCtrl() {
}

void TestEcho() {
    auto t = std::make_shared<std::string>("gateway");
    uint32_t gateway = Sunnet::Inst->NewService(t);
}


int main() {
    new Sunnet();
    Sunnet::Inst->Start();
    // 开启系统后的一些逻辑
    auto t = std::make_shared<std::string>("main");
    Sunnet::Inst->NewService(t);
    Sunnet::Inst->Wait();
    return 0;
}