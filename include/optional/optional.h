#pragma once

#include <optional>
#include <type_traits>
#include <utility>

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
struct InPlace {
  explicit InPlace() = default;
};

inline constexpr InPlace in_place;

template <class T>
class Optional {
  public:
  template <class... Ts>
  explicit Optional(InPlace, Ts&&... value_args)
      : m_has_value(true),
        m_value(std::forward<Ts>(value_args)...) {}

  template <class U, class... Ts>
  explicit Optional(InPlace, std::initializer_list<U> ilist, Ts&&... value_args)
      : m_has_value(true),
        m_value(ilist, std::forward<Ts>(value_args)...) {}

  template <class... Ts>
  explicit Optional(std::in_place_t, Ts&&... value_args)
      : m_has_value(true),
        m_value(std::forward<Ts>(value_args)...) {}

  template <class U, class... Ts>
  explicit Optional(std::in_place_t, std::initializer_list<U> ilist, Ts&&... value_args)
      : m_has_value(true),
        m_value(ilist, std::forward<Ts>(value_args)...) {}

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

  // bool operator
  explicit operator bool() const { return m_has_value; }

  bool operator==(nullopt_t) const noexcept { return !m_has_value; }
  bool operator==(std::nullopt_t) const noexcept { return !m_has_value; }

  bool operator!=(nullopt_t) const noexcept { return m_has_value; }
  bool operator!=(std::nullopt_t) const noexcept { return m_has_value; }

  friend bool operator==(nullopt_t, const Optional& self) noexcept { return !self.m_has_value; }
  friend bool operator==(std::nullopt_t, const Optional& self) noexcept {
    return !self.m_has_value;
  }

  friend bool operator!=(nullopt_t, const Optional& self) noexcept { return self.m_has_value; }
  friend bool operator!=(std::nullopt_t, const Optional& self) noexcept { return self.m_has_value; }

  bool operator==(const Optional& other) const noexcept {
    if (m_has_value != other.m_has_value) {
      return false;
    }
    if (m_has_value) {
      return m_value == other.m_value;
    }
    return true;
  }

  bool operator!=(Optional const& that) const noexcept {
    if (m_has_value != that.m_has_value) {
      return true;
    }
    if (m_has_value) {
      return m_value != that.m_value;
    }
    return false;
  }

  bool operator>(Optional const& that) const noexcept {
    if (!m_has_value || !that.m_has_value) return false;
    return m_value > that.m_value;
  }

  bool operator<(Optional const& that) const noexcept {
    if (!m_has_value || !that.m_has_value) return false;
    return m_value < that.m_value;
  }

  bool operator>=(Optional const& that) const noexcept {
    if (!m_has_value || !that.m_has_value) return true;
    return m_value >= that.m_value;
  }

  bool operator<=(Optional const& that) const noexcept {
    if (!m_has_value || !that.m_has_value) return true;
    return m_value <= that.m_value;
  }

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

  template <class F>
  auto and_then(F&& f) const& {
    using RetType = std::remove_cv_t<std::remove_reference_t<std::invoke_result_t<F, T>>>;
    if (m_has_value) {
      return std::forward<F>(f)(m_value);
    } else {
      return RetType{};
    }
  }

  template <class F>
  auto and_then(F&& f) & {
    using RetType = std::remove_cv_t<std::remove_reference_t<std::invoke_result_t<F, T>>>;
    if (m_has_value) {
      return std::forward<F>(f)(m_value);
    } else {
      return RetType{};
    }
  }

  template <class F>
  auto and_then(F&& f) const&& {
    using RetType = std::remove_cv_t<std::remove_reference_t<std::invoke_result_t<F, T>>>;
    if (m_has_value) {
      return std::forward<F>(f)(std::move(m_value));
    } else {
      return RetType{};
    }
  }

  template <class F>
  auto and_then(F&& f) && {
    using RetType = std::remove_cv_t<std::remove_reference_t<std::invoke_result_t<F, T>>>;
    if (m_has_value) {
      return std::forward<F>(f)(std::move(m_value));
    } else {
      return RetType{};
    }
  }

  template <class F>
  auto transform(F&& f)
      const& -> Optional<std::remove_cv_t<std::remove_reference_t<std::invoke_result_t<F, T>>>> {
    if (m_has_value) {
      return std::forward<F>(f)(m_value);
    } else {
      return nullopt;
    }
  }

  template <class F>
  auto transform(
      F&& f) & -> Optional<std::remove_cv_t<std::remove_reference_t<std::invoke_result_t<F, T>>>> {
    if (m_has_value) {
      return std::forward<F>(f)(m_value);
    } else {
      return nullopt;
    }
  }

  template <class F>
  auto transform(F&& f)
      const&& -> Optional<std::remove_cv_t<std::remove_reference_t<std::invoke_result_t<F, T>>>> {
    if (m_has_value) {
      return std::forward<F>(f)(std::move(m_value));
    } else {
      return nullopt;
    }
  }

  template <class F>
  auto transform(
      F&& f) && -> Optional<std::remove_cv_t<std::remove_reference_t<std::invoke_result_t<F, T>>>> {
    if (m_has_value) {
      return std::forward<F>(f)(std::move(m_value));
    } else {
      return nullopt;
    }
  }

  template <class F, std::enable_if_t<std::is_copy_constructible_v<T>, int> = 0>
  Optional or_else(F&& f) const& {
    if (m_has_value) {
      return *this;
    } else {
      return std::forward<F>(f)();
    }
  }

  template <class F, std::enable_if_t<std::is_move_constructible_v<T>, int> = 0>
  Optional or_else(F&& f) && {
    if (m_has_value) {
      return std::move(*this);
    } else {
      return std::forward<F>(f)();
    }
  }

  void swap(Optional& that) noexcept {
    if (m_has_value && that.m_has_value) {
      using std::swap;  // ADL
      swap(m_value, that.m_value);
    } else if (!m_has_value && !that.m_has_value) {
      // do nothing
    } else if (m_has_value) {
      that.emplace(std::move(m_value));
      reset();
    } else {
      emplace(std::move(that.m_value));
      that.reset();
    }
  }

  private:
  bool m_has_value;
  // union to avoid default constructor of T
  union {
    T m_value;
  };
};

#if __cpp_deduction_guides
template <class T>  // CTAD
Optional(T) -> Optional<T>;
#endif

template <class T>
Optional<T> make_optional(T value) {
  return Optional<T>(std::move(value));
}