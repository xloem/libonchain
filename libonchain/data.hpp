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
        DATA_ARBITRARY_KEY,
	DATA_FAST,
	DATA_IMMUTABLE
    };

    // please unify similarity with chain.hpp .  maybe even chain would be a subclass of data, uncertain.  it could auto-connect on use if this doesn't connect.
    std::string const name;
    std::string const technology;
    std::string const address;
    std::string const key;
    std::vector<std::string> const values;
    std::unordered_set<Flag> const flags;

    // add more features/etc as use is discovered
    virtual std::string add(std::vector<std::vector<uint8_t>> const & values);
    virtual std::vector<std::vector<uint8_t>> get(std::string const & key);
    virtual void drop(std::string const & key);

    inline virtual_iterator<char const *> begin() { return new_begin(); }
    inline virtual_iterator<char const *> end() { return new_end(); }

protected:
    Data(std::string const & technology, std::string const & address, std::string const & key, std::vector<std::string> const & values, std::vector<Flag> const & flags);
    ~Data();

    virtual virtual_iterator<char const *>::impl * new_begin() = 0;
    virtual virtual_iterator<char const *>::impl * new_end() = 0;
    //virtual virtual_iterator<std::vector<std::vector<uint8_t>>>::impl * get(std::map<std::string> const & keys);
};

} // namespace libonchain
