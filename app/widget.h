#ifndef WIDGET_H
#define WIDGET_H

#include "ct_dataset.h"

#include <ui_widget.h>

#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QWidget>
#include <QMouseEvent>
#include <QPainter>
#include <QDebug>
#include <QDataStream>

#include <iostream>
#include <QtMath>

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
  void Update2DSlice();
  void Update2DSliceFromCursor(int depth, int cursor_x, int cursor_y);
  void Update3DRender();
  void UpdateRotationMatrix(QPoint const &position_delta);
  void RenderRegionGrowing();
  void ShowLabelNextToCursor(QPoint cursor_global_pos, QPoint cursor_local_pos);
  void DrawRadiusAtCursor(QPoint cursor_global_pos, QPoint cursor_local_pos);

 private:
  Ui::Widget *ui;
  CTDataset m_ctimage;
  QImage m_qImage;
  QImage m_qImage_2d;
  bool m_render3dClicked{false};
  bool m_depthBufferIsRendered{false};
  Eigen::Matrix3d m_rotationMat;
  QPoint m_currentMousePos{0, 0};
  Eigen::Vector3i m_currentSeed;
  bool m_seedPicked{false};
  QLabel *m_labelAtCursor;

 private slots:
  void LoadImage3D();
  void UpdateWindowingCenter(const int val);
  void UpdateWindowingWindowSize(const int val);
  void UpdateDepthValue(const int val);
  void UpdateThresholdValue(const int val);
  void Render3D();
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void StartRegionGrowingFromSeed();
};

#endif //WIDGET_H
