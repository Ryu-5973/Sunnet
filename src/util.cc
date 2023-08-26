

#include "util.h"

std::string format(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    const auto len = vsnprintf(nullptr, 0, fmt, args);
    if(len == 0) {
        return fmt;
    }
    va_end(args);
    std::string r;
    r.resize(static_cast<size_t>(len) + 1);
    va_start(args, fmt);
    vsnprintf(&r.front(), len + 1, fmt, args);
    va_end(args);
    r.resize(static_cast<size_t>(len));
 
    return r;
}

std::string format(std::string str) {
    return str;
}
