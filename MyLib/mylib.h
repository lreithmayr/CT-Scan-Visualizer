#ifndef MYLIB_H
#define MYLIB_H

#include "MyLib_global.h"
#include "status.h"
#include "Eigen/Core"

#include <QDebug>

class MYLIB_EXPORT MyLib {
 public:
  MyLib() = default;

  static std::vector<Eigen::Vector2i> FindNeighbours2D(Eigen::Vector2i &point_2d, const int img_width, const int img_height);


};

#endif  // MYLIB_H
