#include "widget.h"

// #define THRHLD_UPDATE_BOTH
// #define ONLY_3DRENDER

Widget::Widget(QWidget *parent)
  : QWidget(parent),
	ui(new Ui::Widget),
	m_qImage(QImage(512, 512, QImage::Format_RGB32)) {
  // Initialize rotation matrix
  m_rotationMat.setIdentity();

  // Housekeeping
  ui->setupUi(this);
  m_qImage.fill(qRgb(0, 0, 0));

  // Activate mouse tracking
  setMouseTracking(true);
  ui->label_image3D->setMouseTracking(true);

  // Buttons
  connect(ui->pushButton_loadImage3D, SIGNAL(clicked()), this,
		  SLOT(LoadImage3D()));
  connect(ui->pushButton_render3D, SIGNAL(clicked()), this, SLOT(Render3D()));
  connect(ui->pushButton_clearBuffers, SIGNAL(clicked()), this, SLOT(ClearAllBuffers()));

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
  ui->horizontalSlider_threshold->setValue(500);
  ui->verticalSlider_depth->setValue(0);
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
  m_rotationMat = Eigen::AngleAxisd(position_delta.y() / 180. * M_PI, Eigen::Vector3d::UnitX())
	* Eigen::AngleAxisd(position_delta.x() / 180. * M_PI, -Eigen::Vector3d::UnitY()) * m_rotationMat;
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
	this, "Open Image", "../../external/images", "Raw Image Files (*.raw)");

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
}

void Widget::mousePressEvent(QMouseEvent *event) {
  QPoint global_pos = event->pos();
  QPoint local_pos = ui->label_image3D->mapFromParent(global_pos);

  if (event->button() == Qt::LeftButton) {
	int depth_at_cursor = m_ctimage.GetDepthBuffer()[local_pos.x() + local_pos.y() * m_qImage.width()];
	if (ui->label_image3D->rect().contains(local_pos)) {
	  Eigen::Vector3i seed(local_pos.x(), local_pos.y(), depth_at_cursor);
	  m_ctimage.RegionGrowing3D(seed, ui->horizontalSlider_threshold->value());
	  RenderRegionGrowing();
	}
  }
  if (event->button() == Qt::RightButton) {
	m_currentMousePos = local_pos;
  }
}

void Widget::mouseMoveEvent(QMouseEvent *event) {
  if (m_render3dClicked) {
	QPoint global_pos = event->pos();
	QPoint local_pos = ui->label_image3D->mapFromParent(global_pos);

	int cursor_x_px = local_pos.x();
	double cursor_x_mm = cursor_x_px * 0.523; // Pixel x position * Voxel length in x
	int cursor_y_px = local_pos.y();
	double cursor_y_mm = cursor_y_px * 0.523; // Pixel y position * Voxel length in y

	int depth_at_cursor = m_ctimage.GetDepthBuffer()[local_pos.x() + local_pos.y() * m_qImage.width()];
	auto depth_mm = depth_at_cursor * 0.7; // Depth value * Voxel height

	if (ui->label_image3D->rect().contains(local_pos)) {
	  ui->label_xPos->setText("X [px]: " + QString::number(cursor_x_px));
	  ui->label_xPos_mm->setText("X [mm]: " + QString::number(cursor_x_mm));
	  ui->label_yPos->setText("Y [px]: " + QString::number(cursor_y_px));
	  ui->label_yPos_mm->setText("Y [mm]: " + QString::number(cursor_y_mm));

	  if (m_depthBufferIsRendered) {
		ui->label_depthPos->setText("Depth [px]: " + QString::number(depth_at_cursor));
		ui->label_depthPos_mm->setText("Depth [mm]: " + QString::number(depth_mm));
	  }

	  if (event->buttons() == Qt::RightButton) {
		QPoint position_delta = local_pos - m_currentMousePos;
		UpdateRotationMatrix(position_delta);
		m_currentMousePos = local_pos;
		RenderRegionGrowing();
	  }
	  // if (event->buttons() != Qt::RightButton) {
	// 	m_currentMousePos = local_pos;
	  // }
	}
  }
}

void Widget::ClearAllBuffers() {
	m_ctimage.ResetBuffers();
}
