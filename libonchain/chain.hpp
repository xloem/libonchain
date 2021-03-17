#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <mutex>

namespace libonchain {

class Chain
{
public:
    enum Flag {
        CHAIN_TEST,
        CHAIN_PRIVATE,
    };

    enum ConnectionState {
        DISCONNECTED,
        CONNECTING,
        CONNECTED
    };

    struct Block {
        std::string id;
        uint64_t height;
        std::string parent;
    };

    std::string const name;
    std::string const technology;
    std::string const chain;
    std::unordered_set<Flag> const flags;

    virtual void connect(/*TODO: optionally pass datadir or list of seed peer ips*/) = 0;
    virtual void disconnect() = 0;
    ConnectionState const & connectionState() { return _connectionState; }

    virtual std::string root() = 0;
    virtual std::vector<std::string> tips() = 0;
    virtual Block block(std::string const & id) = 0;

    virtual std::vector<std::string> txs(std::string const & block = "mempool") = 0;
    
    virtual std::string txBroadcast(std::vector<uint8_t> const & tx) = 0;

    static std::unordered_map<std::string, Chain *> const & chains() { return _chains; }

protected:
    Chain(std::string const & technology, std::string const & chain, std::vector<Flag> const & flags);
    ~Chain();

    void on_connectionstate(ConnectionState state);
    void on_block(std::string id);
    void on_tx(std::string id);

    ConnectionState _connectionState;

    static std::mutex _staticMtx;
    static std::unordered_map<std::string, Chain *> _chains;
    static std::unordered_map<std::string, Chain *> _connectedChains;
};

}
