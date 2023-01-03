#include "widget.h"

#define LOG(x) std::cout << x << "\n";

Widget::Widget(QWidget *parent)
  : QWidget(parent),
	ui(new Ui::Widget),
	m_qImage(QImage(512, 512, QImage::Format_RGB32)),
	m_render3dClicked(false) {
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

void Widget::UpdateDepthImage() {
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

void Widget::UpdateDepthImageFromCursor(int depth, int cursor_x, int cursor_y) {
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
		  val = m_ctimage.RenderedDepthBuffer()[x + y * m_qImage.width()];
		  m_qImage.setPixel(x, y, qRgb(val, val, val));
		}
	  }
	}
  }
  ui->label_image3D->setPixmap(QPixmap::fromImage(m_qImage));
}

// Slots

void Widget::LoadImage3D() {
  QString img_path = QFileDialog::getOpenFileName(
	this, "Open Image", "../external/images", "Raw Image Files (*.raw)");

  if (!m_ctimage.load(img_path).Ok()) {
	QMessageBox::critical(this, "Error",
						  "The specified file could not be opened!");
	return;
  }
  UpdateDepthImage();
}

void Widget::UpdateWindowingCenter(const int val) {
  ui->label_sliderCenter->setText("Center: " + QString::number(val));
  UpdateDepthImage();
}

void Widget::UpdateWindowingWindowSize(const int val) {
  ui->label_sliderWSize->setText("Window Size: " + QString::number(val));
  UpdateDepthImage();
}

void Widget::UpdateDepthValue(const int val) {
  ui->label_currentDepth->setText("Depth: " + QString::number(val));
  UpdateDepthImage();
}

void Widget::UpdateThresholdValue(const int val) {
  ui->label_sliderThreshold->setText("Threshold: " + QString::number(val));
  UpdateDepthImage();
  if (m_render3dClicked) {
	Update3DRender();
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

  int cursor_x_px = local_pos.x();
  double cursor_x_mm = cursor_x_px * 0.523; // Pixel x position * Voxel length in x
  int cursor_y_px = local_pos.y();
  double cursor_y_mm = cursor_y_px * 0.523; // Pixel y position * Voxel length in y

  int depth_at_cursor = m_ctimage.GetDepthBuffer()[local_pos.x() + local_pos.y() * m_qImage.width()] - 1;
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
  }

  Eigen::Vector3i seed(cursor_x_px, cursor_y_px, depth_at_cursor);
  auto region = m_ctimage.RegionGrowing(seed, ui->horizontalSlider_threshold->value());
  QMessageBox::information(this, "Test", QString::number(region.value().size()));
}

void Widget::mouseMoveEvent(QMouseEvent *event) {
  if (m_render3dClicked) {
	QPoint global_pos = event->pos();
	QPoint local_pos = ui->label_image3D->mapFromParent(global_pos);

	if (ui->label_image3D->rect().contains(local_pos)) {
	  // ui->label_xPos->setText("X: " + QString::number(local_pos.x()));
	  // ui->label_yPos->setText("Y: " + QString::number(local_pos.y()));

	  if (m_depthBufferIsRendered) {
		int depth_at_cursor = m_ctimage.GetDepthBuffer()[local_pos.x() + local_pos.y() * m_qImage.width()] - 1;
		// ui->label_depthPos->setText("Depth: " + QString::number(depth_at_cursor));
		UpdateDepthImageFromCursor(depth_at_cursor, local_pos.x(), local_pos.y());
	  }
	}
  }
}
