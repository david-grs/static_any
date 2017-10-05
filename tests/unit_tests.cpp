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
	static_any<64> a;

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

TEST(any, capacity)
{
	constexpr std::size_t size = sizeof(std::string);
	static_any<size> a;
	ASSERT_EQ(size, a.capacity());

	a = std::string("hello world");
	ASSERT_EQ(size, a.capacity());

	a.reset();
	ASSERT_EQ(size, a.capacity());
}

TEST(any, size)
{
	constexpr std::size_t size = std::max(sizeof(std::string), sizeof(int));
	static_any<size> a;
	ASSERT_EQ(0, a.size());

	a = 1234;
	ASSERT_EQ(sizeof(1234), a.size());

	a = std::string("foobar");
	ASSERT_EQ(sizeof(std::string), a.size());

	a.reset();
	ASSERT_EQ(0, a.size());
}

struct CallCounter
{
	CallCounter() noexcept { alive_count++; default_constructions++; }
	CallCounter(const CallCounter&) noexcept { alive_count++; copy_constructions++; }
	CallCounter(CallCounter&&) noexcept { alive_count++; move_constructions++; }
	~CallCounter() {
		assert(alive);
		alive = false;
		destructions++;
		alive_count--;
		assert(alive_count >= 0);
	}

	static void reset_counters()
	{
		// Do not reset alive_count, it is for correctness checking
		default_constructions = 0;
		copy_constructions = 0;
		move_constructions = 0;
		destructions = 0;
	}

	bool alive = true;

	static int alive_count;
	static int default_constructions;
	static int copy_constructions;
	static int move_constructions;
	static int destructions;
};

int CallCounter::alive_count = 0;
int CallCounter::default_constructions = 0;
int CallCounter::copy_constructions = 0;
int CallCounter::move_constructions = 0;
int CallCounter::destructions = 0;

TEST(any, default_constructed_is_empty)
{
	static_any<16> a;
	ASSERT_TRUE(a.empty());
}

TEST(any, contructed_with_param_non_empty)
{
	static_any<sizeof(int)> a(77); // will contain integer
	ASSERT_FALSE(a.empty());
}

TEST(any, has)
{
	static_any<sizeof(int)> a(77); // will contain integer
	ASSERT_TRUE(a.has<int>());
	ASSERT_FALSE(a.has<double>());
}

TEST(any, move_construct)
{
	CallCounter::reset_counters();
	CallCounter counter;
	static_any<sizeof(CallCounter)> a(std::move(counter));

	ASSERT_EQ(1, CallCounter::default_constructions);
	ASSERT_EQ(0, CallCounter::copy_constructions);
	ASSERT_EQ(1, CallCounter::move_constructions);
	ASSERT_EQ(0, CallCounter::destructions);
}

TEST(any, copy_construct)
{
	CallCounter::reset_counters();
	CallCounter counter;
	static_any<sizeof(CallCounter)> a(counter);

	ASSERT_EQ(1, CallCounter::default_constructions);
	ASSERT_EQ(1, CallCounter::copy_constructions);
	ASSERT_EQ(0, CallCounter::move_constructions);
	ASSERT_EQ(0, CallCounter::destructions);
}

TEST(any, destruction)
{
	CallCounter::reset_counters();
	CallCounter counter;
	{
		static_any<sizeof(CallCounter)> a(counter);
	}

	ASSERT_EQ(1, CallCounter::destructions);
}

TEST(any, reset_destruction)
{
	CallCounter::reset_counters();
	CallCounter counter;

	static_any<sizeof(CallCounter)> a(counter);
	a.reset();

	ASSERT_EQ(1, CallCounter::destructions);
}

TEST(any, copy_assignment)
{
	constexpr auto size = std::max(sizeof(int), sizeof(CallCounter));
	CallCounter::reset_counters();
	CallCounter counter;

	static_any<size> a(1);
	a = counter;

	ASSERT_EQ(1, CallCounter::default_constructions);
	ASSERT_EQ(1, CallCounter::copy_constructions);
	ASSERT_EQ(0, CallCounter::move_constructions);
}

TEST(any, move_assignment)
{
	CallCounter::reset_counters();
	CallCounter counter;

	static_any<sizeof(CallCounter)> a;
	a = std::move(counter);

	ASSERT_EQ(1, CallCounter::default_constructions);
	ASSERT_EQ(0, CallCounter::copy_constructions);
	ASSERT_EQ(1, CallCounter::move_constructions);
}

TEST(any, any_move_ctor)
{
	CallCounter counter;
	static_any<sizeof(CallCounter)> a(counter);

	CallCounter::reset_counters();

	static_any<sizeof(CallCounter)> b(std::move(a));

	ASSERT_EQ(0, CallCounter::default_constructions);
	ASSERT_EQ(0, CallCounter::copy_constructions);
	ASSERT_EQ(1, CallCounter::move_constructions);
}

