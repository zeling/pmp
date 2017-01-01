//
// Created by 冯泽灵 on 2016/12/18.
//

#include <iostream>
#include <fstream>

#include "logger.h"

namespace pmp {
    namespace logging {

        const char * log_level_values[NUM_LOG_LEVELS] = {
#define T(e, v) v
                LOG_LEVEL_LIST(T)
#undef T
        };

        struct stdout_log_policy;
        typedef log_entry<stdout_log_policy> stdout_logger_entry;


        struct stdout_log_policy {
            void log_write(log_level level, const std::string &message) {
                if (level >= LOG_INFO )
                    std::cout << message << '\n';
            }
        };

        struct file_log_policy;
        typedef log_entry<file_log_policy> file_logger_entry;

        struct file_log_policy {
        private:
            log_level level_;
            std::ofstream fs_;

        public:
            file_log_policy(const char *filename, log_level level = LOG_INFO):
                    level_(level),
                    fs_(filename, std::ios_base::out | std::ios_base::app) {}
            file_log_policy(const std::string &filename, log_level level = LOG_INFO):
                    level_(level),
                    fs_(filename, std::ios_base::out | std::ios_base::app) {}
            file_log_policy(const file_log_policy &rhs) = delete;
            file_log_policy(file_log_policy &&rhs) = delete;
            log_level level() { return level_; }
            void set_level(log_level level) { level_ = level; }
            void log_write(log_level level, const std::string &message) {
                if (level >= level_)
                    fs_ << message << std::endl;
            }
        };


    }

}

#define LOG(level) (pmp::logging::stdout_logger_entry(LOG_##level).stream())
#define FLOG(level, filename) (pmp::logging::file_logger_entry(LOG_##level, #filename).stream())

int main(void) {
    using namespace pmp::logging;
    FLOG(DEBUG, 1.log) << "hello, this is a debug message";
    FLOG(INFO, 1.log) << "hello, this is a debug message";
    FLOG(FATAL, 1.log) << "hello, this is a debug message";
    LOG(DEBUG) << "hello, this is a debug message";
    LOG(INFO) << "hello, this is a info message";
    LOG(FATAL) << "hello, this is a fatal message";

}
