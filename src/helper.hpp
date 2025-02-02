#ifndef VC_HELPER_HPP
#define VC_HELPER_HPP

#include "./types.hpp"
#include <linux/input.h>
#include <unistd.h>

namespace vc::uinput {

static i32 emit(i32 fd, u16 type, u16 code, i32 value) noexcept {
  input_event event{
      .type = type,
      .code = code,
      .value = value,
  };
  return write(fd, &event, sizeof(event));
}

} // namespace vc::uinput

#endif
