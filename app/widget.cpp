#include "widget.h"

#define THRHLD_UPDATE_BOTH
// #define ONLY_3DRENDER

Widget::Widget(QWidget *parent)
  : QWidget(parent),
	ui(new Ui::Widget),
	m_labelAtCursor(new QLabel(this)),
	m_qImage_2d(QImage(512, 512, QImage::Format_RGB32)),
	m_qImage(QImage(512, 512, QImage::Format_RGB32)) {
  // Initialize rotation matrix
  m_rotationMat.setIdentity();

  // Housekeeping
  ui->setupUi(this);
  m_qImage.fill(qRgb(0, 0, 0));
  m_qImage_2d.fill(qRgb(0, 0, 0));

  // Activate mouse tracking
  setMouseTracking(true);
  ui->label_image3D->setMouseTracking(true);
  ui->label_imgArea->setMouseTracking(true);

  // Initialize the cursor label
  m_labelAtCursor->resize(170, 40);
  m_labelAtCursor->setAutoFillBackground(false);
  m_labelAtCursor->setStyleSheet("color: white");

  // Buttons
  connect(ui->pushButton_render3D, SIGNAL(clicked()), this, SLOT(Render3D()));
  connect(ui->pushButton_startRegionGrowing, SIGNAL(clicked()), this, SLOT(StartRegionGrowingFromSeed()));
  connect(ui->pushButton_targetArea, SIGNAL(clicked()), this, SLOT(SelectTargetArea()));
  connect(ui->pushButton_safeArea, SIGNAL(clicked()), this, SLOT(SelectSafeArea()));
  connect(ui->pushButton_writeAreas, SIGNAL(clicked()), this, SLOT(WriteAreasToFile()));
  connect(ui->pushButton_startCalib, SIGNAL(clicked()), this, SLOT(StartTransformationMatrixCalibration()));

  // Horizontal sliders
  connect(ui->horizontalSlider_threshold, SIGNAL(valueChanged(int)), this,
		  SLOT(UpdateThresholdValue(int)));
  connect(ui->horizontalSlider_center, SIGNAL(valueChanged(int)), this,
		  SLOT(UpdateWindowingCenter(int)));
  connect(ui->horizontalSlider_windowSize, SIGNAL(valueChanged(int)), this,
		  SLOT(UpdateWindowingWindowSize(int)));

  // Vertical sliders
  connect(ui->verticalSlider_depth, SIGNAL(valueChanged(int)), this,
		  SLOT(UpdateDepthValue(int)));

  // Initial slider values
  ui->horizontalSlider_center->setValue(0);
  ui->horizontalSlider_windowSize->setValue(1200);
  ui->horizontalSlider_threshold->setValue(0);
  ui->verticalSlider_depth->setValue(0);

  // Fill 3D image area
  ui->label_image3D->setPixmap(QPixmap::fromImage(m_qImage));
  ui->label_imgArea->setPixmap(QPixmap::fromImage(m_qImage_2d));
}

Widget::~Widget() {
  delete m_labelAtCursor;
  delete ui;
}

// Private member functions

void Widget::Update2DSlice() {
  int depth = ui->verticalSlider_depth->value();
  int threshold = ui->horizontalSlider_threshold->value();
  int center = ui->horizontalSlider_center->value();
  int window_size = ui->horizontalSlider_windowSize->value();

  for (int y = 0; y < m_qImage_2d.height(); ++y) {
	for (int x = 0; x < m_qImage_2d.width(); ++x) {
	  int raw_value =
		m_ctimage.Data()[(x + y * m_qImage_2d.width()) +
		  (m_qImage_2d.height() * m_qImage_2d.width() * depth)];
	  if (raw_value > threshold) {
		m_qImage_2d.setPixel(x, y, qRgb(255, 0, 0));
		continue;
	  }
	  if (CTDataset::WindowInputValue(raw_value, center, window_size).Ok()) {
		int windowed_value =
		  CTDataset::WindowInputValue(raw_value, center, window_size).value();
		m_qImage_2d.setPixel(x, y,
							 qRgb(windowed_value, windowed_value, windowed_value));
	  }
	}
  }

  if (m_targetAreaHasBeenDrawn) {
	QPainter painter(&m_qImage_2d);
	painter.setPen(Qt::white);
	painter.drawEllipse(QPoint(m_targetArea.x(), m_targetArea.y()), m_targetArea.w(), m_targetArea.w());
  }
  if (m_safeAreaHasBeenDrawn) {
	QPainter painter(&m_qImage_2d);
	painter.setPen(Qt::green);
	painter.drawEllipse(QPoint(m_safeArea.x(), m_safeArea.y()), m_safeArea.w(), m_safeArea.w());
  }

  ui->label_imgArea->setPixmap(QPixmap::fromImage(m_qImage_2d));
}

