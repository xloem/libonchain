#pragma once

#include <libonchain/IChain.hpp>
#include <mutex>
#include <thread>

struct btc_chainparams_;
struct btc_spv_client_;
struct btc_node_;
struct btc_blockindex;
struct btc_tx_;

namespace libonchain {

class ChainLibbtc : public IChain
{
public:
    typedef struct ::btc_chainparams_ btc_chainparams;

    ChainLibbtc(std::string const & technology, btc_chainparams const & chainparams, std::vector<Flag> const &flags);
    ~ChainLibbtc();

    void connect() override;
    void disconnect() override;

    std::string root() override;
    std::vector<std::string> tips() override;
    Block block(std::string const & id) override;
    std::vector<std::string> txs(std::string const & block = "mempool") override;

    btc_chainparams const & libbtc_chainparams;

private:
    typedef struct ::btc_spv_client_ btc_spv_client;
    typedef struct ::btc_node_ btc_node;
    typedef struct ::btc_blockindex btc_blockindex;
    typedef struct ::btc_tx_ btc_tx;
    typedef unsigned char btc_bool;

    std::unique_ptr<btc_spv_client,void(*)(btc_spv_client*)> libbtc_spvclient;

    static void netspv_header_connected(btc_spv_client *libbtc_spvclient);
    static void netspv_sync_completed(btc_spv_client *libbtc_spvclient);
    static btc_bool netspv_header_message_processed(btc_spv_client *libbtc_spvclient, btc_node *node, btc_blockindex *newtip);
    static void netspv_sync_transaction(void *sync_ctx, btc_tx *tx, unsigned int pos, btc_blockindex *blockindex);

    std::thread runloop;
    bool stopping;
    std::mutex mtx;
    std::unique_lock<std::mutex> lock;
    void run();
};

}
