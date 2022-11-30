#ifndef CT_DATASET_H
#define CT_DATASET_H

#include <QFile>

#include "status.h"

/**
 * @brief The CTDataset class contains the CT image Data as provided by the ICOM file format
 */

class MYLIB_EXPORT CTDataset {
 public:
  CTDataset();
  ~CTDataset();

  /// Load CT image Data from a specified path
  Status load(QString &img_path);

  /// Return image Data
  [[no_discard]] int16_t *Data() const;

  [[nodiscard]] int16_t *RenderedDepthBuffer() const;

  static StatusOr<int> WindowInputValue(const int &input_value, const int &center, const int &window_size);

  Status CalculateDepthBuffer(int threshold);

  Status RenderDepthBuffer();

 private:
  int m_imgHeight;
  int m_imgWidth;
  int m_layers;
  int16_t *m_imgData;
  int16_t *m_depthBuffer;
  int16_t *m_renderedDepthBuffer;
};

#endif  // CT_DATASET_H
