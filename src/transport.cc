#include "transport.h"

namespace rmq {

// combine open_dev and alloc_pd so that we don't need to store ctx in the class
void Transport::open_device_and_alloc_pd() {
    struct ibv_context *ctx;
    int num_devices = 0;
    struct ibv_device **dev_list = ibv_get_device_list(&num_devices);
    assert_exit(num_devices > 0, "Failed to find ib devices.");

    int i = 0;
    for (; i < num_devices; i++) {
        ctx = ibv_open_device(dev_list[i]);
        if (ctx) break;
    }
    assert_exit(ctx != nullptr, "Failed to open ib device " + std::string(ibv_get_device_name(dev_list[i])));
    LOG_INFO("Pick ib device %s\n", ibv_get_device_name(dev_list[i]));
    // TODO: (low priority) query device to pick the correct MTU
    // TODO: (medium priority) create complete channel
    pd = ibv_alloc_pd(ctx);     // at this ctx can be accessed thru pd;
    assert_exit(pd, "Failed to allocate protection domain.");
}

void Transport::init() {
    open_device_and_alloc_pd();
}

void Transport::create_cq() {
    channel = ibv_create_comp_channel(pd->context);
    cq = ibv_create_cq(pd->context, tr_max_cqe, NULL, NULL, 0);
    assert_exit(cq, "Failed to create CQ.");
}

void Transport::create_qp() {
    struct ibv_qp_init_attr init_attr;
    memset(&init_attr, 0, sizeof(struct ibv_qp_init_attr));
    init_attr.send_cq = cq;
    init_attr.recv_cq = cq;
    init_attr.cap.max_send_wr  = tr_max_send_wr;
    init_attr.cap.max_recv_wr  = tr_max_recv_wr;
    init_attr.cap.max_send_sge = tr_max_send_sge;
    init_attr.cap.max_recv_sge = tr_max_recv_sge;
    init_attr.cap.max_inline_data = tr_max_inline_data;
    init_attr.qp_type = IBV_QPT_RC;
    init_attr.qp_context = (void *)1;       // could be used later for Justitia
    qp = ibv_create_qp(pd, &init_attr);
    assert_exit(qp, "Failed to create QP.");
}

void Transport::modify_qp_to_INIT() {
    struct ibv_qp_attr attr;
    struct ibv_qp_init_attr init_attr;
    assert_exit(ibv_query_qp(qp, &attr, IBV_QP_STATE, &init_attr) == 0, "Failed to query QP.");
    assert_exit(attr.qp_state == IBV_QPS_RESET, "Error: QP state not RESET when calling modify_qp_to_INIT().");
    memset(&attr, 0, sizeof(struct ibv_qp_attr));
    attr.qp_state = IBV_QPS_INIT;
    attr.pkey_index = 0;
    attr.port_num = tr_phy_port_num;
    attr.qp_access_flags = IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_WRITE |
                            IBV_ACCESS_REMOTE_READ | IBV_ACCESS_REMOTE_ATOMIC;
    assert_exit(ibv_modify_qp(qp, &attr,
            IBV_QP_STATE            |
            IBV_QP_PKEY_INDEX       |
            IBV_QP_PORT             |
            IBV_QP_ACCESS_FLAGS) == 0, "Failed to modify QP to INIT.");
}

void Transport::modify_qp_to_RTR() {

}

void Transport::modify_qp_to_RTS() {

}


}