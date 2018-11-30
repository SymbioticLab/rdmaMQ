#ifndef CONFIG_RMQ_H_
#define CONFIG_RMQ_H_

/**
 * Configuration for rmq.
 */

namespace rmq {
    /* Message Buffer config */


    /* Broker buffer config */
    //constexpr int bkr_x = 18;        // 2^18 is # of data blocks broker buffer contains
    //constexpr size_t bkr_buff_cap = 262144; // 2^18 = 256 KB
    constexpr int bkr_x = 10;
    constexpr size_t bkr_buff_cap = 1024;

    /* RDMA transport config */
    constexpr int tr_max_cqe = 10000;
    constexpr uint32_t tr_max_send_wr = 8000;
    constexpr uint32_t tr_max_recv_wr = 8000;
    constexpr uint32_t tr_max_send_sge = 1;
    constexpr uint32_t tr_max_recv_sge = 1;
    constexpr uint32_t tr_max_inline_data = 100;
    constexpr uint8_t tr_phy_port_num = 1;      // normally your ib port
    constexpr int tr_path_mtu = 2048;
    constexpr int tr_tcp_port = 18515;



}

#endif // CONFIG_RMQ_H_