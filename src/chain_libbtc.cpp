#include <libonchain/chain_libbtc.hpp>

#include <btc/chainparams.h>
#include <btc/netspv.h>

ChainLibbtc::ChainLibbtc(std::string const & technology, struct btc_chainparams_ const & chainparams)
: IChain(technology, chainparams.chainname),
  libbtc_chainparams(chainparams)
{ }

ChainLibbtc::~ChainLibbtc()
{ }

static void ChainLibbtc::netspv_header_connected(btc_spv_client *libbtc_spvclient)
{
    ChainLibbtc *self = libbtc_spvclient->headers_db_ctx;
}

static void netspv_sync_completed(btc_spv_client *libbtc_spvclient)
{
    ChainLibbtc *self = libbtc_spvclient->headers_db_ctx;
}

static void netspv_header_message_processed(btc_spv_client *libbtc_spvclient, btc_node *node, btc_blockindex *newtip)
{
    ChainLibbtc *self = libbtc_spvclient->headers_db_ctx;
}

static void netspv_header_message_processed(void *ctx, btc_tx *tx, unsigned int pos, btc_blockindex *blockindex)
{
    ChainLibbtc *self = ctx;
}

void ChainLibbtc::connect()
{
    if (!libbtc_spvclient) {
#if defined(NDEBUG)
        bool debug = false;
#else
        bool debug = true;
#endif
        libbtc_spvclient = std::unique_ptr<btc_spv_client>(
            btc_spv_client_new(chainparams.chainname, debug, /*mem only*/false),
            btc_spv_client_free
        );
        libbtc_spvclient->headers_db_ctx = this;
        libbtc_spvclient->sync_transaction_ctx = this;
        libbtc_spvclient->header_connected = netspv_header_connected;
        libbtc_spvclient->sync_completed = netspv_sync_completed;
        libbtc_spvclient->header_message_processed = netspv_header_message_processed;
        libbtc_spvclient->sync_transaction = netspv_sync_transaction;

        on_connectionstate(ConnectionState::CONNECTING);
        runloop = std::thread(btc_spv_client_runloop);
    }

}

void ChainLibbtc::disconnect()
{
    if (runlop.joinable()) {
        this should go in netspv_period_timer, which is part of nodegroup in netspv
        btc_node_group_shutdown(btc_spv_client->nodegroup);
        runloop.join();
    }
    btc_spv_client.reset();
}

std::string ChainLibbtc::root()
{
    convert  unit256 libbtc_chainparams.genesisblockhash  to string
}

std::vector<std::string> ChainLibbtc::tips()
{
    this likely requires 'connection', enumerate all tips using a state object
}

IChain::Block ChainLibbtc::block(std::string const & id)
{
    
}

std::vector<std::string> txs(std::string const & block = "mempool")
{

}
