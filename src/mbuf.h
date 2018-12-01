#ifndef MBUF_H_
#define MBUF_H_

#include <infiniband/verbs.h>
//#include "rmq.h"
//#include "transport.h"
#include "common.h"

namespace rmq {

/**
 * Applications use MessageBuffer to store data to avoid low-level memory management
 * details in RDMA. The size of the buffer is fixed once declared. The buffer can
 * contains multiple chunks, but is not circular. Users take the responsibility to
 * ensure the data is stored in the expected position in the buffer.
 * 
 * Transport object should have been constructed before construct a MessageBuffer
 * object, or MR registration would fail due to the absense of a protection domain.
 */ 
template <typename T>
class MessageBuffer {
private:
    // Basic info
    size_t capacity;                // maximum number of data blocks
    size_t num_blocks;              // current number of data blocks
    size_t block_size;              // size of a data block in bytes
    size_t total_size;              // size of data capacity in bytes
    //std::unique_ptr<T[]> data;      // fixed sized array
    T* data;                        // fixed sized array

    // RDMA transport metadata
    //std::shared_ptr<Transport> transport;

    // Memory region ptr
    struct ibv_mr *mr;
    
    /**
     * register a memory region for RDMA
     */
    void init(struct ibv_pd *pd, int need_atomic);

public:
    MessageBuffer() {}
    MessageBuffer(size_t capacity, struct ibv_pd *pd, int need_atomic = 0)
    : capacity(capacity),
      num_blocks(0),
      block_size(sizeof(T)),
      total_size(capacity * sizeof(T)),
      data(nullptr) {
        init(pd, need_atomic);
    }
    ~MessageBuffer() {
        assert_exit(ibv_dereg_mr(mr) == 0, "Error deregister mr.");
        delete[] data;
        printf("mbuf destructor gets called\n");
    }

    inline T *get_data() { return data; }   // TODO: check if this is exact what mr->data
    inline struct ibv_mr* get_mr() { return mr; }
    inline size_t get_capacity() { return capacity; }
    inline size_t get_block_size() { return block_size; }

};

} // namespace rmq

#endif // MBUF_H_