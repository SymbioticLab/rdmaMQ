#include "transport.h"

namespace rmq {

void Transport::init() {
    open_device();
    // TODO: (low priority) query device to pick the correct MTU
    // TODO: (medium priority) create complete channel
    alloc_pd();
}

void Transport::open_device() {
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
}

void Transport::alloc_pd() {
    pd = ibv_alloc_pd(ctx);
    assert_exit(pd, "Failed to allocate protection domain.");
}

void Transport::create_qp() {
    
}



}