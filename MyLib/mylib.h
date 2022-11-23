#ifndef MYLIB_H
#define MYLIB_H

#include "MyLib_global.h"
#include "error.h"

class MYLIB_EXPORT MyLib {
 public:
  MyLib();

  static ErrorOr<int, ReturnCode> WindowInputValue(const int &input_value,
                                       const int &center,
                                       const int &window_size);

  static ErrorOr<void, ReturnCode> CalculateDepthBuffer(int16_t *input_data, int16_t *output_buffer,
                                  int width, int height, int layers,
                                  int threshold);

};

#endif  // MYLIB_H
