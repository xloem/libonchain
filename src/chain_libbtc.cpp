#include <libonchain/chain_libbtc.hpp>

#include <btc/chainparams.h>
#include <btc/netspv.h>
#include <btc/utils.h>
#include <event2/event.h>

std::string utils_uint256_to_reversed_hex(uint256 bin)
{
    static char const digits[] = "0123456789abcdef";
    char hex[sizeof(bin) * 2];

    size_t i, j;
    for (i = 0, j = sizeof(hex); i < sizeof(bin); i ++) {
        hex[-- j] = digits[(bin[i] >> 4) & 0xF];
        hex[-- j] = digits[bin[i] & 0xF];
    }

    return {hex, sizeof(hex)};
}

ChainLibbtc::ChainLibbtc(std::string const & technology, struct btc_chainparams_ const & chainparams)
: IChain(technology, chainparams.chainname),
  libbtc_chainparams(chainparams),
  stopping(false)
{ }

ChainLibbtc::~ChainLibbtc()
{
    disconnect();
}

static void ChainLibbtc::netspv_header_connected(btc_spv_client *libbtc_spvclient)
{
    ChainLibbtc *self = libbtc_spvclient->nodegroup->ctx;
}

static void netspv_sync_completed(btc_spv_client *libbtc_spvclient)
{
    ChainLibbtc *self = libbtc_spvclient->nodegroup->ctx;
    self->on_connectionstate(ConnectionState::CONNECTED);
}

static void netspv_header_message_processed(btc_spv_client *libbtc_spvclient, btc_node *node, btc_blockindex *newtip)
{
    ChainLibbtc *self = libbtc_spvclient->nodegroup->ctx;
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
        libbtc_spvclient->nodegroup->ctx = this;
        libbtc_spvclient->header_connected = netspv_header_connected;
        libbtc_spvclient->sync_completed = netspv_sync_completed;
        libbtc_spvclient->header_message_processed = netspv_header_message_processed;
        libbtc_spvclient->sync_transaction = netspv_sync_transaction;
        // libbtc_spvclient->nodegroup->log_write_cb
        // libbtc_spvclient->nodegroup->parse_cmd_cb
        // libbtc_spvclient->nodegroup->postcmd_cb
        // libbtc_spvclient->nodegroup->node_connection_state_changed_cb
        // libbtc_spvclient->nodegroup->should_connect_to_more_nodes_cb
        // libbtc_spvclient->nodegroup->handshake_done_cb
        // libbtc_spvclient->nodegroup->periodic_timer_cb 
        // libbtc_spvclient->headers_db
        
        // libbtc_spvclient->nodegroup->event_base // the libevent2 event structure
    }
    if (!runloop.joinable()) {
        on_connectionstate(ConnectionState::CONNECTING);
        runloop = std::thread(btc_spv_client_runloop);
    }
}

static void ChainLibbtc::run()
{
    int event_state;
    do {
        {
            std::unique_lock lk(mtx);
            btc_node_group_connect_next_nodes(libbtc_spvclient->nodegroup);
        }
    
        // process all pending events until there are none, which happens when all nodes are disconnected
        do {
            std::unique_lock lk(mtx);
            event_state = event_base_loop(libbtc_spvclient->nodegroup->event_base, EVLOOP_ONCE);
            if (event_state == -1) {
                throw std::runtime_error("libevent error");
            }
        } while (event_state == 0);
    } while (!stopping);
}

void ChainLibbtc::disconnect()
{
    if (runloop.joinable()) {
        {
            std::unique_lock lk(mtx);
            stopping = true;
            btc_node_group_shutdown(btc_spv_client->nodegroup); // this disconnects all peers but does not terminate the loop
        }
        runloop.join();
        stopping = false;
    }
    btc_spv_client.reset();
}

std::string ChainLibbtc::root()
{
    return utils_uint256_to_reversed_hex(libbtc_chainparams.genesisblockhash);
}

#include <iostream>
std::vector<std::string> ChainLibbtc::tips()
{
    btc_blockindex *tip = libbtc_spvclient->headers_db->getchaintip(libbtc_spvclient->headers_db);
    std::cerr << "warning: tips() function is plural but presently returning only the longest tip" << std::endl;
    std::cerr << "        it would be polite to make sure all transactions are in the longest tip" << std::endl;
    return {utils_uint256_to_reversed_hex(tip->hash)};
}

IChain::Block ChainLibbtc::block(std::string const & id)
{
    throw std::runtime_error("todo: retrieve block height and parent from headers.");
}

std::vector<std::string> txs(std::string const & block = "mempool")
{
    throw std::runtime_error("todo: retrieve block content from peers.  we have their inventories.");
}
