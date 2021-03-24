#include <libonchain/data_sqlite.hpp>

#include <SQLiteCpp/SQLiteCpp.h>

#include <memory>
#include <mutex>

using namespace SQLite;

namespace libonchain {

class DataSqlite::Stmt : public Statement
{
public:
    template <typename... Binds>
    Stmt(Database & db, std::string const & query, Binds... binds)
    : Statement(db, query)
    {
	index = 1;
        index = bind(binds...);
    }

    Stmt(Database & db)
    : Statement(db, "")
    { }

    Stmt(Stmt && other)
    : Statement(std::move(other)), index(other.index)
    { }

    template <typename... Binds>
    int bind(Binds ... binds)
    {
        try {
            return _bind(index, binds...);
        } catch (std::exception const & e) {
            throw std::runtime_error(e.what() + std::string(" query:") + getQuery() + " bindidx:" + std::to_string(index) + " bindct>=" + std::to_string(sizeof...(binds)));
        }
    }

    template <typename... Binds>
    int exec_noresults_needslock(Binds... binds)
    {
        bind(binds...);
        try {
            return Statement::exec();
        } catch (std::exception const & e) {
            throw std::runtime_error(e.what() + std::string(" query:") + getExpandedSQL());
        }
    }

    template <typename... Binds>
    int exec_noresults_locks(Binds... binds)
    {
        std::unique_lock<std::mutex> lk(mtx);
        return exec_noresults_needslock(binds...);
    }

    template <typename... Binds>
    int exec_results_needslock(Binds ... binds)
    {
        bind(binds...);
        try {
            return Statement::executeStep();
        } catch (std::exception const & e) {
            throw std::runtime_error(e.what() + std::string(" query:") + getExpandedSQL());
        }
    }

    template <typename... Binds>
    int exec_results_locks(Binds ... binds)
    {
        std::unique_lock<std::mutex> lk(mtx); // deadlocking here from somewhere
        return exec_results_needslock(binds...);
    }

    /*
    template <typename Value, typename... More>
    size_t _bind(size_t index, Value const & value, More... more)
    {
        Statement::bindNoCopy(index, value);
        return _bind(index + 1, more...);
    }
    */

    template <typename... More>
    size_t _bind(size_t index, std::string const & value, More... more)
    {
        if (value.find('\0') == std::string::npos) {
            Statement::bindNoCopy(index, value);
            return _bind(index + 1, more...);
        } else {
            return _bind(index, byterange(value), more...);
        }
    }

    template <typename... More>
    size_t _bind(size_t index, void *empty, More... more)
    {
        if (empty != nullptr) { throw std::runtime_error("void * bind is not null"); }
        Statement::bind(index);
        return _bind(index + 1, more...);
    }

    template <typename... More>
    size_t _bind(size_t index, byterange data, More... more)
    {
        Statement::bindNoCopy(index, data.data(), data.size());
        return _bind(index + 1, more...);
    }

    template <typename Value, typename... More>
    size_t _bind(size_t index, std::vector<Value> const & values, More... more)
    {
        for (auto & value : values) {
            index = _bind(index, value);
        }
        return _bind(index, more...);
    }

    size_t _bind(size_t index)
    {
        return index;
    }

