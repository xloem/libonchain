#include "getopt.hpp"

int main(int argc, char *argv[])
{
    auto args = getopt("onchain", argc, argv, {
        {'t', "technology", "tech", "chain technology to use such as 'bsv'"},
        {'c', "chain", "chain", "chain to use, default='main'"},
    });
    (void)args;
    return 0;
}
