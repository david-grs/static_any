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


---

Comparison with Boost.Any
-------------------------

### Advantages
 - **~50x faster** than Boost.Any, as there is no memory allocation or any other overhead around the data
 - **Cache-friendly**: Any.Trivial is stack-based, close to the other class attributes
 - **No space overhead**: 100% of the data space is useful

Thus, the following code:
```c++
void foo(any<16>& a)
{
    a = 1234;
}
```

... is translated to:
```
Dump of assembler code for function foo(any<16ul, void>&):
   0x0000000000431c50 <+0>:     mov    DWORD PTR [rdi],0x4d2
   0x0000000000431c56 <+6>:     ret
End of assembler dump.
```

### Drawbacks
 - **Safety**: Boost.Any stores the type and will throw an exception if you try to get a wrong type
 - **Type support**: Any.Trivial only supports trivially copyable types


---

Roadmap
-------

### 0.1
- Type safety: typeid() of the stored type will be saved and checked when accessing the object