void Widget::Update3DRender() {
  if (m_ctimage.CalculateDepthBuffer(ui->horizontalSlider_threshold->value()).Ok()) {
	if (m_ctimage.RenderDepthBuffer().Ok()) {
	  auto val = 0;
	  for (int y = 0; y < m_qImage.height(); ++y) {
		for (int x = 0; x < m_qImage.width(); ++x) {
		  val = m_ctimage.GetRenderedDepthBuffer()[x + y * m_qImage.width()];
		  m_qImage.setPixel(x, y, qRgb(val, val, val));
		}
	  }
	}
  }
  ui->label_image3D->setPixmap(QPixmap::fromImage(m_qImage));
}

void Widget::UpdateRotationMatrix(QPoint const &position_delta) {
  m_rotationMat = Eigen::AngleAxisd(static_cast<double>(position_delta.y()) / static_cast<double>(180) * M_PI,
									Eigen::Vector3d::UnitX())
	* Eigen::AngleAxisd(static_cast<double>(position_delta.x()) / static_cast<double>(180) * M_PI,
						-Eigen::Vector3d::UnitY())
	* m_rotationMat;
}

void Widget::RenderRegionGrowing() {
  if (m_ctimage.CalculateDepthBufferFromRegionGrowing(m_rotationMat).Ok()) {
	if (m_ctimage.RenderDepthBuffer().Ok()) {
	  auto val = 0;
	  for (int y = 0; y < m_qImage.height(); ++y) {
		for (int x = 0; x < m_qImage.width(); ++x) {
		  val = m_ctimage.GetRenderedDepthBuffer()[x + y * m_qImage.width()];
		  m_qImage.setPixel(x, y, qRgb(val, val, val));
		}
	  }
	}
  }
  ui->label_image3D->setPixmap(QPixmap::fromImage(m_qImage));
}

void Widget::ShowLabelNextToCursor(QPoint const &cursor_global_pos, QPoint const &cursor_local_pos) {
  m_labelAtCursor->show();
  m_labelAtCursor->move(cursor_global_pos + QPoint(-50, 40));
  m_labelAtCursor->setText(QString("(X: %1 | Y: %2 | Z: %3)")
							 .arg(QString::number(cursor_local_pos.x()), QString::number(cursor_local_pos.y()),
								  QString::number(ui->verticalSlider_depth->value())));
  m_labelAtCursor->raise();
}

void Widget::DrawCircleAtCursor(QPoint const &cursor_local_pos, Qt::GlobalColor const &color) {
  QPainter painter(&m_qImage_2d);
  painter.setPen(color);

  auto current_radius = cursor_local_pos - m_currentMousePos2Dslice;
  painter.drawEllipse(m_currentMousePos2Dslice, current_radius.x(), current_radius.x());
  ui->label_imgArea->setPixmap(QPixmap::fromImage(m_qImage_2d));

  if (m_selectTargetArea) {
	m_targetArea.x() = m_currentMousePos2Dslice.x();
	m_targetArea.y() = m_currentMousePos2Dslice.y();
	m_targetArea.z() = ui->verticalSlider_depth->value();
	m_targetArea.w() = std::abs(current_radius.x());
  }
  if (m_selectSafeArea) {
	m_safeArea.x() = m_currentMousePos2Dslice.x();
	m_safeArea.y() = m_currentMousePos2Dslice.y();
	m_safeArea.z() = ui->verticalSlider_depth->value();
	m_safeArea.w() = std::abs(current_radius.x());
  }
}

