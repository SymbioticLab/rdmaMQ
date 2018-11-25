#include "transport.h"

namespace rmq {

// combine open_dev and alloc_pd so that we don't need to store ctx in the class
void Transport::open_device_and_alloc_pd() {
    struct ibv_context ctx;
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

void Transport::create_qp() {
    channel = ibv_create_comp_channel(ctx)
    cq = ibv_create_cq(ctx->context, MAX_CQE, NULL, NULL, 0);
    assert_exit(cq, "Failed to create CQ.");
    
}



}