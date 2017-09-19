#include "dyn_lib.hpp"

static_any<16> get_any_with_int(int x)
{
	static_any<16> a = x;
	return a;
}
