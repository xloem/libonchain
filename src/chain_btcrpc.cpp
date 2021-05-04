#include <libonchain/chain_btcrpc.hpp>

#include <stdexcept>

// bitcoin
#include <chainparams.h>

class ArgsInspector : public ArgsManager
{
public:
	static ArgsInspector const & cast(ArgsManager const * args)
	{
		return *static_cast<ArgsInspector const *>(args);
	}
	static std::string const & network(ArgsManager *args)
	{
		return cast(args).m_network;
	}
	static std::vector<libonchain::Chain::Flag> flags(ArgsManager *args)
	{
		std::vector<libonchain::Chain::Flag> flags;
		std::string const & net = network(args);
		if (net != CBaseChainParams::MAIN) {
			flags.push_back(libonchain::Chain::Flag::CHAIN_TEST);
			if (net != CBaseChainParams::TESTNET) {
				flags.push_back(libonchain::Chain::Flag::CHAIN_PRIVATE);
			}
		}
		return flags;
	}
};

namespace libonchain {

ArgsManager * ChainBtcrpc::withargs(ArgsManager * args, std::vector<std::string> const & params)
{
	std::vector<char const *> cparams = {"libonchain"};
	for (auto const & param : params) {
		cparams.push_back(param.c_str());
	}
	std::string error;
	if (!args->ParseParameters(cparams.size(), cparams.data(), error))
	{
		throw std::invalid_argument(error);
	}
	return args;
}

ChainBtcrpc::ChainBtcrpc(std::string const & technology, ArgsManager * args, bool deleteargs)
: Chain(
	technology,
	ArgsInspector::network(args),
	ArgsInspector::flags(args)
  ),
  args(args),
  deleteargs(deleteargs)
{ }

ChainBtcrpc::ChainBtcrpc(std::string const & technology, std::vector<std::string> args)
: ChainBtcrpc(technology, withargs(new ArgsManager(), args), true)
{ }

ChainBtcrpc::~ChainBtcrpc()
{
	delete args;
}

void ChainBtcrpc::connect(std::string datadir/* = "."*/, std::vector<std::string> seeds/* = {}*/)
{
}

void ChainBtcrpc::disconnect()
{
}

}
