#pragma once

namespace libonchain {

template <typename Type, typename Iterable, typename Func>
std::vector<Type> map(Iterable const & iterable, Func const & func)
{
    std::vector<Type> result;
    for (auto & item : iterable) {
        result.push_back(func(item));
    }
    return result;
}

template <typename Iterable, typename Func>
std::string map_join(Iterable const & iterable, Func const & func, std::string const & joiner = ", ")
{
    std::string result;
    bool first = true;
    for (auto & item : iterable) {
        if (first) {
            first = false;
        } else {
            result += joiner;
        }
        result += func(item);
    }
    return result;
}

template <typename Type>
std::vector<Type> concat(std::vector<Type> prefix, std::vector<Type> const & suffix)
{
    prefix.insert(prefix.end(), suffix.begin(), suffix.end());
    return prefix;
}

template <typename Iterable>
std::string concat_join(Iterable const & iterable, std::string const & suffix, std::string const & joiner = ", ")
{
    return map_join(iterable, [&suffix](std::string const & prefix) { return prefix + suffix; }, joiner);
}

template <typename Iterable>
std::string concat_join(std::string const & prefix, Iterable const & iterable, std::string const & joiner = ", ")
{
    return map_join(iterable, [&prefix](std::string const & suffix) { return prefix + suffix; }, joiner);
}

template <typename Iterable, typename Type>
std::string replace_join(Iterable const & iterable, Type value, std::string const & joiner = ", ")
{
    return map_join(iterable, [&value](std::string const &) { return value; }, joiner);
}

template <typename Iterable>
std::string join(Iterable const & iterable, std::string const & joiner = ", ")
{
    return map_join(iterable, [](std::string const & value) { return value; }, joiner);
}

struct byterange : public std::pair<char const *, size_t>
{
    using std::pair<char const *, size_t>::pair;
    template <typename Container> byterange(Container const & data) : pair(data.data(), data.size()) { }
    char const * data() { return first; }
    size_t size() { return second; }
};

template <typename T>
class virtual_deleter
{
public:
    ~virtual_deleter() = default;
    virtual void operator()(T *ptr) {
        static_assert(sizeof(T) > 0, "can't delete pointer to incomplete type");
        delete ptr;
    }
};

template <typename T>
class static_deleter : public virtual_deleter<T>
{
public:
    virtual void operator()(T *ptr) override { }
};

template <typename T>
using virtual_ptr = std::unique_ptr<T, virtual_deleter<T>>;

template <typename T>
class virtual_iterator : public std::iterator<std::input_iterator_tag, T, int>
{
public:
    struct impl
    {
        virtual void next(int n = 1) = 0;
        virtual T & deref() = 0;
        virtual bool equal(impl const & other) const = 0;
        virtual virtual_ptr<impl> copy() = 0;
        //virtual std::type_info & type() const = 0;
        //virtual ... address() const = 0
        virtual ~impl() = default;
    };

    virtual_iterator(impl *_impl, virtual_deleter<impl> deleter = {}) : _impl(_impl, deleter) { }
    virtual_iterator(virtual_ptr<impl> && _impl) : _impl(_impl) { }
    virtual_iterator(virtual_iterator<T> const & other) : _impl(other._impl->copy()) { }
    virtual_iterator(virtual_iterator<T> && other) : _impl(std::move(other._impl)) { }

    T & operator*()
    {
        return _impl->deref();
    }

    virtual_iterator & operator++()
    {
        _impl->next(1);
        return *this;
    }

    bool operator==(virtual_iterator const & other) const
    {
        return _impl->equal(*other._impl);
    }

    bool operator!=(virtual_iterator const & other) const
    {
        return !(*this == other);
    }

private:
    virtual_ptr<impl> _impl;
};

template <typename T>
using virtual_iterator_const = virtual_iterator<T const>;

} // namespace libonchain
