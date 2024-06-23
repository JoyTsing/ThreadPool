#pragma once

#include <optional>
#include <type_traits>

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
  Optional(T const& value) noexcept
      : m_has_value(true),
        m_value(std::move(value)) {}

  Optional(T&& value) noexcept
      : m_has_value(true),
        m_value(std::move(value)) {}

  Optional() noexcept
      : m_has_value(false) {}

  // our nullptr_t
  Optional(nullopt_t) noexcept
      : m_has_value(false) {}

  // std::nullptr_t
  Optional(std::nullopt_t) noexcept
      : m_has_value(false) {}

  // copy constructor
  Optional(const Optional& other)
      : m_has_value(other.m_has_value) {
    if (m_has_value) {
      new (&m_value) T(other.m_value);  // placement new
      // m_value = other.m_value; // 错误，这种写法调用的是赋值构造函数
    }
  }

  // move constructor
  Optional(Optional&& other) noexcept
      : m_has_value(other.m_has_value) {
    if (m_has_value) {
      new (&m_value) T(std::move(other.m_value));  // placement new
    }
  }

  ~Optional() {
    if (m_has_value) {
      m_value.~T();  // placement delete
    }
  }

  // copy&move assignment
  Optional& operator=(nullopt_t) noexcept {
    if (m_has_value) {
      m_value.~T();
      m_has_value = false;
    }
    return *this;
  }

  Optional& operator=(std::nullopt_t) noexcept {
    if (m_has_value) {
      m_value.~T();
      m_has_value = false;
    }
    return *this;
  }

  Optional& operator=(const T& value) noexcept {
    if (m_has_value) {
      m_value.~T();
      m_has_value = false;
    }
    m_has_value = true;
    new (&m_value) T(value);
    return *this;
  }

  Optional& operator=(T&& value) noexcept {
    if (m_has_value) {
      m_value.~T();
      m_has_value = false;
    }
    new (&m_value) T(std::move(value));
    m_has_value = true;
    return *this;
  }

  Optional& operator=(const Optional& other) {
    if (this == &other) return *this;
    if (m_has_value) {
      m_value.~T();
      m_has_value = false;
    }
    m_has_value = other.m_has_value;
    if (m_has_value) {
      new (&m_value) T(other.m_value);
    }
    return *this;
  }

  Optional& operator=(Optional&& other) noexcept {
    if (this == &other) return *this;
    if (m_has_value) {
      m_value.~T();
      m_has_value = false;
    }
    m_has_value = other.m_has_value;
    if (m_has_value) {
      new (&m_value) T(std::move(other.m_value));
      other.m_value.~T();
    }
    other.m_has_value = false;
    return *this;
  }

  const T& operator*() const& noexcept { return m_value; }

  T& operator*() & noexcept { return m_value; }

  const T&& operator*() const&& noexcept { return std::move(m_value); }

  T&& operator*() && noexcept { return std::move(m_value); }

  const T* operator->() const noexcept { return &m_value; }

  T* operator->() noexcept { return &m_value; }

  void reset() noexcept {
    if (m_has_value) {
      m_value.~T();
      m_has_value = false;
    }
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

  T value_or(T default_value) && noexcept(std::is_nothrow_move_assignable_v<T>) {
    if (m_has_value) return std::move(m_value);
    return default_value;
  }

  template <class... TArgs>
  void emplace(TArgs&&... args) {
    if (m_has_value) {
      m_value.~T();
      m_has_value = false;
    }
    // 保证即使构造失败，m_has_value也为false
    new (&m_value) T(std::forward<TArgs>(args)...);
    m_has_value = true;
  }

  template <class U, class... Ts>
  void emplace(std::initializer_list<U> ilist, Ts&&... value_args) {
    if (m_has_value) {
      m_value.~T();
      m_has_value = false;
    }
    new (&m_value) T(ilist, std::forward<Ts>(value_args)...);
    m_has_value = true;
  }

  private:
  bool m_has_value;
  // union to avoid default constructor of T
  union {
    T m_value;
  };
};
