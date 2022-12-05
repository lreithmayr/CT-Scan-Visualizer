#ifndef STATUS_H
#define STATUS_H

// FIXME: Wrap this whole thing in a namespace. Couldn't think of a witty name
// yet.

/**
 * @brief The StatusCode enum holds a collection of status and/or error codes
 * that can be returned by functions upon failure
 */

enum class StatusCode {
  /// The operation was performed without error
  OK,
  /// Windowing: The input value is out of range
  HU_OUT_OF_RANGE,
  /// Windowing: The center value is out of range
  CENTER_OUT_OF_RANGE,
  /// Windowing: The window size value is out of range
  WIDTH_OUT_OF_RANGE,
  /// Buffers: The buffer that is returned by the function is empty
  BUFFER_EMPTY,
  /// Files: Could not open file
  FOPEN_ERROR
};

/**
 * @brief The Status class represents a StatusCode and is meant for use in
 * return types
 * The whole class is [[nodiscard]], so the error
 * must be used by the caller.
 */
class [[nodiscard]] Status final {
 public:
  /// Default constructor initializes status as OK
  Status() : m_statusCode(StatusCode::OK) {}

  /// Constructs a status from the specified status code
  explicit Status(StatusCode rc) : m_statusCode(rc) {}

  /// Getter for the status code
  StatusCode code() const { return m_statusCode; }

  /// @return True if Status::OK, else False
  bool Ok() const { return (m_statusCode == StatusCode::OK); }

 private:
  StatusCode m_statusCode;
};

/**
 * @brief The StatusOr class can be used when a function returns either
 * an error or a return value. It is constructed either with a return value of
 * generic type T OR with a Status and holds either of the two, never both at
 * once.
 * The whole class is [[nodiscard]], so either the return value or the error
 * must be used by the caller.
 */
template <typename T>
class [[nodiscard]] StatusOr {
 public:
  /// Holds a return value (generic over T) in case of success.
  explicit StatusOr(T value) : m_value(value), m_status(StatusCode::OK) {}
  /// Holds a Status as defined above in case of failure.
  explicit StatusOr(Status stat) : m_status(stat) {}

  T &value() { return m_value; }
  T const &value() const { return m_value; }
  Status &status() { return m_status; }
  Status const &status() const { return m_status; }

  /// @returns True if StatusCode::OK, false otherwise.
  bool Ok() { return (m_status.code() == StatusCode::OK); }

  // NOTE: std::variant<T, Error> would be nicer here, but that's only in C++ 17
  // and up. Unions (tagged or otherwise) are evil (not type-safe), that's why
  // it's just two member variables for now.
 private:
  T m_value;
  Status m_status;
};

#endif  // STATUS_H
