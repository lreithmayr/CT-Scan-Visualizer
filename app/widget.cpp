#include "widget.h"

// #define THRHLD_UPDATE_BOTH
// #define ONLY_3DRENDER

Widget::Widget(QWidget *parent)
  : QWidget(parent),
	ui(new Ui::Widget),
	m_labelAtCursor(new QLabel(this)),
	m_qImage(QImage(512, 512, QImage::Format_RGB32)) {
  // Initialize rotation matrix
  m_rotationMat.setIdentity();

  // Housekeeping
  ui->setupUi(this);
  m_qImage.fill(qRgb(0, 0, 0));

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
}

Widget::~Widget() {
  delete ui;
}

// Private member functions

void Widget::Update2DSlice() {
  int depth = ui->verticalSlider_depth->value();
  int threshold = ui->horizontalSlider_threshold->value();
  int center = ui->horizontalSlider_center->value();
  int window_size = ui->horizontalSlider_windowSize->value();

  for (int y = 0; y < m_qImage.height(); ++y) {
	for (int x = 0; x < m_qImage.width(); ++x) {
	  int raw_value =
		m_ctimage.Data()[(x + y * m_qImage.width()) +
		  (m_qImage.height() * m_qImage.width() * depth)];
	  if (raw_value > threshold) {
		m_qImage.setPixel(x, y, qRgb(255, 0, 0));
		continue;
	  }
	  if (CTDataset::WindowInputValue(raw_value, center, window_size).Ok()) {
		int windowed_value =
		  CTDataset::WindowInputValue(raw_value, center, window_size).value();
		m_qImage.setPixel(x, y,
						  qRgb(windowed_value, windowed_value, windowed_value));
	  }
	}
  }
  ui->label_imgArea->setPixmap(QPixmap::fromImage(m_qImage));
}

