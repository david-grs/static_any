#include "any.hpp"
#include <geiger/geiger.h>
#include <boost/any.hpp>
#include <QVariant>

int main()
{
    geiger::init();
    geiger::suite<> s;

    s.add("qvariant assignment", []()
    {
        QVariant q;
        q = .3224;
    });
    s.add("boost any any assignment", []()
    {
        boost::any a;
        a = .3224;
    });
    s.add("static_any<8> assignment", []()
    {
        static_any<8> a;
        a = .2342;
    });
    s.add("static_any_t<16> assignment", []()
    {
        static_any_t<8> a;
        a = .2342;
    });

    QVariant q = 0xdeadbeef;
    boost::any ba = 0xdeadbeef;
    static_any<8> sa = 0xdeadbeef;
    static_any_t<8> sta = 0xdeadbeef;

    int sum = 0;

    s.add("qvariant get", [&]()
    {
        sum += q.toUInt();
    });
    s.add("boost any get", [&]()
    {
        sum += boost::any_cast<unsigned>(ba);
    });
    s.add("static_any<8> get", [&]()
    {
        sum += any_cast<unsigned>(sa);
    });
    s.add("static_any_t<16> get", [&]()
    {
        sum += sta.get<unsigned>();
    });

    s.set_printer<geiger::printer::console<>>();
    s.run();
}

