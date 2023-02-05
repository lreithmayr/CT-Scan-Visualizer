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

bool MyLib::IsSurfacePoint(const int *buf, Eigen::Vector3i const &point, int width, int height) {
  return (!(buf[(point.x() - 1) + point.y() * width + (height * width * point.z())] == 1
	&& buf[(point.x() + 1) + point.y() * width + (height * width * point.z())] == 1
	&& buf[point.x() + (point.y() - 1) * width + (height * width * point.z())] == 1
	&& buf[point.x() + (point.y() + 1) * width + (height * width * point.z())] == 1
	&& buf[point.x() + point.y() * width + (height * width * (point.z() - 1))] == 1
	&& buf[point.x() + point.y() * width + (height * width * (point.z() + 1))] == 1));
}

Eigen::Isometry3d MyLib::EstimateRigidTransformation3D(std::vector<Eigen::Vector3d> const &source_points,
													   std::vector<Eigen::Vector3d> const &target_points) {
  typedef Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic> MatrixXd;

  assert(source_points.size() == target_points.size());

  // Transform std::vector of voxels to dynamic sized Eigen matrix
  const int m = 3;
  const int n = static_cast<int>(source_points.size());
  MatrixXd X = MatrixXd(m, n);
  MatrixXd Y = MatrixXd(m, n);
  for (int i = 0; i < n; ++i) {
	X.col(i) = source_points[i].head(3);
	Y.col(i) = target_points[i].head(3);
  }

  // Subtract mean
  Eigen::Vector3d mean_X = X.rowwise().mean();
  Eigen::Vector3d mean_Y = Y.rowwise().mean();
  X.colwise() -= mean_X;
  Y.colwise() -= mean_Y;

  // Compute SVD (singular value decomposition) of cross-covariance matrix
  MatrixXd R_XY = X * Y.adjoint();
  Eigen::JacobiSVD<MatrixXd> svd(R_XY, Eigen::ComputeThinU | Eigen::ComputeThinV);

  // Compute estimate of the rotation matrix:
  Eigen::Matrix3d R = svd.matrixV() * svd.matrixU().adjoint();

  // Assure a right-handed coordinate system:
  if (R.determinant() < 0) {
	R = svd.matrixV() * Eigen::Vector3d(1, 1, -1).asDiagonal() * svd.matrixU().transpose();
  }

  // Construct homogeneous transformation matrix.
  Eigen::Matrix4d transformation_mat;
  transformation_mat.block(0, 0, 3, 3) = R;
  transformation_mat.block(0, 3, 3, 1) = mean_Y - R * mean_X;
  transformation_mat.block(3, 0, 1, 3) = Eigen::RowVector3d::Zero();
  transformation_mat(3, 3) = 1.0;

  // Return as Isometry3d
  return Eigen::Isometry3d(transformation_mat);
}