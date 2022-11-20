#ifndef MYLIB_H
#define MYLIB_H

#include "MyLib_global.h"

enum class ReturnCode {
  OK,
  HU_OUT_OF_RANGE,
  CENTER_OUT_OF_RANGE,
  WIDTH_OUT_OF_RANGE
};

static struct ErrorOr {
  ReturnCode rc = ReturnCode::OK;
  int val = 0;
} errorOr;

class MYLIB_EXPORT MyLib {
 public:
  MyLib();

  static ErrorOr WindowInputValue(const int &input_value, const int &center,
                                  const int &window_size);

  static int CalculateDepthBuffer(int16_t *input_data, int16_t *buffer,
                                  int width, int height, int layers,
                                  int threshold);
};

#endif  // MYLIB_H
