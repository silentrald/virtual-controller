#include "./ps4.hpp"
#include "../helper.hpp"
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <linux/input-event-codes.h>
#include <linux/uinput.h>
#include <sys/ioctl.h>
#include <unistd.h>

namespace vc {

PS4Controller::PS4Controller(PS4Controller&& other) noexcept : fd(other.fd) {
  other.fd = -1;
}

PS4Controller& PS4Controller::operator=(PS4Controller&& rhs) noexcept {
  if (this == &rhs) {
    return *this;
  }

  this->fd = rhs.fd;
  rhs.fd = -1;

  return *this;
}

PS4Controller::~PS4Controller() noexcept {
  if (this->fd != -1) {
    ioctl(this->fd, UI_DEV_DESTROY);
    close(this->fd);
    this->fd = -1;
  }
}

error_code PS4Controller::init(const c8* name, bool is_pro) noexcept {
  this->fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
  if (this->fd == -1) {
    return error::CONTROLLER_OPEN;
  }

  uinput_user_dev setup{
      .id =
          {
              .bustype = BUS_USB,
              .vendor = 0x054c,
              .product = static_cast<u16>(is_pro ? 0x09cc : 0x05c4),
              .version = 1,
          }
  };
  std::strcpy(setup.name, name);

  // Setup the buttons
  TRY_IOCTL(this->fd, UI_SET_EVBIT, EV_KEY);
  for (const auto& code : this->mapping) {
    TRY_IOCTL(this->fd, UI_SET_KEYBIT, code);
  }

  TRY_IOCTL(this->fd, UI_SET_EVBIT, EV_ABS);

  // Sticks
  TRY_IOCTL(this->fd, UI_SET_ABSBIT, PS4Stick::LEFT_X);
  TRY_IOCTL(this->fd, UI_SET_ABSBIT, PS4Stick::LEFT_Y);
  TRY_IOCTL(this->fd, UI_SET_ABSBIT, PS4Stick::RIGHT_X);
  TRY_IOCTL(this->fd, UI_SET_ABSBIT, PS4Stick::RIGHT_Y);

  // DPad
  TRY_IOCTL(this->fd, UI_SET_ABSBIT, PS4DPad::X);
  TRY_IOCTL(this->fd, UI_SET_ABSBIT, PS4DPad::Y);

  // Setup the limits
  setup.absmax[PS4Stick::LEFT_X] = 0xff;
  setup.absmin[PS4Stick::LEFT_X] = 0x00;
  setup.absmax[PS4Stick::LEFT_Y] = 0xff;
  setup.absmin[PS4Stick::LEFT_Y] = 0x00;

  setup.absmax[PS4Stick::RIGHT_X] = 0xff;
  setup.absmin[PS4Stick::RIGHT_X] = 0x00;
  setup.absmax[PS4Stick::RIGHT_Y] = 0xff;
  setup.absmin[PS4Stick::RIGHT_Y] = 0x00;

  setup.absmax[PS4DPad::X] = 1;
  setup.absmin[PS4DPad::X] = -1;
  setup.absmax[PS4DPad::Y] = 1;
  setup.absmin[PS4DPad::Y] = -1;

  if (write(this->fd, &setup, sizeof(setup)) == -1) {
    return error::CONTROLLER_CREATE;
  }

  if (ioctl(this->fd, UI_DEV_CREATE) == -1) {
    return error::CONTROLLER_CREATE;
  }

  // Initialize sticks to neutral position
  this->move_stick(PS4Stick::LEFT_X);
  this->move_stick(PS4Stick::LEFT_Y);
  this->move_stick(PS4Stick::RIGHT_X);
  this->move_stick(PS4Stick::RIGHT_Y);

  return error::OK;
}

void PS4Controller::sync() const noexcept {
  uinput::emit(this->fd, EV_SYN, SYN_REPORT, 0);
}

void PS4Controller::remap(PS4Button button, u16 code) noexcept {
  this->mapping[button] = code;
}

void PS4Controller::press_button(PS4Button button) noexcept {
  this->buttons[button] = true;
  this->handle_button(this->mapping[button], true);
}

void PS4Controller::release_button(PS4Button button) noexcept {
  this->buttons[button] = false;
  this->handle_button(this->mapping[button], false);
}

void PS4Controller::press_up() noexcept {
  this->handle_analog(PS4DPad::Y, -1);
}

void PS4Controller::press_down() noexcept {
  this->handle_analog(PS4DPad::Y, 1);
}

void PS4Controller::press_left() noexcept {
  this->handle_analog(PS4DPad::X, -1);
}

void PS4Controller::press_right() noexcept {
  this->handle_analog(PS4DPad::X, 1);
}

void PS4Controller::release_up() noexcept {
  this->handle_analog(PS4DPad::Y, 0);
}

void PS4Controller::release_down() noexcept {
  this->handle_analog(PS4DPad::Y, 0);
}

void PS4Controller::release_left() noexcept {
  this->handle_analog(PS4DPad::X, 0);
}

void PS4Controller::release_right() noexcept {
  this->handle_analog(PS4DPad::X, 0);
}

void PS4Controller::move_stick(PS4Stick stick, u8 value) noexcept {
  this->handle_analog(stick, value);
}

void PS4Controller::move_stickf(PS4Stick stick, f32 value) noexcept {
  this->handle_analog(
      stick, std::clamp(static_cast<u32>(0xff * value), 0x00U, 0xffU)
  );
}

bool PS4Controller::is_button_pressed(PS4Button button) const noexcept {
  return this->buttons[button];
}

u8 PS4Controller::get_stick_u8(PS4Stick stick) const noexcept {
  switch (stick) {
  case PS4Stick::LEFT_X:
    return this->left_x;

  case PS4Stick::LEFT_Y:
    return this->left_y;

  case PS4Stick::RIGHT_X:
    return this->right_x;

  case PS4Stick::RIGHT_Y:
    return this->right_y;
  }

  printf("Stick value %u not supported\n", stick);
  std::abort();
}

// TODO: Check if fast division is needed
f32 PS4Controller::get_stick_f32(PS4Stick stick) const noexcept {
  switch (stick) {
  case PS4Stick::LEFT_X:
    return this->left_x / 255.0F;

  case PS4Stick::LEFT_Y:
    return this->left_y / 255.0F;

  case PS4Stick::RIGHT_X:
    return this->right_x / 255.0F;

  case PS4Stick::RIGHT_Y:
    return this->right_y / 255.0F;
  }

  printf("Stick value %u not supported\n", stick);
  std::abort();
}

u8 PS4Controller::get_dpad(PS4DPad dpad) const noexcept {
  return dpad == PS4DPad::X ? this->dpad_x : this->dpad_y;
}

void PS4Controller::print() const noexcept {
  printf(
      "X: %d ; O: %d ; S: %d ; T: %d\n"
      "L1: %d ; R1: %d ; L2: %d ; R2: %d\n"
      "LS: (%.06f, %.06f) ; RS: (%.06f, %.06f)\n",
      this->buttons[PS4Button::CROSS], this->buttons[PS4Button::CIRCLE],
      this->buttons[PS4Button::SQUARE], this->buttons[PS4Button::TRIANGLE],
      this->buttons[PS4Button::L1], this->buttons[PS4Button::R1],
      this->buttons[PS4Button::L2], this->buttons[PS4Button::R2],
      this->left_x / 255.0F, this->left_y / 255.0F, this->right_x / 255.0F,
      this->right_y / 255.0F
  );
}

void PS4Controller::handle_button(u16 button, bool press) const noexcept {
  uinput::emit(this->fd, EV_KEY, button, press); // 1 for press, 0 for release
}

void PS4Controller::handle_analog(u16 type, u8 value) const noexcept {
  uinput::emit(this->fd, EV_ABS, type, value);
}

} // namespace vc
