#pragma once

#include <list>
#include <memory>

#include <stdint.h>

class WriteObject {
public:
    std::streamsize m_Start;
    std::streamsize m_Len;
    std::shared_ptr<char> m_Buff;
};

class ConnWriter {
public:
    int m_Fd;
private:
    // 是否正在关闭
    bool m_IsClosing = false;
    std::list<std::shared_ptr<WriteObject>> m_Objs;

public:
    void EntireWrite(std::shared_ptr<char> buff, std::streamsize len);
    void LingerClose(); // 全部发完后关闭
    void OnWritable();
};
