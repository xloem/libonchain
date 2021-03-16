#include <libonchain/IChain.hpp>

#include <iostream> // debugging

namespace libonchain {

std::mutex IChain::_staticMtx;
std::unordered_map<std::string, IChain *> IChain::_chains;
std::unordered_map<std::string, IChain *> IChain::_connectedChains;

IChain::IChain(std::string const & technology, std::string const & chain, std::vector<Flag> const & flags)
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

IChain::~IChain()
{
    std::unique_lock<std::mutex> lk(_staticMtx);
    _chains.erase(name);
}

void IChain::on_connectionstate(ConnectionState state)
{
    if (state == _connectionState) {
        return;
    }

    if (state == DISCONNECTED) {
        std::unique_lock<std::mutex> lk(_staticMtx);
        _connectedChains.erase(name);
    } else if (_connectionState == DISCONNECTED) {
        std::unique_lock<std::mutex> lk(_staticMtx);
        _connectedChains.insert({name, this});
    }
    _connectionState = state;
}

void IChain::on_block(std::string id)
{
    std::cerr << "Block: " << id << std::endl;
}

void IChain::on_tx(std::string id)
{
    std::cerr << "Tx: " << id << std::endl;
}

} // namespace libonchain
