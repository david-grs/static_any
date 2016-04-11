#include "any.hpp"

#include <gtest/gtest.h>

struct A
{
    explicit A(int i) :
        m_i(i) {}
    int m_i;
};

TEST(any_basic, readme_example)
{
    any<16> a;
    static_assert(sizeof(a) == 16, "impossible");

    a = 1234;
    ASSERT_EQ(1234, a.get<int>());

    a = "foobar";
    ASSERT_EQ(std::string("foobar"), a.get<const char*>());

    struct A
    {
        explicit A(long i = 0, double d = .0)
         : i_(i), d_(d) {}

        long i_;
        double d_;
    };

    a = A(12, .34);
}

TEST(any_basic, size)
{
    any<16> a;
    ASSERT_EQ(16, sizeof(a));
}

TEST(any_basic, trivial)
{
    any<16> a;
    ASSERT_TRUE(std::is_trivial<decltype(a)>::value);
}

TEST(any_basic, pod)
{
    any<16> a;
    ASSERT_TRUE(std::is_pod<decltype(a)>::value);
}

TEST(any_pod, assign_rvalue_int)
{
    any<16> a;
    a = 5;
    ASSERT_EQ(5, a.get<int>());
}

TEST(any_pod, assign_lvalue_int)
{
    any<16> a;
    int i = 6;
    a = i;
    ASSERT_EQ(6, a.get<int>());
}

TEST(any_pod, assign_rvalue_struct)
{
    any<16> a;
    a = A(0xdeadbeef);
    ASSERT_EQ(0xdeadbeef, a.get<A>().m_i);
}

TEST(any_deleter, size)
{
    struct del
    {
        void operator()(any<16>&) { }
    };

    any<16, del> s;
    ASSERT_EQ(16, sizeof(s));
}

TEST(any_deleter, called)
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

TEST(any_deleter, pointer_freed)
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
    ASSERT_EQ(1234, a2.get<unsigned int>());
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

/*
TEST(exception, wrong_type)
{
    any<16> a;
    a = 1234;

    try {
        a.get<double>();
        FAIL();
    }
    catch(std::runtime_error&) { }
}
*/

struct CallCounter
{
    CallCounter() { default_constructions++; }
    CallCounter(const CallCounter&) { copy_constructions++; }
    CallCounter(CallCounter&&) { move_constructions++; }
    ~CallCounter() { destructions++; }

    static void reset_counters()
    {
        default_constructions = 0;
        copy_constructions = 0;
        move_constructions = 0;
        destructions = 0;
    }

    static int default_constructions;
    static int copy_constructions;
    static int move_constructions;
    static int destructions;
};

int CallCounter::default_constructions = 0;
int CallCounter::copy_constructions = 0;
int CallCounter::move_constructions = 0;
int CallCounter::destructions = 0;

TEST(any_p, default_constructed_is_empty)
{
    any_p<16> a;
    ASSERT_TRUE(a.empty());
}

TEST(any_p, contructed_with_param_non_empty)
{
    any_p<16> a(77); // will contain integer
    ASSERT_FALSE(a.empty());
}

TEST(any_p, is_stored_type)
{
    any_p<16> a(77); // will contain integer
    ASSERT_TRUE(a.is_stored_type<int>());
    ASSERT_FALSE(a.is_stored_type<double>());
}

TEST(any_p, move_construct)
{
    CallCounter::reset_counters();
    CallCounter counter;
    any_p<16> a(std::move(counter));

    ASSERT_EQ(1, CallCounter::default_constructions);
    ASSERT_EQ(0, CallCounter::copy_constructions);
    ASSERT_EQ(1, CallCounter::move_constructions);
    ASSERT_EQ(0, CallCounter::destructions);
}

TEST(any_p, copy_construct)
{
    CallCounter::reset_counters();
    CallCounter counter;
    any_p<16> a(counter);

    ASSERT_EQ(1, CallCounter::default_constructions);
    ASSERT_EQ(1, CallCounter::copy_constructions);
    ASSERT_EQ(0, CallCounter::move_constructions);
    ASSERT_EQ(0, CallCounter::destructions);
}

TEST(any_p, destruction)
{
    CallCounter::reset_counters();
    CallCounter counter;
    {
        any_p<16> a(counter);
    }

    ASSERT_EQ(1, CallCounter::destructions);
}

TEST(any_p, copy_assignment)
{
    CallCounter::reset_counters();
    CallCounter counter;

    any_p<16> a;
    a = counter;

    ASSERT_EQ(1, CallCounter::default_constructions);
    ASSERT_EQ(1, CallCounter::copy_constructions);
    ASSERT_EQ(0, CallCounter::move_constructions);
}

TEST(any_p, move_assignment)
{
    CallCounter::reset_counters();
    CallCounter counter;

    any_p<16> a;
    a = std::move(counter);

    ASSERT_EQ(1, CallCounter::default_constructions);
    ASSERT_EQ(0, CallCounter::copy_constructions);
    ASSERT_EQ(1, CallCounter::move_constructions);
}

TEST(any_p, reassignment)
{
    CallCounter::reset_counters();
    CallCounter counter;

    any_p<16> a(counter);
    a = counter;

    ASSERT_EQ(1, CallCounter::default_constructions);
    ASSERT_EQ(2, CallCounter::copy_constructions);
    ASSERT_EQ(0, CallCounter::move_constructions);
    ASSERT_EQ(1, CallCounter::destructions);
}

TEST(any_p, not_empty_after_assignment)
{
    any_p<16> a;
    ASSERT_TRUE(a.empty());
    a = 7;
    ASSERT_FALSE(a.empty());
}

TEST(any_p, different_type_after_assignment)
{
    any_p<16> a(7);
    ASSERT_TRUE(a.is_stored_type<int>());
    ASSERT_FALSE(a.is_stored_type<double>());
    a = 3.14;
    ASSERT_FALSE(a.is_stored_type<int>());
    ASSERT_TRUE(a.is_stored_type<double>());
}

TEST(any_p, get_good_type)
{
    any_p<16> a(7);
    auto i = a.get<int>();
    ASSERT_EQ(7, i);
}

TEST(any_p, get_bad_type)
{
    any_p<16> a(7);
    EXPECT_THROW(a.get<double>(), std::bad_cast);
}

TEST(any_p, mutable_get)
{
    any_p<16> a(7);
    a.get<int>() = 6;
    const any_p<16>& const_ref = a;
    auto i = const_ref.get<int>();
    ASSERT_EQ(6, i);
}

TEST(any_p, any_to_any_copy_uninitialized)
{
    any_p<16> a;
    any_p<16> b(a);

    ASSERT_TRUE(a.empty());
    ASSERT_TRUE(b.empty());
}

TEST(any_p, any_to_any_copy_construction)
{
    any_p<16> a(7);
    any_p<16> b(a);

    ASSERT_EQ(7, a.get<int>());
    ASSERT_EQ(7, b.get<int>());
}

TEST(any_p, any_to_any_assignment)
{
    any_p<32> a(std::string("Hello"));
    any_p<32> b;

    ASSERT_TRUE(b.empty());
    b = a;
    ASSERT_FALSE(b.empty());

    ASSERT_EQ("Hello", b.get<std::string>());
    ASSERT_EQ("Hello", a.get<std::string>());
}
