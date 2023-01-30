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