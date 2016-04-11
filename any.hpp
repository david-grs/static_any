#pragma once

#include <array>
#include <memory>
#include <cstring>
#include <type_traits>
#include <typeinfo>
#include <cassert>

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

    static constexpr size_type size() { return _N; }

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
    typename std::enable_if<!std::is_pointer<_T>::value, _T&>::type
    get() { return reinterpret_cast<_T&>(*buff_.data()); }

    template <typename _T>
    typename std::enable_if<!std::is_pointer<_T>::value, const _T&>::type
    get() const { return reinterpret_cast<const _T&>(*buff_.data()); }

    template <typename _T>
    typename std::enable_if<std::is_pointer<_T>::value, _T>::type
    get() { return reinterpret_cast<_T>(buff_.data()); }

    template <typename _T>
    typename std::enable_if<std::is_pointer<_T>::value, const _T>::type
    get() const { return reinterpret_cast<const _T>(buff_.data()); }

private:
    template <typename _T>
    void copy_object(_T&& t)
    {
        static_assert(std::is_trivially_copyable<_T>::value, "_T is not trivially copyable");
        static_assert(size() >= sizeof(_T), "_T is too big to be copied to any");

        std::memcpy(buff_.data(), (char*)&t, sizeof(_T));
    }

    std::array<char, _N> buff_;
};

template<typename T> class TP;

template <std::size_t _N>
struct any_p
{
    typedef std::size_t size_type;

    static constexpr size_type size() { return _N; }

    bool empty() const { return function_ == nullptr; }

    /// Creates empty object
    any_p() = default;

    /// Creates any_p containing a copy v
    template<typename _T>
    any_p(const _T& v)
    {
        copy_object(v);
    }

    /// Creates any_p containing moved v
    template<typename _T>
    any_p(const _T&& v)
    {
        move_object(v);
    }

    ~any_p()
    {
        destroy();
    }

    template <typename _T>
    any_p& operator=(const _T& t)
    {
        destroy();
        copy_object(t);
        return *this;
    }

    template <typename _T>
    any_p& operator=(_T&& t)
    {
        destroy();
        move_object(t);
        return *this;
    }

//    template <typename _T>
//    typename std::enable_if<!std::is_pointer<_T>::value, _T&>::type
//    get()
//    {
//        is_stored_type<_T>();
//        return reinterpret_cast<_T&>(*buff_.data());
//    }

//    template <typename _T>
//    typename std::enable_if<!std::is_pointer<_T>::value, const _T&>::type
//    get() const
//    {
//        is_stored_type<_T>();
//        return reinterpret_cast<const _T&>(*buff_.data());
//    }

//    template <typename _T>
//    typename std::enable_if<std::is_pointer<_T>::value, _T>::type
//    get()
//    {
//        is_stored_type<_T>();
//        return reinterpret_cast<_T>(buff_.data());
//    }

//    template <typename _T>
//    typename std::enable_if<std::is_pointer<_T>::value, const _T>::type
//    get() const
//    {
//        is_stored_type<_T>();
//        return reinterpret_cast<const _T>(buff_.data());
//    }

    template <typename _T>
    bool is_stored_type() const
    {
        using NonConstT = std::remove_cv_t<_T>;
        return function_ == get_function_for_type<NonConstT>();
    }

private:

    // Pointer to administrative function, function that will by type-specific, and will be able to perform all the required operations
    enum class operation_t { copy, move, destroy };
    using function_ptr_t = void(*)(operation_t operation, void* this_ptr, void* other_ptr);

    template<typename _T>
    static function_ptr_t get_function_for_type()
    {
        return [](operation_t operation, void* this_void_ptr, void* other_void_ptr)
        {
            _T* this_ptr = reinterpret_cast<_T*>(this_void_ptr);
            _T* other_ptr = reinterpret_cast<_T*>(other_void_ptr);

            switch(operation)
            {
                case operation_t::copy:
                    assert(this_ptr);
                    assert(other_ptr);
                    new(this_ptr)_T(*other_ptr);
                    break;
                case operation_t::move:
                    assert(this_ptr);
                    assert(other_ptr);
                    new(this_ptr)_T(std::move(*other_ptr));
                    break;
                case operation_t::destroy:
                    assert(this_ptr);
                    this_ptr->~_T();
                    break;
            }
        };
    }

    template <typename _T>
    void copy_object(const _T& t)
    {
        static_assert(size() >= sizeof(_T), "_T is too big to be copied to any_p");
        assert(!function_);
        using NonConstT = std::remove_cv_t<_T>;
        function_ = get_function_for_type<NonConstT>();
        _T* non_const_t = const_cast<_T*>(&t);
        function_(operation_t::copy, buff_.data(), non_const_t);
    }

    template <typename _T>
    void move_object(_T& t)
    {
        static_assert(size() >= sizeof(_T), "_T is too big to be copied to any_p");
        assert(!function_);
        using NonConstT = std::remove_cv_t<_T>;
        function_ = get_function_for_type<NonConstT>();
        //TP<_T> x;
        NonConstT* non_const_t = const_cast<NonConstT*>(&t);
        function_(operation_t::move, buff_.data(), &non_const_t);
    }


    void destroy()
    {
        if (function_)
        {
            function_(operation_t::destroy, buff_.data(), nullptr);
            function_ = nullptr;
        }
    }

    std::array<char, _N> buff_;
    function_ptr_t function_ = nullptr;
};
