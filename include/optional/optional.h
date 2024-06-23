#pragma once

#include <optional>

class BadOptionalAccess : public std::bad_optional_access {
  public:
  BadOptionalAccess() = default;
  virtual ~BadOptionalAccess() = default;

  const char* what() const noexcept override { return "bad optional access"; }
};

struct nullopt_t {
  explicit nullopt_t() = default;
};

inline constexpr nullopt_t nullopt;  // global nullopt

template <class T>
class Optional {
  public:
  Optional(T value)
      : m_has_value(true),
        m_value(value) {}

  Optional()
      : m_has_value(false) {}

  // our nullptr_t
  Optional(nullopt_t)
      : m_has_value(false) {}

  // std::nullptr_t
  Optional(std::nullopt_t)
      : m_has_value(false) {}

  ~Optional() {
    if (m_has_value) m_value.~T();
  }

  bool has_value() const { return m_has_value; }

  // distinguish lvalue and rvalue
  const T& value() const& {
    if (!m_has_value) throw BadOptionalAccess();
    return m_value;
  }

  T& value() & {
    if (!m_has_value) throw BadOptionalAccess();
    return m_value;
  }

  const T&& value() const&& {
    if (!m_has_value) throw BadOptionalAccess();
    return std::move(m_value);
  }

  T&& value() && {
    if (!m_has_value) throw BadOptionalAccess();
    return std::move(m_value);
  }

  // value_or
  T value_or(T default_value) const& {
    if (m_has_value) return m_value;
    return default_value;
  }

  T value_or(T default_value) && {
    if (m_has_value) return std::move(m_value);
    return default_value;
  }

  private:
  bool m_has_value;
  // union to avoid default constructor of T
  union {
    T m_value;
  };
};
