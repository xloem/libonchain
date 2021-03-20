#include <libonchain/data_sqlite.hpp>

#include <SQLiteCpp/SQLiteCpp.h>

#include <mutex>

using namespace SQLite;

namespace libonchain {

class DataSqlite::Stmt : public Statement
{
public:
    template <typename... Binds>
    Stmt(Database db, std::string const & query, Binds... binds)
    : Statement(db, query)
    {
        index = _bind(0, binds...);
    }

    Stmt(Database db)
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
    size_t _bind(size_t index, std::vector<uint8_t> const & value, More... more)
    {
        Statement::bindNoCopy(index, value.data(), value.size());
        return _bind(index + 1, more...);
    }

    template <typename Value, typename... More>
    size_t _bind(size_t index, std::vector<Value> const & values, More... more)
    {
        for (auto & value : values) {
            index = bind(index, value);
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
    if (!sqlite_db.tableExists(table)) {
        exec("CREATE TABLE " + table + " (" + key + " TEXT PRIMARY KEY, " + concat_join(columnValues, " BLOB") + ")");
    }
    // TODO: add extra columns to existing tables? could default to null
    //       SQLITE_ENABLE_COLUMN_METADATA might help, haven't reviewed

    add_stmt = std::make_unique(Stmt(sqlite_db, "INSERT INTO " + table + " VALUES (?, ", replace_join(columnValues, "?")));
    get_stmt = std::make_unique(Stmt(sqlite_db, "SELECT " + join(columnValues) + " FROM " + table + " WHERE " + key + " == ?"));
    drop_stmt = std::make_unique(Stmt(sqlite_db, "DELETE FROM " + table + " WHERE " + key + " == ?"));
    end_stmt = std::make_unique(Stmt(sqlite_db, ""));
    end_stmt->executeStep();
}

DataSqlite::~DataSqlite()
{
}

std::string DataSqlite::add(std::vector<std::string> const & values)
{
    if (values.size() != this->values.size()) {
        throw std::runtime_error("wrong value count")
    }
    add_stmt->exec(values);
    return values[0];
}

std::string DataSqlite::get(std::string const & key)
{
    Stmt & stmt = *get_stmt;
    std::unique_lock<std::mutex> mtx(stmt.lk);

    if (!stmt.executeStep()) {
        throw std::runtime_error("not found");
    }
    return stmt.getColumn(0).getString();
}

void DataSqlite::drop(std::string const & key)
{
    stmt_drop->exec(key);
}

class SqliteIteratorImpl : public Data::iteratorImpl {
public:
    SqliteIteratorImpl(DataSqlite::Stmt && stmt, bool fill, size_t hash)
    : stmt(std::move(stmt)), hash(hash), count(0)
    {
        if (fill) {
            (*this) ++;
        }
    }
    void operator++() override
    {
        stmt.executeStep();
        count ++;
    }
    bool operator==(iteratorImpl const & uncast_other) const override
    {
        SQliteIteratorImpl const & other = dynamic_cast<SQLiteIteratorImpl const &>(uncast_other);
        if (empty()) { return other.empty(); }
        else if (other.empty()) { return false; }
        else { return hash == other.hash && count == other.count; }
    }
    bool empty() {
        return !stmt.hasRow() || stmt.isDone();
    }
    char const * const & operator*() const override
    {
        return stmt.getColumn(0).getText();
    }
    DataSqlite::Stmt stmt;
    size_t hash;
    size_t count;
};

virtual std::unique_ptr<Data::iteratorImpl> Data::begin_ptr()
{
    return std::make_unique<SqliteIteratorImpl>(Stmt(sqlite_db, "SELECT ?, " + replace_join(columnValues, "?") + " FROM " + table), true, this);
}

virtual std::unique_ptr<Data::iteratorImpl> Data::end_ptr()
{
    return std::make_unique<SqliteIteratorImpl>(Stmt(sqlite_db, ""), true, this);
}

template <typename... Binds>
int DataSqlite::exec(std::string const & query, Binds const &... binds)
{
    return Stmt(sqlite_db, query, binds...).exec();
}

}
