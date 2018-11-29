#include "producer.h"
#include <inttypes.h>

namespace rmq {

template <typename T>
void Producer<T>::fetch_and_add_write_addr(size_t num_msg) {
    uint64_t compare_add = num_msg * sizeof(T);
    transport->post_ATOMIC_FA(compare_add);
    transport->poll_from_cq(1);
    // at this point remote write addr is read into ctrl_buf->get_data()
}

template <typename T>
size_t Producer<T>::push(size_t num_msg) {
    fetch_and_add_write_addr(num_msg);
    uint64_t write_addr = reinterpret_cast<uintptr_t>(ctrl_buf->get_data());
    LOG_DEBUG("getting WRITE ADDR: %" PRIu64 "\n", write_addr);
    // post WRITE_with_IMM
     
}

}