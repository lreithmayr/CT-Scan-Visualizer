#include "widget.h"

#define LOG(x) std::cout << x << "\n";

Widget::Widget(QWidget *parent)
    : QWidget(parent),
      ui(new Ui::Widget),
      m_img(QImage(512, 512, QImage::Format_RGB32)),
      m_depthImage(QImage(512, 512, QImage::Format_RGB32)) {
  // Housekeeping
  ui->setupUi(this);
  m_img.fill(qRgb(0, 0, 0));

  // Buttons
  connect(ui->pushButton_loadImage, SIGNAL(clicked()), this, SLOT(LoadImage()));
  connect(ui->pushButton_loadImage3D, SIGNAL(clicked()), this,
          SLOT(LoadImage3D()));
  connect(ui->pushButton_depthBuffer, SIGNAL(clicked()), this,
          SLOT(DrawDepthBuffer()));

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
  ui->horizontalSlider_threshold->setValue(300);
  ui->verticalSlider_depth->setValue(0);
}

Widget::~Widget() {
  delete ui;
  delete[] m_imageData;
}

// Private member functions

void Widget::UpdateSliceView() {
  int center = ui->horizontalSlider_center->value();
  int window_size = ui->horizontalSlider_windowSize->value();

  for (int y = 0; y < m_img.height(); ++y) {
    for (int x = 0; x < m_img.width(); ++x) {
      int raw_value = m_imageData[x + (y * 512)];
        int windowed_value = MyLib::WindowInputValue(raw_value, center, window_size);
      m_img.setPixel(x, y,
                     qRgb(windowed_value, windowed_value, windowed_value));
    }
  }
  ui->label_imgArea->setPixmap(QPixmap::fromImage(m_img));
}

void Widget::UpdateDepthImage() {
  int depth = ui->verticalSlider_depth->value();
  int threshold = ui->horizontalSlider_threshold->value();
  int center = ui->horizontalSlider_center->value();
  int window_size = ui->horizontalSlider_windowSize->value();

  for (int y = 0; y < m_img.height(); ++y) {
    for (int x = 0; x < m_img.width(); ++x) {
      int raw_value = m_imageData[(x + y * m_img.width()) +
                                  (m_img.height() * m_img.width() * depth)];
      if (raw_value > threshold) {
        m_img.setPixel(x, y, qRgb(255, 0, 0));
        continue;
      }
      int windowed_value = MyLib::WindowInputValue(raw_value, center, window_size);
      m_img.setPixel(x, y,
                     qRgb(windowed_value, windowed_value, windowed_value));
    }
  }
  ui->label_imgArea->setPixmap(QPixmap::fromImage(m_img));
}

void Widget::CalculateDepthBuffer(int16_t *input_data, int width, int height,
                                  int layers, int threshold) {
  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      for (int d = 0; d < layers; ++d) {
        int raw_value = input_data[(x + y * width) + (height * width * d)];
        if (raw_value >= threshold) {
          m_depthImage.setPixel(x, y, qRgb(d, d, d));
          break;
        }
        m_depthImage.setPixel(x, y, qRgb(185, 189, 186));
      }
    }
  }
  ui->label_image3D->setPixmap(QPixmap::fromImage(m_depthImage));
}

// Slots

void Widget::LoadImage() {
  QString img_path = QFileDialog::getOpenFileName(
      this, "Open Image", "../external/images", "Raw Image Files (*.raw)");
  QFile img_file(img_path);
  bool fopen = img_file.open(QIODevice::ReadOnly);
  if (!fopen) {
    QMessageBox::critical(this, "Error",
                          "The specified file could not be opened!");
    return;
  }

  img_file.read(reinterpret_cast<char *>(m_imageData),
                m_img.height() * m_img.width() * sizeof(int16_t));
  img_file.close();

  UpdateSliceView();
}

void Widget::LoadImage3D() {
  m_imageData = new int16_t[m_img.height() * m_img.width() * 130];
  QString img_path = QFileDialog::getOpenFileName(
      this, "Open Image", "../external/images", "Raw Image Files (*.raw)");
  QFile img_file(img_path);
  bool fopen = img_file.open(QIODevice::ReadOnly);
  if (!fopen) {
    QMessageBox::critical(this, "Error",
                          "The specified file could not be opened!");
    return;
  }

  img_file.read(reinterpret_cast<char *>(m_imageData),
                m_img.height() * m_img.width() * 130 * sizeof(int16_t));
  img_file.close();

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
}

void Widget::DrawDepthBuffer() {
  m_depthImageData = new int16_t[m_img.height() * m_img.width() * 130];
  QString img_path = QFileDialog::getOpenFileName(
      this, "Open Image", "../external/images", "Raw Image Files (*.raw)");
  QFile img_file(img_path);
  bool fopen = img_file.open(QIODevice::ReadOnly);
  if (!fopen) {
    QMessageBox::critical(this, "Error",
                          "The specified file could not be opened!");
    return;
  }

  img_file.read(reinterpret_cast<char *>(m_depthImageData),
                m_img.height() * m_img.width() * 130 * sizeof(int16_t));
  img_file.close();
  CalculateDepthBuffer(m_depthImageData, m_depthImage.width(),
                       m_depthImage.height(), 130,
                       ui->horizontalSlider_threshold->value());
}
