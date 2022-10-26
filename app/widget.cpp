#include "widget.h"

#define LOG(x) std::cout << x << "\n";

Widget::Widget(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::Widget),
  m_img(QImage(512, 512, QImage::Format_RGB32)),
  m_imageData(new int16_t[512 * 512]) {
  ui->setupUi(this);
  m_img.fill(qRgb(0, 0, 0));

  connect(ui->pushButton_loadImage, SIGNAL(clicked()), this, SLOT(LoadImage()));
  connect(ui->horizontalSlider_center, SIGNAL(valueChanged(int)), this, SLOT(UpdateWindowingCenter(int)));
  connect(ui->horizontalSlider_windowSize, SIGNAL(valueChanged(int)), this, SLOT(UpdateWindowingWindowSize(int)));

  ui->horizontalSlider_center->setValue(0);
  ui->horizontalSlider_windowSize->setValue(1200);
}

Widget::~Widget() {
  delete ui;
  delete[] m_imageData;
}

// Private member functions

int Widget::WindowInputValue(const int &input_value, const int &center, const int &window_size) {
  float half_window_size = 0.5 * static_cast<float>(window_size);
  int lower_bound = static_cast<float>(center) - half_window_size;
  int upper_bound = static_cast<float>(center) + half_window_size;
  if (input_value < lower_bound) {
	return 0;
  } else if (input_value > upper_bound) {
	return 255;
  } else {
	return (input_value - lower_bound) * (255.0f / static_cast<float>(window_size));
  }
}

void Widget::UpdateSliceView() {
  for (int y = 0; y < m_img.height(); ++y) {
    for (int x = 0; x < m_img.width(); ++x) {
      int raw_value = m_imageData[x + (y * 512)];
      int windowed_value = WindowInputValue(raw_value, ui->horizontalSlider_center->value(), ui->horizontalSlider_windowSize->value());
      m_img.setPixel(x, y, qRgb(windowed_value, windowed_value, windowed_value));
    }
  }
  ui->label_imgArea->setPixmap(QPixmap::fromImage(m_img));
}

// Slots

void Widget::LoadImage() {
  QString img_path = QFileDialog::getOpenFileName(this, "Open Image", "../images", "Raw Image Files (*.raw)");
  QFile img_file(img_path);
  bool fopen = img_file.open(QIODevice::ReadOnly);
  if (!fopen) {
	QMessageBox::critical(this, "Error", "The specified file could not be opened!");
	return;
  }

  img_file.read(reinterpret_cast<char *>(m_imageData), 512 * 512 * sizeof(int16_t));
  img_file.close();

  UpdateSliceView();
}

void Widget::UpdateWindowingCenter(const int &val) {
    ui->label_sliderCenter->setText("Center: " + QString::number(val));
    UpdateSliceView();
}

void Widget::UpdateWindowingWindowSize(const int &val) {
    ui->label_sliderWSize->setText("Window Size: " + QString::number(val));
    UpdateSliceView();
}
