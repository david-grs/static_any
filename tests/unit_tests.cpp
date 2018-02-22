#include "../any.hpp"
#include "dyn_lib.hpp"

#include <gtest/gtest.h>

struct A
{
	explicit A(int i) :
		m_i(i) {}
	int m_i;
};

TEST(any, readme_example)
{
	static_any<32> a; // on g++ 5.x sizeof(std::string) is 32
	static_assert(sizeof(a) == 32 + sizeof(std::ptrdiff_t), "impossible");

	a = 1234;
	ASSERT_EQ(1234, a.get<int>());

	a = std::string("Hello world");
	ASSERT_EQ(std::string("Hello world"), a.get<std::string>());

	struct AA
	{
		explicit AA(long i = 0, double d = .0)
		 : i_(i), d_(d) {}

		long i_;
		double d_;
	};

	a = AA(12, .34);
}

TEST(any, test_sizeof)
{
	static_any<16> a;
	ASSERT_EQ(16 + sizeof(std::ptrdiff_t), sizeof(a));
}

TEST(any, capacity)
{
	static_any<32> a;
	ASSERT_EQ(32, a.capacity());

	a = std::string("hello world");
	ASSERT_EQ(32, a.capacity());

	a.reset();
	ASSERT_EQ(32, a.capacity());
}

TEST(any, size)
{
	static_any<32> a;
	ASSERT_EQ(0, a.size());

	a = 1234;
	ASSERT_EQ(sizeof(1234), a.size());

	a = std::string("foobar");
	ASSERT_EQ(sizeof(std::string), a.size());

	a.reset();
	ASSERT_EQ(0, a.size());
}

TEST(any, default_constructed_is_empty)
{
	static_any<16> a;
	ASSERT_TRUE(a.empty());
}

TEST(any, contructed_with_param_non_empty)
{
	static_any<16> a(77); // will contain integer
	ASSERT_FALSE(a.empty());
}

TEST(any, has)
{
	static_any<16> a(77); // will contain integer
	ASSERT_TRUE(a.has<int>());
	ASSERT_FALSE(a.has<double>());
}

template <std::size_t Index>
class CallCounter
{
public:
	CallCounter() { ++constructions; }
	CallCounter(const CallCounter&) { ++copy_constructions; }
	CallCounter& operator=(const CallCounter&) { ++copy_constructions; return *this; }
	CallCounter(CallCounter&&) { ++move_constructions; }
	CallCounter& operator=(CallCounter&&) { ++move_constructions; return *this;  }
	~CallCounter() { ++destructions; }

	static void reset_counters()
	{
		constructions = 0;
		copy_constructions = 0;
		move_constructions = 0;
		destructions = 0;
	}

	static int constructions;
	static int copy_constructions;
	static int move_constructions;
	static int destructions;
};

template <std::size_t Index> int CallCounter<Index>::constructions = 0;
template <std::size_t Index> int CallCounter<Index>::copy_constructions = 0;
template <std::size_t Index> int CallCounter<Index>::move_constructions = 0;
template <std::size_t Index> int CallCounter<Index>::destructions = 0;

TEST(any, move_construct)
{
	CallCounter<0>::reset_counters();

	CallCounter<0> counter;
	static_any<16> a(std::move(counter));

	ASSERT_EQ(1, CallCounter<0>::constructions);
	ASSERT_EQ(0, CallCounter<0>::copy_constructions);
	ASSERT_EQ(1, CallCounter<0>::move_constructions);
	ASSERT_EQ(0, CallCounter<0>::destructions);
}

TEST(any, copy_construct)
{
	CallCounter<0>::reset_counters();
	CallCounter<0> counter;
	static_any<16> a(counter);

	ASSERT_EQ(1, CallCounter<0>::constructions);
	ASSERT_EQ(1, CallCounter<0>::copy_constructions);
	ASSERT_EQ(0, CallCounter<0>::move_constructions);
	ASSERT_EQ(0, CallCounter<0>::destructions);
}

TEST(any, destruction)
{
	CallCounter<0>::reset_counters();
	CallCounter<0> counter;
	{
		static_any<16> a(counter);
	}

	ASSERT_EQ(1, CallCounter<0>::destructions);
}