TEST(any, any_move_assignment)
{
	constexpr auto size = std::max(sizeof(int), sizeof(CallCounter));
	CallCounter counter;
	static_any<size> a(counter);

	CallCounter::reset_counters();

	static_any<size> b = 123;
	b = std::move(a);

	ASSERT_EQ(0, CallCounter::default_constructions);
	ASSERT_EQ(0, CallCounter::copy_constructions);
	ASSERT_EQ(1, CallCounter::move_constructions);
}

TEST(any, reassignment)
{
	CallCounter::reset_counters();
	CallCounter counter;

	static_any<sizeof(CallCounter)> a(counter);
	a = counter;

	ASSERT_EQ(1, CallCounter::default_constructions);
	ASSERT_EQ(2, CallCounter::copy_constructions);
	ASSERT_EQ(0, CallCounter::move_constructions);
	ASSERT_EQ(1, CallCounter::destructions);
}

TEST(any, not_empty_after_assignment)
{
	static_any<sizeof(int)> a;
	ASSERT_TRUE(a.empty());
	a = 7;
	ASSERT_FALSE(a.empty());
}

TEST(any, different_type_after_assignment)
{
	constexpr auto size = std::max(sizeof(int), sizeof(double));
	static_any<size> a(7);
	ASSERT_TRUE(a.has<int>());
	ASSERT_FALSE(a.has<double>());
	a = 3.14;
	ASSERT_FALSE(a.has<int>());
	ASSERT_TRUE(a.has<double>());
}

TEST(any, get_good_type)
{
	static_any<sizeof(int)> a(7);
	auto i = a.get<int>();
	ASSERT_EQ(7, i);
}

TEST(any, get_bad_type)
{
	static_any<sizeof(int)> a(7);
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
	static_any<sizeof(int)> a(7);
	a.get<int>() = 6;
	const static_any<sizeof(int)>& const_ref = a;
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
	static_any<sizeof(int)> a(7);
	static_any<sizeof(int)> b(a);

	ASSERT_EQ(7, a.get<int>());
	ASSERT_EQ(7, b.get<int>());
}

TEST(any, any_to_any_assignment)
{
	static_any<sizeof(std::string)> a(std::string("Hello"));
	static_any<sizeof(std::string)> b;

	ASSERT_TRUE(b.empty());
	b = a;
	ASSERT_FALSE(b.empty());

	ASSERT_EQ("Hello", b.get<std::string>());
	ASSERT_EQ("Hello", a.get<std::string>());
}

TEST(any, any_to_any_move_construction)
{
	static_any<sizeof(std::string)> a(std::string("Hello"));
	static_any<sizeof(std::string)> b = std::move(a);

	ASSERT_FALSE(a.empty());
	ASSERT_FALSE(b.empty());

	ASSERT_EQ("Hello", b.get<std::string>());
}

TEST(any, any_to_bigger_any)
{
	static_any<sizeof(int)> a(1);
	ASSERT_EQ(1, a.get<int>());

	static_any<2 * sizeof(int)> b(2);
	b = a;

	ASSERT_EQ(1, b.get<int>());
}

TEST(any, any_to_bigger_any_copy)
{
	static_any<sizeof(int)> a(1);
	ASSERT_EQ(1, a.get<int>());

	static_any<2 * sizeof(int)> b = a;
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
	static_any<sizeof(InitCtor)> a;
	a.emplace<InitCtor>();

	ASSERT_FALSE(a.empty());
	EXPECT_EQ(1, a.get<InitCtor>().x);
	EXPECT_EQ(2, a.get<InitCtor>().y);
}

TEST(any, emplace_params)
{
	static_any<sizeof(InitCtor)> a;
	a.emplace<InitCtor>(77, 88);

	ASSERT_FALSE(a.empty());
	EXPECT_EQ(77, a.get<InitCtor>().x);
	EXPECT_EQ(88, a.get<InitCtor>().y);
}

TEST(any, empty_any_to_any_assignment)
{
	static_any<sizeof(int)> a(1234);
	static_any<sizeof(int)> b;

	ASSERT_FALSE(a.empty());
	a = b;
	ASSERT_TRUE(a.empty());
}

TEST(any, destroyed_after_emplace)
{
	CallCounter::reset_counters();
	{
		static_any<sizeof(CallCounter)> a;
		a.emplace<CallCounter>();
	}

	EXPECT_EQ(1, CallCounter::default_constructions);
	EXPECT_EQ(1, CallCounter::destructions);
}

TEST(any, any_cast_pointer_correct_type)
{
	static_any<sizeof(int)> a(7);
	ASSERT_EQ(7, *any_cast<int>(&a));
}

TEST(any, any_cast_pointer_constness)
{
	static_any<sizeof(int)> a(7);
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
	static_any<sizeof(int)> a(7);
	ASSERT_EQ(nullptr, any_cast<float>(&a));
}

TEST(any, any_cast_reference_correct_type)
{
	static_any<sizeof(int)> a(7);
	ASSERT_EQ(7, any_cast<int>(a));
}


TEST(any, any_cast_reference_constness)
{
	static_any<sizeof(int)> a(7);
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
	static_any<sizeof(int)> a(7);
	EXPECT_THROW(any_cast<float>(a), bad_any_cast);
}