void Widget::PickCalibrationPoints() {
  Eigen::Vector3i rotated_point(m_currentMousePos3DImage.x(), m_currentMousePos3DImage.y(), m_currentDepthAtCursor);
  auto calib_point = m_rotationMat.transpose() * rotated_point.cast<double>();
  m_calibPoints.emplace_back(calib_point);

  switch (m_calibPoints.size()) {
	case 1:
	  QMessageBox::information(this, "Point 1",
							   "Point 1:   X: " + QString::number(calib_point.x()) + "   " + "Y: "
								 + QString::number(calib_point.y())
								 + "   "
								 + "Z: " + QString::number(calib_point.z()));
	  break;
	case 2:
	  QMessageBox::information(this, "Point 2",
							   "Point 2:   X: " + QString::number(calib_point.x()) + "   " + "Y: "
								 + QString::number(calib_point.y())
								 + "   "
								 + "Z: " + QString::number(calib_point.z()));
	  break;
	case 3:
	  QMessageBox::information(this, "Point 3",
							   "Point 3:   X: " + QString::number(calib_point.x()) + "   " + "Y: "
								 + QString::number(calib_point.y())
								 + "   "
								 + "Z: " + QString::number(calib_point.z()));
	  break;
	case 4:
	  QMessageBox::information(this, "Point 4",
							   "Point 4:   X: " + QString::number(calib_point.x()) + "   " + "Y: "
								 + QString::number(calib_point.y())
								 + "   "
								 + "Z: " + QString::number(calib_point.z()));
	  break;
	case 5:
	  QMessageBox::information(this, "Point 5",
							   "Point 5:   X: " + QString::number(calib_point.x()) + "   " + "Y: "
								 + QString::number(calib_point.y())
								 + "   "
								 + "Z: " + QString::number(calib_point.z()));
	  break;
	case 6:
	  QMessageBox::information(this, "Point 6",
							   "Point 6:   X: " + QString::number(calib_point.x()) + "   " + "Y: "
								 + QString::number(calib_point.y())
								 + "   "
								 + "Z: " + QString::number(calib_point.z()));
	  QMessageBox::information(this,
							   "Calibration Procedure",
							   "Six calibration points have been picked");
	  CalculateTransformationMatrix();
	  break;
  }
}

void Widget::CalculateTransformationMatrix() {
  std::vector<Eigen::Vector3d> target_points = {Eigen::Vector3d(843.101, -446.136, 1351.37),
												Eigen::Vector3d(818.798, -509.529, 1281.21),
												Eigen::Vector3d(705.728, -587.419, 1248.07),
												Eigen::Vector3d(727.323, -641.464, 1299.31),
												Eigen::Vector3d(588.693, -568.988, 1328.29),
												Eigen::Vector3d(563.985, -515.414, 1399.54)};

  m_transformationMatrix = MyLib::EstimateRigidTransformation3D(m_calibPoints, target_points);
  QMessageBox::information(this,
						   "Calibration Procedure",
						   "Transformation matrix has been computed!\nCalibration has finished!");
  m_calibrationOccured = true;
}

void Widget::TransformSelectedAreas() {
  Eigen::Vector3i target_area_XYZ(m_targetArea.x(), m_targetArea.y(), m_targetArea.z());
  Eigen::Vector3i safe_area_XYZ(m_safeArea.x(), m_safeArea.y(), m_safeArea.z());
  m_transformedTargetArea = m_transformationMatrix * target_area_XYZ.cast<double>();
  m_transformedSafeArea = m_transformationMatrix * safe_area_XYZ.cast<double>();
}

// =============== Slots ===============

void Widget::LoadImage3D() {
  QString img_path = QFileDialog::getOpenFileName(
	this, "Open Image", "../external/images", "Raw Image Files (*.raw)");

  if (!m_ctimage.load(img_path).Ok()) {
	QMessageBox::critical(this, "Error",
						  "The specified file could not be opened!");
	m_render3dClicked = false;
	return;
  }
#ifdef ONLY_3DRENDER
  return;
#endif

  Update2DSlice();
}

void Widget::UpdateWindowingCenter(int const val) {
  ui->label_sliderCenter->setText("Center: " + QString::number(val));
  Update2DSlice();
}

void Widget::UpdateWindowingWindowSize(int const val) {
  ui->label_sliderWSize->setText("Window Size: " + QString::number(val));
  Update2DSlice();
}

void Widget::UpdateDepthValue(int const val) {
  ui->label_currentDepth->setText("Depth: " + QString::number(val));
  Update2DSlice();
}

void Widget::UpdateThresholdValue(int const val) {
  ui->label_sliderThreshold->setText("Threshold: " + QString::number(val));
  Update2DSlice();
  if (m_render3dClicked) {
#ifdef THRHLD_UPDATE_BOTH
	Update3DRender();
#endif
  }
  m_seedPicked = false;
  m_regionGrowingIsRendered = false;
}

void Widget::Render3D() {
  LoadImage3D();
  Update3DRender();
  m_render3dClicked = true;
  m_depthBufferIsRendered = true;
  m_seedPicked = false;
}

