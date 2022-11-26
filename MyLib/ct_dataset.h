#ifndef CT_DATASET_H
#define CT_DATASET_H

#include <QFile>

#include "status.h"

/**
 * @brief The CTDataset class contains the CT image data as provided by the ICOM file format
 */

class MYLIB_EXPORT CTDataset {
 public:
  CTDataset() = default;
  ~CTDataset() { delete[] m_imageData; }

  /// Load CT image data from a specified path
  Status load(QString &img_path);

  /**
   * @details Accessor for image data member variable.
   * @return Pointer to the array of image data loaded in via load().
   */
  [[no_discard]] int16_t *data() const { return m_imageData; }

 private:
  int16_t *m_imageData{new int16_t[512 * 512 * 130]()};
};

#endif  // CT_DATASET_H
