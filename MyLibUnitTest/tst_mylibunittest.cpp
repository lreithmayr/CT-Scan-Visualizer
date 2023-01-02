#include <QString>
#include <QtTest>
#include <algorithm>

#include "mylib.h"
#include "ct_dataset.h"

class MyLibUnitTest : public QObject {
 Q_OBJECT

 public:
  MyLibUnitTest() = default;

 private Q_SLOTS:
  static void WindowingTest();
};

/**
 Test cases for CTDataset::windowing(...)
 HIER OBEN kurze Beschreibung des Testfalls in eigenen Worten einfügen, z.B. die
 erlaubten Grenzen einmal nennen
 */
void MyLibUnitTest::WindowingTest() {
  StatusCode retCode = StatusCode::OK;

  // VALID case 1: Testing clean zero for bottom HU boundary
  int greyValue = 0;
  retCode = CTDataset::WindowInputValue(-34, -34 + 50, 100).status().code();
  QVERIFY2(retCode == StatusCode::OK,
		   "returns an error although input is valid");
  QVERIFY2(greyValue == 0, "windowing function lower bound");

  // VALID case 2: Testing center of windowed domain
  greyValue = CTDataset::WindowInputValue(50, 50, 100).value();
  retCode = CTDataset::WindowInputValue(50, 50, 100).status().code();
  QVERIFY2(retCode == StatusCode::OK,
		   "returns an error although input is valid");
  QVERIFY2(greyValue == 128,
		   qPrintable(
			 QString("windowing function medium value, was %1 instead of 128")
			   .arg(greyValue)));

  // VALID case 3: Testing window size parameter
  greyValue = CTDataset::WindowInputValue(50, 50, 200).value();
  retCode = CTDataset::WindowInputValue(50, 50, 200).status().code();
  QVERIFY2(retCode == StatusCode::OK,
		   "returns an error although input is valid");
  QVERIFY2(greyValue == 128,
		   qPrintable(
			 QString("windowing function medium value, was %1 instead of 128")
			   .arg(greyValue)));

  // INVALID case 1: HU input too low
  retCode = CTDataset::WindowInputValue(-4100, -1000, 2000).status().code();
  QVERIFY2(retCode == StatusCode::HU_OUT_OF_RANGE,
		   "No error code returned although input HU value was < -1024");

  // INVALID case 2: HU input too high
  retCode = CTDataset::WindowInputValue(3500, -1000, 2000).status().code();
  QVERIFY2(retCode == StatusCode::HU_OUT_OF_RANGE,
		   "No error code returned although input HU value was > 3071");

  // INVALID case 3: Center value too low
  retCode = CTDataset::WindowInputValue(400, -1500, 2000).status().code();
  QVERIFY2(retCode == StatusCode::CENTER_OUT_OF_RANGE,
		   "No error code returned although center value was < -1024");

  // INVALID case 4: Center value too high
  retCode = CTDataset::WindowInputValue(400, 3500, 2000).status().code();
  QVERIFY2(retCode == StatusCode::CENTER_OUT_OF_RANGE,
		   "No error code returned although center value was > 3071");

  // INVALID case 5: Window size too low
  retCode = CTDataset::WindowInputValue(400, -1000, 0).status().code();
  QVERIFY2(retCode == StatusCode::WIDTH_OUT_OF_RANGE,
		   "No error code returned although center value was < 1");

  // INVALID case 6: Window size too high
  retCode = CTDataset::WindowInputValue(400, -1000, 5000).status().code();
  QVERIFY2(retCode == StatusCode::WIDTH_OUT_OF_RANGE,
		   "No error code returned although center value was > 4095");
}

QTEST_APPLESS_MAIN(MyLibUnitTest)

#include "tst_mylibunittest.moc"
