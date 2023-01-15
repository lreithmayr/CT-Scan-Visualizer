#include "mylib.h"

void MyLib::FindNeighbors3D(const Eigen::Vector3i &pt, std::vector<Eigen::Vector3i> &neighbors) {
  neighbors.clear();
  neighbors.emplace_back(pt.x() - 1, pt.y(), pt.z());
  neighbors.emplace_back(pt.x() + 1, pt.y(), pt.z());
  neighbors.emplace_back(pt.x(), pt.y() - 1, pt.z());
  neighbors.emplace_back(pt.x(), pt.y() + 1, pt.z());
  neighbors.emplace_back(pt.x(), pt.y(), pt.z() - 1);
  neighbors.emplace_back(pt.x(), pt.y(), pt.z() + 1);
}

bool MyLib::IsSurfacePoint(Eigen::Vector3i const &point_3d,
						   const int img_width,
						   const int img_height,
						   const int img_layers) {
  int num_of_neighbours = 0;

  for (int x = std::max(0, point_3d.x() - 1); x <= std::min(point_3d.x() + 1, img_width); ++x) {
	for (int y = std::max(0, point_3d.y() - 1); y <= std::min(point_3d.y() + 1, img_height); ++y) {
	  for (int z = std::max(0, point_3d.z() - 1); z <= std::min(point_3d.z() + 1, img_layers); ++z) {
		if (x != point_3d.x() || y != point_3d.y() || z != point_3d.z()) {
		  ++num_of_neighbours;
		}
	  }
	}
  }
  return (num_of_neighbours != 26);
}