void Widget::mousePressEvent(QMouseEvent *event) {
  QPoint global_pos = event->pos();
  QPoint local_pos_3Dimg = ui->label_image3D->mapFromParent(global_pos);
  QPoint local_pos_2Dslice = ui->label_imgArea->mapFromParent(global_pos);

  if (m_render3dClicked) {
	if (ui->label_image3D->rect().contains(local_pos_3Dimg)) {
	  if (event->button() == Qt::LeftButton) {
		int depth_at_cursor = m_ctimage.GetDepthBuffer()[local_pos_3Dimg.x() + local_pos_3Dimg.y() * m_qImage.width()];
		ui->label_currentSeed->setText(
		  "Current Seed [px]:   X: " + QString::number(local_pos_3Dimg.x()) + "   " + "Y: "
			+ QString::number(local_pos_3Dimg.y())
			+ "   "
			+ "Depth: " + QString::number(depth_at_cursor));

		// Pick seed for region growing
		m_currentSeed = Eigen::Vector3i(local_pos_3Dimg.x(), local_pos_3Dimg.y(), depth_at_cursor);
		m_seedPicked = true;

		// Pick points for calibration and start calibration procedure
		if (m_calibrationStarted) {
		  PickCalibrationPoints();
		}
	  }
	  if (event->button() == Qt::RightButton) {
		m_currentMousePos = global_pos;
	  }
	}
	if (ui->label_imgArea->rect().contains(local_pos_2Dslice)) {
	  if (event->button() == Qt::LeftButton) {
		m_currentMousePos2Dslice = local_pos_2Dslice;
		if (m_targetAreaHasBeenDrawn && m_selectTargetArea) {
		  m_targetAreaHasBeenDrawn = false;
		}
		if (m_safeAreaHasBeenDrawn && m_selectSafeArea) {
		  m_safeAreaHasBeenDrawn = false;
		}
	  }
	}
  }
}

void Widget::mouseMoveEvent(QMouseEvent *event) {
  QPoint global_pos = event->pos();
  m_currentMouseGlobalPos = global_pos;
  QPoint local_pos_3Dimg = ui->label_image3D->mapFromParent(global_pos);
  QPoint local_pos_2Dslice = ui->label_imgArea->mapFromParent(global_pos);

  if (m_render3dClicked) {
	int cursor_x_px_3Dimg = local_pos_3Dimg.x();
	double cursor_x_mm_3Dimg = cursor_x_px_3Dimg * 0.523; // Pixel x position * Voxel length in x
	int cursor_y_px_3Dimg = local_pos_3Dimg.y();
	double cursor_y_mm_3Dimg = cursor_y_px_3Dimg * 0.523; // Pixel y position * Voxel length in y

	if (ui->label_image3D->rect().contains(local_pos_3Dimg)) {
	  int depth_at_cursor = m_ctimage.GetDepthBuffer()[local_pos_3Dimg.x() + local_pos_3Dimg.y() * m_qImage.width()];
	  // auto depth_at_cursor = 0;
	  m_currentDepthAtCursor = depth_at_cursor;
	  auto depth_mm = depth_at_cursor * 0.7; // Depth value * Voxel height
	  m_currentMousePos3DImage = local_pos_3Dimg;
	  ui->label_xPos->setText("X [px]: " + QString::number(cursor_x_px_3Dimg));
	  ui->label_xPos_mm->setText("X [mm]: " + QString::number(cursor_x_mm_3Dimg));
	  ui->label_yPos->setText("Y [px]: " + QString::number(cursor_y_px_3Dimg));
	  ui->label_yPos_mm->setText("Y [mm]: " + QString::number(cursor_y_mm_3Dimg));

	  if (m_depthBufferIsRendered) {
		ui->label_depthPos->setText("Depth [px]: " + QString::number(depth_at_cursor));
		ui->label_depthPos_mm->setText("Depth [mm]: " + QString::number(depth_mm));

		if (event->buttons() == Qt::RightButton) {
		  QPoint position_delta = m_currentMousePos - global_pos;
		  UpdateRotationMatrix(position_delta);
		  RenderRegionGrowing();
		  m_currentMousePos = global_pos;
		}
	  }
	}

	if (ui->label_imgArea->rect().contains(local_pos_2Dslice)) {
	  ShowLabelNextToCursor(global_pos, local_pos_2Dslice);

	  if (event->buttons() == Qt::LeftButton) {
		if (m_selectTargetArea) {
		  Update2DSlice();
		  DrawCircleAtCursor(local_pos_2Dslice, Qt::GlobalColor::white);
		}
		if (m_selectSafeArea) {
		  Update2DSlice();
		  DrawCircleAtCursor(local_pos_2Dslice, Qt::GlobalColor::green);
		}
	  }
	} else {
	  m_labelAtCursor->hide();
	}
  }
}

