#ifndef TRANSPORT_H_
#define TRANSPORT_H_

#include <infiniband/verbs.h>
#include "common.h"
//#include "rmq.h"

namespace rmq {

//template<typename T>
//class MessageBuffer;


// a small struct which holds the remote node info to establish RDMA conn
struct dest_info {
    uint16_t lid;
    uint32_t qpn;
    uint32_t psn;
    uint32_t rkey;
    uint64_t vaddr;
    union ibv_gid gid;  // only relevant for rem_dest
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
 */

class Transport {
//template<typename T>
//friend class MessageBuffer<T>;
private:
    struct ibv_pd *pd;                  // contains ibv_context once created
    struct ibv_cq *cq;                  // for both sq & rq
    struct ibv_comp_channel *channel;   // for both sq & rq
    struct ibv_qp *qp;
    struct dest_info rem_dest;       // remote node info
    // TODO: and init my_dest
    struct dest_info my_dest;           // local node info

    void open_device_and_alloc_pd();
    // TODO: decide what to put in init() later
    // TODO: add RoCE support (gid_idx, ibv_query_gid(), etc.) later

    // calls open_device_and_alloc_pd()
    void init();

    // create comp channel and cq
    void create_cq();

    // create RC qp
    void create_qp();

    // gets called after create_qp();
    void modify_qp_to_INIT();

    // gets called after exchaning info with the remote node
    void modify_qp_to_RTR(uint8_t sl = 0, int gid_idx = -1);
    void modify_qp_to_RTS(uint32_t psn = 23333);

    // exchange node info for RDMA (routing, raddr, etc.)
    void hand_shake_client(const char * server_addr);
    void hand_shake_server();

public:
    Transport() { init(); }
    //Transport(int is_server): is_server(is_server) { init(); };
    ~Transport() {}
    inline struct ibv_context *ibv_get_ctx() { return pd->context; }
    inline struct ibv_pd *get_pd() { return pd; }
    inline struct ibv_qp *get_qp() { return qp; }
    inline struct ibv_cq *get_cq() { return cq; }

    // calls hand_shake_client() hand_shake_server()
    void qp_hand_shake(const char *server_addr);

    // calls create_cq() and create_qp()
    void create_cq_and_qp();

    // modify qp states
    void modify_qp_state(enum ibv_qp_state target_state);
};




}

#endif // TRANSPORT_H_