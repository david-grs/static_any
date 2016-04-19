#include "any.hpp"

#include <geiger/geiger.h>

#include <boost/any.hpp>
#include <QVariant>

Q_DECLARE_METATYPE(std::string)

int main()
{
    qRegisterMetaType<std::string>();

    geiger::init();
    geiger::suite<> s;

    s.add("qvariant double assignment", []()
    {
        QVariant q;
        q = .3224;
    });
    s.add("boost.any double assignment", []()
    {
        boost::any a;
        a = .3224;
    });
    s.add("static_any<8> double assignment", []()
    {
        static_any<8> a;
        a = .2342;
    });
    s.add("static_any_t<8> double assignment", []()
    {
        static_any_t<8> a;
        a = .2342;
    });

    QVariant q = 0xdeadbeef;
    boost::any ba = 0xdeadbeef;
    static_any<8> sa = 0xdeadbeef;
    static_any_t<8> sta = 0xdeadbeef;

    int sum = 0;

    s.add("qvariant get uint", [&]()
    {
        sum += q.toUInt();
    });
    s.add("boost.any get uint", [&]()
    {
        sum += boost::any_cast<unsigned>(ba);
    });
    s.add("static_any<8> get uint", [&]()
    {
        sum += any_cast<unsigned>(sa);
    });
    s.add("static_any_t<8> get uint", [&]()
    {
        sum += sta.get<unsigned>();
    });

    s.add("qvariant string assignment", []()
    {
        QVariant q;
        q.setValue(std::string("foobar"));
    });
    s.add("boost.any string assignment", []()
    {
        boost::any a;
        a = std::string("foobar");
    });
    s.add("static_any<32> string assignment", []()
    {
        static_any<32> a;
        a = std::string("foobar");
    });

    QVariant qstr;
    qstr.setValue(std::string("foobar"));
    boost::any bstr = std::string("foobar");
    static_any<32> sstr = std::string("foobar");

    s.add("qvariant get string", [&]()
    {
        sum += qstr.value<std::string>().size();
    });
    s.add("boost.any get string", [&]()
    {
        sum += boost::any_cast<std::string>(bstr).size();
    });
    s.add("static_any<32> get string", [&]()
    {
        sum += any_cast<std::string>(sstr).size();
    });

    s.set_printer<geiger::printer::console<>>();
    s.run();
}

