#include "mylib.h"

std::vector<Eigen::Vector2i> MyLib::FindNeighbours2D(Eigen::Vector2i &point_2d,
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
