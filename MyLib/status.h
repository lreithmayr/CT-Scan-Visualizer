#ifndef STATUS_H
#define STATUS_H

#include "MyLib_global.h"

enum class StatusCode {
  OK,
  HU_OUT_OF_RANGE,
  CENTER_OUT_OF_RANGE,
  WIDTH_OUT_OF_RANGE,
  BUFFER_EMPTY,
  FOPEN_ERROR
};

class [[nodiscard]] Status final {
 public:
  /// Default constructor initlaizes status as OK
  Status() : m_statusCode(StatusCode::OK) {}

  /// Constructs a status from the specified status code
  Status(StatusCode rc) : m_statusCode(rc) {}

  StatusCode code() const { return m_statusCode; }
  bool Ok() const { return (m_statusCode == StatusCode::OK); }

 private:
  StatusCode m_statusCode;
};

template <typename T>
class [[nodiscard]] StatusOr {
 public:
  StatusOr(T value) : m_value(value), m_status(StatusCode::OK) {}
  StatusOr(Status stat) : m_status(stat) {}

  T& value() { return m_value; }
  T const& value() const { return m_value; }
  Status& status() { return m_status; }
  Status const& status() const { return m_status; }
  bool Ok() { return (m_status.code() == StatusCode::OK); }

 private:
  T m_value;
  Status m_status;
};

template <>
class StatusOr<void> : public StatusOr<int> {
 public:
  using StatusOr<int>::StatusOr;
};

#endif  // STATUS_H
