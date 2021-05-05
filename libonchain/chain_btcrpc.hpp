#pragma once

#include <libonchain/chain.hpp>

// bitcoin
#ifndef HAVE_CONFIG_H
#define HAVE_CONFIG_H
#endif
#pragma GCC diagnostic ignored "-Wpedantic"
#include <util/system.h>

namespace libonchain {

class ChainBtcrpc : public Chain
{
public:
	static ArgsManager * withargs(ArgsManager * args, std::vector<std::string> const & params);

	ChainBtcrpc(std::string const & technology, ArgsManager * args, bool deleteargs);
	ChainBtcrpc(std::string const & technology, std::vector<std::string> args = {});
	~ChainBtcrpc();

	void connect(std::string datadir = ".", std::vector<std::string> seeds = {}) override;
	void disconnect() override;

	std::string root() override;
	std::vector<std::string> tips() override;
	Block block(std::string const & id) override;
	std::vector<std::string> txs(std::string const & block = "mempool") override;
	std::string txBroadcast(std::vector<uint8_t> const & tx) override;

private:
	ArgsManager * args;
	bool deleteargs;
};

}