void Widget::Update2DSliceFromCursor(int depth, int cursor_x, int cursor_y) {
  int threshold = ui->horizontalSlider_threshold->value();
  int center = ui->horizontalSlider_center->value();
  int window_size = ui->horizontalSlider_windowSize->value();

  for (int y = 0; y < m_qImage.height(); ++y) {
	for (int x = 0; x < m_qImage.width(); ++x) {
	  int raw_value =
		m_ctimage.Data()[(x + y * m_qImage.width()) +
		  (m_qImage.height() * m_qImage.width() * depth)];
	  if (raw_value > threshold) {
		m_qImage.setPixel(x, y, qRgb(255, 0, 0));
		continue;
	  }
	  if (CTDataset::WindowInputValue(raw_value, center, window_size).Ok()) {
		int windowed_value =
		  CTDataset::WindowInputValue(raw_value, center, window_size).value();
		m_qImage.setPixel(x, y,
						  qRgb(windowed_value, windowed_value, windowed_value));
	  }
	}
  }
  QPainter painter(&m_qImage);
  painter.setPen(Qt::green);
  painter.drawText(cursor_x, cursor_y, "X");

  ui->label_imgArea->setPixmap(QPixmap::fromImage(m_qImage));
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
  m_rotationMat = Eigen::AngleAxisd(static_cast<double>(position_delta.y() * 0.5f) / static_cast<double>(180) * M_PI,
									Eigen::Vector3d::UnitX())
	* Eigen::AngleAxisd(static_cast<double>(position_delta.x() * 0.5f) / static_cast<double>(180) * M_PI,
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

// =============== Slots ===============

void Widget::LoadImage3D() {
  QString img_path = QFileDialog::getOpenFileName(
	this, "Open Image", "../external/images", "Raw Image Files (*.raw)");

  if (!m_ctimage.load(img_path).Ok()) {
	QMessageBox::critical(this, "Error",
						  "The specified file could not be opened!");
	return;
  }
#ifdef ONLY_3DRENDER
  return;
#endif

  Update2DSlice();
}

void Widget::UpdateWindowingCenter(const int val) {
  ui->label_sliderCenter->setText("Center: " + QString::number(val));
  Update2DSlice();
}

void Widget::UpdateWindowingWindowSize(const int val) {
  ui->label_sliderWSize->setText("Window Size: " + QString::number(val));
  Update2DSlice();
}

void Widget::UpdateDepthValue(const int val) {
  ui->label_currentDepth->setText("Depth: " + QString::number(val));
  Update2DSlice();
}

void Widget::UpdateThresholdValue(const int val) {
  ui->label_sliderThreshold->setText("Threshold: " + QString::number(val));
  Update2DSlice();
  if (m_render3dClicked) {
#ifdef THRHLD_UPDATE_BOTH
	Update3DRender();
#endif
  }
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
  QPoint local_pos = ui->label_image3D->mapFromParent(global_pos);

  if (ui->label_image3D->rect().contains(local_pos) && m_render3dClicked) {
	if (event->button() == Qt::LeftButton) {
	  int depth_at_cursor = m_ctimage.GetDepthBuffer()[local_pos.x() + local_pos.y() * m_qImage.width()];
	  ui->label_currentSeed->setText(
		"Current Seed [px]:   X: " + QString::number(local_pos.x()) + "   " + "Y: " + QString::number(local_pos.y())
		  + "   "
		  + "Depth: " + QString::number(depth_at_cursor));
	  m_currentSeed = Eigen::Vector3i(local_pos.x(), local_pos.y(), depth_at_cursor);
	  m_seedPicked = true;
	}
	if (event->button() == Qt::RightButton) {
	  m_currentMousePos = local_pos;
	  qDebug() << "RMB clicked at: " << m_currentMousePos << "\n";
	}
  }
}

void Widget::mouseMoveEvent(QMouseEvent *event) {

  if (m_render3dClicked) {
	QPoint global_pos = event->pos();
	QPoint local_pos_3Dimg = ui->label_image3D->mapFromParent(global_pos);
	QPoint local_pos_2Dslice = ui->label_imgArea->mapFromParent(global_pos);

	int cursor_x_px_3Dimg = local_pos_3Dimg.x();
	double cursor_x_mm_3Dimg = cursor_x_px_3Dimg * 0.523; // Pixel x position * Voxel length in x
	int cursor_y_px_3Dimg = local_pos_3Dimg.y();
	double cursor_y_mm_3Dimg = cursor_y_px_3Dimg * 0.523; // Pixel y position * Voxel length in y

	int depth_at_cursor = m_ctimage.GetDepthBuffer()[local_pos_3Dimg.x() + local_pos_3Dimg.y() * m_qImage.width()];
	auto depth_mm = depth_at_cursor * 0.7; // Depth value * Voxel height

	if (ui->label_image3D->rect().contains(local_pos_3Dimg)) {
	  ui->label_xPos->setText("X [px]: " + QString::number(cursor_x_px_3Dimg));
	  ui->label_xPos_mm->setText("X [mm]: " + QString::number(cursor_x_mm_3Dimg));
	  ui->label_yPos->setText("Y [px]: " + QString::number(cursor_y_px_3Dimg));
	  ui->label_yPos_mm->setText("Y [mm]: " + QString::number(cursor_y_mm_3Dimg));

	  if (m_depthBufferIsRendered) {
		ui->label_depthPos->setText("Depth [px]: " + QString::number(depth_at_cursor));
		ui->label_depthPos_mm->setText("Depth [mm]: " + QString::number(depth_mm));

		if (event->buttons() == Qt::RightButton) {
		  QPoint position_delta = m_currentMousePos - local_pos_3Dimg;
		  UpdateRotationMatrix(position_delta);
		  RenderRegionGrowing();
		  m_currentMousePos = local_pos_3Dimg;
		}
	  }
	}

	if (ui->label_imgArea->rect().contains(local_pos_2Dslice)) {
	  m_labelAtCursor->show();
	  m_labelAtCursor->move(global_pos + QPoint(-50, 40));
	  m_labelAtCursor->setText(QString("(X: %1 | Y: %2 | Z: %3)")
								 .arg(QString::number(local_pos_2Dslice.x()), QString::number(local_pos_2Dslice.y()),
									  QString::number(ui->verticalSlider_depth->value())));
	  m_labelAtCursor->raise();

	  if (event->buttons() == Qt::LeftButton) {
		
	  }
	} else {
	  m_labelAtCursor->hide();
	}
  }
}

void Widget::mouseReleaseEvent(QMouseEvent *event) {
  QPoint global_pos = event->pos();
  QPoint local_pos = ui->label_image3D->mapFromParent(global_pos);

  if (ui->label_image3D->rect().contains(local_pos)) {
	if (event->button() == Qt::RightButton) {
	  m_currentMousePos = local_pos;
	  qDebug() << "RMB released at: " << m_currentMousePos << "\n";
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
						  "No initial seed has been picked. Please pick a seed by left-clicking on the rendered 3D image.");
	return;
  }
  m_ctimage.RegionGrowing3D(m_currentSeed, ui->horizontalSlider_threshold->value());
  RenderRegionGrowing();
}

