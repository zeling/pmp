//
// Created by 冯泽灵 on 2016/12/18.
//

#ifndef PMP_LOGGING_H
#define PMP_LOGGING_H

#include <iosfwd>

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

        struct logging {
        private:
            std::stringstream ss;


        public:
            logging(log_level level) : ss(std::stringstream(get_name(level))) {};

            logging (const logging & that) = delete;
            logging (logging && that) = delete;

        };
    }
}



#endif //PMP_LOGGING_H
