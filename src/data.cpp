#include <libonchain/data.hpp>
#include <libonchain/data_sqlite.hpp>

#include <iostream> // if needed for debugging

namespace libonchain {

Data::Data(std::string const & technology, std::string const & address, std::string const & key, std::vector<std::string> const & values, std::vector<Flag> const & flags)
: name(technology + "." + address + "." + key + "." + join(values, ".")),
  technology(technology),
  address(address),
  key(key),
  values(values),
  flags(flags.begin(), flags.end())
{ }

}
