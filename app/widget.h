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
  void ShowLabelNextToCursor(const QPoint &cursor_global_pos, const QPoint &cursor_local_pos);
  void DrawCircleAtCursor(const QPoint &cursor_local_pos, Qt::GlobalColor const &color);
  void PickCalibrationPoints();
  void CalculateTransformationMatrix();
  void TransformSelectedAreas();

 private:
  Ui::Widget *ui;
  CTDataset m_ctimage;
  QImage m_qImage;
  QImage m_qImage_2d;
  Eigen::Matrix3d m_rotationMat;
  QLabel *m_labelAtCursor;

  QPoint m_currentMousePos;
  QPoint m_currentMouseGlobalPos;
  QPoint m_currentMousePos2Dslice;
  QPoint m_currentMousePos3DImage;
  int m_currentDepthAtCursor{0};
  Eigen::Vector3i m_currentSeed;
  bool m_seedPicked{false};

  Eigen::Vector4i m_targetArea;
  Eigen::Vector4i m_safeArea;
  Eigen::Vector3d m_transformedTargetArea;
  Eigen::Vector3d m_transformedSafeArea;

  std::vector<Eigen::Vector3d> m_calibPoints;
  Eigen::Isometry3d m_transformationMatrix;

  bool m_render3dClicked{false};
  bool m_depthBufferIsRendered{false};
  bool m_regionGrowingIsRendered{false};
  bool m_selectTargetArea{false};
  bool m_selectSafeArea{false};
  bool m_targetAreaHasBeenDrawn{false};
  bool m_safeAreaHasBeenDrawn{false};
  bool m_calibrationStarted{false};
  bool m_calibrationOccured{false};


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
  void SelectTargetArea();
  void SelectSafeArea();
  void WriteAreasToFile();
  void StartTransformationMatrixCalibration();
};

#endif //WIDGET_H
