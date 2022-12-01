#ifndef WIDGET_H
#define WIDGET_H

#include "ct_dataset.h"

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
  void UpdateSliceView();
  void UpdateDepthImage();
  void Update3DRender();

 private:
  Ui::Widget *ui;
  CTDataset m_ctimage;
  QImage m_qImage;
  bool m_render3dClicked;

 private slots:
  void LoadImage3D();
  void UpdateWindowingCenter(const int val);
  void UpdateWindowingWindowSize(const int val);
  void UpdateDepthValue(const int val);
  void UpdateThresholdValue(const int val);
  void Render3D();
};

#endif //WIDGET_H
