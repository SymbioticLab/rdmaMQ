#ifndef COMMON_H_
#define COMMON_H_

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>

/**
 * Common functions/macros.
 */

namespace rmq {

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

inline void assert_true(bool condition, std::string error_msg) {
    if (unlikely(!condition)) {
        fprintf(stderr, "%s. Exiting.\n", error_msg.c_str());
        fflush(stderr);
        exit(EXIT_FAILURE);
    }
}

}

#endif // COMMON_H_