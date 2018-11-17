#ifndef MBUF_H_
#define MBUF_H_

#include <infiniband/verbs.h>
#include "config_rmq.h"

namespace rmq {

/**
 * Applications use MessageBuffer to store data to avoid low-level memory management 
 * details in RDMA. The size of the buffer is fixed once declared. The buffer can 
 * contains multiple chunks, but is not circular. Users take the responsibility to 
 * ensure the data is stored in the expected position in the buffer.
 */ 
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