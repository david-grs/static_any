#include "any.hpp"

#include <geiger/geiger.h>

#include <boost/any.hpp>
#include <QVariant>
#include <experimental/any>

struct small_struct
{
    int i;
    void* v;
    double d;

    int h() const { return i; }
};

Q_DECLARE_METATYPE(std::string)
Q_DECLARE_METATYPE(small_struct)

int main()
{
    qRegisterMetaType<std::string>();
    qRegisterMetaType<small_struct>();

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
    s.add("std::any double assignment", []()
    {
        std::experimental::any a;
        a = .2342;
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

    double d = .42;

    QVariant qd = d;
    boost::any bda = d;
    std::experimental::any stdda = d;
    static_any<8> sda = d;
    static_any_t<8> stda = d;

    int sum = 0;

    s.add("qvariant get double", [&sum, &qd]()
    {
        sum += qd.toDouble();
    });
    s.add("boost.any get double", [&sum, &bda]()
    {
        sum += boost::any_cast<double>(bda);
    });
    s.add("std::any get double", [&sum, &stdda]()
    {
        sum += std::experimental::any_cast<double>(stdda);
    });
    s.add("static_any<8> get double", [&sum, &sda]()
    {
        sum += any_cast<double>(sda);
    });
    s.add("static_any_t<8> get double", [&sum, &stda]()
    {
        sum += stda.get<double>();
    });

    s.add("qvariant small_struct assignment", []()
    {
        QVariant q;
        q.setValue(small_struct{2, nullptr, .45});
    });
    s.add("boost.any small_struct assignment", []()
    {
        boost::any a;
        a = small_struct{2, nullptr, .45};
    });
    s.add("std::any small_struct assignment", []()
    {
        std::experimental::any a;
        a = small_struct{2, nullptr, .45};
    });
    s.add("static_any<32> small_struct assignment", []()
    {
        static_any<32> a;
        a = small_struct{2, nullptr, .45};
    });
    s.add("static_any_t<32> small_struct assignment", []()
    {
        static_any_t<32> a;
        a = small_struct{2, nullptr, .45};
    });

    small_struct sss{2, nullptr, .45};
    QVariant qs;
    qs.setValue(sss);
    boost::any bas = sss;
    std::experimental::any stdsa = sss;
    static_any<32> ssa = sss;

    s.add("qvariant get small_struct", [&sum, &qs]()
    {
        sum += qs.value<small_struct>().h();
    });
    s.add("boost.any get small_struct", [&sum, &bas]()
    {
        sum += boost::any_cast<small_struct>(bas).h();
    });
    s.add("std::any get small_struct", [&sum, &stdsa]()
    {
        sum += std::experimental::any_cast<small_struct>(stdsa).h();
    });
    s.add("static_any<32> get small_struct", [&sum, &ssa]()
    {
        sum += any_cast<small_struct>(ssa).h();
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
    s.add("std::any string assignment", []()
    {
        std::experimental::any a;
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
    std::experimental::any astr = std::string("foobar");
    static_any<32> sstr = std::string("foobar");

    s.add("qvariant get string", [&sum, &qstr]()
    {
        sum += qstr.value<std::string>().size();
    });
    s.add("boost.any get string", [&sum, &bstr]()
    {
        sum += boost::any_cast<std::string>(bstr).size();
    });
    s.add("std::any get string", [&sum, &astr]()
    {
        sum += std::experimental::any_cast<std::string>(astr).size();
    });
    s.add("static_any<32> get string", [&sum, &sstr]()
    {
        sum += any_cast<std::string>(sstr).size();
    });

    s.set_printer<geiger::printer::console<>>();
    s.run();
}