TEST(any, reset_destruction)
{
	CallCounter<0>::reset_counters();
	CallCounter<0> counter;

	static_any<16> a(counter);
	a.reset();

	ASSERT_EQ(1, CallCounter<0>::destructions);
}

TEST(any, copy_assignment)
{
	CallCounter<0>::reset_counters();
	CallCounter<0> counter;

	static_any<16> a(1);
	a = counter;

	ASSERT_EQ(1, CallCounter<0>::constructions);
	ASSERT_EQ(1, CallCounter<0>::copy_constructions);
	ASSERT_EQ(0, CallCounter<0>::move_constructions);
}

TEST(any, move_assignment)
{
	CallCounter<0>::reset_counters();
	CallCounter<0> counter;

	static_any<16> a;
	a = std::move(counter);

	ASSERT_EQ(1, CallCounter<0>::constructions);
	ASSERT_EQ(0, CallCounter<0>::copy_constructions);
	ASSERT_EQ(1, CallCounter<0>::move_constructions);
}

TEST(any, any_move_ctor)
{
	CallCounter<0> counter;
	static_any<16> a(counter);

	CallCounter<0>::reset_counters();

	static_any<16> b(std::move(a));

	ASSERT_EQ(0, CallCounter<0>::constructions);
	ASSERT_EQ(0, CallCounter<0>::copy_constructions);
	ASSERT_EQ(1, CallCounter<0>::move_constructions);
}

TEST(any, any_move_assignment)
{
	CallCounter<0> counter;
	static_any<16> a(counter);

	CallCounter<0>::reset_counters();

	static_any<16> b = 123;
	b = std::move(a);

	ASSERT_EQ(0, CallCounter<0>::constructions);
	ASSERT_EQ(0, CallCounter<0>::copy_constructions);
	ASSERT_EQ(1, CallCounter<0>::move_constructions);
}

TEST(any, reassignment)
{
	CallCounter<0> counter;

	static_any<16> a = 1234;
	ASSERT_EQ(1234, a.get<int>());

	CallCounter<0>::reset_counters();
	a = counter;

	ASSERT_EQ(0, CallCounter<0>::constructions);
	ASSERT_EQ(1, CallCounter<0>::copy_constructions);
	ASSERT_EQ(0, CallCounter<0>::move_constructions);
	ASSERT_EQ(0, CallCounter<0>::destructions);
}

TEST(any, not_empty_after_assignment)
{
	static_any<16> a;
	ASSERT_TRUE(a.empty());
	a = 7;
	ASSERT_FALSE(a.empty());
}

TEST(any, different_type_after_assignment)
{
	static_any<16> a(7);
	ASSERT_TRUE(a.has<int>());
	ASSERT_FALSE(a.has<double>());
	a = 3.14;
	ASSERT_FALSE(a.has<int>());
	ASSERT_TRUE(a.has<double>());
}

TEST(any, get_good_type)
{
	static_any<16> a(7);
	auto i = a.get<int>();
	ASSERT_EQ(7, i);
}

TEST(any, get_bad_type)
{
	static_any<16> a(7);
	EXPECT_THROW(a.get<double>(), std::bad_cast);
}

TEST(any, get_empty)
{
	static_any<16> a;
	EXPECT_THROW(a.get<double>(), std::bad_cast);
}

TEST(any, cast_empty)
{
	static_any<16> a;
	EXPECT_THROW(any_cast<int>(a), std::bad_cast);
}

TEST(any, mutable_get)
{
	static_any<16> a(7);
	a.get<int>() = 6;
	const static_any<16>& const_ref = a;
	auto i = const_ref.get<int>();
	ASSERT_EQ(6, i);
}

TEST(any, any_to_any_copy_uninitialized)
{
	static_any<16> a;
	static_any<16> b(a);

	ASSERT_TRUE(a.empty());
	ASSERT_TRUE(b.empty());
}

TEST(any, any_to_any_copy_construction)
{
	static_any<16> a(7);
	static_any<16> b(a);

	ASSERT_EQ(7, a.get<int>());
	ASSERT_EQ(7, b.get<int>());
}

