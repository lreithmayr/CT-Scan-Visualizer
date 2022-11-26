#include <QString>
#include <QtTest>
#include <algorithm>

#include "mylib.h"

class MyLibUnitTest : public QObject {
  Q_OBJECT

 public:
  MyLibUnitTest();

 private Q_SLOTS:
  void windowingTest();
};

MyLibUnitTest::MyLibUnitTest() {}

/**
 Test cases for MyLib::windowing(...)
 HIER OBEN kurze Beschreibung des Testfalls in eigenen Worten einfügen, z.B. die
 erlaubten Grenzen einmal nennen
 */
void MyLibUnitTest::windowingTest() {
  // VALID case 1: testing clean zero for bottom HU boundary
  StatusCode retCode;
  int greyValue = 0;
  retCode = MyLib::WindowInputValue(-34, -34 + 50, 100).status().code();
  QVERIFY2(retCode == StatusCode::OK,
           "returns an error although input is valid");
  QVERIFY2(greyValue == 0, "windowing function lower bound");

  // VALID case 2: testing center of windowed domain
  greyValue = MyLib::WindowInputValue(50, 50, 100).value();
  retCode = MyLib::WindowInputValue(50, 50, 100).status().code();
  QVERIFY2(retCode == StatusCode::OK,
           "returns an error although input is valid");
  QVERIFY2(greyValue == 128,
           qPrintable(
               QString("windowing function medium value, was %1 instead of 128")
                   .arg(greyValue)));
  // ADD FURTHER VALID Testcases here

  // INVALID case 1: HU input too low
  retCode = MyLib::WindowInputValue(-4100, -1000, 2000).status().code();
  QVERIFY2(retCode == StatusCode::HU_OUT_OF_RANGE,
           "No error code returned although input HU value was <-1024");

  // ADD FURTHER INVALID Testcases here
}

QTEST_APPLESS_MAIN(MyLibUnitTest)

#include "tst_mylibunittest.moc"
