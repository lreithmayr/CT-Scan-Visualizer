#ifndef CT_DATASET_H
#define CT_DATASET_H

#include <QFile>

#include "error.h"

/**
 * @brief The CTDataset class contains the CT image data as provided by the ICOM file format
 */

class MYLIB_EXPORT CTDataset {
 public:
  CTDataset() = default;
  ~CTDataset() { delete[] m_imageData; }

  ErrorOr<void, ReturnCode> load(QString &img_path);
  [[no_discard]] int16_t *data() const { return m_imageData; }

 private:
  int16_t *m_imageData{new int16_t[512 * 512 * 130]()};
};

#endif  // CT_DATASET_H