TEST(any, any_cast_reference_wrong_type_from_to)
{
	static_any<sizeof(int)> a(7);

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
	constexpr auto size = std::max(sizeof(int), sizeof(std::string));
	static_any<size> a(7);
	ASSERT_EQ(typeid(int), a.type());

	a = std::string("f00");
	ASSERT_EQ(typeid(std::string), a.type());
}

TEST(any, reset_empty)
{
	static_any<sizeof(int)> a(7);
	ASSERT_FALSE(a.empty());
	a.reset();
	ASSERT_TRUE(a.empty());
}

TEST(any, reset_has)
{
	static_any<sizeof(int)> a(7);
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

struct unsafe_to_copy
{
	unsafe_to_copy() =default;
	[[noreturn]] unsafe_to_copy(const unsafe_to_copy&) { throw 123; }
};

TEST(any_exception, init)
{
	EXPECT_THROW(static_any<sizeof(unsafe_to_copy)> a = unsafe_to_copy(), int);
}

TEST(any_exception, move)
{
	static_any<sizeof(unsafe_to_copy)> a;

	EXPECT_THROW(a = unsafe_to_copy(), int);
	EXPECT_TRUE(a.empty());
}

TEST(any_exception, copy)
{
	static_any<sizeof(unsafe_to_copy)> a;
	unsafe_to_copy u;

	EXPECT_THROW(a = u, int);
	EXPECT_TRUE(a.empty());
}

TEST(any_exception, restore_when_failed)
{
	constexpr auto size = std::max(sizeof(int), sizeof(unsafe_to_copy));
	static_any<size> a(1234);

	EXPECT_THROW(a = unsafe_to_copy(), int);

	EXPECT_FALSE(a.empty());
	EXPECT_EQ(1234, a.get<int>());
	EXPECT_EQ(typeid(int), a.type());
}

// doesn't compile on VS
#ifndef _MSC_VER

struct unsafe_to_construct
{
	[[noreturn]] unsafe_to_construct() { throw 123; }
};

TEST(any_exception, emplace)
{
	static_any<sizeof(unsafe_to_construct)> a;
	EXPECT_THROW(a.emplace<unsafe_to_construct>(), int);
	EXPECT_TRUE(a.empty());
}

#endif

TEST(any_exception, copy_from_any)
{
	constexpr auto size = std::max(sizeof(CallCounter), sizeof(unsafe_to_copy));
	static_any<size> a;
	a.emplace<unsafe_to_copy>();

	static_any<size> b((CallCounter()));
	CallCounter::reset_counters();
	EXPECT_THROW(b = a, int);

	EXPECT_FALSE(b.empty());
	EXPECT_EQ(typeid(CallCounter), b.type());
}

TEST(any_exception, move_from_any)
{
	constexpr auto size = std::max(sizeof(CallCounter), sizeof(unsafe_to_copy));
	static_any<size> a;
	a.emplace<unsafe_to_copy>();

	static_any<size> b((CallCounter()));
	CallCounter::reset_counters();
	EXPECT_THROW(b = std::move(a), int);

	EXPECT_FALSE(b.empty());
	EXPECT_EQ(typeid(CallCounter), b.type());
}

TEST(any_exception, throwing_construct_strong_guarantee)
{
	constexpr auto size = std::max(sizeof(CallCounter), sizeof(unsafe_to_construct));
	static_any<size> a;

	CallCounter::reset_counters();
	{
		a.emplace<CallCounter>();
		EXPECT_TRUE(a.has<CallCounter>());
		EXPECT_THROW(a.emplace<unsafe_to_construct>(), int);
		EXPECT_TRUE(a.has<CallCounter>());
	}

	EXPECT_EQ(1, CallCounter::default_constructions);
}

struct safe_to_move
{
	safe_to_move() noexcept = default;
	[[noreturn]] safe_to_move(const safe_to_move&) { throw 123.0; }
	safe_to_move(safe_to_move&&) noexcept = default;
};

TEST(any_exception, no_copy_with_strong_guarantee)
{
	constexpr auto size = std::max(sizeof(unsafe_to_copy), sizeof(unsafe_to_copy));
	static_any<size> a;

	CallCounter::reset_counters();
	{
		a.emplace<safe_to_move>();
		EXPECT_THROW(a.emplace<unsafe_to_construct>(), int);
		EXPECT_TRUE(a.has<safe_to_move>());
	}
}

TEST(any_exception, move_from_different_any)
{
	constexpr auto size = std::max(sizeof(int), sizeof(unsafe_to_copy));
	static_any<size> a;
	a.emplace<unsafe_to_copy>();

	static_any<2 * size> b(1234);
	EXPECT_THROW(b = std::move(a), int);

	EXPECT_EQ(1234, b.get<int>());
}

TEST(any_exception, emplace_strong_guarantee)
{
	constexpr auto size = std::max(sizeof(int), sizeof(unsafe_to_construct));
	static_any<size> a(1234);
	EXPECT_THROW(a.emplace<unsafe_to_construct>(), int);
	EXPECT_TRUE(a.has<int>());
}
