#include "kdebug.h"
#include <QtGui/QWidget>
#include <kcomponentdata.h>
#include <iostream>
#include <QtGui/QApplication>
#include <QtGui/QPen>
#include <QtCore/QVariant>
#include <map>
#include <list>

class TestClass1
{
public:
    enum Something { foo };

    void func_void() { kDebug(); }
    int func_int() { kDebug(); return 0; }
    unsigned func_unsigned() { kDebug(); return 0; }
    long func_long() { kDebug(); return 0; }
    long long func_ll() { kDebug(); return 0; }
    unsigned long long func_ull() { kDebug(); return 0; }
    char func_char() { kDebug(); return 0; }
    signed char func_schar() { kDebug(); return 0; }
    unsigned char func_uchar() { kDebug(); return 0; }
    char *func_Pchar() { kDebug(); return 0; }
    const char *func_KPchar() { kDebug(); return 0; }
    const volatile char *func_VKPchar() { kDebug(); return 0; }
    const volatile unsigned long long * const volatile func_KVPKVull() { kDebug(); return 0; }
    const void * const volatile *func_KPKVvoid() { kDebug(); return 0; }

    QList<int> func_ai() { kDebug(); return QList<int>(); }
    QList<unsigned long long const volatile*> func_aptr() { kDebug(); return QList<unsigned long long const volatile*>(); }

    QList<Something> func_aenum() { kDebug(); return QList<Something>(); }
    QList<QList<const void *> > func_aaptr() { kDebug(); return QList<QList<const void *> >(); }

    QMap<int, Something> func_ienummap() { kDebug(); return QMap<int, Something>(); }

    template<typename T>
    T* func_template1() { kDebug(); return 0; }
    template<Something val>
    long func_template2() { kDebug(); return long(val); }

public:
    TestClass1()
        {
            // instantiate
            func_void();
            func_int();
            func_unsigned();
            func_long();
            func_ll();
            func_ull();
            func_char();
            func_schar();
            func_uchar();
            func_Pchar();
            func_KPchar();
            func_VKPchar();
            func_KVPKVull();
            func_KPKVvoid();
            func_ai();
            func_aptr();
            func_aenum();
            func_aaptr();
            func_ienummap();
            func_template1<TestClass1>();
            func_template2<foo>();
        }
};

template<typename T> class TestClass2
{
    long func_long() { kDebug(); return 0; }
    template<typename S>
    T* func_template1() { kDebug(); return 0; }
    template<TestClass1::Something val>
    long func_template2() { kDebug(); return long(val); }
public:
    TestClass2()
        {
            func_long();
            func_template1<TestClass2>();
            func_template2<TestClass1::foo>();
        }
};

template<typename T, TestClass1::Something v> class TestClass3
{
    long func_long() { kDebug(); return 0; }
    template<typename S>
    S* func_template1() { kDebug(); return 0; }
    template<TestClass1::Something val>
    long func_template2() { kDebug(); return long(val); }
public:
    struct Foo { TestClass3 foo; };
    TestClass3()
        {
            func_long();
            func_template1<TestClass2<T> >();
            func_template2<TestClass1::foo>();
        }
};

class TestClass4
{
    TestClass1 c1;

    TestClass2<std::map<long, const void *> > func2()
        { kDebug(); return TestClass2<std::map<long, const void *> >(); }
    TestClass3<std::map<std::list<int>, const void *>, TestClass1::foo>::Foo func3()
        { kDebug(); return TestClass3<std::map<std::list<int>, const void *>, TestClass1::foo>::Foo(); }
public:
    TestClass4()
        {
            func2();
            func3();
            kDebug();
        }
    ~TestClass4()
        {
            kDebug();
        }
};

void testKDebug()
{
    QString test = "%20C this is a string";
    kDebug(150) << test;
    QByteArray cstr = test.toLatin1();
    kDebug(150) << test;
    QChar ch = 'a';
    kDebug() << "QChar a: " << ch;
    ch = '\r';
    kDebug() << "QChar \\r: " << ch;
    kDebug() << k_lineinfo << "error on this line";
    kDebug(2 == 2) << "this is right " << perror;
    kDebug() << "Before instance creation";
    kDebug(1202) << "Before instance creation";
    KComponentData i("kdebugtest");
    kDebug(1) << "kDebugInfo with inexisting area number";
    kDebug(1202) << "This number has a value of " << 5;
    // kDebug() << "This number should come out as appname " << 5 << " " << "test";
    kWarning() << "1+1 = " << 1+1+1;
    kError(1+1 != 2) << "there is something really odd!";
    QString s = "mystring";
    kDebug() << s;
    kError(1202) << "Error !!!";
    kError() << "Error with no area";

    kDebug() << "Printing a null QWidget pointer: " << (QWidget*)0;

    kDebug() << "char " << '^' << " " << char(26);
    QPoint p(0,9);
    kDebug() << p;

    QRect r(9,12,58,234);
    kDebug() << r;

    QStringList sl;
    sl << "hi" << "this" << "list" << "is" << "short";
    kDebug() << sl;

    QList<int> il;
    kDebug() << "Empty QList<int>: " << il;
    il << 1 << 2 << 3 << 4 << 5;
    kDebug() << "QList<int> filled: " << il;

    qint64 big = 65536LL*65536*500;
    kDebug() << big;

    QVariant v( 0.12345 );
    kDebug() << "Variant: " << v;

    QByteArray data;
    data.resize( 6 );
    data[0] = 42;
    data[1] = 'H';
    data[2] = 'e';
    data[3] = 'l';
    data[4] = 'l';
    data[5] = 'o';
    kDebug() << data;
    data.resize( 80 );
    data.fill( 42 );
    kDebug() << data;

#ifdef Q_CC_GNU
    bool dotest = true;
#else
    bool dotest = false;
#endif
    if (dotest) {
        kDebug() << "Testing the function names. The following should display only the base name of the functions";
        TestClass4 c4;
    }
}

int main(int, char** )
{
    testKDebug();
    return 0;
}

