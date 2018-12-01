#ifndef RMQ_H_
#define RMQ_H_

#include "config_rmq.h"

namespace rmq {

/**
 * rmq is the top-level model. It manages tranport, producer/consumer/brokers.
 * Each application must first instantiate rmq.
 */

enum Role {rmq_Producer, rmq_Consumer, rmq_Broker};



}

#endif // RMQ_H_