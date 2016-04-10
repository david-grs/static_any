#pragma once

#include <type_traits>
#include <cstring>
#include <array>
#include <cassert>
#include <memory>

#if 0

template <std::size_t _N, typename _DeleterT = void>
struct small_object : private detail::deleter<_DeleterT>
{
    typedef std::size_t size_type;

    template <typename _T>
    typename std::enable_if<std::is_same<_T, void>, small_object&>::type
    operator=(_T&& t)
    {
        static_assert(std::is_trivially_copyable<_T>::value, "_T is not trivially copyable");
        static_assert(std::is_trivially_destructible<_T>::value, "_T is not trivially destructible and does not use any deleter");

        static_assert(size() >= sizeof(_T), "_T is too big to be copied to small_object");

        std::copy((char*)&t, (char*)&t + sizeof(_T), buff_.data());
        return *this;
    }
/*
    template <typename _T,
              typename std::enable_if<!std::is_same<_DeleterT, void>>::type* = nullptr>
    small_object& operator=(_T&& t)
    {
        static_assert(std::is_trivially_copyable<_T>::value, "_T is not trivially copyable");

        static_assert(size() >= sizeof(_T), "_T is too big to be copied to small_object");

        std::copy((char*)&t, (char*)&t + sizeof(_T), buff_.data());
        return *this;
    }
*/
    /*
    template <std::size_t _M>
    small_object& operator=(const small_object<_M>& t)
    {
        static_assert(size() >= sizeof(_M), "source small_object too big to be copied to the targeted small_object");

        std::copy((char*)&t, (char*)&t + sizeof(_M), buff_.data());
        return *this;
    }
    */

    template <typename _T>
    _T& get() { return reinterpret_cast<_T&>(*buff_.data()); }

    template <typename _T>
    const _T& get() const { return reinterpret_cast<_T&>(*buff_.data()); }

    static constexpr size_type size() { return _N; }

private:
    std::array<char, _N> buff_;
};

#endif



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
        std::copy((char*)&t, (char*)&t + sizeof(_T), buff_.data());
    }

    template <typename _T>
    _T& get() { return reinterpret_cast<_T&>(*buff_.data()); }

    template <typename _T>
    const _T& get() const { return reinterpret_cast<_T&>(*buff_.data()); }

    static constexpr size_type size() { return _N; }

private:
    std::array<char, _N> buff_;
};
