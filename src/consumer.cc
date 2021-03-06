#include "consumer.h"
#include <inttypes.h>

namespace rmq {

template <typename T>
// loop_cnt is the higher portion of the uint64_t
// write_idx is the lower portion of the uint64_t
// # of bits (of lower portion) depends on bkr_x defined in config_rmq.h
size_t Consumer<T>::fetch_write_idx() {
    uint64_t compare_add = 0;
    transport->post_ATOMIC_FA(compare_add);

    transport->poll_from_cq(1);
    
    // at this point remote write idx is read into ctrl_buf->data()[0]
    // decimal version print
    LOG_DEBUG("getting WRITE IDX: %" PRIu64 "\n", ctrl_buf->get_data()[0] & bkr_low_mask);
    LOG_DEBUG("getting LOOP CNT: %" PRIu64 "\n", (ctrl_buf->get_data()[0] & bkr_high_mask) >> bkr_x);
    // Hex version print
    //LOG_DEBUG("getting WRITE IDX: 0x%08lx\n", ctrl_buf->get_data()[0] & bkr_low_mask);
    //LOG_DEBUG("getting LOOP CNT: 0x%08lx\n", (uint64_t)((ctrl_buf->get_data()[0] & bkr_high_mask) >> bkr_x));

    // Consumer decide what to do with the write idx. (e.g., use as a hint, etc.)
    size_t write_idx = ctrl_buf->get_data()[0] & bkr_low_mask;
    return write_idx;
}

// READ is just like WRITE
template <typename T>
size_t Consumer<T>::pull(size_t start_idx, size_t read_idx, size_t num_msg) {
    // basic checks
    assert_exit(start_idx <= data_buf->get_capacity(), "Error: Invalid start_idx (greater than capacity).");
    assert_exit(num_msg > 0 && num_msg <= bkr_buff_cap, "Error: Invalid num_msg value");

    // First local buffer overflow check
    if (unlikely(num_msg > data_buf->get_capacity() - start_idx)) {
        num_msg = data_buf->get_capacity() - start_idx;
    }

    // Then check remote buffer(circular) overflow 
    if (unlikely(num_msg > bkr_buff_cap - read_idx)) {
        num_msg = bkr_buff_cap - read_idx;
    }

    uint64_t local_addr = start_idx * sizeof(T) + transport->get_local_info()[0].data_vaddr;
    uint32_t length = num_msg * sizeof(T);
    uint64_t remote_addr = read_idx * sizeof(T) + transport->get_remote_info()[0].data_vaddr;
    transport->post_READ(local_addr, length, remote_addr);

    transport->poll_from_cq(1);
    return num_msg;
}

template <typename T>
size_t Consumer<T>::pull_batch(size_t start_idx, size_t read_idx, size_t num_msg) {
    // basic checks
    assert_exit(start_idx <= data_buf->get_capacity(), "Error: Invalid start_idx (greater than capacity).");
    assert_exit(num_msg > 0 && num_msg <= bkr_buff_cap, "Error: Invalid num_msg value");

    // First local buffer overflow check
    if (unlikely(num_msg > data_buf->get_capacity() - start_idx)) {
        num_msg = data_buf->get_capacity() - start_idx;
    }

    // Then check remote buffer(circular) overflow 
    if (unlikely(num_msg > bkr_buff_cap - read_idx)) {
        num_msg = bkr_buff_cap - read_idx;
    }

    uint64_t local_addr = start_idx * sizeof(T) + transport->get_local_info()[0].data_vaddr;
    uint32_t length = sizeof(T);
    uint64_t remote_addr = read_idx * sizeof(T) + transport->get_remote_info()[0].data_vaddr;
    int send_flag = 0;
    for (size_t i = 0; i < num_msg; i++) {
        if (i == num_msg - 1) {
            send_flag = 1;
        }
        transport->post_READ_with_flag(local_addr, length, remote_addr, send_flag);
        local_addr += length;
        remote_addr += length;
    }

    transport->poll_from_cq(1);
    return num_msg;
}

// explicit instantiations
template class Consumer<char>;
template class Consumer<int>;
template class Consumer<uint64_t>;
template class Consumer<std::string>;
template class Consumer<msg16_t>;
template class Consumer<msg64_t>;
template class Consumer<msg256_t>;

}