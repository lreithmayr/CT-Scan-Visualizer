#include "ct_dataset.h"

ErrorOr<void, ReturnCode> CTDataset::load(QString &img_path) {
  QFile img_file(img_path);
  bool fopen = img_file.open(QIODevice::ReadOnly);
  if (!fopen) {
      return {ReturnCode::FOPEN_ERROR};
  }

  img_file.read(reinterpret_cast<char *>(m_imageData),
                512 * 512 * 130 * sizeof(int16_t));
  img_file.close();
  return {ReturnCode::OK};
}
