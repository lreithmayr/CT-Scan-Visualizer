#ifndef CT_DATASET_H
#define CT_DATASET_H

#include "status.h"
#include "mylib.h"
#include "Eigen/Core"
#include "Eigen/Dense"

#include <QFile>
#include <QDebug>

#include <vector>
#include <cmath>
#include <cassert>

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

  /// Transform all voxels to 3-D vectors
  std::vector<Eigen::Vector3i> GetVoxelsInCTImage() const;

  /// Get a pointer to the non-3D rendered depth buffer
  [[nodiscard]] int16_t *GetDepthBuffer() const;

  /// Get a pointer to the 3D rendered image buffer
  [[nodiscard]] int16_t *RenderedDepthBuffer() const;

  /// Normalize pixel values to a pre-defined grey-value range
  static StatusOr<int> WindowInputValue(const int &input_value, const int &center, const int &window_size);

  /// Calculate the depth value for each pixel in the CT image
  Status CalculateDepthBuffer(int threshold);

  /// Render a shaded 3D image from the depth buffer
  Status RenderDepthBuffer();

  /// Extract HU value from a point specified as a vector
  [[nodiscard]] int GetGreyValue(const Eigen::Vector3i &pt) const;

  /// Threshold-based region growing algorithm for CT image segmentation
  std::vector<Eigen::Vector2i> RegionGrowing2D(Eigen::Vector2i &seed, int threshold) const;

 private:
  /// Height of the provided CT image (in pixels)
  int m_imgHeight;

  /// Width of the provided CT image
  int m_imgWidth;

  /// Number of depth layers of the provided CT image
  int m_layers;

  /// Buffer for the raw image data
  int16_t *m_imgData;

  /// Buffer for the calculated depth values
  int16_t *m_depthBuffer;

  /// Buffer for the rendered image
  int16_t *m_renderedDepthBuffer;

  /// Buffer for the region growing image
  int16_t *m_regionGrowingBuffer;
};

#endif  // CT_DATASET_H
