#include "ct_dataset.h"

CTDataset::CTDataset() :
  m_imgHeight(512),
  m_imgWidth(512),
  m_imgLayers(256),
  m_imgData(new int16_t[m_imgHeight * m_imgWidth * m_imgLayers]{0}),
  m_regionBuffer(new int[m_imgHeight * m_imgWidth * m_imgLayers]{0}),
  m_visitedBuffer(new int[m_imgHeight * m_imgWidth * m_imgLayers]{0}),
  m_depthBuffer(new int[m_imgHeight * m_imgWidth]{0}),
  m_renderedDepthBuffer(new int[m_imgHeight * m_imgWidth]{0}) {
}

CTDataset::~CTDataset() {
  delete[] m_imgData;
  delete[] m_regionBuffer;
  delete[] m_visitedBuffer;
  delete[] m_depthBuffer;
  delete[] m_renderedDepthBuffer;
}

/**
 * @details File location is specified via a GUI window selection
 * @param img_path The file path of the CT image.
 * @return StatusCode::OK if loading was succesfull, else StatusCode::FOPEN_ERROR.
 */
Status CTDataset::load(QString &img_path) {
  QFile img_file(img_path);
  bool fopen = img_file.open(QIODevice::ReadOnly);
  if (!fopen) {
	return Status(StatusCode::FOPEN_ERROR);
  }

  img_file.read(reinterpret_cast<char *>(m_imgData), m_imgHeight * m_imgWidth * m_imgLayers * sizeof(int16_t));
  img_file.close();
  return Status(StatusCode::OK);
}

/**
 * @return Pointer of type in16_t (short) to the image data array
 * @attention Null-checks and bounds-checks are caller's responsiblity
 */
int16_t *CTDataset::Data() const {
  return m_imgData;
}
/**
 * @return Pointer of type int16_t to the non-3D rendered depth buffer
 * @attention Null-checks and bounds-checks are caller's responsiblity
 */
int *CTDataset::GetDepthBuffer() const {
  return m_depthBuffer;
}

/**
 * @return Pointer of type int16_t to the 3d-rendered depth image buffer
 * @attention Null-checks and bounds-checks are caller's responsiblity
 */
int *CTDataset::GetRenderedDepthBuffer() const {
  return m_renderedDepthBuffer;
}

int *CTDataset::GetRegionGrowingBuffer() const {
  return m_regionBuffer;
}

void CTDataset::GetAllRenderedPoints() {
  m_allRenderedPoints.clear();

  Eigen::Vector3i rendered_point(0, 0, 0);
  for (int y = 0; y < m_imgHeight; ++y) {
	for (int x = 0; x < m_imgWidth; ++x) {
	  for (int d = 0; d < m_imgLayers; ++d) {
		int pt = x + y * m_imgWidth + (m_imgHeight * m_imgWidth * d);
		if (m_depthBuffer[pt] != 0) {
		  rendered_point.x() = x;
		  rendered_point.y() = y;
		  rendered_point.z() = d;
		  m_allRenderedPoints.push_back(rendered_point);
		}
	  }
	}
  }
}

/**
 * @details Windowing maps a desired slice of the raw grey values (in Hounsfield Units) to a an RGB scale from 0 to
 * \255. This makes it possible to highlight regions of interest such as bone, organ tissue or soft tissue which all
 * have a distinct range of HU values.
 * @param input_value The HU value read from the corresponding CT image pixel
 * @param center The center of the range window
 * @param window_size The size of the range window in which to normalize the HU values
 * @return StatusOr<int> depending on the error. Handles out-of-range HU values, center values and
 * window sizes. If
 * no error occured, the windowed HU value cast to an integer will be returned
 */
