#include <libonchain/chain_libbtc.hpp>

#include <btc/chainparams.h>

static const btc_chainparams bsv_chainparams_main = {
    // strNetworkID
    "main",
    // base58Prefixes[PUBKEY_ADDRESS}
    0x00,
    // base58Prefixes[SCRIPT_ADDRESS]
    0x05,
    // bech32_hrp (bsv doesn't actually use this)
    "bs",
    // base58Prefixes[SECRET_KEY]
    128,
    // base58Prefixes[EXT_SECRET_KEY]
    0x0488ADE4,
    // base58Prefixes[EXT_PUBLIC_KEY]
    0x0488B21E,
    // pchMessageStart or netMagic
    {0xe3, 0xe1, 0xf3, 0xe8},
    // hashGenesisBlock (reversed)
    {0x6f, 0xe2, 0x8c, 0x0a, 0xb6, 0xf1, 0xb3, 0x72, 0xc1, 0xa6, 0xa2, 0x46, 0xae, 0x63, 0xf7, 0x4f, 0x93, 0x1e, 0x83, 0x65, 0xe1, 0x5a, 0x08, 0x9c, 0x68, 0xd6, 0x19, 0x00, 0x00, 0x00, 0x00, 0x00},
    // nDefaultPort
    8333,
    // vSeeds
    {{"seed.bitcoinsv.io"}, {"seed.cascharia.com"}, {"seed.satoshisvision.network"}, 0},
};

static const btc_chainparams bsv_chainparams_stn = {
    // strNetworkID
    "stn",
    // base58Prefixes[PUBKEY_ADDRESS}
    111,
    // base58Prefixes[SCRIPT_ADDRESS]
    196,
    // bech32_hrp (bsv doesn't actually use this)
    "st",
    // base58Prefixes[SECRET_KEY]
    239,
    // base58Prefixes[EXT_SECRET_KEY]
    0x04358394,
    // base58Prefixes[EXT_PUBLIC_KEY]
    0x043587CF,
    // pchMessageStart or netMagic
    {0xfb, 0xce, 0xc4, 0xf9},
    // hashGenesisBlock (reversed)
    {0x43, 0x49, 0x7f, 0xd7, 0xf8, 0x26, 0x95, 0x71, 0x08, 0xf4, 0xa3, 0x0f, 0xd9, 0xce, 0xc3, 0xae, 0xba, 0x79, 0x97, 0x20, 0x84, 0xe9, 0x0e, 0xad, 0x01, 0xea, 0x33, 0x09, 0x00, 0x00, 0x00, 0x00},
    // nDefaultPort
    9333,
    // vSeeds
    {{"stn-seed.bitcoinsv.io"}, 0},
};

static const btc_chainparams bsv_chainparams_testnet = {
    // strNetworkID
    "test",
    // base58Prefixes[PUBKEY_ADDRESS}
    111,
    // base58Prefixes[SCRIPT_ADDRESS]
    196,
    // bech32_hrp (bsv doesn't actually use this)
    "ts",
    // base58Prefixes[SECRET_KEY]
    239,
    // base58Prefixes[EXT_SECRET_KEY]
    0x04358394,
    // base58Prefixes[EXT_PUBLIC_KEY]
    0x043587CF,
    // pchMessageStart or netMagic
    {0xf4, 0xe5, 0xf3, 0xf4},
    // hashGenesisBlock (reversed)
    {0x43, 0x49, 0x7f, 0xd7, 0xf8, 0x26, 0x95, 0x71, 0x08, 0xf4, 0xa3, 0x0f, 0xd9, 0xce, 0xc3, 0xae, 0xba, 0x79, 0x97, 0x20, 0x84, 0xe9, 0x0e, 0xad, 0x01, 0xea, 0x33, 0x09, 0x00, 0x00, 0x00, 0x00},
    // nDefaultPort
    18333,
    // vSeeds
    {{"testnet-seed.bitcoinsv.io"}, {"testnet-seed.cascharia.com"}, {"testnet-seed.bitcoincloud.net"}, 0},
};

static const btc_chainparams bsv_chainparams_regtest = {
    // strNetworkID
    "regtest",
    // base58Prefixes[PUBKEY_ADDRESS}
    111,
    // base58Prefixes[SCRIPT_ADDRESS]
    196,
    // bech32_hrp (bsv doesn't actually use this)
    "bsrt",
    // base58Prefixes[SECRET_KEY]
    239,
    // base58Prefixes[EXT_SECRET_KEY]
    0x04358394,
    // base58Prefixes[EXT_PUBLIC_KEY]
    0x043587CF,
    // pchMessageStart or netMagic
    {0xda, 0xb5, 0xbf, 0xfa},
    // hashGenesisBlock (reversed)
    {0x06, 0x22, 0x6e, 0x46, 0x11, 0x1a, 0x0b, 0x59, 0xca, 0xaf, 0x12, 0x60, 0x43, 0xeb, 0x5b, 0xbf, 0x28, 0xc3, 0x4f, 0x3a, 0x5e, 0x33, 0x2a, 0x1f, 0xc7, 0xb2, 0xb7, 0x3c, 0xf1, 0x88, 0x91, 0x0f},
    // nDefaultPort
    18444,
    // vSeeds
    {0},
};

libonchain::ChainLibbtc bsv_main("bsv", bsv_chainparams_main);
libonchain::ChainLibbtc bsv_stn("bsv", bsv_chainparams_stn);
libonchain::ChainLibbtc bsv_testnet("bsv", bsv_chainparams_testnet);
libonchain::ChainLibbtc bsv_regtest("bsv", bsv_chainparams_regtest);
