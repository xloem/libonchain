#include <libonchain/data_sqlite.hpp>

#define assert(...) if (!(__VA_ARGS__)) { throw "Assertion failed: " #__VA_ARGS__; }

// this could probably be expandable to other data backends too
// cmake likes it if each test is a different binary

// This test depends on specifics of the implementation.  If you can improve the implementation, it is ok to sacrifice this test.  Hopefully integration tests will exist to test everything at once.

using namespace libonchain;

int main()
{
    {
        DataSqlite sqlite("test.sqlite");
        assert(sqlite.flags.count(Data::ARBITRARY_KEY));
        assert(sqlite.flags.count(Data::FAST));
        assert(!sqlite.flags.count(Data::IMMUTABLE));
        for (auto key : sqlite) {
            sqlite.drop(key);
        }
    }
    {
        DataSqlite sqlite("test.sqlite");
        sqlite.add({"key-one", "value-one"});
        assert(sqlite.get("key-one") == std::vector<std::string>({"key-one", "value-one"}));
        sqlite.add({"key-two", "value-two"});
    }
    {
        DataSqlite sqlite("test.sqlite");
        assert((sqlite.get("key-one") == std::vector<std::string>{"key-one", "value-one"}));
        assert(sqlite.get("key-two") == std::vector<std::string>{"key-two", "value-two"});
        try {
            sqlite.get("key-three");
            assert(!"threw error on nonexistent key");
        } catch(...) { }
        std::vector<std::string> keys(sqlite.begin(), sqlite.end());
        assert(keys == std::vector<std::string>{"key-one", "key-two"});
        for (auto key : sqlite) {
            sqlite.drop(key);
        }
    }
    {
        DataSqlite sqlite("test.sqlite");
        assert(sqlite.begin() == sqlite.end());
        try {
            sqlite.get("key-one");
            assert(!"threw error on nonexistent key");
        } catch(...) { }
    }
}
