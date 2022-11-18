#ifndef MYLIB_H
#define MYLIB_H

#include "MyLib_global.h"

class MYLIB_EXPORT MyLib {
 public:
  MyLib();
  static int WindowInputValue(const int &HU_value, const int &center,
                              const int &window_size);
};

#endif  // MYLIB_H
