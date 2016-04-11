#pragma once

#include <array>
#include <memory>
#include <cstring>
#include <type_traits>
#include <typeinfo>

namespace detail
{

template <typename _DeleterT,
          typename _AnyT>
struct deleter
{
    ~deleter() { _DeleterT()(reinterpret_cast<_AnyT&>(*this)); }
};

template <typename _AnyT>
struct deleter<void, _AnyT> {};

}

template <std::size_t _N, typename _DeleterT = void>
struct any : public detail::deleter<_DeleterT, any<_N>>
{
    typedef std::size_t size_type;

    template <typename _T,
              typename _D = _DeleterT>
    typename std::enable_if<std::is_same<_D, void>::value, any&>::type
    operator=(_T&& t)
    {
        static_assert(std::is_trivially_destructible<_T>::value, "_T is not trivially destructible and does not use any deleter");
        copy_object(std::move(t));
        return *this;
    }

    template <typename _T,
              typename _D = _DeleterT>
    typename std::enable_if<!std::is_same<_D, void>::value, any&>::type
    operator=(_T&& t)
    {
        copy_object(std::move(t));
        return *this;
    }

    template <typename _T>
    void copy_object(_T&& t)
    {
        static_assert(std::is_trivially_copyable<_T>::value, "_T is not trivially copyable");
        static_assert(size() >= sizeof(_T), "_T is too big to be copied to any");

        std::memcpy(buff_.data(), (char*)&t, sizeof(_T));
    }

    template <typename _T>
    typename std::enable_if<!std::is_pointer<_T>::value, _T&>::type
    get() { return reinterpret_cast<_T&>(*buff_.data()); }

    template <typename _T>
    typename std::enable_if<!std::is_pointer<_T>::value, const _T&>::type
    get() const { return reinterpret_cast<const _T>(*buff_.data()); }

    template <typename _T>
    typename std::enable_if<std::is_pointer<_T>::value, _T>::type
    get() { return reinterpret_cast<_T>(buff_.data()); }

    template <typename _T>
    typename std::enable_if<std::is_pointer<_T>::value, const _T>::type
    get() const { return reinterpret_cast<const _T>(buff_.data()); }

    static constexpr size_type size() { return _N; }

private:
    std::array<char, _N> buff_;
};

template <std::size_t _N, typename _DeleterT = void>
struct any_p : public detail::deleter<_DeleterT, any_p<_N>>
{
    typedef std::size_t size_type;

    template <typename _T,
              typename _D = _DeleterT>
    typename std::enable_if<std::is_same<_D, void>::value, any_p&>::type
    operator=(_T&& t)
    {
        static_assert(std::is_trivially_destructible<_T>::value, "_T is not trivially destructible and does not use any_p deleter");
        copy_object(std::move(t));
        return *this;
    }

    template <typename _T,
              typename _D = _DeleterT>
    typename std::enable_if<!std::is_same<_D, void>::value, any_p&>::type
    operator=(_T&& t)
    {
        copy_object(std::move(t));
        return *this;
    }

    template <typename _T>
    void copy_object(_T&& t)
    {
        static_assert(std::is_trivially_copyable<_T>::value, "_T is not trivially copyable");
        static_assert(size() >= sizeof(_T), "_T is too big to be copied to any_p");

        std::memcpy(buff_.data(), (char*)&t, sizeof(_T));
        type_ = typeid(_T).hash_code();
    }

    template <typename _T>
    typename std::enable_if<!std::is_pointer<_T>::value, _T&>::type
    get()
    {
        is_stored_type<_T>();
        return reinterpret_cast<_T&>(*buff_.data());
    }

    template <typename _T>
    typename std::enable_if<!std::is_pointer<_T>::value, const _T&>::type
    get() const
    {
        is_stored_type<_T>();
        return reinterpret_cast<const _T&>(*buff_.data());
    }

    template <typename _T>
    typename std::enable_if<std::is_pointer<_T>::value, _T>::type
    get()
    {
        is_stored_type<_T>();
        return reinterpret_cast<_T>(buff_.data());
    }

    template <typename _T>
    typename std::enable_if<std::is_pointer<_T>::value, const _T>::type
    get() const
    {
        is_stored_type<_T>();
        return reinterpret_cast<const _T>(buff_.data());
    }

    static constexpr size_type size() { return _N; }

private:
    template <typename _T>
    void is_stored_type()
    {
        if (type_ != typeid(_T).hash_code())
            throw std::bad_cast();
    }

    std::array<char, _N> buff_;
    std::size_t          type_;
};
