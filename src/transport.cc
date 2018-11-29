#include "transport.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>

namespace rmq {

// helper function to convert int mtu to enum ibv_mtu (to not include ibverbs in config)
enum ibv_mtu get_ibv_mtu(int mtu) {
    enum ibv_mtu ib_mtu;
	switch (mtu) {
        case 256:  ib_mtu = IBV_MTU_256;
        case 512:  ib_mtu = IBV_MTU_512;
        case 1024: ib_mtu = IBV_MTU_1024;
        case 2048: ib_mtu = IBV_MTU_2048;
        case 4096: ib_mtu = IBV_MTU_4096;
        default:   assert_exit(false, "Invalid MTU value");
	}
    return ib_mtu;
}

// helper functions to convert gid byte order
void wire_gid_to_gid(const char *wgid, union ibv_gid *gid) {
    char tmp[9];
    uint32_t v32;
    uint32_t *raw = (uint32_t *)gid->raw;
    int i;

    for (tmp[8] = 0, i = 0; i < 4; ++i) {
        memcpy(tmp, wgid + i * 8, 8);
        sscanf(tmp, "%x", &v32);
        raw[i] = ntohl(v32);
    }
}

void gid_to_wire_gid(const union ibv_gid *gid, char wgid[]) {
    int i;
    uint32_t *raw = (uint32_t *)gid->raw;

    for (i = 0; i < 4; ++i) {
        sprintf(&wgid[i * 8], "%08x", htonl(raw[i]));
    }
}

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

    pd = ibv_alloc_pd(ctx);     // at this ctx can be accessed thru pd;
    assert_exit(pd, "Failed to allocate protection domain.");
}

void Transport::create_cq() {
    channel = ibv_create_comp_channel(pd->context);
    cq = ibv_create_cq(pd->context, tr_max_cqe, NULL, NULL, 0);
    assert_exit(cq, "Failed to create CQ.");
}

void Transport::create_qp() {
    struct ibv_qp_init_attr init_attr;
    memset(&init_attr, 0, sizeof(struct ibv_qp_init_attr));

    init_attr.send_cq               = cq;
    init_attr.recv_cq               = cq;
    init_attr.cap.max_send_wr       = tr_max_send_wr;
    init_attr.cap.max_recv_wr       = tr_max_recv_wr;
    init_attr.cap.max_send_sge      = tr_max_send_sge;
    init_attr.cap.max_recv_sge      = tr_max_recv_sge;
    init_attr.cap.max_inline_data   = tr_max_inline_data;
    init_attr.qp_type               = IBV_QPT_RC;
    init_attr.qp_context = (void *)1;       // could be used later for Justitia

    qp = ibv_create_qp(pd, &init_attr);
    assert_exit(qp, "Failed to create QP.");
}

void Transport::init_local_info(struct ibv_mr *data_mr, struct ibv_mr *ctrl_mr, int gid_idx) {
    LOG_DEBUG("gid_idx is %d.\n", gid_idx);
    struct ibv_port_attr port_attr;
	assert_exit(ibv_query_port(pd->context, tr_phy_port_num, &port_attr) == 0, "Failed to query ib port.");

    local_info.lid = port_attr.lid;
    local_info.qpn = qp->qp_num;
    local_info.psn = lrand48() & 0xffffff;
    local_info.data_rkey = data_mr->rkey;
    local_info.data_vaddr = reinterpret_cast<uintptr_t>(data_mr->addr);
    local_info.ctrl_rkey = ctrl_mr->rkey;
    local_info.ctrl_vaddr = reinterpret_cast<uintptr_t>(ctrl_mr->addr);
    local_info.gid_idx = gid_idx;

    if (gid_idx >= 0) {
        assert_exit(ibv_query_gid(pd->context, tr_phy_port_num, gid_idx, &local_info.gid) == 0, "Failed to query local gid.");
    }
}

void Transport::modify_qp_to_INIT() {
    struct ibv_qp_attr attr;
    struct ibv_qp_init_attr init_attr;
    assert_exit(ibv_query_qp(qp, &attr, IBV_QP_STATE, &init_attr) == 0, "Failed to query QP.");
    assert_exit(attr.qp_state == IBV_QPS_RESET, "Error: QP state not RESET when calling modify_qp_to_INIT().");

    memset(&attr, 0, sizeof(struct ibv_qp_attr));
    attr.qp_state           = IBV_QPS_INIT;
    attr.pkey_index         = 0;
    attr.port_num           = tr_phy_port_num;
    attr.qp_access_flags    = IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_WRITE |
                                IBV_ACCESS_REMOTE_READ | IBV_ACCESS_REMOTE_ATOMIC;

    assert_exit(ibv_modify_qp(qp, &attr,
        IBV_QP_STATE            |
        IBV_QP_PKEY_INDEX       |
        IBV_QP_PORT             |
        IBV_QP_ACCESS_FLAGS) == 0, "Failed to modify QP to INIT.");
    LOG_DEBUG("Modify QP to INIT state.\n");
}

