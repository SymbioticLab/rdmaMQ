#include "mbuf.h"

namespace rmq {

template <typename T>
void MessageBuffer<T>::init(struct ibv_pd *pd, int need_atomic) {
    //addr = memalign(sysconf(_SC_PAGESIZE), total_size);
    //addr = malloc(total_size);
    //data = std::make_unique<T[]>(num_blocks);
    data = new T[capacity];

    //mr = ibv_reg_mr(transport->get_pd(), static_cast<void *>(data.get()), total_size,
    if (need_atomic) {
        mr = ibv_reg_mr(pd, static_cast<void *>(data), total_size,
                IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_WRITE |
                IBV_ACCESS_REMOTE_READ | IBV_ACCESS_REMOTE_ATOMIC);
    } else {    // just in case the ATOMIC flag affects performance
        mr = ibv_reg_mr(pd, static_cast<void *>(data), total_size,
                IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_WRITE | IBV_ACCESS_REMOTE_READ);
    }
    assert_exit(mr, "Failed to register MR.");
    LOG_DEBUG("Managed to register MR for mbuf with length = %lu.\n", mr->length);
}

// explicit instantiations
template class MessageBuffer<char>;
template class MessageBuffer<int>;
template class MessageBuffer<uint64_t>;
template class MessageBuffer<std::string>;
template class MessageBuffer<msg16_t>;
template class MessageBuffer<msg256_t>;

}