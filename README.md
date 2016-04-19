static\_any\<S\>
================
A container for generic (as *general*) data type &mdash; like boost.any. However:

 - It is **~10x faster** than boost.any, mainly because there is no memory allocation
 - As it lies on the stack, it is **cache-friendly**, close to your other class attributes
 - There is a very **small space overhead**: a fixed overhead of 8 bytes


static\_any\<S\> is also **safe**:
 - compile time check during the assignment, to ensure that its buffer is big enough to store the value
 - runtime check before any conversions, to ensure that the stored type is the one's requested by the user


Example
-------

```c++
    static_any<32> a;
    static_assert(sizeof(a) == 32 + 8, "any has a fixed overhead of 8 bytes");

    a = 1234;
    ASSERT_EQ(1234, a.get<int>());

    a = std::string("foobar");
    ASSERT_EQ("foobar", a.get<std::string>());

    try {
      // this will throw it is a string
      std::cout << any_cast<int>(a);
    }
    catch(bad_any_cast& ex) {
    }

    struct A : std::array<char, 32> {};
    a = A();

    struct B : A { char c; };  
    
    // Does not build: sizeof(B) is too big for static_any<16>
    a = B();
```


---

static\_any\_t\<S\>
===================
A container similar to static\_any\<S\>, but for trivially copyable types only. The differences:

 - **No space overhead**
 - **Faster**
 - **Unsafe**: there is no check when you try to access your data


On my laptop with a i7-3537U CPU, I got the following results when benchmarking during one second the assignment of a double to boost.any, static\_any\<S\> and static\_any\_t\<S\>:

```
Test              Time (ns)
---------------------------
boost.any               50
static_any<16>           4
static_any_t<16>         0
```

