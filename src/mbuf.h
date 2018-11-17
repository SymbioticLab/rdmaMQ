#ifndef MBUF_H_
#define MBUF_H_

#include <infiniband/verbs.h>
#include "config_rmq.h"

namespace rmq {

template <typename T>
class MessageBuffer {
private:
    // Basic info
    size_t size;            // in bytes
    size_t block_size;      // in bytes
    size_t num_blocks;
    T data;

    // RDMA-related
    struct ibv_pd *pd;
    struct ibv_mr *mr;
    void *addr;
    
    //TODO: init() to call ibv_reg_mr

public:
    MessageBuffer() {}
    //TODO: custom costr
    ~MessageBuffer() {}

};

} // namespace rmq

#endif // MBUF_H_