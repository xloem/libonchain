#pragma once

#include <libonchain/data.hpp>
#include <mutex>
#include <thread>

namespace SQLite { class Database; class Statement; }

namespace libonchain {

class DataSqlite : public Data
{
public:
    std::string const filename;
    std::string const table;

    DataSqlite(std::string const & filename = "libonchain.sqlite", std::string const & table = "libonchain", std::string const & key = "key", std::vector<std::string> const & values = {"value"});
    ~DataSqlite();

    virtual std::string add(std::vector<std::vector<uint8_t>> const & values) override;
    virtual std::vector<std::vector<uint8_t>> get(std::string const & key) override;
    virtual void drop(std::string const & key) override;

protected:
    virtual virtual_iterator<char const *>::impl * new_begin() override;
    virtual virtual_iterator<char const *>::impl * new_end() override;

    template <typename... Binds> int exec(std::string const & query, Binds const &... binds);
    template <typename... Binds> int exec(SQLite::Statement & query, Binds const &... binds);

    class Stmt;

    std::unique_ptr<SQLite::Database> sqlite_db;
    std::unique_ptr<Stmt> add_stmt, get_stmt, drop_stmt, iter_stmt, end_stmt;
};

};
