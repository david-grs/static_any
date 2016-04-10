Any.Trivial
===========
A container for generic (as *general*) data type &mdash; like boost.any &mdash; but :

 - is stack-based, i.e. does not do any memory allocation
 - is a trivial type
 - can only store trivially copyable types


Example
-------

```c++
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

    struct B : A { long j_; };  
    
    // Does not build: sizeof(B) is too big for any<16>
    a = B();
```


