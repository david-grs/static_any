#include "any.hpp"

#include <gtest/gtest.h>
#include <iostream>
struct A
{
    explicit A(int i) :
        m_i(i) {}
    int m_i;
};

TEST(basic, size)
{
    any<16> a;
    ASSERT_EQ(sizeof(a), 16);
}

TEST(pod, assign_rvalue_int)
{
    any<16> a;
    a = 5;
    ASSERT_EQ(a.get<int>(), 5);
}

TEST(pod, assign_lvalue_int)
{
    any<16> a;
    int i = 6;
    a = i;
    ASSERT_EQ(a.get<int>(), 5);
}

TEST(pod, assign_rvalue_struct)
{
    any<16> a;
    a = A(0xdeadbeef);
    ASSERT_EQ(a.get<A>().m_i, 0xdeadbeef);
}

TEST(deleter, size)
{
    struct del
    {
        void operator()() {}
    };

    any<16, del> s;
    std::cout << sizeof(s) << std::endl;
    assert(sizeof(s) == 16 + sizeof(void*));
}

