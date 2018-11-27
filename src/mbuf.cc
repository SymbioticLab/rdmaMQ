#include "mbuf.h"
#include <unistd.h>

namespace rmq {

template <typename T>
void MessageBuffer<T>::init(struct ibv_pd *pd) {
    //addr = memalign(sysconf(_SC_PAGESIZE), total_size);
    //addr = malloc(total_size);
    //data = std::make_unique<T[]>(num_blocks);
    data = new T[num_blocks];

    //mr = ibv_reg_mr(transport->get_pd(), static_cast<void *>(data.get()), total_size,
    mr = ibv_reg_mr(pd, static_cast<void *>(data), total_size,
            IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_WRITE | IBV_ACCESS_REMOTE_READ);
    assert_exit(mr, "Failed to register MR.");
    LOG_DEBUG("Managed to register MR for mbuf.\n");
}

// explicit instantiations
template class MessageBuffer<char>;
template class MessageBuffer<int>;
template class MessageBuffer<std::string>;

}