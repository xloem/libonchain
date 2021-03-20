#include <libonchain/data_sqlite.hpp>

#include <SQLiteCpp/SQLiteCpp.h>

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

    Stmt(Stmt const & other)
    : Statement(other), index(other.index)
    { }

    template <typename... Binds>
    int exec(Binds ... binds)
    {
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
        return _bind(index, binds...);
    }

    size_t _bind(size_t index)
    {
        return index;
    }

    size_t index;
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

DataSqlite::DataSqlite(std::string const & filename /*= "libonchain"*/, std::string const & table /*= "libonchain"*/, std::vector<std::string> const & columnKeys /*= {"key"}*/, std::vector<std::string> const & columnValues /*= {"value"}*/)
: Data("sqlite", filename + ":" + table, columnKeys, columnValues, {DATA_KEYVALUE}),
  filename(filename),
  table(table),
  sqlite_db(std::make_unique<Database>(filename, OPEN_READWRITE | OPEN_CREATE))
{
    if (!sqlite_db.tableExists(table)) {
        exec("CREATE TABLE " + table + " (" + concat_all(columnKeys, " TEXT PRIMARY KEY") + ", " + concat_all(columnValues, " BLOB") + ")");
    }
    // TODO: multikeys
    // TODO: add extra columns to existing tables? could default to null
    //       SQLITE_ENABLE_COLUMN_METADATA might help, haven't reviewed

    add_stmt = std::make_unique(Stmt(sqlite_db, "INSERT INTO " + table + " VALUES (" + replace_all(columnKeys, "?") + ", ", replace_all(columnValues, "?")));
    get_stmt = std::make_unique(Stmt(sqlite_db, "SELECT " + join(columnValues) + " FROM " + table + " WHERE " + concat_all(columnKeys, " == ?", " AND ")));
    drop_stmt = std::make_unique(Stmt(sqlite_db, "DELETE FROM " + table + " WHERE " + concat_all(columnKeys, " == ?", " AND ")));
    iter_stmt = std::make_unique(Stmt(sqlite_db, "SELECT " + replace_all(columnKeys, "?") + ", " + replace_all(columnValues, "?") + " FROM " + table));
    end_stmt = std::make_unique(Stmt(sqlite_db, ""));
}

DataSqlite::~DataSqlite()
{
}

void DataSqlite::add(std::string const & key, std::string const & value)
{
    exec(*add_stmt, key, value)
}

std::string DataSqlite::get(std::string const & key)
{
    auto stmt = *get_stmt;
    if (!stmt.executeStep()) {
        throw std::runtime_error("not found");
    }
    return stmt.getColumn(0).getString();
}

void DataSqlite::drop(std::string const & key)
{
    exec(*stmt_drop, key);
}

class SqliteIteratorImpl : public Data::iteratorImpl {
public:
    SqliteIteratorImpl(std::unique_ptr<DataSqlite::Stmt> const & stmt, bool fill, size_t hash)
    : stmt(*stmt), hash(hash), count(0)
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
    return std::make_unique<SqliteIteratorImpl>(iter_stmt, true, iter_stmt.get());
}

virtual std::unique_ptr<Data::iteratorImpl> Data::end_ptr()
{
    return std::make_unique<SqliteIteratorImpl>(end_stmt, false, iter_stmt.get());
}

template <typename... Binds>
int DataSqlite::exec(std::string const & query, Binds const &... binds)
{
    return Stmt(sqlite_db, query, binds...).exec();
}

}
