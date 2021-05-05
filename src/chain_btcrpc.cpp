// bitcoin
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#define main bitcoin_cli_main
#include "../deps/bitcoin/src/bitcoin-cli.cpp"
#pragma GCC diagnostic pop

#include <libonchain/chain_btcrpc.hpp>

#include <stdexcept>

namespace libonchain {

ArgsManager * ChainBtcrpc::withargs(ArgsManager * args, std::vector<std::string> const & params)
{
	SetupCliArgs(*args);
	std::vector<char const *> cparams = {"libonchain"};
	for (auto const & param : params) {
		cparams.push_back(param.c_str());
	}
	std::string error;
	if (!args->ParseParameters(cparams.size(), cparams.data(), error))
	{
		throw std::invalid_argument("Error parsing parameters: " + error);
	}
	if (!args->ReadConfigFiles(error, true)) {
		throw std::invalid_argument("Error reading config file: " + error);
	}
	args->SelectConfigNetwork(args->GetChainName());
	return args;
}

class BitcoinSystem {
public:
	BitcoinSystem()
	{
		SetupEnvironment();
		if (!SetupNetworking()) {
			throw std::runtime_error("Initializing bitcoin networking failed.");
		}
		event_set_log_callback(&libevent_log_cb);
	}
	static std::vector<libonchain::Chain::Flag> net2flags(std::string const & net)
	{
		std::vector<libonchain::Chain::Flag> flags;
		if (net != CBaseChainParams::MAIN) {
			flags.push_back(libonchain::Chain::Flag::CHAIN_TEST);
			if (net != CBaseChainParams::TESTNET) {
				flags.push_back(libonchain::Chain::Flag::CHAIN_PRIVATE);
			}
		}
		return flags;
	}
} static bitcoin_system;

ChainBtcrpc::ChainBtcrpc(std::string const & technology, ArgsManager * args, bool deleteargs)
: Chain(
	technology,
	args->GetChainName(),
	BitcoinSystem::net2flags(args->GetChainName())
  ),
  args(args),
  deleteargs(deleteargs)
{
	std::unique_ptr<CBaseChainParams> chainBaseParams = CreateBaseChainParams(chain);
	std::unique_ptr<BaseRequestHandler> rh;
	rh.reset(new DefaultRequestHandler());
	std::string method = "somemethod";
	const UniValue reply = ConnectAndCallRPC(rh.get(), method, {/*params vector*/}, {/*std::optional wallet_name*/});
	UniValue result = find_value(reply, "result");
	const UniValue & error = find_value(reply, "error");
	if (!error.isNull()) {
		int code;
		std::string descr;
		ParseError(error, descr, code);
		throw std::runtime_error("RPC Error " + std::to_string(code) + ": " + descr);
	}
	result.get_str(); // if result.isStr()
	result.write(2); // i guess this converts to string if not result.isStr()
}

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

