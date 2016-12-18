//
// Created by 冯泽灵 on 2016/12/18.
//

#include <iostream>

#include "logging.h"

namespace pmp {
    namespace logging {

        const char * log_level_values[NUM_LOG_LEVELS] = {
#define T(e, v) v
                LOG_LEVEL_LIST(T)
#undef T
        };



    }

}


