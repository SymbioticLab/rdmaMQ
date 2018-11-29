#include "producer.h"
#include <inttypes.h>

namespace rmq {

template <typename T>
void Producer<T>::fetch_and_add_write_addr(size_t num_msg) {
    uint64_t compare_add = num_msg * sizeof(T);
    transport->post_ATOMIC_FA(compare_add);
    /*
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
    */

    // poll wc
    // TODO: add event-triggered polling later
    /*
    struct ibv_wc wc;
    do {
		ne = ibv_poll_cq(ctx->send_cq, 1,&wc);
	} while (ne == 0);
    */
    // at this point remote write addr is read into ctrl_buf->get_data()
}

template <typename T>
size_t Producer<T>::push(size_t num_msg) {
    fetch_and_add_write_addr(num_msg);
    uint64_t write_addr = reinterpret_cast<uintptr_t>(ctrl_buf->get_data());
    LOG_DEBUG("getting WRITE ADDR: %" PRIu64 "\n", write_addr);
    // post WRITE_with_IMM
    
}

}