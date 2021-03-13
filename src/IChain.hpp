#pragma once

#include <string>
#include <vector>

class IChain
{
public:
    std::string const technology;
    std::string const chain;

    struct Block {
        std::string id;
        uint64_t height;
        std::string parent;
    };

    virtual std::string root() = 0;
    virtual std::vector<std::string> tips() = 0;
    virtual Block block(std::string id) = 0;

    virtual std::vector<std::string> txs(std::string const & block = "mempool") = 0;

protected:
    IChainBackend(std::string technology, std::string chain);

    void on_block(std::string id);
    void on_tx(std::string id);
};
