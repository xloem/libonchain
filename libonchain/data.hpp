#pragma once

#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

#include "util.hpp"

namespace libonchain {

class Data
{
public:
    enum Flag {
        DATA_KEYVALUE,
    };

    // please unify similarity with chain.hpp .  maybe even chain would be a subclass of data, uncertain.  it could auto-connect on use if this doesn't connect.
    std::string const name;
    std::string const technology;
    std::string const address;
    std::vector<std::string> const keys;
    std::vector<std::string> const values;
    std::unordered_set<Flag> const flags;

    // add more features/etc as use is discovered
    virtual void add(std::string const & key, std::vector<uint8_t> const & value);
    virtual std::vector<uint8_t> get(std::string const & key);
    virtual void drop(std::string const & key);

    inline virtual_iterator<char const *> begin() { return begin_new(); }
    inline virtual_iterator<char const *> end() { return end_new(); }

protected:
    Data(std::string const & technology, std::string const & address, std::vector<std::string> const & keys, std::vector<std::string> const & values, std::vector<Flag> const & flags);
    ~Data();

    virtual virtual_iterator<char const *>::impl * begin_new() = 0;
    virtual virtual_iterator<char const *>::impl * end_new() = 0;
};

};
