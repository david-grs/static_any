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

template <std::size_t _N>
struct any_p
{
    typedef std::size_t size_type;

    static constexpr size_type size() { return _N; }

    bool empty() const { return function_ == nullptr; }

    /// Creates empty object
    any_p() = default;

    /// Creates any_p containing copied/moved v
    template<typename _T, typename X = std::enable_if_t<!std::is_same<any_p, std::remove_reference_t<_T>>::value>>
    any_p(_T&& v)
    {
        copy_or_move(std::forward<_T>(v));
    }

    // Copy from another any of the same size
    any_p(const any_p& another)
    {
        copy_from_another(another);
    }

    any_p(any_p&& another)
    {
        copy_from_another(another);
    }

    // TODO bonus points for copying from any of the same _or smaller_ size
    // TODO: maybe size check should be performed at runtime?

    ~any_p()
    {
        destroy();
    }

    template <typename _T, typename X = std::enable_if_t<!std::is_same<any_p, std::remove_reference_t<_T>>::value>>
    any_p& operator=(_T&& t)
    {
        destroy();
        copy_or_move(std::forward<_T>(t));
        return *this;
    }

    any_p& operator=(const any_p& another)
    {
        destroy();
        copy_from_another(another);
        return *this;
    }

    template<typename _T>
    const _T& get() const
    {
        if (!is_stored_type<_T>())
            throw std::bad_cast();

        return *as<_T>();
    }

    template<typename _T>
    _T& get()
    {
        if (!is_stored_type<_T>())
            throw std::bad_cast();

        return *as<_T>();
    }

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
    static void operation(operation_t operation, void* this_void_ptr, void* other_void_ptr)
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
    }

    template<typename _T>
    static function_ptr_t get_function_for_type()
    {
        return &any_p::operation<_T>;
    }

    template <typename _T>
    void copy_or_move(_T&& t)
    {
        static_assert(size() >= sizeof(_T), "_T is too big to be copied to any_p");
        assert(!function_);
        using NonConstT = std::remove_cv_t<std::remove_reference_t<_T>>;
        function_ = get_function_for_type<NonConstT>();
        NonConstT* non_const_t = const_cast<NonConstT*>(&t);
        call_function<_T&&>(buff_.data(), non_const_t);
    }

    template <typename Ref>
    std::enable_if_t<std::is_rvalue_reference<Ref>::value>
    call_function(void* this_void_ptr, void* other_void_ptr)
    {
        function_(operation_t::move, this_void_ptr, other_void_ptr);
    }

    template <typename Ref>
    std::enable_if_t<!std::is_rvalue_reference<Ref>::value>
    call_function(void* this_void_ptr, void* other_void_ptr)
    {
        function_(operation_t::copy, this_void_ptr, other_void_ptr);
    }

    void destroy()
    {
        if (function_)
        {
            function_(operation_t::destroy, buff_.data(), nullptr);
            function_ = nullptr;
        }
    }

    template<typename _T>
    const _T* as() const
    {
        return reinterpret_cast<const _T*>(buff_.data());
    }

    template<typename _T>
    _T* as()
    {
        return reinterpret_cast<_T*>(buff_.data());
    }

    void copy_from_another(const any_p& another)
    {
        if (another.function_)
        {
            function_= another.function_;
            char* other_data = const_cast<char*>(another.buff_.data());
            function_(operation_t::copy, buff_.data(), other_data);
        }
    }

    std::array<char, _N> buff_;
    function_ptr_t function_ = nullptr;
};
