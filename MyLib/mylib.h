#ifndef MYLIB_H
#define MYLIB_H

#include "MyLib_global.h"
#include "status.h"
#include "Eigen/Core"

#include <QDebug>

#include <iostream>

class MYLIB_EXPORT MyLib {
 public:
  MyLib() = default;

  static std::vector<Eigen::Vector2i> FindNeighbours2D(const Eigen::Vector2i &point_2d,
													   const int img_width,
													   const int img_height);
  static std::vector<Eigen::Vector3i> FindNeighbours3D(const Eigen::Vector3i &point_3d,
													   const int img_width,
													   const int img_height,
													   const int img_layers);

  static bool IsSurfacePoint(Eigen::Vector3i const &point_3d,
							 const int img_width,
							 const int img_height,
							 const int img_layers);

};

namespace utils {
inline static void ProgressBar(int progress) {
  int barWidth = 70;

  std::cout << "[";
  int pos = barWidth * progress;
  for (int i = 0; i < barWidth; ++i) {
	if (i < pos) {
	  std::cout << "=";
	} else if (i == pos) {
	  std::cout << ">";
	} else {
	  std::cout << " ";
	}
  }
  std::cout << "] " << static_cast<int>(progress * 100.0) << " %\r";
  std::cout.flush();
}
} // namespace utils
#endif  // MYLIB_H