void Widget::mouseReleaseEvent(QMouseEvent *event) {
  QPoint global_pos = event->pos();
  QPoint local_pos = ui->label_image3D->mapFromParent(global_pos);
  QPoint local_pos_2Dslice = ui->label_imgArea->mapFromParent(global_pos);

  if (ui->label_image3D->rect().contains(local_pos)) {
	if (event->button() == Qt::RightButton) {
	  m_currentMousePos = local_pos;
	}
  }

  if (ui->label_imgArea->rect().contains(local_pos_2Dslice)) {
	if (m_selectTargetArea) {
	  m_targetAreaHasBeenDrawn = true;
	}
	if (m_selectSafeArea) {
	  m_safeAreaHasBeenDrawn = true;
	}
  }
}

void Widget::StartRegionGrowingFromSeed() {
  if (!m_render3dClicked) {
	QMessageBox::critical(this,
						  "No 3D image available",
						  "No rendered 3D image available. Please render an image first!");
	return;
  }

  if (!m_seedPicked) {
	QMessageBox::critical(this,
						  "No seed picked",
						  "No initial seed has been picked or the threshold value has been changed. Please pick a seed by left-clicking on the rendered 3D image.");
	return;
  }

  m_ctimage.RegionGrowing3D(m_currentSeed, ui->horizontalSlider_threshold->value());
  RenderRegionGrowing();
  m_regionGrowingIsRendered = true;
}

void Widget::SelectTargetArea() {
  m_selectSafeArea = false;
  m_selectTargetArea = true;
  m_targetAreaHasBeenDrawn = false;
}

void Widget::SelectSafeArea() {
  m_selectTargetArea = false;
  m_selectSafeArea = true;
  m_safeAreaHasBeenDrawn = false;
}

void Widget::WriteAreasToFile() {
  if (!m_calibrationOccured) {
	QMessageBox::critical(this,
						  "External Coordinates not Calibrated",
						  "The coordinate transformation to the external device has not been calibrated.\nPlease run the calibration procedure first.");
	return;
  }

  if (!m_safeAreaHasBeenDrawn || !m_targetAreaHasBeenDrawn) {
	QMessageBox::critical(this,
						  "No coordinates selected",
						  "There are no coordinates to write to file. Please select target and safe zones in the 2D image.");
	return;
  }

  TransformSelectedAreas();

  QString file_name = QFileDialog::getSaveFileName(this,
												   tr("Speichern der Planung"),
												   tr("../Planung.txt"),
												   tr("Text Files (*.txt)"));

  if (file_name != " ") {
	QFile file(QFileInfo(file_name).absoluteFilePath());

	if (!file.open(QIODevice::WriteOnly)) {
	  QMessageBox::critical(this, "Error!", "Failed to save file!");
	  return;
	}

	// All OK -> Save data
	QTextStream out(&file);
	out << "Zielbereich:" << "\n" << "	" << "X [px]: " << m_targetArea.x() << "\n" << "	" << "Y [px]: "
		<< m_targetArea.y() << "\n"
		<< "	" << "Z [px]: " << m_targetArea.z() << "\n" << "	" << "Radius [px]: " << m_targetArea.w() << "\n\n";

	out << "Schonbereich:" << "\n" << "	" << "X [px]: " << m_safeArea.x() << "\n" << "	" << "Y [px]: "
		<< m_safeArea.y() << "\n"
		<< "	" << "Z [px]: " << m_safeArea.z() << "\n" << "	" << "Radius [px]: " << m_safeArea.w() << "\n\n";

	out << "Zielbereich transformiert:" << "\n" << "	" << "X [px]: " << m_transformedTargetArea.x() << "\n" << "	"
		<< "Y [px]: "
		<< m_transformedTargetArea.y() << "\n"
		<< "	" << "Z [px]: " << m_transformedTargetArea.z() << "\n" << "	" << "Radius [px]: " << m_targetArea.w()
		<< "\n\n";

	out << "Schonbereich transformiert:" << "\n" << "	" << "X [px]: " << m_transformedSafeArea.x() << "\n" << "	"
		<< "Y [px]: "
		<< m_transformedSafeArea.y() << "\n"
		<< "	" << "Z [px]: " << m_transformedSafeArea.z() << "\n" << "	" << "Radius [px]: " << m_safeArea.w()
		<< "\n\n";

	file.close();
  }
}

void Widget::StartTransformationMatrixCalibration() {
  if (!m_regionGrowingIsRendered) {
	QMessageBox::critical(this,
						  "No Region Growing Render",
						  "There is no rendered 3D model from which to pick calibration points.\nPlease run the region growing algorithm first.");
	return;
  }
  m_calibrationStarted = true;
  m_calibPoints.clear();
  QMessageBox::information(this,
						   "Calibration Procedure",
						   "Welcome to the calibration procedure.\nPlease pick 6 calibration points from the 3D model!");
}



