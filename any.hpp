#pragma once

#include <array>
#include <memory>
#include <cstring>
#include <type_traits>
#include <typeinfo>
#include <cassert>

namespace detail { namespace any {

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
    return &any::operation<std::remove_cv_t<std::remove_reference_t<_T>>>;
}

}}

template <std::size_t _N>
struct any
{
    typedef std::size_t size_type;

    any() = default;

    ~any()
    {
        destroy();
    }

    template<typename _T>
    any(_T&& v)
    {
        copy_or_move(std::forward<_T>(v));
    }

    any(const any& another)
    {
        copy_from_another(another);
    }

    any(any& another)
    {
        copy_from_another(another);
    }

    any(any&& another)
    {
        copy_from_another(another);
    }

    template<std::size_t _M>
    any(const any<_M>& another)
    {
        copy_from_another(another);
    }

    template<std::size_t _M>
    any(any<_M>& another)
    {
        copy_from_another(another);
    }

    template<std::size_t _M>
    any(any<_M>&& another)
    {
        copy_from_another(another);
    }

    any& operator=(const any& another)
    {
        destroy();
        copy_from_another(another);
        return *this;
    }

    any& operator=(any& another)
    {
        destroy();
        copy_from_another(another);
        return *this;
    }

    any& operator=(any&& another)
    {
        destroy();
        copy_from_another(another);
        return *this;
    }

    template<std::size_t _M>
    any& operator=(const any<_M>& another)
    {
        destroy();
        copy_from_another(another);
        return *this;
    }

    template<std::size_t _M>
    any& operator=(any<_M>& another)
    {
        destroy();
        copy_from_another(another);
        return *this;
    }

    template<std::size_t _M>
    any& operator=(any<_M>&& another)
    {
        destroy();
        copy_from_another(another);
        return *this;
    }

    template <typename _T>
    any& operator=(const _T& t)
    {
        destroy();
        copy_or_move(std::forward<_T>(t));
        return *this;
    }

    template <typename _T>
    any& operator=(_T&& t)
    {
        destroy();
        copy_or_move(std::forward<_T>(t));
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
        return function_ == detail::any::get_function_for_type<_T>();
    }

    bool empty() const { return function_ == nullptr; }

    static constexpr size_type size() { return _N; }

    // Initializes with object of type T, created in-place with specified constructor params
    template<typename _T, typename... Args>
    void emplace(Args&&... args)
    {
        destroy();
        new(buff_.data()) _T(std::forward<Args>(args)...);
        function_ = detail::any::get_function_for_type<_T>();
    }

private:
    using operation_t = detail::any::operation_t;
    using function_ptr_t = detail::any::function_ptr_t;

    template <typename _T>
    void copy_or_move(_T&& t)
    {
        static_assert(size() >= sizeof(_T), "_T is too big to be copied to any");
        assert(function_ == nullptr);

        function_ = detail::any::get_function_for_type<_T>();

        using NonConstT = std::remove_cv_t<std::remove_reference_t<_T>>;
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

    template<std::size_t _M>
    void copy_from_another(const any<_M>& another)
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


    template<std::size_t _S>
    friend class any;
};
