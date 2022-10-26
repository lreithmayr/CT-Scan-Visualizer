#ifndef A2_WIDGET_H_
#define A2_WIDGET_H_

#include <QMessageBox>
#include <QWidget>
#include <ui_widget.h>
#include <QFile>
#include <QFileDialog>

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
  static int WindowInputValue(const int &HU_value, const int &center, const int &window_size);

 private:
  Ui::Widget *ui;
  QImage m_img;
  int16_t *m_imageData{nullptr};

 private slots:
  void LoadImage();
};

#endif //A2_WIDGET_H_
