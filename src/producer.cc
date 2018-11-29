#include "producer.h"

namespace rmq {

template <typename T>
void Producer<T>::fetch_and_add_write_addr(size_t num_msg) {
    struct ibv_sge sg;
    struct ibv_send_wr wr;
    struct ibv_send_wr *bad_wr;

    sg.addr	  = reinterpret_cast<uintptr_t>(ctrl_buf->get_data());
    sg.length = sizeof(uint64_t);
    sg.lkey	  = ctrl_buf->get_mr()->lkey;

    memset(&wr, 0, sizeof(wr));
    wr.wr_id      = 0;
    wr.sg_list    = &sg;
    wr.num_sge    = 1;
    wr.opcode     = IBV_WR_ATOMIC_FETCH_AND_ADD;
    wr.send_flags = IBV_SEND_SIGNALED;
    wr.wr.atomic.remote_addr = transport->remote_info.ctrl_vaddr;
    wr.wr.atomic.rkey        = transport->remote_info.ctrl_rkey;
    wr.wr.atomic.compare_add = num_msg * sizeof(T);

    assert_exit(ibv_post_send(transport->get_qp(), &wr, &bad_wr) == 0, "Failed to post sr to fetch & add write addr.");
    // at this point remote write addr is read into ctrl_buf->get_data()
}

}