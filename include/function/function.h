#pragma once
#include <functional>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>

template <class Fnsig>  // 如果模板参数仅为一个没有传入参数，则报错
struct Function {
  static_assert(!std::is_same_v<Fnsig, Fnsig>,
                "not a valid function signature");
};

template <class Ret, class... Args>
struct Function<Ret(Args...)> {
 private:
  struct FunctionBase {
    virtual Ret call(Args... args) = 0;  // 类型擦除后的统一接口
    virtual ~FunctionBase() = default;   // 应对F可能有非平凡析构的情况
  };

  template <class F>
  struct FunctionImpl : FunctionBase {
    F m_f;
    FunctionImpl(F f) : m_f(std::move(f)) {}
    virtual Ret call(Args... args) override {
      return std::invoke(
          m_f, std::forward<Args>(
                   args)...);  // 完美转发所有参数给构造时保存的仿函数对象
    }
  };

  std::shared_ptr<FunctionBase>
      m_base;  // 使用智能指针管理仿函数对象，用shared而不是unique是为了让Function支持拷贝

 public:
  Function() = default;
  // 阻止 Function 从不可调用的对象中初始化
  template <class F,
            class = std::enable_if_t<std::is_invocable_r_v<Ret, F&, Args...>>>
  Function(F f)
      : m_base(std::make_shared<FunctionImpl<F>>(std::move(
            f))){};  // 没有 explicit，允许 lambda 表达式隐式转换成 Function

  Ret operator()(Args... args) const {
    if (!m_base) [[unlikely]]
      throw std::runtime_error("function pointer not initialized");
    return m_base->call(std::forward<Args>(args)...);
  }
};