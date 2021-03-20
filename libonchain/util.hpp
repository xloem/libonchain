#pragma once

namespace libonchain {

template <typename value_type>
class virtual_iterator
{
public:
    virtual_iterator(struct impl * _impl) : _impl(_impl) { }
    virtual_iterator(virtual_iterator<value_type> const & other) : _impl(other->_impl->copy()) { }
    virtual_iterator(virtual_iterator<value_type> && other) : _impl(std::move(other->_impl)) { }

    value_type const & operator*() const
    {
        return _impl->deref_const();
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
    struct impl
    {
        virtual void next(int n = 1) = 0;
        virtual value_type const & deref_const() const = 0;
        virtual bool equal(impl const & other) const = 0;
        virtual std::unique_ptr<impl> copy() const = 0;
        //virtual std::type_info & type() const = 0;
        //virtual ... address() const = 0
        virtual ~impl() = default;
    };

    std::unique_ptr<impl> _impl;
};

} // namespace libonchain
