#include <libonchain/chain.hpp>
#include <libonchain/chain_libbtc.hpp>

#include <iostream> // debugging

namespace libonchain {

std::mutex Chain::_staticMtx;
std::unordered_map<std::string, Chain *> Chain::_chains;
std::unordered_map<std::string, Chain *> Chain::_connectedChains;

Chain::Chain(std::string const & technology, std::string const & chain, std::vector<Flag> const & flags)
: name(technology + "." + chain),
  technology(technology),
  chain(chain),
  flags(flags.begin(), flags.end()),
  _connectionState(DISCONNECTED)
{
    std::unique_lock<std::mutex> lk(_staticMtx);
    if (_chains.count(name)) {  
        throw std::runtime_error("Already have a chain named " + name);
    }
    _chains.insert({name, this});
}

Chain::~Chain()
{
    std::unique_lock<std::mutex> lk(_staticMtx);
    _chains.erase(name);
}

void Chain::on_connectionstate(ConnectionState state)
{
    if (state == _connectionState) {
        return;
    }

    std::cerr << name << ": ";
    switch(state) {
    case DISCONNECTED:
        std::cerr << "disconnected";
        break;
    case CONNECTING:
        std::cerr << "connecting";
        break;
    case CONNECTED:
        std::cerr << "connected";
        break;
    }
    std::cerr << std::endl;

    if (state == DISCONNECTED) {
        std::unique_lock<std::mutex> lk(_staticMtx);
        _connectedChains.erase(name);
    } else if (_connectionState == DISCONNECTED) {
        std::unique_lock<std::mutex> lk(_staticMtx);
        _connectedChains.insert({name, this});
    }
    _connectionState = state;
}

void Chain::on_block(std::string id)
{
    std::cerr << name << " block: " << id << std::endl;
}

void Chain::on_tx(std::string id)
{
    std::cerr << name << " tx: " << id << std::endl;
}

} // namespace libonchain

// same translation unit so that the linker doesn't optimise away
#include "chain_libbtc_bsv.ipp"
