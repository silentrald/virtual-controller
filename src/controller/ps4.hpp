#ifndef VC_CONTROLLER_PS4_HPP
#define VC_CONTROLLER_PS4_HPP

#include "../types.hpp"
#include <array>
#include <linux/input-event-codes.h>

namespace vc {

enum PS4Button : u16 {
  // With their default values
  // Action buttons
  // CROSS = BTN_SOUTH,
  // CIRCLE = BTN_EAST,
  // SQUARE = BTN_WEST,
  // TRIANGLE = BTN_NORTH,
  CROSS = 0,
  CIRCLE,
  SQUARE,
  TRIANGLE,

  // Trigger buttons
  // R1 = BTN_TR,
  // L1 = BTN_TL,
  // R2 = BTN_TR2,
  // L2 = BTN_TL2,
  L1,
  R1,
  L2,
  R2,

  // Misc buttons
  // SHARE = BTN_SELECT,
  // OPTIONS = BTN_START,
  // HOME = BTN_MODE,
  // R3 = BTN_THUMBR,
  // L3 = BTN_THUMBL,
  SHARE,
  OPTIONS,
  HOME,
  L3,
  R3,
};

enum PS4Stick : u8 {
  LEFT_X = ABS_X,
  LEFT_Y = ABS_Y,

  RIGHT_X = ABS_RX,
  RIGHT_Y = ABS_RY,
};

enum PS4DPad : u8 {
  X = ABS_HAT0X,
  Y = ABS_HAT0Y,
};

class PS4Controller {
public:
  PS4Controller() noexcept = default;
  PS4Controller(const PS4Controller&) = delete;
  PS4Controller& operator=(const PS4Controller&) = delete;

  PS4Controller(PS4Controller&& other) noexcept;
  PS4Controller& operator=(PS4Controller&& rhs) noexcept;

  ~PS4Controller() noexcept;

  [[nodiscard]] error_code init(const c8* name, bool is_pro) noexcept;

  // Needs to be called everytime an action is called
  void sync() const noexcept;

  void remap(PS4Button button, u16 code) noexcept;

  void press_button(PS4Button button) noexcept;
  void release_button(PS4Button button) noexcept;

  void press_up() noexcept;
  void press_down() noexcept;
  void press_left() noexcept;
  void press_right() noexcept;

  void release_up() noexcept;
  void release_down() noexcept;
  void release_left() noexcept;
  void release_right() noexcept;

  void move_stick(PS4Stick stick, u8 value = 0x7f) noexcept;
  void move_stickf(PS4Stick stick, float value = 0.5F) noexcept;

  [[nodiscard]] bool is_button_pressed(PS4Button button) const noexcept;
  [[nodiscard]] u8 get_stick_u8(PS4Stick stick) const noexcept;
  [[nodiscard]] f32 get_stick_f32(PS4Stick stick) const noexcept;
  [[nodiscard]] u8 get_dpad(PS4DPad dpad) const noexcept;

  void print() const noexcept;

private:
  i32 fd = -1;

  std::array<u16, 13> mapping{
      BTN_SOUTH,  BTN_EAST,   BTN_WEST, BTN_NORTH,  BTN_TL,
      BTN_TR,     BTN_TL2,    BTN_TR2,  BTN_SELECT, BTN_START,
      BTN_THUMBR, BTN_THUMBL, BTN_MODE,
  };
  std::array<bool, 13> buttons{false};

  u8 left_x = 0x7f;
  u8 left_y = 0x7f;
  u8 right_x = 0x7f;
  u8 right_y = 0x7f;
  u8 dpad_x = 0x7f;
  u8 dpad_y = 0x7f;

  void handle_button(u16 button, bool press) const noexcept;
  void handle_analog(u16 type, u8 value) const noexcept;
};

} // namespace vc

// evtest info of r
// Event type 0 (EV_SYN)
// Event type 1 (EV_KEY)
//   Event code 304 (BTN_SOUTH)
//   Event code 305 (BTN_EAST)
//   Event code 307 (BTN_NORTH)
//   Event code 308 (BTN_WEST)
//   Event code 310 (BTN_TL)
//   Event code 311 (BTN_TR)
//   Event code 312 (BTN_TL2)
//   Event code 313 (BTN_TR2)
//   Event code 314 (BTN_SELECT)
//   Event code 315 (BTN_START)
//   Event code 316 (BTN_MODE)
//   Event code 317 (BTN_THUMBL)
//   Event code 318 (BTN_THUMBR)
// Event type 3 (EV_ABS)
//   Event code 0 (ABS_X)
//     Value    122
//     Min        0
//     Max      255
//   Event code 1 (ABS_Y)
//     Value    127
//     Min        0
//     Max      255
//   Event code 2 (ABS_Z)
//     Value      0
//     Min        0
//     Max      255
//   Event code 3 (ABS_RX)
//     Value    123
//     Min        0
//     Max      255
//   Event code 4 (ABS_RY)
//     Value    126
//     Min        0
//     Max      255
//   Event code 5 (ABS_RZ)
//     Value      0
//     Min        0
//     Max      255
//   Event code 16 (PS4DPad::X)
//     Value      0
//     Min       -1
//     Max        1
//   Event code 17 (PS4DPad::Y)
//     Value      0
//     Min       -1
//     Max        1
// Event type 21 (EV_FF)
//   Event code 80 (FF_RUMBLE)
//   Event code 81 (FF_PERIODIC)
//   Event code 88 (FF_SQUARE)
//   Event code 89 (FF_TRIANGLE)
//   Event code 90 (FF_SINE)
//   Event code 96 (FF_GAIN)

#endif
