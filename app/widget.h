#pragma once

#include <ui_widget.h>

#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QWidget>
#include <iostream>

QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

class Widget : public QWidget {
  Q_OBJECT

 public:
  explicit Widget(QWidget *parent = nullptr);
  ~Widget() override;

 private:
  static int WindowInputValue(const int &HU_value, const int &center,
                              const int &window_size);
  void UpdateSliceView();

 private:
  Ui::Widget *ui;
  QImage m_img;
  int16_t *m_imageData{nullptr};

 private slots:
  void LoadImage();
  void UpdateWindowingCenter(const int &val);
  void UpdateWindowingWindowSize(const int &val);
};
