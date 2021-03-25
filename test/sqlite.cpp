#include <libonchain/data_sqlite.hpp>
#include <iostream>

#define tassert(...) if (!(__VA_ARGS__)) { std::cerr << "Assertion failed: " #__VA_ARGS__ << std::endl; exit(-1); }

// this could probably be expandable to other data backends too
// cmake likes it if each test is a different binary

// This test depends on specifics of the implementation.  If you can improve the implementation, it is ok to sacrifice this test.  Hopefully integration tests will exist to test everything at once.

using namespace libonchain;

int main()
{
    try {
    {
        DataSqlite sqlite("test.sqlite");
        tassert(sqlite.flags.count(Data::ARBITRARY_KEY));
        tassert(sqlite.flags.count(Data::FAST));
        tassert(!sqlite.flags.count(Data::IMMUTABLE));
        for (auto it = sqlite.begin(); it != sqlite.end(); ++ it) {
            auto & key = *it;
            sqlite.drop(key);
        }
    }
    {
        DataSqlite sqlite("test.sqlite");
        sqlite.add({"key-one", "value-one"});
        tassert(sqlite.get("key-one") == std::vector<std::string>({"key-one", "value-one"}));
        sqlite.add({"key-two", "value-two"});
    }
    {
        DataSqlite sqlite("test.sqlite");
        tassert((sqlite.get("key-one") == std::vector<std::string>{"key-one", "value-one"}));
        tassert(sqlite.get("key-two") == std::vector<std::string>{"key-two", "value-two"});
        try {
            sqlite.get("key-three");
            tassert(!"threw error on nonexistent key");
        } catch(...) { }
        std::vector<std::string> keys(sqlite.begin(), sqlite.end());
        tassert(keys == std::vector<std::string>{"key-one", "key-two"});
        for (auto key : sqlite) {
            sqlite.drop(key);
        }
    }
    {
        DataSqlite sqlite("test.sqlite");
        tassert(sqlite.begin() == sqlite.end());
        try {
            sqlite.get("key-one");
            tassert(!"threw error on nonexistent key");
        } catch(...) { }
    }
    } catch (std::exception const & e) {
        std::cerr << e.what() << std::endl;
        exit(-1);
    }
}
