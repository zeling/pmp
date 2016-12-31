//
// Created by 冯泽灵 on 2016/12/18.
//

#ifndef PMP_LOGGING_H
#define PMP_LOGGING_H

#include <sstream>
#include <ctime>
#include <sys/time.h>

#define LOG_LEVEL_LIST(T)     \
    T(LOG_TRACE, "TRACE"),    \
    T(LOG_DEBUG, "DEBUG"),    \
    T(LOG_INFO,  "INFO"),     \
    T(LOG_WARN,  "WARN"),     \
    T(LOG_ERROR, "ERROR"),    \
    T(LOG_FATAL, "FATAL")     \

namespace pmp {
    namespace logging {

        enum log_level{
#define T(e, v) e
            LOG_LEVEL_LIST(T),
            NUM_LOG_LEVELS
#undef T
        };

        extern const char* log_level_values[NUM_LOG_LEVELS];

        static inline const char * get_name(log_level level) {
            return log_level_values[level];
        }


        template <typename LogPolicy>
        struct logger: public LogPolicy {
        private:
            log_level level_;
            std::ostringstream os_;

            std::string get_time() {

                struct timeval tv;
                gettimeofday(&tv, nullptr);
                char str[128];
                std::size_t offset = std::strftime(str, sizeof(str), "%Y-%b-%d %H:%M:%S", std::localtime(&tv.tv_sec));
                snprintf(str + offset, sizeof(str) - offset, ".%06d", tv.tv_usec);
                return { str };
            }

        public:
            logger(): level_(LOG_INFO) {}
            logger(const logger &rhs) = delete;
            logger(logger &&rhs) = delete;

            log_level level() { return level_; }

            std::ostringstream& stream(log_level level) {
                level_ = level;
                os_ << '[' << get_time() << ']' << '\t' << get_name(level) << '\t';
                return os_;
            }

            ~logger() {
                this->log_write(level_, os_.str());
            }

        };
    }
}



#endif //PMP_LOGGING_H
