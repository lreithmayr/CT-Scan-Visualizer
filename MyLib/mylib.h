#ifndef MYLIB_H
#define MYLIB_H

#include "MyLib_global.h"
#include "status.h"

#include <QDebug>

class MYLIB_EXPORT MyLib {
 public:
  MyLib() = default;

  static Status CalculateDepthBuffer(const int16_t *input_data, int16_t *output_buffer,
									 int width, int height, int layers,
									 int threshold);

  static Status RenderDepthBuffer(int16_t *depth_buffer, int16_t *output_buffer, int width, int height);
};

#endif  // MYLIB_H
