#include "getopt.hpp"

#include <libonchain/IChain.hpp>

using namespace libonchain;

int main(int argc, char *argv[])
{
    for (auto & chain : IChain::chains()) {
        std::cout << "chain: " << chain.first << std::endl;
    }
    auto args = getopt("onchain", argc, argv, {
        {'t', "technology", "tech", "chain technology to use such as 'bsv'"},
        {'c', "chain", "chain", "chain to use, default='main'"},
    });
    return 0;
}
