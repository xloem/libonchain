#include "getopt.hpp"

#include <libonchain/chain.hpp>

#include <mutex>

using namespace libonchain;

int main(int argc, char *argv[])
{
    auto args = getopt("onchain", argc, argv, {
        {'c', "chain", "TECH.CHAIN", "chain to use, defualt='bsv.main'", "bsv.main", false},
    });

    for (auto & chainname : args["chain"]) {
        Chain * chain = Chain::chains().at(chainname);
        chain->connect();

        std::cout << "Chain: " << chain->name << std::endl;
        std::cout << "root: " << chain->root() << std::endl;
    }

    std::mutex mtx;
    std::unique_lock<std::mutex> lk1(mtx), deadlk(mtx); 
    
    return 0;
}