TEST(any, any_to_any_assignment)
{
	static_any<32> a(std::string("Hello"));
	static_any<32> b;

	ASSERT_TRUE(b.empty());
	b = a;
	ASSERT_FALSE(b.empty());

	ASSERT_EQ("Hello", b.get<std::string>());
	ASSERT_EQ("Hello", a.get<std::string>());
}

TEST(any, any_to_any_move_construction)
{
	static_any<32> a(std::string("Hello"));
	static_any<32> b = std::move(a);

	ASSERT_FALSE(a.empty());
	ASSERT_FALSE(b.empty());

	ASSERT_EQ("Hello", b.get<std::string>());
}

TEST(any, any_to_bigger_any)
{
	static_any<16> a(1);
	ASSERT_EQ(1, a.get<int>());

	static_any<32> b(2);
	b = a;

	ASSERT_EQ(1, b.get<int>());
}

TEST(any, any_to_bigger_any_copy)
{
	static_any<16> a(1);
	ASSERT_EQ(1, a.get<int>());

	static_any<32> b = a;
	ASSERT_EQ(1, b.get<int>());
}

struct InitCtor
{
	InitCtor() = default;
	InitCtor(int _x, int _y) : x(_x), y(_y) {}

	int x = 1;
	int y = 2;
};

TEST(any, emplace_no_params)
{
	static_any<32> a;
	a.emplace<InitCtor>();

	ASSERT_FALSE(a.empty());
	EXPECT_EQ(1, a.get<InitCtor>().x);
	EXPECT_EQ(2, a.get<InitCtor>().y);
}

TEST(any, emplace_params)
{
	static_any<32> a;
	a.emplace<InitCtor>(77, 88);

	ASSERT_FALSE(a.empty());
	EXPECT_EQ(77, a.get<InitCtor>().x);
	EXPECT_EQ(88, a.get<InitCtor>().y);
}

TEST(any, destroyed_after_emplace)
{
	CallCounter<0>::reset_counters();

	{
		static_any<32> a;
		a.emplace<CallCounter<0>>();
	}

	EXPECT_EQ(1, CallCounter<0>::constructions);
	EXPECT_EQ(1, CallCounter<0>::destructions);
}

TEST(any, any_cast_pointer_correct_type)
{
	static_any<16> a(7);
	ASSERT_EQ(7, *any_cast<int>(&a));
}

TEST(any, any_cast_pointer_constness)
{
	static_any<16> a(7);
	const auto* a2 = &a;

	auto* pv  = any_cast<int>(&a);
	auto* pv2 = any_cast<int>(a2);

	ASSERT_FALSE(std::is_const<std::remove_pointer<decltype(pv)>::type>::value);
	ASSERT_TRUE(std::is_const<std::remove_pointer<decltype(pv2)>::type>::value);

	ASSERT_EQ(7, *pv);
	ASSERT_EQ(7, *pv2);
}

TEST(any, any_cast_pointer_wrong_type)
{
	static_any<16> a(7);
	ASSERT_EQ(nullptr, any_cast<float>(&a));
}

TEST(any, any_cast_reference_correct_type)
{
	static_any<16> a(7);
	ASSERT_EQ(7, any_cast<int>(a));
}


TEST(any, any_cast_reference_constness)
{
	static_any<16> a(7);
	const auto& a2 = a;

	auto& pv = any_cast<int>(a);
	auto& pv2 = any_cast<int>(a2);

	ASSERT_FALSE(std::is_const<std::remove_reference<decltype(pv)>::type>::value);
	ASSERT_TRUE(std::is_const<std::remove_reference<decltype(pv2)>::type>::value);

	ASSERT_EQ(7, pv);
	ASSERT_EQ(7, pv2);
}

TEST(any, any_cast_reference_wrong_type)
{
	static_any<16> a(7);
	EXPECT_THROW(any_cast<float>(a), bad_any_cast);
}

TEST(any, any_cast_reference_wrong_type_from_to)
{
	static_any<16> a(7);

	try {
		auto f = any_cast<float>(a);
		FAIL();
		ASSERT_EQ(.1234, f); // to avoid a warning, never reached
	}
	catch(bad_any_cast& ex) {
		ASSERT_EQ(typeid(int), ex.stored_type());
		ASSERT_EQ(typeid(float), ex.target_type());
	}
}