    size_t index;
    std::mutex mtx;
};

DataSqlite::DataSqlite(std::string const & filename /*= "libonchain"*/, std::string const & table /*= "libonchain"*/, std::string const & key /*= "key"*/, std::vector<std::string> const & values /*= {"value"}*/)
: Data("sqlite", filename + ":" + table, key, concat({key}, values), {ARBITRARY_KEY, FAST}),
  filename(filename),
  table(table),
  sqlite_db(nullptr),
  add_stmt(nullptr),
  get_stmt(nullptr),
  drop_stmt(nullptr)
{ }

void DataSqlite::connect()
{
    if (sqlite_db) { return; }

    std::vector<std::string> nonkey_values(++ this->values.begin(), this->values.end());

    sqlite_db = new Database(filename, OPEN_READWRITE | OPEN_CREATE);

    if (!sqlite_db->tableExists(table)) {
        Stmt(*sqlite_db, "CREATE TABLE " + table + " (" + key + " TEXT PRIMARY KEY, " + concat_join(nonkey_values, " BLOB") + ")").exec_noresults_needslock();
    }

    // TODO: add extra columns to existing tables? could default to null
    //       SQLITE_ENABLE_COLUMN_METADATA might help, haven't reviewed

    add_stmt = new Stmt(*sqlite_db, "INSERT INTO " + table + " VALUES (" + replace_join(values, "?") + ")");
    get_stmt = new Stmt(*sqlite_db, "SELECT " + join(nonkey_values) + " FROM " + table + " WHERE " + key + " == ?");
    drop_stmt = new Stmt(*sqlite_db, "DELETE FROM " + table + " WHERE " + key + " == ?");
}

void DataSqlite::disconnect()
{
    if (sqlite_db == nullptr) { return; }
    delete add_stmt; add_stmt = nullptr;
    delete get_stmt; get_stmt = nullptr;
    delete drop_stmt; drop_stmt = nullptr;
    delete sqlite_db; sqlite_db = nullptr;
}

DataSqlite::~DataSqlite()
{
    disconnect();
}

std::string DataSqlite::add(std::vector<std::string> const & values)
{
    if (values.size() != this->values.size()) {
        throw std::runtime_error("wrong value count");
    }
    connect();
    add_stmt->exec_noresults_locks(values);
    //add_stmt->exec_noresults(map<byterange>(values, [](std::string const & str) { return byterange(str.data(), str.size());}));
    return values[0];
}

std::vector<std::string> DataSqlite::get(std::string const & key)
{
    connect();
    Stmt & stmt = *get_stmt;
    std::unique_lock<std::mutex> lk(stmt.mtx);

    stmt.bind(key);
    if (!stmt.exec_results_needslock()) {
        throw std::runtime_error("not found: " + key);
    }
    std::vector<std::string> results = { key };
    while (results.size() < values.size()) {
        results.push_back(stmt.getColumn(results.size() - 1).getString());
    }
    return results;
}

void DataSqlite::drop(std::string const & key)
{
    connect();
    drop_stmt->exec_noresults_locks(key);
}

class DataSqlite::iterator_impl : public virtual_iterator_const<std::string>::impl {
public:
    template<typename... Binds>
    iterator_impl(size_t hash, Database & db, std::string query, Binds... binds)
    : db(&db),
      stmt(std::make_unique<Stmt>(db, query, binds...)),
      hash(hash),
      count(0)
    {
        if (!next(1)) {
            throw std::runtime_error("no results: " + stmt->getExpandedSQL());
        }
    }
    iterator_impl(DataSqlite::iterator_impl & other)
    : db(other.db),
      stmt(std::make_unique<Stmt>(*db, other.stmt->getExpandedSQL())),
      hash(other.hash),
      count(other.count)
    { }
    virtual bool next(int n) override
    {
        if (!n) { return true; }
        value.clear();
        do {
            if (!stmt->exec_results_needslock()) {
                count = -1;
                return false;
            }
            count ++;
        } while (-- n);
        return true;
    }
    virtual bool equal(impl const & uncast_other) const override
    {
        iterator_impl const & other = dynamic_cast<iterator_impl const &>(uncast_other);
        return hash == other.hash && count == other.count;
    }
    virtual std::string const & deref() override
    {
        if (value.size() == 0) {
            value = stmt->getColumn(0).getString();
        }
        return value;
    }
    virtual impl * new_copy() override
    {
        return new iterator_impl(*this);
    }
    Database * db;
    std::unique_ptr<Stmt> stmt;
    size_t hash;
    ssize_t count;
    std::string value;
};

DataSqlite::iterator DataSqlite::begin()
{
    connect();
    try {
        return {new iterator_impl(std::hash<DataSqlite*>()(this), *sqlite_db, "SELECT " + join(values) + " FROM " + table), true};
    } catch (std::runtime_error const &) {
        return end();
    }
}

}
