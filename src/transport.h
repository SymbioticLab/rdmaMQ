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
 */

class Transport {
//template<typename T>
//friend class MessageBuffer<T>;
private:
    struct ibv_pd *pd;                  // contains ibv_context once created
    struct ibv_cq *cq;                  // for both sq & rq
    struct ibv_comp_channel *channel;   // for both sq & rq
    struct ibv_qp *qp;
    struct dest_info dest;              // remote node info (for RDMA)

    void open_device_and_alloc_pd();

    // calls open_device_and_alloc_pd()
    void init();

    // create comp channel and cq
    void create_cq();

    // create RC qp
    void create_qp();

    // gets called after create_qp();
    void modify_qp_to_INIT();

    // gets called after info exchanged with the remote node
    void modify_qp_to_RTR(uint8_t sl = 0);
    void modify_qp_to_RTS();


public:
    Transport() { init(); }
    ~Transport() {}
    inline struct ibv_context *ibv_get_ctx() { return pd->context; }
    inline struct ibv_pd *get_pd() { return pd; }
    //inline struct ibv_mr get_mr() { return mr; }
    //inline void set_mr(struct ibv_mr *mr) { this->mr = mr; }
};




}

#endif // TRANSPORT_H_