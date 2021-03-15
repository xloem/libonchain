#pragma once

#include <libonchain/IChain.hpp>
#include <thread>

namespace libonchain {

class ChainLibbtc : public IChain
{
public:

    ChainLibbtc(std::string const & technology, struct btc_chainparams_ const & chainparams);
    ~ChainLibbtc();

    override void connect();
    override void disconnect();

    override std::string root();
    override std::vector<std::string> tips();
    override Block block(std::string const & id);
    override std::vector<std::string> txs(std::string const & block = "mempool");

    struct btc_chainparams_ const & libbtc_chainparams;
    std::unique_ptr<struct btc_spv_client_> libbtc_spvclient;

private:
    static void netspv_header_connected(struct btc_spv_client_ *libbtc_spvclient);
    static void netspv_sync_completed(struct btc_spv_client_ *libbtc_spvclient);
    static void netspv_header_message_processed(struct btc_spv_client_ *libbtc_spvclient, struct btc_node_ *node, struct btc_blockindex_ *newtip);
    static void netspv_sync_transaction(void *sync_ctx, struct btc_tx_ *tx, unsigned int pos, struct btc_blockindex_ *blockindex);
    static uint8_t netspv_periodic_timer(struct btc_node_ *node, uint864_t *time);

    std::thread runloop;
};

}
