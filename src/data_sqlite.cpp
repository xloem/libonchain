#include <libonchain/data_sqlite.hpp>

#include <SQLiteCpp/SQLiteCpp.h>

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
        index = _bind(0, binds...);
    }

    Stmt(Database & db)
    : Statement(db, "")
    { }

    Stmt(Stmt && other)
    : Statement(std::move(other)), index(other.index)
    { }

    template <typename... Binds>
    int exec(Binds ... binds)
    {
	std::unique_lock<std::mutex> lk(mtx);
        _bind(index, binds...);
        return Statement::exec();
    }

    template <typename Value, typename... More>
    size_t _bind(size_t index, Value const & value, More... more)
    {
        Statement::bindNoCopy(index, value);
        return _bind(index + 1, more...);
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

/*
template <typename... Binds>
static std::pair<Statement, size_t> bind(Statement statement, size_t index, std::string & value, Binds... values)
{
    statement.bindNoCopy(index, value);
    return bind(statement, index + 1, values...);
};

template <typename Bind, typename... Binds>
static std::pair<Statement, size_t> bind(Statement statement, size_t index, std::vector<Bind> & value, Binds... values)
{
    for (size_t i = 0; i < value.size(); i ++) {
        auto result = bind(statement
    }
    statement.bindNoCopy(index, value);
    return bind<index + 1>(statement, values...);
};

static Statement bind(Statement statement, size_t index = 0)
{
    return statement;
}
*/

DataSqlite::DataSqlite(std::string const & filename /*= "libonchain"*/, std::string const & table /*= "libonchain"*/, std::string const & key /*= "key"*/, std::vector<std::string> const & values /*= {"value"}*/)
: Data("sqlite", filename + ":" + table, key, concat({key}, values), {DATA_ARBITRARY_KEY, DATA_FAST}),
  filename(filename),
  table(table),
  sqlite_db(std::make_unique<Database>(filename, OPEN_READWRITE | OPEN_CREATE))
{
    if (!sqlite_db->tableExists(table)) {
        Stmt(*sqlite_db, "CREATE TABLE " + table + " (" + key + " TEXT PRIMARY KEY, " + concat_join(values, " BLOB") + ")").exec();
    }
    // TODO: add extra columns to existing tables? could default to null
    //       SQLITE_ENABLE_COLUMN_METADATA might help, haven't reviewed

    add_stmt = std::make_unique<Stmt>(*sqlite_db, "INSERT INTO " + table + " VALUES (?, " + replace_join(values, "?") + ")");
    get_stmt = std::make_unique<Stmt>(*sqlite_db, "SELECT " + join(values) + " FROM " + table + " WHERE " + key + " == ?");
    drop_stmt = std::make_unique<Stmt>(*sqlite_db, "DELETE FROM " + table + " WHERE " + key + " == ?");
    //end_stmt = std::make_unique<Stmt>(*sqlite_db, "");
    //end_stmt->executeStep();
}

DataSqlite::~DataSqlite()
{
}

std::string DataSqlite::add(std::vector<std::string> const & values)
{
    if (values.size() != this->values.size()) {
        throw std::runtime_error("wrong value count");
    }
    add_stmt->exec(map<byterange>(values, [](std::string const & str) { return byterange(str.data(), str.size());}));
    return values[0];
}

std::vector<std::string> DataSqlite::get(std::string const & key)
{
    Stmt & stmt = *get_stmt;
    std::unique_lock<std::mutex> lk(stmt.mtx);

    if (!stmt.executeStep()) {
        throw std::runtime_error("not found");
    }
    std::vector<std::string> results;
    while (results.size() < values.size()) {
        results.push_back(stmt.getColumn(results.size()).getString());
    }
    return results;
}

void DataSqlite::drop(std::string const & key)
{
    drop_stmt->exec(key);
}

class DataSqlite::iterator_impl : public virtual_iterator<std::string>::impl {
public:
    template<typename... Binds>
    iterator_impl(bool fill, size_t hash, Database & db, std::string query, Binds... binds)
    : db(db),
      stmt(Stmt(db, query, binds...)),
      hash(hash),
      count(0)
    {
        if (fill) {
            next(1);
        }
    }
    iterator_impl(DataSqlite::iterator_impl & other)
    : db(other.db),
      stmt(Stmt(db, other.stmt.getExpandedSQL())),
      hash(other.hash),
      count(other.count)
    { }
    virtual void next(int n) override
    {
        if (!n) { return; }
        value.clear();
        do {
            stmt.executeStep();
            count ++;
        } while (-- n);
    }
    virtual bool equal(impl const & uncast_other) const override
    {
        iterator_impl const & other = dynamic_cast<iterator_impl const &>(uncast_other);
        if (empty()) { return other.empty(); }
        else if (other.empty()) { return false; }
        else { return hash == other.hash && count == other.count; }
    }
    bool empty() const {
        return !stmt.hasRow() || stmt.isDone();
    }
    virtual std::string const & deref() override
    {
        if (value.size() == 0) {
            value = stmt.getColumn(0).getString();
        }
        return value;
    }
    virtual std::unique_ptr<impl> copy() override
    {
        return std::unique_ptr<impl>(new iterator_impl(*this));
    }
    Database & db;
    DataSqlite::Stmt stmt;
    size_t hash;
    size_t count;
    std::string value;
};

virtual_iterator<std::string> DataSqlite::begin()
{
    return new iterator_impl(true, std::hash<DataSqlite*>()(this), *sqlite_db, "SELECT " + key + ", " + join(values) + " FROM " + table);
}

virtual_iterator<std::string> DataSqlite::end()
{
    return new iterator_impl(false, std::hash<DataSqlite*>()(this), *sqlite_db, "");
}

}