StatusOr<int> CTDataset::WindowInputValue(const int &input_value, const int &center, const int &window_size) {
  if ((input_value < -1024) || (input_value > 3071)) {
	return StatusOr<int>(Status(StatusCode::HU_OUT_OF_RANGE));
  }

  if ((center < -1024) || (center > 3071)) {
	return StatusOr<int>(Status(StatusCode::CENTER_OUT_OF_RANGE));
  }

  if ((window_size < 1) || (window_size > 4095)) {
	return StatusOr<int>(Status(StatusCode::WIDTH_OUT_OF_RANGE));
  }

  float half_window_size = 0.5f * static_cast<float>(window_size);
  int lower_bound = static_cast<float>(center) - half_window_size;
  int upper_bound = static_cast<float>(center) + half_window_size;

  if (input_value < lower_bound) {
	return StatusOr<int>(0);
  } else if (input_value > upper_bound) {
	return StatusOr<int>(255);
  }

  return StatusOr<int>(std::roundf((input_value - lower_bound) * (255.0f / static_cast<float>(window_size))));
}

/**
 * @details The calculation is accomplished by traversing all image layers for each pixel. If a pixel with an HU
 * value greater than a chosen threshold is reached, its depth value (the number of layer the pixel is on) is written
 * to a buffer. If no value greater than the threshold value was encountered, the maximum depth value is written to
 * the buffer.
 * @param threshold Pixel grey value (HU value) above which the depth value will be buffered.
 * @return StatusCode::OK if the result buffer is not empty, else StatusCode::BUFFER_EMPTY
 */
Status CTDataset::CalculateDepthBuffer(int threshold) {
  std::fill_n(m_depthBuffer, m_imgWidth * m_imgHeight, m_imgLayers - 1);
  m_allRenderedPoints.clear();
  Eigen::Vector3i rendered_point(0, 0, 0);
  for (int y = 0; y < m_imgHeight; ++y) {
	for (int x = 0; x < m_imgWidth; ++x) {
	  for (int d = 0; d < m_imgLayers; ++d) {
		int curr_pt = (x + y * m_imgWidth) + (m_imgHeight * m_imgWidth * d);
		if (m_imgData[curr_pt] >= threshold) {
		  m_depthBuffer[x + y * m_imgWidth] = d;
		  rendered_point.x() = x;
		  rendered_point.y() = y;
		  rendered_point.z() = d;
		  m_allRenderedPoints.push_back(rendered_point);
		  break;
		}
	  }
	}
  }
  if (m_depthBuffer == nullptr) {
	return Status(StatusCode::BUFFER_EMPTY);
  }
  return Status(StatusCode::OK);
}

Status CTDataset::CalculateDepthBufferFromRegionGrowing(Eigen::Matrix3d &rotation_mat) {
  std::fill_n(m_depthBuffer, m_imgWidth * m_imgHeight, m_imgLayers - 1);
  std::cout << rotation_mat.format(CleanFmt) << "\n=====\n";

  if (m_surfacePoints.empty()) {
	qDebug() << "No surface points!" << "\n";
	return Status(StatusCode::BUFFER_EMPTY);
  }

  Eigen::Vector3d pt_rot(0, 0, 0);
  for (auto &point : m_surfacePoints) {
	pt_rot = (rotation_mat * (point.cast<double>() - m_regionVolumeCenter)) + m_regionVolumeCenter;
	if ((static_cast<int>(pt_rot.x()) < m_imgWidth && static_cast<int>(pt_rot.x()) >= 0) &&
	  (static_cast<int>(pt_rot.y()) < m_imgHeight && static_cast<int>(pt_rot.y()) >= 0)) {
	  m_depthBuffer[static_cast<int>(pt_rot.x()) + static_cast<int>(pt_rot.y()) * m_imgWidth]
		= m_depthBuffer[(static_cast<int>(pt_rot.x()) - 1) + static_cast<int>(pt_rot.y()) * m_imgWidth]
		= m_depthBuffer[(static_cast<int>(pt_rot.x()) + 1) + static_cast<int>(pt_rot.y()) * m_imgWidth]
		= m_depthBuffer[static_cast<int>(pt_rot.x()) + (static_cast<int>(pt_rot.y()) - 1) * m_imgWidth]
		= m_depthBuffer[static_cast<int>(pt_rot.x()) + (static_cast<int>(pt_rot.y()) + 1) * m_imgWidth]
		= static_cast<int>(pt_rot.z());
	}
  }

  if (m_depthBuffer == nullptr) {
	qDebug() << "Depth buffer empty!" << "\n";
	return Status(StatusCode::BUFFER_EMPTY);
  }

  return Status(StatusCode::OK);
}

