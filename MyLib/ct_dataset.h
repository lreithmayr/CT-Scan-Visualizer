#ifndef CT_DATASET_H
#define CT_DATASET_H

#include "status.h"
#include "mylib.h"
#include "Eigen/Core"
#include "Eigen/Dense"

#include <QFile>
#include <QDebug>
#include <QPoint>

#include <stack>
#include <cmath>
#include <cassert>
#include <chrono>

/**
 * @brief The CTDataset class is the central class to initialize and process CT scan images.
 * @details
 * CTDataset provides methods for loading the raw CT image files, processing the grey values contained within it, and
 * rendering 3D representations of the image data.
 */

class CTDataset {
 public:
  CTDataset();
  ~CTDataset();

  /// Load CT image data from the specified file path
  Status load(QString &img_path);

  /// Get a pointer to the image data
  [[nodiscard]] int16_t *Data() const;

  /// Get a pointer to the non-3D rendered depth buffer
  [[nodiscard]] int *GetDepthBuffer() const;

  /// Get a pointer to the 3D rendered image buffer
  [[nodiscard]] int *GetRenderedDepthBuffer() const;

  /// Get a pointer to the region growing buffer
  [[nodiscard]] int *GetRegionGrowingBuffer() const;

  /// Calculates all rendered points and saves them in a member vector
  void CalculateAllRenderedPoints();

  /// Normalize pixel values to a pre-defined grey-value range
  static StatusOr<int> WindowInputValue(const int input_value, const int center, const int window_size);

  /// Calculate the depth value for each pixel in the CT image
  Status CalculateDepthBuffer(int const threshold);

  /// Calculate the depth value for each pixel in the region determined by region growing
  Status CalculateDepthBufferFromRegionGrowing(Eigen::Matrix3d const &rotation_mat);

  /// Render a shaded 3D image from the depth buffer
  Status RenderDepthBuffer();

  /// Extract HU value from a 3D point specified as a vector
  [[nodiscard]] int GetGreyValue(Eigen::Vector3i const &pt) const;

  /// 3D region growing algorithm
  void RegionGrowing3D(Eigen::Vector3i &seed, int const threshold);

  /// Saves all points from the region growing algorithm in a member vector
  void AggregatePointsInRegion();

  /// Traverses all region growing points and determines the surface points
  Status FindSurfacePoints();

  /// Traverses all points in the region and computes the average of their coordinates
  Status FindPointCloudCenter();

 private:
  /// Height of the provided CT image (in pixels)
  int m_imgHeight;

  /// Width of the provided CT image
  int m_imgWidth;

  /// Number of depth layers of the provided CT image
  int m_imgLayers;

  /// Buffer for the raw image data
  int16_t *m_imgData;

  /// Buffer for the calculated depth values
  int *m_depthBuffer;

  /// Buffer for the rendered image
  int *m_renderedDepthBuffer;

  /// Buffer for the region growing image
  int *m_regionBuffer;

  /// Buffer for visited points during RG
  int *m_visitedBuffer;

  /// Surface points of the region determined by RG
  std::vector<Eigen::Vector3i> m_surfacePoints;

  /// All points fo the region growing region
  std::vector<Eigen::Vector3i> m_allPointsInRegion;

  /// All points with rendered elements
  std::vector<Eigen::Vector3i> m_allRenderedPoints;

  /// Barycentric coordinates of the point cloud produced by region growing
  Eigen::Vector3d m_regionVolumeCenter;
};

#endif  // CT_DATASET_H
