#ifndef CONFIG_RMQ_H_
#define CONFIG_RMQ_H_


/**
 * Configuration for rmq.
 */

namespace rmq {


    /* RDMA transport config */
    constexpr int tr_max_cqe = 10000;
    constexpr uint32_t tr_max_send_wr = 8000;
    constexpr uint32_t tr_max_recv_wr = 8000;
    constexpr uint32_t tr_max_send_sge = 1;
    constexpr uint32_t tr_max_recv_sge = 1;
    constexpr uint32_t tr_max_inline_data = 100;



}

#endif // CONFIG_RMQ_H_