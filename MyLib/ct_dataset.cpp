#include "ct_dataset.h"

/**
 * @details Loads in an image file at the location specified via img_path
 * @param img_path The file path of the CT image.
 * @return Returns an ErrorOr type. If loading was successful, ReturnCode::OK will be returned, else ReturnCode::FOPEN_ERROR.
 */
Status CTDataset::load(QString &img_path) {
  QFile img_file(img_path);
  bool fopen = img_file.open(QIODevice::ReadOnly);
  if (!fopen) {
      return Status(StatusCode::FOPEN_ERROR);
  }

  img_file.read(reinterpret_cast<char *>(m_imageData),
                512 * 512 * 130 * sizeof(int16_t));
  img_file.close();
  return Status(StatusCode::OK);
}
