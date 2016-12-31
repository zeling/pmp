//
// Created by 冯泽灵 on 2016/12/18.
//

#include <iostream>

#include "logger.h"

namespace pmp {
    namespace logging {

        const char * log_level_values[NUM_LOG_LEVELS] = {
#define T(e, v) v
                LOG_LEVEL_LIST(T)
#undef T
        };

        struct stdout_log_policy;
        typedef logger<stdout_log_policy> stdout_logger;


        struct stdout_log_policy {
            void log_write(log_level level, const std::string &message) {
                if (level >= LOG_INFO )
                    std::cout << message << '\n';
            }
        };



    }

}

#define LOG(level) (pmp::logging::stdout_logger().stream(level))

int main(void) {
    using namespace pmp::logging;
    LOG(LOG_DEBUG) << "hello, this is a debug message";
    LOG(LOG_INFO) << "hello, this is a info message";
    LOG(LOG_FATAL) << "hello, this is a fatal message";

}
