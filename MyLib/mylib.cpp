#include "mylib.h"

MyLib::MyLib() {}

ErrorOr MyLib::WindowInputValue(const int &input_value, const int &center,
                                const int &window_size) {
  if ((input_value < -1024) || (input_value > 3071)) {
    errorOr.rc = ReturnCode::HU_OUT_OF_RANGE;
    errorOr.val = -1;
    return errorOr;
  }

  if ((window_size < 1) || (window_size > 4095)) {
    errorOr.rc = ReturnCode::WIDTH_OUT_OF_RANGE;
    errorOr.val = -1;
    return errorOr;
  }

  if ((center < -1024) || (center > 3071)) {
    errorOr.rc = ReturnCode::CENTER_OUT_OF_RANGE;
    errorOr.val = -1;
    return errorOr;
  }

  float half_window_size = 0.5 * static_cast<float>(window_size);
  int lower_bound = static_cast<float>(center) - half_window_size;
  int upper_bound = static_cast<float>(center) + half_window_size;

  errorOr.rc = ReturnCode::OK;

  if (input_value < lower_bound) {
    errorOr.val = 0;
    return errorOr;
  } else if (input_value > upper_bound) {
    errorOr.val = 255;
    return errorOr;
  }

  errorOr.val = std::roundf((input_value - lower_bound) *
                            (255.0f / static_cast<float>(window_size)));
  return errorOr;
}

int MyLib::CalculateDepthBuffer(int16_t *input_data, int16_t *buffer, int width,
                                int height, int layers, int threshold) {
  if (buffer == nullptr) {
    return -1;
  }

  int raw_value = 0;
  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      for (int d = 0; d < layers; ++d) {
        raw_value = input_data[(x + y * width) + (height * width * d)];
        if (raw_value >= threshold) {
          buffer[x + y * width] = d;
          break;
        }
        buffer[x + y * width] = 0;
      }
    }
  }

  return 0;
}
