#ifndef COMMON_H_
#define COMMON_H_

/**
 * Common stl/functions/macros/configs.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <memory>
#include <string>
#include <string.h>
#include <vector>
#include "config_rmq.h"

namespace rmq {

#define DEBUG_MODE 1
#define INFO_MODE 1

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

//#define assert_debug(x) do { if (DEBUG_MODE) assert(x); } while (0)
#ifdef DEBUG_MODE
#define assert_debug(x) do { if (DEBUG_MODE) assert(x); } while (0)
#else
#define assert_debug(x)
#endif

#ifdef DEBUG_MODE
#define LOG_DEBUG(...) do { fprintf(stderr, __VA_ARGS__); } while (0)
#else
#define LOG_DEBUG(x)
#endif

#ifdef INFO_MODE
#define LOG_INFO(...) do { fprintf(stderr, __VA_ARGS__); } while (0)
#else
#define LOG_INFO(x)
#endif

inline void assert_exit(bool condition, std::string error_msg) {
    if (unlikely(!condition)) {
        fprintf(stderr, "%s Exiting.\n", error_msg.c_str());
        fflush(stderr);
        exit(EXIT_FAILURE);
    }
}

}

#endif // COMMON_H_