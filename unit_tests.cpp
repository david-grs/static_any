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
    ASSERT_EQ(a.get<A>().m_i, 0xdeadbeef);
}

TEST(deleter, size)
{
    static bool deleter_called = false;

    struct del
    {
        void operator()() { deleter_called = true; }
    };

    {
        any<16, del> s;
        ASSERT_EQ(16, sizeof(s));
    }

    ASSERT_TRUE(deleter_called);
}

