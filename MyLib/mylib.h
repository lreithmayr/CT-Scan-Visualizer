#ifndef MYLIB_H
#define MYLIB_H

#include "MyLib_global.h"
#include "status.h"
#include "Eigen/Core"
#include "Eigen/Geometry"

#include <QDebug>

#include <iostream>

// Global Eigen::IOFormat definition for debugging purposes
// FIXME: Remove this when done debugging!
Eigen::IOFormat const CleanFmt(4, 0, ", ", "\n", "[", "]");

class MYLIB_EXPORT MyLib {
 public:
  MyLib() = default;

  static void FindNeighbors3D(const Eigen::Vector3i &pt, std::vector<Eigen::Vector3i> &neighbors);

  /// Computes rigid transformation matrix for transformation from source to target
  static Eigen::Isometry3d EstimateRigidTransformation3D(const std::vector<Eigen::Vector3d> &source_points,
														 const std::vector<Eigen::Vector3d> &target_points);
};

namespace utils {
inline static void ProgressBar(float progress) {
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
