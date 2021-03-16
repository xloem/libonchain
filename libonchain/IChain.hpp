#pragma once

#include <string>
#include <vector>

namespace libonchain {

class IChain
{
public:
    std::string const technology;
    std::string const chain;

    enum class Flag {
        TEST,
        PRIVATE,
        DEFAULT
    };

    enum class ConnectionState {
        DISCONNECTED,
        CONNECTING,
        CONNECTED
    };

    struct Block {
        std::string id;
        uint64_t height;
        std::string parent;
    };

    virtual void connect() = 0;
    virtual void disconnect() = 0;
    ConnectionState connectionState();

    virtual std::string root() = 0;
    virtual std::vector<std::string> tips() = 0;
    virtual Block block(std::string const & id) = 0;

    virtual std::vector<std::string> txs(std::string const & block = "mempool") = 0;

protected:
    IChain(std::string technology, std::string chain, std::vector<Flag> flags = {});

    void on_connectionstate(ConnectionState state);
    void on_block(std::string id);
    void on_tx(std::string id);
};

}
