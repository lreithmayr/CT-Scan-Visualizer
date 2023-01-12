#include "mylib.h"

std::vector<Eigen::Vector2i> MyLib::FindNeighbours2D(const Eigen::Vector2i &point_2d,
													 const int img_width,
													 const int img_height) {
  std::vector<Eigen::Vector2i> neighbours;
  for (int x = std::max(0, point_2d.x() - 1); x <= std::min(point_2d.x() + 1, img_width); ++x) {
	for (int y = std::max(0, point_2d.y() - 1); y <= std::min(point_2d.y() + 1, img_height); ++y) {
	  if (x != point_2d.x() || y != point_2d.y()) {
		neighbours.emplace_back(x, y);
	  }
	}
  }
  return neighbours;
}

std::vector<Eigen::Vector3i> MyLib::FindNeighbours3D(const Eigen::Vector3i &point_3d,
													 const int img_width,
													 const int img_height,
													 const int img_layers) {
  std::vector<Eigen::Vector3i> neighbours;
  for (int x = std::max(0, point_3d.x() - 1); x <= std::min(point_3d.x() + 1, img_width); ++x) {
	for (int y = std::max(0, point_3d.y() - 1); y <= std::min(point_3d.y() + 1, img_height); ++y) {
	  for (int z = std::max(0, point_3d.z() - 1); z <= std::min(point_3d.z() + 1, img_layers); ++z) {
		if (x != point_3d.x() || y != point_3d.y() || z != point_3d.z()) {
		  neighbours.emplace_back(x, y, z);
		}
	  }
	}
  }
  return neighbours;
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
