#include "any.hpp"

#include <gtest/gtest.h>

struct A
{
    explicit A(int i) :
        m_i(i) {}
    int m_i;
};

TEST(basic, size)
{
    any<16> a;
    ASSERT_EQ(16, sizeof(a));
}

TEST(basic, trivial)
{
    any<16> a;
    ASSERT_TRUE(std::is_trivial<decltype(a)>::value);
}

TEST(basic, pod)
{
    any<16> a;
    ASSERT_TRUE(std::is_pod<decltype(a)>::value);
}

TEST(pod, assign_rvalue_int)
{
    any<16> a;
    a = 5;
    ASSERT_EQ(5, a.get<int>());
}

TEST(pod, assign_lvalue_int)
{
    any<16> a;
    int i = 6;
    a = i;
    ASSERT_EQ(6, a.get<int>());
}

TEST(pod, assign_rvalue_struct)
{
    any<16> a;
    a = A(0xdeadbeef);
    ASSERT_EQ(0xdeadbeef, a.get<A>().m_i);
}

TEST(deleter, size)
{
    struct del
    {
        void operator()(any<16>&) { }
    };

    any<16, del> s;
    ASSERT_EQ(16, sizeof(s));
}

TEST(deleter, called)
{
    static bool deleter_called = false;

    struct del
    {
        void operator()(any<16>&) { deleter_called = true; }
    };

    {
        any<16, del> s;
    }

    ASSERT_TRUE(deleter_called);
}

struct B
{
    explicit B(int i = 0) :
        m_i(new int)
    {
        *m_i = i;
    }

    int *m_i;
};

TEST(deleter, pointer_freed)
{
    struct del
    {
        void operator()(any<16>& a)
        {
            B& b = a.get<B>();
            delete b.m_i;
        }
    };

    any<16, del> a;
    a = B(1234);
    ASSERT_EQ(1234, *a.get<B>().m_i);
}

TEST(any_assignment, same_type)
{
    any<16> a;
    a = 1234;

    any<16> a2;
    a2 = 0xdeadbeef;
    a2 = a;

    ASSERT_EQ(1234,  a.get<int>());
    ASSERT_EQ(1234, a2.get<int>());
}

TEST(any_assignment, bigger_size)
{
    any<16> a;
    a = 1234;

    any<32> a2;
    a2 = 0xdeadbeef;
    a2 = a;

    ASSERT_EQ(1234,  a.get<int>());
    ASSERT_EQ(1234, a2.get<int>());
}
