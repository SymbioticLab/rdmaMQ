#include "producer.h"
#include <inttypes.h>

namespace rmq {

template <typename T>
// loop_cnt is the higher portion of the uint64_t
// write_idx is the lower portion of the uint64_t
// # of bits depends on bkr_x defined in config_rmq.h
// Assumes num_msg is never 0
// TODO: keep track of network/host order later
size_t Producer<T>::fetch_and_add_write_idx(size_t start_idx, size_t num_msg) {
    // First local buffer overflow check
    if (unlikely(num_msg > data_buf->get_capacity() - start_idx)) {
        num_msg = data_buf->get_capacity() - start_idx;
    }

    // Then check remote buffer(circular) overflow 
    if (unlikely(num_msg > bkr_buff_cap - (ctrl_buf->get_data()[0] & bkr_low_mask))) {
        num_msg = bkr_buff_cap - (ctrl_buf->get_data()[0] & bkr_low_mask);
    }

    uint64_t compare_add = num_msg;
    transport->post_ATOMIC_FA(compare_add);

    transport->poll_from_cq(1);
    // at this point remote write idx is read into ctrl_buf->data()[0]
    LOG_DEBUG("getting WRITE IDX: %" PRIu64 "\n", ctrl_buf->get_data()[0] & bkr_low_mask);
    LOG_DEBUG("getting LOOP CNT: %" PRIu64 "\n", (ctrl_buf->get_data()[0] & bkr_high_mask) >> bkr_x);
    //LOG_DEBUG("getting WRITE IDX: 0x%08lx\n", ctrl_buf->get_data()[0] & bkr_low_mask);
    //LOG_DEBUG("getting LOOP CNT: 0x%08lx\n", (uint64_t)((ctrl_buf->get_data()[0] & bkr_high_mask) >> bkr_x));
    return num_msg;
}

template <typename T>
size_t Producer<T>::push(size_t start_idx, size_t num_msg) {
    // basic checks
    assert_exit(start_idx <= data_buf->get_capacity(), "Error: Invalid start_idx (greater than capacity).");
    assert_exit(num_msg > 0 && num_msg <= bkr_buff_cap, "Error: Invalid num_msg value");

    // atomically get next_write_idx and set new idx at the broker
    num_msg = fetch_and_add_write_idx(start_idx, num_msg);

    uint64_t local_addr = start_idx * sizeof(T) + transport->get_local_info()[0].data_vaddr;
    uint32_t length = num_msg * sizeof(T);
    uint64_t write_idx = ctrl_buf->get_data()[0] & bkr_low_mask;
    uint64_t remote_addr = write_idx * sizeof(T) + transport->get_remote_info()[0].data_vaddr;
    //std::cout << "start_idx: " << start_idx << std::endl;
    //printf("local_addr: %016lx\n", local_addr);
    //std::cout << "length: " << length << std::endl;
    //std::cout << "write_idx: " << write_idx << std::endl;
    //printf("remote_addr: %016lx\n", remote_addr);
    transport->post_WRITE(local_addr, length, remote_addr);

    transport->poll_from_cq(1);
    return num_msg;
}

// explicit instantiations
template class Producer<char>;
template class Producer<int>;
template class Producer<uint64_t>;
template class Producer<std::string>;

}