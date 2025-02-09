/*=============================
 * Author/s: silentrald
 * Date Created: 2025-02-06
 *=============================*/

#include "./keyboard.hpp"
#include "../helper.hpp"
#include <cassert>

namespace vc {

Keyboard::Keyboard(Keyboard&& other) noexcept : fd(other.fd) {
  other.fd = -1;
}

Keyboard& Keyboard::operator=(Keyboard&& rhs) noexcept {
  if (this == &rhs) {
    return *this;
  }

  this->fd = rhs.fd;
  rhs.fd = -1;

  return *this;
}

Keyboard::~Keyboard() noexcept {
  if (this->fd == -1) {
    return;
  }

  ioctl(this->fd, UI_DEV_DESTROY);
  close(this->fd);
  this->fd = -1;
}

[[nodiscard]] error_code Keyboard::init() noexcept {
  this->fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
  if (this->fd == -1) {
    return error::CONTROLLER_OPEN;
  }

  uinput_user_dev setup{
      .id =
          {
              .bustype = BUS_USB,
              .vendor = 0x1111,
              .product = 0x1111,
              .version = 1,
          }
  };
  std::strcpy(setup.name, "Simulated keyboard");

  TRY_IOCTL(this->fd, UI_SET_EVBIT, EV_KEY);
  for (int key = KEY_ESC; key <= KEY_KPDOT; ++key) {
    TRY_IOCTL(this->fd, UI_SET_KEYBIT, key);
  }

  if (write(this->fd, &setup, sizeof(setup)) == -1) {
    return error::CONTROLLER_CREATE;
  }

  if (ioctl(this->fd, UI_DEV_CREATE) == -1) {
    return error::CONTROLLER_CREATE;
  }

  return error::OK;
}

void Keyboard::set_delay(u32 delay) noexcept {
  this->delay = delay;
}

void Keyboard::remap(u8 key, u16 code) noexcept {
  assert(key < this->key_map.size());
  this->key_map[key] = code;
}

void Keyboard::key_press(c8 key) const noexcept {
  if (key >= this->key_map.size()) {
    return;
  }
  key = this->key_map[key];
  if (key == 0) {
    return;
  }

  if (key & Modifiers::SHIFT) {
    uinput::emit(this->fd, EV_KEY, KEY_LEFTSHIFT, 1);
    uinput::emit(this->fd, EV_SYN, SYN_REPORT, 0);
    usleep(this->delay);
  }

  uinput::emit(this->fd, EV_KEY, key & ~Modifiers::SHIFT, 1);
  uinput::emit(this->fd, EV_SYN, SYN_REPORT, 0);
  usleep(this->delay);

  if (key & Modifiers::SHIFT) {
    uinput::emit(this->fd, EV_KEY, KEY_LEFTSHIFT, 0);
  }
  uinput::emit(this->fd, EV_KEY, key & ~Modifiers::SHIFT, 0);
  uinput::emit(this->fd, EV_SYN, SYN_REPORT, 0);
  usleep(this->delay);
}

} // namespace vc
