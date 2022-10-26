#include <QString>
#include <QtTest>
#include "mylib.h"
#include <algorithm>

class MyLibUnitTest : public QObject
{
    Q_OBJECT

public:
    MyLibUnitTest();

private Q_SLOTS:

};

MyLibUnitTest::MyLibUnitTest()
{
}


QTEST_APPLESS_MAIN(MyLibUnitTest)

#include "tst_mylibunittest.moc"