/**
 * @details The 3D image is rendered by computing the depth-value gradient in x and y for each pixel (in essence,
 * computing the dot product). The step-size
 * of the algorithm is two, i.e. each pixel is compared to it's left and right as well as it's above and below
 * neighbor. The result is then normalized, multiplied by 255 to yield a valid RGB value and written to an ouput buffer.
 * @return StatusCode::OK if the result buffer is not empty, else StatusCode::BUFFER_EMPTY
 */
Status CTDataset::RenderDepthBuffer() {
  if (m_depthBuffer == nullptr) {
	qDebug() << "Depth buffer empty!" << "\n";
	return Status(StatusCode::BUFFER_EMPTY);
  }

  int s_x = 8;
  int s_y = 8;
  int T_x = 0;
  int T_y = 0;
  double sxTy_sq = 0;
  double syTx_sq = 0;
  double nom = 255 * s_x * s_y;
  double denom = 0;
  double inv = 0;
  int I_ref = 0;

  double s_x_sq = s_x * s_x;
  double s_y_sq = s_y * s_y;
  double s_pow_four = s_x_sq * s_y_sq;

  for (int y = 0; y < m_imgHeight; ++y) {
	for (int x = 0; x < m_imgWidth; ++x) {
	  auto current_point = x + y * m_imgWidth;
	  T_x = m_depthBuffer[(x + 1) + y * m_imgWidth] - m_depthBuffer[(x - 1) + y * m_imgWidth];
	  T_y = m_depthBuffer[x + (y + 1) * m_imgWidth] - m_depthBuffer[x + (y - 1) * m_imgWidth];
	  syTx_sq = s_y_sq * T_x * T_x;
	  sxTy_sq = s_x_sq * T_y * T_y;
	  denom = std::sqrt(syTx_sq + sxTy_sq + s_pow_four);
	  inv = 1 / denom;
	  I_ref = nom * inv;
	  m_renderedDepthBuffer[current_point] = I_ref;
	}
  }

  if (m_renderedDepthBuffer == nullptr) {
	qDebug() << "Depth buffer couldn't be rendered!" << "\n";
	return Status(StatusCode::BUFFER_EMPTY);
  }

  return Status(StatusCode::OK);
}

int CTDataset::GetGreyValue(const Eigen::Vector3i &pt) const {
  return m_imgData[(pt.x() + pt.y() * m_imgWidth) + (m_imgHeight * m_imgWidth * pt.z())];
}

Status CTDataset::FindSurfacePoints() {
  if (m_regionBuffer == nullptr) {
	return Status(StatusCode::BUFFER_EMPTY);
  }
  m_surfacePoints.clear();

  for (int y = 0; y < m_imgHeight; ++y) {
	for (int x = 0; x < m_imgWidth; ++x) {
	  for (int d = 0; d < m_imgLayers; ++d) {
		int pt = x + y * m_imgWidth + (m_imgHeight * m_imgWidth * d);
		if (m_regionBuffer[pt] == 1) {
		  if (m_regionBuffer[(x - 1) + y * m_imgWidth + (m_imgHeight * m_imgWidth * d)] == 1
			&& m_regionBuffer[(x + 1) + y * m_imgWidth + (m_imgHeight * m_imgWidth * d)] == 1
			&& m_regionBuffer[x + (y - 1) * m_imgWidth + (m_imgHeight * m_imgWidth * d)] == 1
			&& m_regionBuffer[x + (y + 1) * m_imgWidth + (m_imgHeight * m_imgWidth * d)] == 1
			&& m_regionBuffer[x + y * m_imgWidth + (m_imgHeight * m_imgWidth * (d - 1))] == 1
			&& m_regionBuffer[x + y * m_imgWidth + (m_imgHeight * m_imgWidth * (d + 1))] == 1) {
			continue;
		  }
		  Eigen::Vector3i surface_point;
		  surface_point.x() = x;
		  surface_point.y() = y;
		  surface_point.z() = d;
		  m_surfacePoints.push_back(surface_point);
		}
	  }
	}
  }
  return Status(StatusCode::OK);
}

