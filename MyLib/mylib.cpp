#include "mylib.h"

MyLib::MyLib() {}

ErrorOr<int, ReturnCode> MyLib::WindowInputValue(const int &input_value,
                                                 const int &center,
                                                 const int &window_size) {
  if ((input_value < -1024) || (input_value > 3071)) {
    return {ReturnCode::HU_OUT_OF_RANGE};
  }

  if ((window_size < 1) || (window_size > 4095)) {
    return {ReturnCode::WIDTH_OUT_OF_RANGE};
  }

  if ((center < -1024) || (center > 3071)) {
    return {ReturnCode::CENTER_OUT_OF_RANGE};
  }

  float half_window_size = 0.5 * static_cast<float>(window_size);
  int lower_bound = static_cast<float>(center) - half_window_size;
  int upper_bound = static_cast<float>(center) + half_window_size;

  if (input_value < lower_bound) {
    return {0};
  } else if (input_value > upper_bound) {
    return {255};
  }

  return {std::roundf((input_value - lower_bound) *
                      (255.0f / static_cast<float>(window_size)))};
}

ErrorOr<void, ReturnCode> MyLib::CalculateDepthBuffer(int16_t *input_data, int16_t *output_buffer, int width,
                                int height, int layers, int threshold) {
  if (output_buffer == nullptr) {
        return {ReturnCode::BUFFER_EMPTY};
  }

  int raw_value = 0;
  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      for (int d = 0; d < layers; ++d) {
        raw_value = input_data[(x + y * width) + (height * width * d)];
        if (raw_value >= threshold) {
          output_buffer[x + y * width] = d;
          break;
        }
        output_buffer[x + y * width] = 0;
      }
    }
  }
}
