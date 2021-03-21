#pragma once

#include <libonchain/data.hpp>
#include <mutex>
#include <thread>

namespace SQLite { class Database; }

namespace libonchain {

class DataSqlite : public Data
{
public:
    std::string const filename;
    std::string const table;

    DataSqlite(std::string const & filename = "libonchain.sqlite", std::string const & table = "libonchain", std::string const & key = "key", std::vector<std::string> const & values = {"value"});
    ~DataSqlite();

    virtual void connect() override;
    virtual void disconnect() override;

    virtual std::string add(std::vector<std::string> const & values) override;
    virtual std::vector<std::string> get(std::string const & key) override;
    virtual void drop(std::string const & key) override;

    virtual virtual_iterator_const<std::string> begin() override;
    virtual virtual_iterator_const<std::string> end() override;

protected:
    class Stmt;
    class iterator_impl;

    std::unique_ptr<SQLite::Database> sqlite_db;
    std::unique_ptr<Stmt> add_stmt, get_stmt, drop_stmt;//, iter_stmt, end_stmt;
};

}
