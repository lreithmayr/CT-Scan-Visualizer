#ifndef CT_DATASET_H
#define CT_DATASET_H

#include "status.h"
#include "mylib.h"
#include "Eigen/Core"
#include "Eigen/Dense"

#include <QFile>
#include <QDebug>

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

  [[nodiscard]] int *GetRegionGrowingBuffer() const;

  void GetAllRenderedPoints();

  /// Normalize pixel values to a pre-defined grey-value range
  static StatusOr<int> WindowInputValue(const int &input_value, const int &center, const int &window_size);

  /// Calculate the depth value for each pixel in the CT image
  Status CalculateDepthBuffer(int threshold);

  Status CalculateDepthBufferFromRegionGrowing(Eigen::Matrix3d &rotation_mat);

  /// Render a shaded 3D image from the depth buffer
  Status RenderDepthBuffer();

  /// Extract HU value from a 3D point specified as a vector
  [[nodiscard]] int GetGreyValue(const Eigen::Vector3i &pt) const;

  /// 3D region growing algorithm
  void RegionGrowing3D(Eigen::Vector3i &seed, int threshold);

  Status FindSurfacePoints();

  Status FindPointCloudCenter();

  void ResetBuffers();

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

  int *m_visitedBuffer;

  int *m_surfacePointBuffer;

  std::vector<Eigen::Vector3i> m_surfacePoints;

  std::vector<Eigen::Vector3i> m_allPointsInRegion;

  Eigen::Vector3d m_regionVolumeCenter;

  int m_minDepth{m_imgLayers - 1};

};

#endif  // CT_DATASET_H
