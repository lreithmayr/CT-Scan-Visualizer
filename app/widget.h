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
  void UpdateDepthImage();
  void CalculateDepthBuffer(int16_t *input_data, int width, int height,
                            int layers, int threshold);

 private:
  Ui::Widget *ui;
  QImage m_img;
  QImage m_depthImage;
  int16_t *m_imageData{new int16_t[512 * 512]};
  int16_t *m_depthImageData{new int16_t[512 * 512]};
 private slots:
  void LoadImage();
  void LoadImage3D();
  void UpdateWindowingCenter(const int val);
  void UpdateWindowingWindowSize(const int val);
  void UpdateDepthValue(const int val);
  void UpdateThresholdValue(const int val);
  void DrawDepthBuffer();
};
