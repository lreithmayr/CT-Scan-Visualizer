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

void Widget::UpdateSliceView() {
  int center = ui->horizontalSlider_center->value();
  int window_size = ui->horizontalSlider_windowSize->value();

  for (int y = 0; y < m_qImage.height(); ++y) {
	for (int x = 0; x < m_qImage.width(); ++x) {
	  int raw_value = m_ctimage.Data()[x + (y * 512)];
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
	ui->label_imgArea->setPixmap(QPixmap::fromImage(m_qImage));
  }
}

void Widget::Update3DRender() {
  if (m_ctimage.CalculateDepthBuffer(ui->horizontalSlider_threshold->value()).Ok()) {
	if (m_ctimage.RenderDepthBuffer().Ok()) {
	  for (int y = 0; y < m_qImage.height(); ++y) {
		for (int x = 0; x < m_qImage.width(); ++x) {
		  m_qImage.setPixel(x, y,
							qRgb(m_ctimage.RenderedDepthBuffer()[x + y * m_qImage.width()],
								 m_ctimage.RenderedDepthBuffer()[x + y * m_qImage.width()],
								 m_ctimage.RenderedDepthBuffer()[x + y * m_qImage.width()]));
		}
	  }
	  ui->label_image3D->setPixmap(QPixmap::fromImage(m_qImage));
	}
  }
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
}
