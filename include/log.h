#include "util.h"

#include "spdlog/spdlog.h"

#define LOG_INFO(args...)       (spdlog::info(format(args)))
#define LOG_DEBUG(args...)      (spdlog::debug(format(args)))
#define LOG_WARN(args...)       (spdlog::warn(format(args)))
#define LOG_ERR(args...)        (spdlog::error(format(args)))

