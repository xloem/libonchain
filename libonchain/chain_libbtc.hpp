#pragma once

#include <libonchain/chain.hpp>
#include <mutex>
#include <thread>

struct btc_chainparams_;
struct btc_spv_client_;
struct btc_wallet;
struct btc_node_;
struct btc_blockindex;
struct btc_tx_;
struct btc_p2p_msg_hdr_;
struct const_buffer;

namespace libonchain {

class ChainLibbtc : public Chain
{
public:
    typedef struct ::btc_chainparams_ btc_chainparams;

    ChainLibbtc(std::string const & technology, btc_chainparams const & chainparams, std::vector<Flag> const &flags);
    ~ChainLibbtc();

    void connect(std::string datadir = ".", std::vector<std::string> seeds = {}) override;
    void disconnect() override;

    std::string root() override;
    std::vector<std::string> tips() override;
    Block block(std::string const & id) override;
    std::vector<std::string> txs(std::string const & block = "mempool") override;
    std::string txBroadcast(std::vector<uint8_t> const & tx) override;

    btc_chainparams const & libbtc_chainparams;

private:
    typedef struct ::btc_spv_client_ btc_spv_client;
    typedef struct ::btc_node_ btc_node;
    typedef struct ::btc_blockindex btc_blockindex;
    typedef struct ::btc_tx_ btc_tx;
    typedef struct ::btc_p2p_msg_hdr_ btc_p2p_msg_hdr;
    typedef unsigned char btc_bool;

    std::unique_ptr<btc_spv_client,void(*)(btc_spv_client*)> libbtc_spvclient;
    std::unique_ptr<btc_wallet,void(*)(btc_wallet*)> libbtc_wallet;
    std::string datadir;
    std::unordered_map<std::string,std::vector<uint8_t>> pending_txs;

    static void netspv_header_connected(btc_spv_client *libbtc_spvclient);
    static void netspv_sync_completed(btc_spv_client *libbtc_spvclient);
    static btc_bool netspv_header_message_processed(btc_spv_client *libbtc_spvclient, btc_node *node, btc_blockindex *newtip);
    static void netspv_sync_transaction(void *sync_ctx, btc_tx *tx, unsigned int pos, btc_blockindex *blockindex);

    static int net_log_write(char const *format, ...);
    static btc_bool net_parse_cmd(btc_node *node, btc_p2p_msg_hdr *hdr, const_buffer *buf);
    static void net_node_connection_state_changed(btc_node *node);
    static btc_bool net_should_connect_to_more_nodes(btc_node *node);
    static void net_postcmd(btc_node *node, btc_p2p_msg_hdr *hdr, const_buffer *buf);
    static void net_handshake_done(btc_node *node);
    static btc_bool net_periodic_timer(btc_node *node, uint64_t *time);

    btc_bool (*_netspv_parse_cmd)(btc_node *node, btc_p2p_msg_hdr *hdr, const_buffer *buf);
    void (*_netspv_node_connection_state_changed)(btc_node *node);
    btc_bool (*_netspv_should_connect_to_more_nodes)(btc_node *node);
    void (*_netspv_postcmd)(btc_node *node, btc_p2p_msg_hdr *hdr, const_buffer *buf);
    void (*_netspv_handshake_done)(btc_node *node);
    btc_bool (*_netspv_periodic_timer)(btc_node *node, uint64_t *time);

    std::thread runloop;
    bool stopping;
    std::mutex mtx;
    std::unique_lock<std::mutex> lock;
    void run();
};

extern ChainLibbtc bsv_main;
extern ChainLibbtc bsv_stn;
extern ChainLibbtc bsv_testnet;
extern ChainLibbtc bsv_regtest;

}
