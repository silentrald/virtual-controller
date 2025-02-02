#ifndef VC_TYPE_HPP
#define VC_TYPE_HPP

#include <cstdint>

namespace vc {

using i64 = int64_t;
using i32 = int32_t;
using i16 = int16_t;
using i8 = int8_t;
using u64 = uint64_t;
using u32 = uint32_t;
using u16 = uint16_t;
using u8 = uint8_t;

using isize = i64;
using usize = u64;

using f32 = float;
using f64 = double;

using c8 = char;

using error_code = u32;

// NOLINTNEXTLINE
enum error : u32 {
  OK = 0U,

  CONTROLLER_OPEN,
  CONTROLLER_CREATE,

  UNKNOWN = UINT32_MAX,
};

} // namespace vc

// NOLINTNEXTLINE
#define VC_GET_MACRO(_1, _2, NAME, ...) NAME

// NOLINTNEXTLINE
#define VC_TRY(...) GET_MACRO(__VA_ARGS__, RP_TRY2, RP_TRY1)(__VA_ARGS__)

// NOLINTNEXTLINE
#define VC_TRY1(expression)                                                    \
  {                                                                            \
    auto error = expression;                                                   \
    if (rp::is_error(error)) {                                                 \
      return std::move(error);                                                 \
    }                                                                          \
  }

// NOLINTNEXTLINE
#define VC_TRY2(expression, converter)                                         \
  {                                                                            \
    auto error = expression;                                                   \
    if (rp::is_error(error)) {                                                 \
      return std::move(converter(error));                                      \
    }                                                                          \
  }

#endif
