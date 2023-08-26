#pragma once
#include <thread>
#include <memory>

class Sunnet;
class Service;

class Worker {
public:
    int m_Id;             // 编号
    int m_EachNum;        // 每次处理多少条信息
    void operator()();  // 线程函数

private:
   void CheckAndPutGlobal(std::shared_ptr<Service>);
};