Status CTDataset::FindPointCloudCenter() {
  if (m_regionBuffer == nullptr) {
	return Status(StatusCode::BUFFER_EMPTY);
  }

  m_allPointsInRegion.clear();

  Eigen::Vector3i region_point(0, 0, 0);
  for (int y = 0; y < m_imgHeight; ++y) {
	for (int x = 0; x < m_imgWidth; ++x) {
	  for (int d = 0; d < m_imgLayers; ++d) {
		int pt = x + y * m_imgWidth + (m_imgHeight * m_imgWidth * d);
		if (m_regionBuffer[pt] == 1) {
		  region_point.x() = x;
		  region_point.y() = y;
		  region_point.z() = d;
		  m_allPointsInRegion.push_back(region_point);
		}
	  }
	}
  }

  int64_t x_tot = 0;
  int64_t y_tot = 0;
  int64_t z_tot = 0;
  for (auto &pt : m_allPointsInRegion) {
	x_tot += pt.x();
	y_tot += pt.y();
	z_tot += pt.z();
  }

  auto region_size = static_cast<double>(m_allPointsInRegion.size());

  m_regionVolumeCenter = Eigen::Vector3d(static_cast<double>(x_tot) / region_size,
										 static_cast<double>(y_tot) / region_size,
										 static_cast<double>(z_tot) / region_size);

  return Status(StatusCode::OK);
}

void CTDataset::RegionGrowing3D(Eigen::Vector3i &seed, int threshold) {
  std::fill_n(m_regionBuffer, m_imgHeight * m_imgWidth * m_imgLayers, 0);
  qDebug() << "Starting region growing algorithm!" << "\n";
  auto t1 = std::chrono::high_resolution_clock::now();

  std::stack<Eigen::Vector3i> stack;
  std::vector<Eigen::Vector3i> neighbors;

  stack.push(seed);
  while (!stack.empty()) {
	m_regionBuffer[seed.x() + seed.y() * m_imgWidth + (m_imgHeight * m_imgWidth * seed.z())] = 1;
	stack.pop();

	MyLib::FindNeighbors3D(seed, neighbors);
	for (auto &nb : neighbors) {
	  // 0: Voxel has not been visited
	  if (m_regionBuffer[nb.x() + nb.y() * m_imgWidth + (m_imgHeight * m_imgWidth * nb.z())] == 0) {
		// 2: Voxel has been visited
		m_regionBuffer[nb.x() + nb.y() * m_imgWidth + (m_imgHeight * m_imgWidth * nb.z())] = 2;
		if (GetGreyValue(nb) >= threshold) {
		  // 1: Voxel belongs to region
		  m_regionBuffer[nb.x() + nb.y() * m_imgWidth + (m_imgHeight * m_imgWidth * nb.z())] = 1;
		  stack.push(nb);
		}
	  }
	}
	if (!stack.empty()) {
	  seed = stack.top();
	}
  }
  auto t2 = std::chrono::high_resolution_clock::now();
  auto duration_ms = std::chrono::duration<double, std::milli>(t2 - t1);
  qDebug() << "Region growing took: " << duration_ms.count() << "ms\n";

  if (FindSurfacePoints().Ok()) {
	qDebug() << m_surfacePoints.size() << " surface points calculated!" << "\n";
  }
  if (FindPointCloudCenter().Ok()) {
	qDebug() << m_allPointsInRegion.size() << " total points in the region!" << "\n";
	qDebug() << "Centroid: " << m_regionVolumeCenter.x() << m_regionVolumeCenter.y() << m_regionVolumeCenter.z()
			 << "\n";
  }
}

Eigen::Isometry3d CTDataset::EstimateRigidTransformation3D(const std::vector<Eigen::Vector3d> &source_points,
														   const std::vector<Eigen::Vector3d> &target_points) {
  typedef Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic> MatrixXd;

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

