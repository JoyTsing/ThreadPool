#include <cxxabi.h>
#include <execinfo.h>
#include <unistd.h>

#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <mutex>

#include "hook/alloc_action.h"
#include "utils/addr2symbol.h"

namespace {
struct GlobalHookCheck {
  bool enable = false;
  std::mutex lock;
  std::map<void *, AllocAction> allocated;

  GlobalHookCheck() { enable = true; }

  ~GlobalHookCheck() {
    enable = false;
    for (auto &&[ptr, info] : allocated) {
      printf("memory leak, ptr = %p, size = %zd, caller = %s\n", ptr, info.size,
             addr_to_symbol(info.caller).c_str());
    }
  }

  void on(AllocOp op, void *ptr, size_t size, size_t align, void *caller) {
    if (ptr) {
      // printf("%s(ptr=%p, size=%zd, align=%zd, caller=%p)\n",
      // kAllocOpNames[(size_t)op], ptr, size, align, caller);
      std::lock_guard<std::mutex> guard(lock);
      if (kAllocOpIsAllocation[(size_t)op]) {
        if (!allocated
                 .insert({ptr, AllocAction{op, 0, ptr, size, align, caller, 0}})
                 .second) {
          printf(
              "检测到内存多次分配同一个地址 ptr = %p, size = %zd, "
              "caller = %s\n",
              ptr, size, addr_to_symbol(caller).c_str());
        }
      } else {
        auto it = allocated.find(ptr);
        if (it == allocated.end()) {
          printf(
              "检测到尝试释放不存在的内存 ptr = %p, size = %zd, "
              "caller = %s\n",
              ptr, size, addr_to_symbol(caller).c_str());
        } else {
          if (kAllocOpFreeFunction[(size_t)it->second.op] != op) {
            printf(
                "检测到内存释放时使用了错误的释放函数 ptr = %p, "
                "size = %zd, caller = %s\n",
                ptr, size, addr_to_symbol(caller).c_str());
          }
          if (size != kNone) {
            if (it->second.size != size) {
              printf(
                  "检测到内存释放时指定了错误的大小 ptr = %p, "
                  "size = %zd, caller = %s\n",
                  ptr, size, addr_to_symbol(caller).c_str());
            }
          }
          if (align != kNone) {
            if (it->second.align != align) {
              printf(
                  "检测到内存释放时指定了错误的对齐 ptr = %p, "
                  "size = %zd, align = %zd, caller = %s\n",
                  ptr, size, align, addr_to_symbol(caller).c_str());
            }
          }
          allocated.erase(it);
        }
      }
    }
  }

} __global_hook;

struct EnableGuard {
  bool was_enable;

  EnableGuard() {
    was_enable = __global_hook.enable;
    __global_hook.enable = false;
  }

  explicit operator bool() { return was_enable; }

  ~EnableGuard() { __global_hook.enable = was_enable; }
};

}  // namespace

#if __GNUC__

extern "C" void *__libc_malloc(size_t size) noexcept;
extern "C" void __libc_free(void *ptr) noexcept;
extern "C" void *__libc_calloc(size_t nmemb, size_t size) noexcept;
extern "C" void *__libc_realloc(void *ptr, size_t size) noexcept;
extern "C" void *__libc_reallocarray(void *ptr, size_t nmemb,
                                     size_t size) noexcept;
extern "C" void *__libc_valloc(size_t size) noexcept;
extern "C" void *__libc_memalign(size_t align, size_t size) noexcept;

#define REAL_LIBC(name) __libc_##name
#define MAY_OVERRIDE_MALLOC 1
#define MAY_SUPPORT_MEMALIGN 1
#define RETURN_ADDRESS __builtin_return_address(0)
#else
#define REAL_LIBC(name) name
#define MAY_OVERRIDE_MALLOC 0
#define MAY_SUPPORT_MEMALIGN 0
#define RETURN_ADDRESS ((void *)1)
#endif

#if MAY_OVERRIDE_MALLOC
extern "C" void *malloc(size_t size) noexcept {
  EnableGuard ena;
  void *ptr = REAL_LIBC(malloc)(size);
  if (ena) {
    __global_hook.on(AllocOp::Malloc, ptr, size, kNone, RETURN_ADDRESS);
  }
  return ptr;
}

extern "C" void free(void *ptr) noexcept {
  EnableGuard ena;
  if (ena) {
    __global_hook.on(AllocOp::Free, ptr, kNone, kNone, RETURN_ADDRESS);
  }
  REAL_LIBC(free)(ptr);
}

extern "C" void *calloc(size_t nmemb, size_t size) noexcept {
  EnableGuard ena;
  void *ptr = REAL_LIBC(calloc)(nmemb, size);
  if (ena) {
    __global_hook.on(AllocOp::Malloc, ptr, nmemb * size, kNone, RETURN_ADDRESS);
  }
  return ptr;
}