void Transport::modify_qp_to_RTR(uint8_t sl) {
    struct ibv_qp_attr attr;
    struct ibv_qp_init_attr init_attr;
    assert_exit(ibv_query_qp(qp, &attr, IBV_QP_STATE, &init_attr) == 0, "Failed to query QP.");
    assert_exit(attr.qp_state == IBV_QPS_INIT, "Error: QP state not INIT when calling modify_qp_to_RTR().");

    memset(&attr, 0, sizeof(attr));
    attr.qp_state               = IBV_QPS_RTR;
    attr.path_mtu               = get_ibv_mtu(tr_path_mtu);
    attr.dest_qp_num            = remote_info.qpn;
    attr.rq_psn                 = remote_info.psn;
    attr.max_dest_rd_atomic     = 1;
    attr.min_rnr_timer          = 12;
    attr.ah_attr.is_global      = 0;
    attr.ah_attr.dlid           = remote_info.lid;
    attr.ah_attr.sl             = sl;
    attr.ah_attr.src_path_bits  = 0;
    attr.ah_attr.port_num       = tr_phy_port_num;

    // check for RoCE
    if (remote_info.gid.global.interface_id) {
		attr.ah_attr.is_global = 1;
		attr.ah_attr.grh.hop_limit = 1;
		attr.ah_attr.grh.dgid = remote_info.gid;
		attr.ah_attr.grh.sgid_index = local_info.gid_idx;
	}

    assert_exit(ibv_modify_qp(qp, &attr,
        IBV_QP_STATE              |
        IBV_QP_AV                 |
        IBV_QP_PATH_MTU           |
        IBV_QP_DEST_QPN           |
        IBV_QP_RQ_PSN             |
        IBV_QP_MAX_DEST_RD_ATOMIC |
        IBV_QP_MIN_RNR_TIMER) == 0, "Failed to modify QP to RTR.");
    LOG_DEBUG("Modify QP to RTR state.\n");
}

void Transport::modify_qp_to_RTS() {
    struct ibv_qp_attr attr;	
    struct ibv_qp_init_attr init_attr;
    assert_exit(ibv_query_qp(qp, &attr, IBV_QP_STATE, &init_attr) == 0, "Failed to query QP.");
    assert_exit(attr.qp_state == IBV_QPS_RTR, "Error: QP state not RTR when calling modify_qp_to_RTS().");

    memset(&attr, 0, sizeof(struct ibv_qp_attr));
    attr.qp_state	    = IBV_QPS_RTS;
    attr.sq_psn	        = local_info.psn;
    attr.timeout	    = 14;
    attr.retry_cnt	    = 7;
    attr.rnr_retry	    = 7;    //infinite
    attr.max_rd_atomic  = 1;

    assert_exit(ibv_modify_qp(qp, &attr,
        IBV_QP_STATE              |
        IBV_QP_TIMEOUT            |
        IBV_QP_RETRY_CNT          |
        IBV_QP_RNR_RETRY          |
        IBV_QP_SQ_PSN             |
        IBV_QP_MAX_QP_RD_ATOMIC) == 0, "Failed to modify QP to RTS");
    LOG_DEBUG("Modify QP to RTS state.\n");
}

