#include "mylib.h"

MyLib::MyLib() {}

int MyLib::WindowInputValue(const int &input_value, const int &center,
                            const int &window_size) {
  float half_window_size = 0.5 * static_cast<float>(window_size);
  int lower_bound = static_cast<float>(center) - half_window_size;
  int upper_bound = static_cast<float>(center) + half_window_size;
  if (input_value < lower_bound) {
    return 0;
  } else if (input_value > upper_bound) {
    return 255;
  } else {
    return (input_value - lower_bound) *
           (255.0f / static_cast<float>(window_size));
  }
}