extern "C" void *realloc(void *ptr, size_t size) noexcept {
  EnableGuard ena;
  void *new_ptr = REAL_LIBC(realloc)(ptr, size);
  if (ena) {
    __global_hook.on(AllocOp::Malloc, new_ptr, size, kNone, RETURN_ADDRESS);
    if (new_ptr) {
      __global_hook.on(AllocOp::Free, ptr, kNone, kNone, RETURN_ADDRESS);
    }
  }
  return new_ptr;
}

extern "C" void *reallocarray(void *ptr, size_t nmemb, size_t size) noexcept {
  EnableGuard ena;
  void *new_ptr = REAL_LIBC(reallocarray)(ptr, nmemb, size);
  if (ena) {
    __global_hook.on(AllocOp::Malloc, new_ptr, nmemb * size, kNone,
                     RETURN_ADDRESS);
    if (new_ptr) {
      __global_hook.on(AllocOp::Free, ptr, kNone, kNone, RETURN_ADDRESS);
    }
  }
  return new_ptr;
}
#endif

#if MAY_SUPPORT_MEMALIGN
extern "C" void *valloc(size_t size) noexcept {
  EnableGuard ena;
  void *ptr = REAL_LIBC(valloc)(size);
  if (ena) {
#if __unix__
    size_t pagesize = sysconf(_SC_PAGESIZE);
#else
    size_t pagesize = 0;
#endif
    __global_hook.on(AllocOp::Malloc, ptr, size, pagesize, RETURN_ADDRESS);
  }
  return ptr;
}

extern "C" void *memalign(size_t align, size_t size) noexcept {
  EnableGuard ena;
  void *ptr = REAL_LIBC(memalign)(align, size);
  if (ena) {
    __global_hook.on(AllocOp::Malloc, ptr, size, align, RETURN_ADDRESS);
  }
  return ptr;
}

extern "C" void *aligned_alloc(size_t align, size_t size) noexcept {
  EnableGuard ena;
  void *ptr = REAL_LIBC(memalign)(align, size);
  if (ena) {
    __global_hook.on(AllocOp::Malloc, ptr, size, align, RETURN_ADDRESS);
  }
  return ptr;
}

extern "C" int posix_memalign(void **memptr, size_t align,
                              size_t size) noexcept {
  EnableGuard ena;
  void *ptr = REAL_LIBC(memalign)(align, size);
  if (ena) {
    __global_hook.on(AllocOp::Malloc, *memptr, size, align, RETURN_ADDRESS);
  }
  int ret = 0;
  if (!ptr) {
    ret = errno;
  } else {
    *memptr = ptr;
  }
  return ret;
}
#endif

void operator delete(void *ptr) noexcept {
  EnableGuard ena;
  if (ena) {
    __global_hook.on(AllocOp::Delete, ptr, kNone, kNone,
                     __builtin_return_address(0));
  }
  REAL_LIBC(free)(ptr);
}

void operator delete[](void *ptr) noexcept {
  EnableGuard ena;
  if (ena) {
    __global_hook.on(AllocOp::DeleteArray, ptr, kNone, kNone,
                     __builtin_return_address(0));
  }
  REAL_LIBC(free)(ptr);
}

void operator delete(void *ptr, std::nothrow_t const &) noexcept {
  EnableGuard ena;
  if (ena) {
    __global_hook.on(AllocOp::Delete, ptr, kNone, kNone,
                     __builtin_return_address(0));
  }
  REAL_LIBC(free)(ptr);
}

void operator delete[](void *ptr, std::nothrow_t const &) noexcept {
  EnableGuard ena;
  if (ena) {
    __global_hook.on(AllocOp::DeleteArray, ptr, kNone, kNone,
                     __builtin_return_address(0));
  }
  REAL_LIBC(free)(ptr);
}

void *operator new(size_t size) noexcept(false) {
  EnableGuard ena;
  void *ptr = REAL_LIBC(malloc)(size);
  if (ena) {
    __global_hook.on(AllocOp::New, ptr, size, kNone,
                     __builtin_return_address(0));
  }
  if (ptr == nullptr) {
    throw std::bad_alloc();
  }
  return ptr;
}

void *operator new[](size_t size) noexcept(false) {
  EnableGuard ena;
  void *ptr = REAL_LIBC(malloc)(size);
  if (ena) {
    __global_hook.on(AllocOp::NewArray, ptr, size, kNone,
                     __builtin_return_address(0));
  }
  if (ptr == nullptr) {
    throw std::bad_alloc();
  }
  return ptr;
}