void Transport::hand_shake_client(const char * server_addr) {
    struct addrinfo *res, *t, hints;
    char *service;
    char msg[sizeof "0000:000000:000000:00000000:0000000000000000:00000000:0000000000000000:00000000000000000000000000000000"];
    int sockfd = -1;
    char gid[33];

    assert_exit(asprintf(&service, "%d", tr_tcp_port) > 0, "Invalid server port number.");

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    assert_exit(getaddrinfo(server_addr, service, &hints, &res) == 0, "Error getting address info of server.");

    for (t = res; t; t = t->ai_next) {
        sockfd = socket(t->ai_family, t->ai_socktype, t->ai_protocol);
        if (sockfd >= 0) {
            if (!connect(sockfd, t->ai_addr, t->ai_addrlen))
                break;
            close(sockfd);
            sockfd = -1;
        }
    }

    freeaddrinfo(res);
    free(service);
    assert_exit(sockfd == 0, "Error connecting to server via socket.");

    gid_to_wire_gid(&local_info.gid, gid);
    sprintf(msg, "%04x:%06x:%06x:%08x:%016lx:%08x:%016lx:%s", local_info.lid, local_info.qpn,
                local_info.psn, local_info.data_rkey, local_info.data_vaddr,
                local_info.ctrl_rkey, local_info.ctrl_vaddr, gid);

    assert_exit(write(sockfd, msg, sizeof(msg)) == sizeof(msg), "Error sending local node info.");

    assert_exit(recv(sockfd, msg, sizeof(msg), MSG_WAITALL) == sizeof(msg), "Error recving remote node info: " + std::string(strerror(errno)) + ".");

    assert_exit(write(sockfd, "done", sizeof("done")) == sizeof("done"), "Error sending done message");

    sscanf(msg, "%hu:%x:%x:%x:%lx:%x:%lx:%s", &remote_info.lid, &remote_info.qpn,
                &remote_info.psn, &remote_info.data_rkey, &remote_info.data_vaddr,
                &remote_info.ctrl_rkey, &remote_info.ctrl_vaddr, gid);
    wire_gid_to_gid(gid, &remote_info.gid);
    LOG_DEBUG("Client hand shake done.\n");
}

void Transport::hand_shake_server() {
    struct addrinfo *res, *t, hints;
    char *service;
    char msg[sizeof "0000:000000:000000:00000000:0000000000000000:00000000:0000000000000000:00000000000000000000000000000000"];
    int sockfd = -1, connfd, optval = 1;
    char gid[33];

    assert_exit(asprintf(&service, "%d", tr_tcp_port) > 0, "Invalid server port number.");

    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    assert_exit(getaddrinfo(nullptr, service, &hints, &res) == 0, "Error getting address info of server.");

    for (t = res; t; t = t->ai_next) {
        sockfd = socket(t->ai_family, t->ai_socktype, t->ai_protocol);
        if (sockfd >= 0) {
            setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
            if (!bind(sockfd, t->ai_addr, t->ai_addrlen))
                break;
            close(sockfd);
            sockfd = -1;
        }
    }

    freeaddrinfo(res);
    free(service);
    assert_exit(sockfd == 0, "Error binding via socket.");

    listen(sockfd, 1);
    connfd = accept(sockfd, NULL, 0);
    close(sockfd);
    assert_exit(connfd >= 0, "Error accepting conn from client via socket.");
    
    assert_exit(recv(connfd, msg, sizeof(msg), MSG_WAITALL) == sizeof(msg), "Error recving remote node info: " + std::string(strerror(errno)) + ".");

    sscanf(msg, "%hu:%x:%x:%x:%lx:%x:%lx:%s", &remote_info.lid, &remote_info.qpn,
                &remote_info.psn, &remote_info.data_rkey, &remote_info.data_vaddr,
                &remote_info.ctrl_rkey, &remote_info.ctrl_vaddr, gid);
    wire_gid_to_gid(gid, &remote_info.gid);

    gid_to_wire_gid(&local_info.gid, gid);
    sprintf(msg, "%04x:%06x:%06x:%08x:%016lx:%08x:%016lx:%s", local_info.lid, local_info.qpn,
                local_info.psn, local_info.data_rkey, local_info.data_vaddr,
                local_info.ctrl_rkey, local_info.ctrl_vaddr, gid);
    assert_exit(write(connfd, msg, sizeof(msg)) == sizeof(msg), "Error sending local node info.");

    assert_exit(recv(connfd, msg, sizeof("done"), MSG_WAITALL) == sizeof("done"), "Error recving done message");
    LOG_DEBUG("Server hand shake done.\n");
}

void Transport::init(const char *server_addr, struct ibv_mr *data_mr, struct ibv_mr *ctrl_mr, int gid_idx) {
    init_local_info(data_mr, ctrl_mr, gid_idx);

    create_cq();
    create_qp();
    
    modify_qp_to_INIT();

    if (server_addr) {      // client
        hand_shake_client(server_addr);
    } else {                // server
        hand_shake_server();
    }

    modify_qp_to_RTR();     // sl = 0
    modify_qp_to_RTS();
}

}