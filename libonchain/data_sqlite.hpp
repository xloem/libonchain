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
    std::string const columnKey;
    std::string const columnValue;

    DataSqlite(std::string const & filename = "libonchain.sqlite", std::string const & table = "libonchain", std::string const & column = "key", std::string const & column = "value");
    ~DataSqlite();

    void add(std::string const & key, std::string const & value) override;
    std::string get(std::string const & key) override;
    void drop(std::string const & key) override;

private:
    virtual std::unique_ptr<Data::iteratorImpl> begin_ptr() override;
    virtual std::unique_ptr<Data::iteratorImpl> end_ptr() override;

    template <typename... Binds> int exec(std::string const & query, Binds const &... binds);
    template <typename... Binds> int exec(SQLite::Statement & query, Binds const &... binds);

    class Stmt;

    std::unique_ptr<SQLite::Database> sqlite_db;
    std::unique_ptr<Stmt> add_stmt, get_stmt, drop_stmt, iter_stmt, end_stmt;
};

};
