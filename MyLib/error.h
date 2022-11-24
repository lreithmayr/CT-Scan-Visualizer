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

template <typename T, typename ErrorType>
class MYLIB_EXPORT ErrorOr {
 public:
  ErrorOr(T return_value)
      : m_returnValue(return_value), m_error(ErrorType::OK) {}
  ErrorOr(ErrorType error) : m_returnValue(), m_error(error) {}

  [[nodiscard]] T& value() { return m_returnValue; }
  [[nodiscard]] T const& value() const { return m_returnValue; }
  [[nodiscard]] ErrorType& error() { return m_error; }
  [[nodiscard]] ErrorType const& error() const { return m_error; }
  static bool IsError() { return (!ErrorType::OK); }

 private:
  T m_returnValue;
  ErrorType m_error;
};

template <typename ErrorType>
class ErrorOr<void, ErrorType> {
 public:
  ErrorOr(ErrorType error) : m_error(error) {}

  [[nodiscard]] ErrorType& error() { return m_error; }
  [[nodiscard]] ErrorType const& error() const { return m_error; }
  static bool IsError() { return (!ErrorType::OK); }

 private:
  ErrorType m_error;
};

#endif  // ERROR_H
