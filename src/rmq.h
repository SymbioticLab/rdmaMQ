#ifndef RMQ_H_
#define RMQ_H_

#include "config_rmq.h"

namespace rmq {

/**
 * rmq is the top-level model. It manages tranport, producer/consumer/brokers.
 * Each application must first instantiate rmq.
 */

enum Role {rmq_Producer, rmq_Consumer, rmq_Broker};

// byte array defs
struct msg4_t {
    char msg[4];
};

struct msg16_t {
    char msg[16];
};

struct msg64_t {
    char msg[64];
};

struct msg256_t {
    char msg[256];
};

}

#endif // RMQ_H_