#include "transport.h"

namespace rmq {

void Transport::init() {
    int num_devices = 0;
    struct ibv_device **dev_list = ibv_get_device_list(&num_devices);
    assert_true(num_devices > 0, "Failed to find ib devices.");

    int i = 0;
    for (; i < num_devices; i++) {
        ctx = ibv_open_device(dev_list[i]);
        if (ctx) break;
    }
    assert_true(ctx != nullptr, "Failed to open ib device " + std::to_string(ibv_get_device_name(i)));
    printf("Pick ib device %s\n", ibv_get_device_name(i));

    // TODO: (low priority) query device to pick the correct MTU
    // TODO: (medium priority) create complete channel
    pd = ibv_alloc_pd(ctx);
    assert_true(pd != nullptr, "Failed to allocate protection domain.");
}


}