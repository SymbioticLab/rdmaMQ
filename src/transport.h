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
    uint32_t lid;
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
    size_t num_qp;
    struct ibv_pd *pd;                  // contains ibv_context once created
    struct ibv_cq **cq;                  // for both sq & rq
    struct ibv_comp_channel **channel;   // for both sq & rq
    struct ibv_qp **qp;
    struct dest_info *remote_info;       // remote node info
    struct dest_info *local_info;        // local node info
    struct ibv_mr *data_mr;             // MR for data exchange, which Procuder::data_buf holds
    struct ibv_mr *ctrl_mr;             // MR for control such as offset, which Procuder::ctrl_buf holds

    void open_device_and_alloc_pd();

    // create comp channel and cq
    void create_cq();

    // create RC qp
    void create_qp();

    // init local_info
    void init_local_info(int gid_idx);

    // gets called after create_qp();
    void modify_qp_to_INIT(size_t qp_idx);

    // gets called after exchaning info with the remote node
    void modify_qp_to_RTR(size_t qp_idx, uint8_t sl = 0);
    void modify_qp_to_RTS(size_t qp_idx);

    // exchange node info for RDMA (routing, raddr, etc.)
    void hand_shake_client(size_t qp_idx, const char * server_addr);
    void hand_shake_server(size_t qp_idx);

public:
    Transport() { open_device_and_alloc_pd(); }
    //Transport(int is_server): is_server(is_server) { init(); };
    ~Transport() {
        LOG_DEBUG("Transport destructor gets called\n");
        delete[] local_info;
        delete[] remote_info;
        delete[] channel;
        delete[] cq;
        delete[] qp;
    }
    inline struct ibv_context *ibv_get_ctx() { return pd->context; }
    inline struct ibv_pd *get_pd() { return pd; }
    inline struct ibv_qp **get_qp() { return qp; }
    inline struct ibv_cq **get_cq() { return cq; }
    inline size_t get_num_qp() { return num_qp; }
    inline struct dest_info *get_local_info() { return local_info; }
    inline struct dest_info *get_remote_info() { return remote_info; }

    // init transport (between sender & receiver) after MessageBuffer is constructed
    // After init() returns, qp has transited to RTS.
    // qp_num : number of qps and (same) number of cqs/comp_channels (send_cq/recv_cq shared)
    void init(const char *server_addr, size_t num_qp, struct ibv_mr *data_mr, struct ibv_mr *ctrl_mr, int gid_idx);

    // poll wc from cq
    void poll_from_cq(int num_entries, size_t qp_idx = 0);
    
    // post a send request using ATOMIC_FETCH_AND_ADD
    // used by Producer to get write addr from the broker
    // value read is put in ctrl_mr
    void post_ATOMIC_FA(uint64_t compare_add, size_t qp_idx = 0);

    // post a send request using RDMA_WRITE
    // uses data_mr
    // local_addr specifies sge.addr in local buffer,
    void post_WRITE(uint64_t local_addr, uint32_t length, uint64_t remote_addr, size_t qp_idx = 0);

    // post a send request using RMDA_READ
    // uses data_mr
    void post_READ(uint64_t local_addr, uint32_t length, uint64_t remote_addr, size_t qp_idx = 0);

};




}

#endif // TRANSPORT_H_