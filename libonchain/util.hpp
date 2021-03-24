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
class virtual_iterator : public std::iterator<std::input_iterator_tag, T, int>
{
public:
    struct impl
    {
        virtual bool next(int n = 1) = 0;
        virtual T & deref() = 0;
        virtual bool equal(impl const & other) const { return false; }
        virtual impl * new_copy() = 0;
        virtual ~impl() = default;
    };

    virtual_iterator() : ptr(nullptr), own(false) { }
    virtual_iterator(impl *ptr, bool own) : ptr(ptr), own(own) { }
    virtual_iterator(virtual_iterator<T> const & other) : ptr(other.ptr ? other.ptr->new_copy(): nullptr), own(true) { }
    virtual_iterator(virtual_iterator<T> && other) : ptr(other.ptr), own(other.own) { other.ptr = nullptr;}
    ~virtual_iterator() { free(); }

    T & operator*()
    {
        return ptr->deref();
    }

    virtual_iterator & operator++()
    {
        if (!ptr->next(1)) {
            if (own) {
                delete ptr;
            }
            ptr = nullptr;
        }
        return *this;
    }

    bool operator==(virtual_iterator const & other) const
    {
        if (ptr == nullptr) {
            return other.ptr == nullptr;
        } else if (other.ptr == nullptr) {
            return false;
        }
        return ptr->equal(*other.ptr);
    }

    bool operator!=(virtual_iterator const & other) const
    {
        return !(*this == other);
    }

    virtual_iterator & operator=(virtual_iterator const & other)
    {
        free();
        own = other.own;
        if (own) {
            ptr = other.ptr->new_copy();
        } else {
            ptr = other.ptr;
        }
        return *this;
    }

    virtual_iterator & operator=(virtual_iterator && other)
    {
        free();
        own = other.own;
        ptr = other.ptr;
        other.own = false;
        other.ptr = nullptr;
        return *this;
    }

private:
    void free()
    {
        if (ptr && own) {
            delete ptr;
        }
        ptr = nullptr;
        own = false;
    }
    impl * ptr;
    bool own;
};

template <typename T>
using virtual_iterator_const = virtual_iterator<T const>;

} // namespace libonchain
