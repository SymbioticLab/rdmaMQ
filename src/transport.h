#ifndef TRANSPORT_H_
#define TRANSPORT_H_

#include <infiniband/verbs.h>
#include "common.h"
//#include "rmq.h"

namespace rmq {

//template<typename T>
//class MessageBuffer;

// a small struct which holds the remote node info to establish RDMA conn
// assume a data mr and a control mr for now
struct dest_info {
    uint16_t lid;
    uint32_t qpn;
    uint32_t psn;
    int gid_idx;
    uint32_t data_rkey;
    uint64_t data_vaddr;
    uint32_t ctrl_rkey;
    uint64_t ctrl_vaddr;
    union ibv_gid gid;
};

/**
 * Handles RDMA context/connection setup before MessageBuffer 
 * registers memory region and Producer and Consumer transfer data.
 * Currently, the class is instantiated with the default constr.
 * 
 * init() is called in the constructor; other calls are done after
 * mbuf is created.
 * 
 * create_cq() creates unified comp_channel & cq for sq & rq for now.
 * 
 * Each tranport object is associated with 2 mrs, one data and the
 * other for control.
 */

class Transport {
//template<typename T>
//friend class MessageBuffer<T>;
private:
    struct ibv_pd *pd;                  // contains ibv_context once created
    struct ibv_cq *cq;                  // for both sq & rq
    struct ibv_comp_channel *channel;   // for both sq & rq
    struct ibv_qp *qp;
    struct dest_info rem_dest;          // remote node info (*mr only has rkey & vaddr)
    struct dest_info my_dest;           // local node info

    void open_device_and_alloc_pd();

    // create comp channel and cq
    void create_cq();

    // create RC qp
    void create_qp();

    // init my_dest
    void init_my_dest(struct ibv_mr *data_mr, struct ibv_mr *ctrl_mr, int gid_idx);

    // gets called after create_qp();
    void modify_qp_to_INIT();

    // gets called after exchaning info with the remote node
    void modify_qp_to_RTR(uint8_t sl = 0);
    void modify_qp_to_RTS();

    // exchange node info for RDMA (routing, raddr, etc.)
    void hand_shake_client(const char * server_addr);
    void hand_shake_server();

public:
    Transport() { open_device_and_alloc_pd(); }
    //Transport(int is_server): is_server(is_server) { init(); };
    ~Transport() {}
    inline struct ibv_context *ibv_get_ctx() { return pd->context; }
    inline struct ibv_pd *get_pd() { return pd; }
    inline struct ibv_qp *get_qp() { return qp; }
    inline struct ibv_cq *get_cq() { return cq; }

    // init transport (between sender & receiver) after MessageBuffer is constructed
    // After init() returns, qp has transited to RTS.
    void init(const char *server_addr, struct ibv_mr *data_mr, struct ibv_mr *ctrl_mr, int gid_idx = -1);

};




}

#endif // TRANSPORT_H_