#include "producer.h"
#include <inttypes.h>

namespace rmq {


template <typename T>
// loop_cnt is the higher portion of the uint64_t
// write_idx is the lower portion of the uint64_t
// # of bits depends on bkr_x defined in config_rmq.h
// Assumes num_msg is never 0
// TODO: keep track of network/host order later
size_t Producer<T>::fetch_and_add_write_addr(size_t start_idx, size_t num_msg) {
    if (unlikely(data_buf->get_capacity() - start_idx < num_msg)) {
        num_msg = data_buf->get_capacity() - start_idx;
    }

    uint64_t compare_add = num_msg;
    transport->post_ATOMIC_FA(compare_add);

    transport->poll_from_cq(1);
    // at this point remote write idx is read into ctrl_buf->data()[0]
    LOG_DEBUG("getting WRITE IDX: %" PRIu64 "\n", ctrl_buf->get_data()[0]);
    return num_msg;
}

template <typename T>
size_t Producer<T>::push(size_t start_idx, size_t num_msg) {
    assert_exit(start_idx <= data_buf->get_capacity(), "Error: Invalid start_idx (greater than capacity).");
    assert_exit(num_msg > 0 && num_msg <= bkr_buff_cap, "Error: Invalid num_msg value");
    num_msg = fetch_and_add_write_addr(num_msg);

    uint64_t local_addr = start_idx * sizeof(T) + transport->local_info.data_vaddr;
    uint32_t length = num_msg * sizeof(T);
    uint64_t remote_addr = ctrl_buf->get_data()[0] * sizeof(T) + transport->remote_info.data_vaddr;
    transport->post_WRITE(local_addr, length, remote_addr);

    transport->poll_from_cq(1);
    return num_msg;
}

}