TEST(any, query_type)
{
	static_any<32> a(7);
	ASSERT_EQ(typeid(int), a.type());

	a = std::string("f00");
	ASSERT_EQ(typeid(std::string), a.type());
}

TEST(any, reset_empty)
{
	static_any<16> a(7);
	ASSERT_FALSE(a.empty());
	a.reset();
	ASSERT_TRUE(a.empty());
}

TEST(any, reset_has)
{
	static_any<16> a(7);
	ASSERT_TRUE(a.has<int>());
	a.reset();
	ASSERT_FALSE(a.has<int>());
}

TEST(any, type_identification_across_dll)
{
	auto a = get_any_with_int(7);
	EXPECT_TRUE(a.has<int>());
	EXPECT_FALSE(a.has<std::string>());

	EXPECT_EQ(7, a.get<int>());

	EXPECT_THROW(a.get<std::string>(), bad_any_cast);
}

TEST(any_t, simple)
{
	static_any_t<16> a(7);
	ASSERT_EQ(7, a.get<int>());
}

class UnsafeCopy
{
public:
	explicit UnsafeCopy(int i) :
		__i(i)
	{}

	UnsafeCopy(const UnsafeCopy& u) :
		__i(u.__i)
	{
		if (__i == 42)
			throw std::runtime_error("foo");
	}

	UnsafeCopy(UnsafeCopy&& u) noexcept :
		__i(u.__i)
	{}

	int get() const { return __i; }

private:
	const int __i;
};


class UnsafeMove
{
public:
	explicit UnsafeMove(int i) :
		__i(i)
	{}

	UnsafeMove(const UnsafeMove& u) :
		__i(u.__i)
	{
		if (__i == 42)
			throw std::runtime_error("foo");
	}

	UnsafeMove(UnsafeMove&& u) :
		__i(u.__i)
	{
		if (__i == 42)
			throw std::runtime_error("foo");
	}

	int get() const { return __i; }

private:
	const int __i;
};


TEST(any, assignment_strong_guarantee)
{
	static_any<16> a(UnsafeCopy(42));

	CallCounter<0>::reset_counters();
	EXPECT_THROW(a = 5, std::runtime_error);

	ASSERT_FALSE(a.empty());
	EXPECT_EQ(42, a.get<UnsafeCopy>().get());
}

TEST(any_exception, init)
{
	EXPECT_THROW(static_any<16> a = UnsafeMove(42), std::runtime_error);
}

TEST(any_exception, move)
{
	static_any<16> a;

	EXPECT_THROW(a = UnsafeMove(42), std::runtime_error);
	EXPECT_TRUE(a.empty());
}

TEST(any_exception, copy)
{
	static_any<16> a;
	UnsafeCopy u(42);

	EXPECT_THROW(a = u, std::runtime_error);
	EXPECT_TRUE(a.empty());
}

// doesn't compile on VS
#ifndef _MSC_VER

struct UnsafeConstructor
{
	UnsafeConstructor(int i)
	{
		if (i == 42)
			throw std::runtime_error("foo");
	}
};

TEST(any_exception, emplace)
{
	static_any<16> a;
	EXPECT_THROW(a.emplace<UnsafeConstructor>(42), std::runtime_error);
	EXPECT_TRUE(a.empty());
}

#endif

TEST(any_exception, copy_from_any)
{
	static_any<16> a;
	a.emplace<UnsafeCopy>(42);

	static_any<16> b(1234);
	EXPECT_THROW(b = a, std::runtime_error);

	EXPECT_FALSE(b.empty());
	EXPECT_EQ(1234, b.get<int>());
	EXPECT_EQ(typeid(int), b.type());
}

TEST(any_exception, move_from_any)
{
	static_any<16> a;
	a.emplace<UnsafeMove>(42);

	static_any<16> b(1234);
	EXPECT_THROW(b = std::move(a), std::runtime_error);

	EXPECT_EQ(1234, b.get<int>());
}

TEST(any_exception, move_from_different_any)
{
	static_any<8> a;
	a.emplace<UnsafeMove>(42);

	static_any<16> b(1234);
	EXPECT_THROW(b = std::move(a), std::runtime_error);

	EXPECT_EQ(1234, b.get<int>());
}


