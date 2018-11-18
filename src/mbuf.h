#ifndef MBUF_H_
#define MBUF_H_

#include <infiniband/verbs.h>
#include "rmq.h"

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
    size_t capacity;        // maximum number of data blocks
    size_t num_blocks;      // current number of data blocks
    size_t block_size;      // size of a data block in bytes
    T *data;                // fixed sized array

    // RDMA transport metadata
    Transport *transport;
    struct ibv_mr *mr;
    void *addr;
    
    /**
     * register memory region for RDMA
     */
    void init();

public:
    MessageBuffer() {}
    //TODO: custom costr
    MessageBuffer(size_t capacity)
    : capacity(capacity),
      num_blocks(0),
      block_size(sizeof(T)),
      data(NULL) {
        init();
    }
    // TODO: implement destructor
    ~MessageBuffer() {}

};

} // namespace rmq

#endif // MBUF_H_