void *operator new(size_t size, std::nothrow_t const &) noexcept {
  EnableGuard ena;
  void *ptr = REAL_LIBC(malloc)(size);
  if (ena) {
    __global_hook.on(AllocOp::New, ptr, size, kNone,
                     __builtin_return_address(0));
  }
  return ptr;
}

void *operator new[](size_t size, std::nothrow_t const &) noexcept {
  EnableGuard ena;
  void *ptr = REAL_LIBC(malloc)(size);
  if (ena) {
    __global_hook.on(AllocOp::NewArray, ptr, size, kNone,
                     __builtin_return_address(0));
  }
  return ptr;
}

#if (__cplusplus >= 201402L || _MSC_VER >= 1916)
void operator delete(void *ptr, size_t size) noexcept {
  EnableGuard ena;
  if (ena) {
    __global_hook.on(AllocOp::Delete, ptr, size, kNone,
                     __builtin_return_address(0));
  }
  REAL_LIBC(free)(ptr);
}

void operator delete[](void *ptr, size_t size) noexcept {
  EnableGuard ena;
  if (ena) {
    __global_hook.on(AllocOp::DeleteArray, ptr, size, kNone,
                     __builtin_return_address(0));
  }
  REAL_LIBC(free)(ptr);
}
#endif

#if (__cplusplus > 201402L || defined(__cpp_aligned_new))
void operator delete(void *ptr, std::align_val_t align) noexcept {
  EnableGuard ena;
  if (ena) {
    __global_hook.on(AllocOp::Delete, ptr, kNone, (size_t)align,
                     __builtin_return_address(0));
  }
  REAL_LIBC(free)(ptr);
}

void operator delete[](void *ptr, std::align_val_t align) noexcept {
  EnableGuard ena;
  if (ena) {
    __global_hook.on(AllocOp::DeleteArray, ptr, kNone, (size_t)align,
                     __builtin_return_address(0));
  }
  REAL_LIBC(free)(ptr);
}

void operator delete(void *ptr, size_t size, std::align_val_t align) noexcept {
  EnableGuard ena;
  if (ena) {
    __global_hook.on(AllocOp::Delete, ptr, size, (size_t)align,
                     __builtin_return_address(0));
  }
  REAL_LIBC(free)(ptr);
}

void operator delete[](void *ptr, size_t size,
                       std::align_val_t align) noexcept {
  EnableGuard ena;
  if (ena) {
    __global_hook.on(AllocOp::DeleteArray, ptr, size, (size_t)align,
                     __builtin_return_address(0));
  }
  REAL_LIBC(free)(ptr);
}

void operator delete(void *ptr, std::align_val_t align,
                     std::nothrow_t const &) noexcept {
  EnableGuard ena;
  if (ena) {
    __global_hook.on(AllocOp::Delete, ptr, kNone, (size_t)align,
                     __builtin_return_address(0));
  }
  REAL_LIBC(free)(ptr);
}

void operator delete[](void *ptr, std::align_val_t align,
                       std::nothrow_t const &) noexcept {
  EnableGuard ena;
  if (ena) {
    __global_hook.on(AllocOp::DeleteArray, ptr, kNone, (size_t)align,
                     __builtin_return_address(0));
  }
  REAL_LIBC(free)(ptr);
}

void *operator new(size_t size, std::align_val_t align) noexcept(false) {
  EnableGuard ena;
  void *ptr = REAL_LIBC(memalign)((size_t)align, size);
  if (ena) {
    __global_hook.on(AllocOp::New, ptr, size, (size_t)align,
                     __builtin_return_address(0));
  }
  if (ptr == nullptr) {
    throw std::bad_alloc();
  }
  return ptr;
}

void *operator new[](size_t size, std::align_val_t align) noexcept(false) {
  EnableGuard ena;
  void *ptr = REAL_LIBC(memalign)((size_t)align, size);
  if (ena) {
    __global_hook.on(AllocOp::NewArray, ptr, size, (size_t)align,
                     __builtin_return_address(0));
  }
  if (ptr == nullptr) {
    throw std::bad_alloc();
  }
  return ptr;
}

void *operator new(size_t size, std::align_val_t align,
                   std::nothrow_t const &) noexcept {
  EnableGuard ena;
  void *ptr = REAL_LIBC(memalign)((size_t)align, size);
  if (ena) {
    __global_hook.on(AllocOp::New, ptr, size, (size_t)align,
                     __builtin_return_address(0));
  }
  return ptr;
}

void *operator new[](size_t size, std::align_val_t align,
                     std::nothrow_t const &) noexcept {
  EnableGuard ena;
  void *ptr = REAL_LIBC(memalign)((size_t)align, size);
  if (ena) {
    __global_hook.on(AllocOp::NewArray, ptr, size, (size_t)align,
                     __builtin_return_address(0));
  }
  return ptr;
}
#endif