#ifndef MYLIB_H
#define MYLIB_H

#include "MyLib_global.h"
#include "error.h"

#include <QDebug>

class MYLIB_EXPORT MyLib {
 public:
  MyLib();

  static ErrorOr<int> WindowInputValue(const int &input_value,
                                       const int &center,
                                       const int &window_size);

  static ErrorOr<void> CalculateDepthBuffer(int16_t *input_data, int16_t *output_buffer,
                                  int width, int height, int layers,
                                  int threshold);

  static ErrorOr<void> CalculateDepthBuffer3D(int16_t *depth_buffer, int16_t *output_buffer, int width, int height);
};

#endif  // MYLIB_H
