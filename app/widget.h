#pragma once

#include "ct_dataset.h"

#include <ui_widget.h>
#include <mylib.h>

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
  void UpdateSliceView();
  void UpdateDepthImage();

 private:
  Ui::Widget *ui;
  CTDataset m_ctimage;
  QImage m_qImage;

  int16_t *m_depthBuffer{new int16_t[512 * 512]()};
  int16_t *m_shaderBuffer{new int16_t[512 * 512]()};

 private slots:
  void LoadImage();
  void LoadImage3D();
  void UpdateWindowingCenter(const int val);
  void UpdateWindowingWindowSize(const int val);
  void UpdateDepthValue(const int val);
  void UpdateThresholdValue(const int val);
  void RenderDepthBuffer();
  void Render3D();
};
