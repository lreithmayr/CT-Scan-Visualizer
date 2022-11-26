#ifndef ERROR_H
#define ERROR_H

#include "MyLib_global.h"

enum class ReturnCode {
  OK,
  HU_OUT_OF_RANGE,
  CENTER_OUT_OF_RANGE,
  WIDTH_OUT_OF_RANGE,
  BUFFER_EMPTY,
  FOPEN_ERROR
};

template <typename T>
class MYLIB_EXPORT ErrorOr {
 public:
  ErrorOr(T value) : m_value(value), m_error(ReturnCode::OK) {}
  ErrorOr(ReturnCode error) : m_error(error) {}

  [[nodiscard]] T& value() { return m_value; }
  [[nodiscard]] T const& value() const { return m_value; }
  [[nodiscard]] ReturnCode& error() { return m_error; }
  [[nodiscard]] ReturnCode const& error() const { return m_error; }
  bool Ok() { return (m_error == ReturnCode::OK); }

 private:
  T m_value;
  ReturnCode m_error;
};

template <>
class ErrorOr<void> : public ErrorOr<int> {
 public:
  using ErrorOr<int>::ErrorOr;
};

#endif  // ERROR_H
