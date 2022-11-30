#include "ct_dataset.h"

CTDataset::CTDataset() :
  m_imgHeight(512),
  m_imgWidth(512),
  m_layers(130),
  m_imgData(new int16_t[m_imgHeight * m_imgWidth * m_layers]),
  m_depthBuffer(new int16_t[m_imgHeight * m_imgWidth]),
  m_renderedDepthBuffer(new int16_t[m_imgHeight * m_imgWidth]) {}

CTDataset::~CTDataset() {
  delete[] m_imgData;
  delete[] m_depthBuffer;
  delete[] m_renderedDepthBuffer;
}

/**
 * @details Loads in an image file at the location specified via img_path
 * @param img_path The file path of the CT image.
 * @return ErrorOr type. If loading was successful, ReturnCode::OK will be returned, else ReturnCode::FOPEN_ERROR.
 */
Status CTDataset::load(QString &img_path) {
  QFile img_file(img_path);
  bool fopen = img_file.open(QIODevice::ReadOnly);
  if (!fopen) {
	return Status(StatusCode::FOPEN_ERROR);
  }

  img_file.read(reinterpret_cast<char *>(m_imgData), m_imgHeight * m_imgWidth * m_layers * sizeof(int16_t));
  img_file.close();
  return Status(StatusCode::OK);
}

/**
 * @details Provides access to the loaded image data
 * @return Pointer of type in16_t (short) to the image data array
 * @attention Null-checks and bounds-checks are the user's responsiblity
 */
int16_t *CTDataset::Data() const {
  return m_imgData;
}

int16_t *CTDataset::RenderedDepthBuffer() const {
  return m_renderedDepthBuffer;
}

StatusOr<int> CTDataset::WindowInputValue(const int &input_value, const int &center, const int &window_size) {
  if ((input_value < -1024) || (input_value > 3071)) {
	return StatusOr<int>(Status(StatusCode::HU_OUT_OF_RANGE));
  }

  if ((window_size < 1) || (window_size > 4095)) {
	return StatusOr<int>(Status(StatusCode::WIDTH_OUT_OF_RANGE));
  }

  if ((center < -1024) || (center > 3071)) {
	return StatusOr<int>(Status(StatusCode::CENTER_OUT_OF_RANGE));
  }

  float half_window_size = 0.5f * static_cast<float>(window_size);
  int lower_bound = static_cast<float>(center) - half_window_size;
  int upper_bound = static_cast<float>(center) + half_window_size;

  if (input_value < lower_bound) {
	return StatusOr<int>(0);
  } else if (input_value > upper_bound) {
	return StatusOr<int>(255);
  }

  return StatusOr<int>(std::roundf((input_value - lower_bound) * (255.0f / static_cast<float>(window_size))));
}

Status CTDataset::CalculateDepthBuffer(int threshold) {
  int raw_value = 0;
  for (int y = 0; y < m_imgHeight; ++y) {
	for (int x = 0; x < m_imgWidth; ++x) {
	  m_depthBuffer[x + y * m_imgWidth] = m_layers;
	  for (int d = 0; d < m_layers; ++d) {
		raw_value = m_imgData[(x + y * m_imgWidth) + (m_imgHeight * m_imgWidth * d)];
		if (raw_value >= threshold) {
		  m_depthBuffer[x + y * m_imgWidth] = d;
		  break;
		}
	  }
	}
  }

  if (m_depthBuffer == nullptr) {
	return Status(StatusCode::BUFFER_EMPTY);
  }

  return Status(StatusCode::OK);
}

Status CTDataset::RenderDepthBuffer() {
  auto s_x = 2;
  auto s_x_sq = s_x * s_x;
  auto s_y = 2;
  auto s_y_sq = s_y * s_y;
  auto s_pow_four = s_x_sq * s_y_sq;
  auto T_x = 0;
  auto T_y = 0;
  auto syTx_sq = 0;
  auto sxTy_sq = 0;
  auto nom = 255 * s_x * s_y;
  double denom = 0;
  double inv = 0;
  int I_ref = 0;

  for (int y = 1; y < m_imgHeight - 1; ++y) {
	for (int x = 1; x < m_imgWidth - 1; ++x) {
	  T_x = m_depthBuffer[(x + 1) + y * m_imgWidth] - m_depthBuffer[(x - 1) + y * m_imgWidth];
	  T_y = m_depthBuffer[x + (y + 1) * m_imgWidth] - m_depthBuffer[x + (y - 1) * m_imgWidth];
	  syTx_sq = s_y_sq * T_x * T_x;
	  sxTy_sq = s_x_sq * T_y * T_y;
	  denom = std::sqrt(syTx_sq + sxTy_sq + s_pow_four);
	  inv = 1 / denom;
	  I_ref = nom * inv;
	  m_renderedDepthBuffer[x + y * m_imgWidth] = I_ref;
	}
  }

  if (m_renderedDepthBuffer == nullptr) {
	return Status(StatusCode::BUFFER_EMPTY);
  }

  return Status(StatusCode::OK);
}
