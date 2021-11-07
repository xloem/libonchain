#pragma once

#include <string>
#include <unordered_set>
#include <vector>

#include "util.hpp"

namespace libonchain {

class Data
{
public:
    enum Flag {
        ARBITRARY_KEY,
        FAST,
        IMMUTABLE // plan: remove this flag.  use the class only for data that is not reasonable to delete.
    };

    // maybe simplify this, unsure
    std::string const name;
    std::string const technology;
    std::string const address;
    std::string const key;
    std::vector<std::string> const values;
    std::unordered_set<Flag> const flags;

    virtual void connect() = 0;
    virtual void disconnect() = 0;

    // add more features/etc as use is discovered
    virtual std::string add(std::vector<std::string> const & values) = 0;
    virtual std::vector<std::string> get(std::string const & key) = 0;

    using iterator = virtual_iterator_const<std::string>;
    virtual iterator begin() = 0;
    virtual iterator end() { return {}; };

protected:
    Data(std::string const & technology, std::string const & address, std::string const & key, std::vector<std::string> const & values, std::vector<Flag> const & flags);
    virtual ~Data() = default;
};

} // namespace libonchain
