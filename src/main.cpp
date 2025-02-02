#include <bits/types/struct_timeval.h>
#include <csignal>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <linux/input-event-codes.h>
#include <linux/joystick.h>
#include <linux/uinput.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define JS_EVENT_BUTTON 0x01
#define JS_EVENT_AXIS 0x02
#define JS_EVENT_INIT 0x80

#define BTN_CROSS 0x00
#define BTN_CIRCLE 0x01
#define BTN_TRIANGLE 0x02
#define BTN_SQUARE 0x03

using i32 = int32_t;
using u32 = uint32_t;
using u16 = uint16_t;
using u8 = uint8_t;

using c8 = char;

// NOLINTNEXTLINE
enum error_code : u32 {
  OK = 0U,

  CONTROLLER_OPEN,
  CONTROLLER_CREATE,

  UNKNOWN = UINT32_MAX,
};

// evtest info of a PS4 Controller
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

static i32 emit(i32 fd, u16 type, u16 code, i32 value) noexcept {
  input_event event{
      .type = type,
      .code = code,
      .value = value,
  };
  return write(fd, &event, sizeof(event));
}

static bool running = true;
static void sigint_callback(i32 _signum) noexcept {
  printf("\nSIGINT received, exiting program\n");
  running = false;
}

enum PS4Button : u16 {
  // Action buttons
  CROSS = BTN_SOUTH,
  CIRCLE = BTN_EAST,
  SQUARE = BTN_WEST,
  TRIANGLE = BTN_NORTH,

  // Trigger buttons
  R1 = BTN_TR,
  L1 = BTN_TL,
  R2 = BTN_TR2,
  L2 = BTN_TL2,

  // Misc buttons
  SHARE = BTN_SELECT,
  OPTIONS = BTN_START,
  HOME = BTN_MODE,
  R3 = BTN_THUMBR,
  L3 = BTN_THUMBL,
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

  PS4Controller(PS4Controller&& rhs) noexcept : fd(rhs.fd) {
    rhs.fd = -1;
  }

  PS4Controller& operator=(PS4Controller&& other) noexcept {
    if (this == &other) {
      return *this;
    }

    this->fd = other.fd;
    other.fd = -1;

    return *this;
  }

  ~PS4Controller() noexcept {
    if (this->fd != -1) {
      ioctl(fd, UI_DEV_DESTROY);
      close(this->fd);
      this->fd = -1;
    }
  }

  [[nodiscard]] error_code init(const c8* name, bool is_pro) noexcept {
    this->fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (this->fd == -1) {
      return error_code::CONTROLLER_OPEN;
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

    // TODO: Throw ioctl error code -1
    // Setup the buttons
    ioctl(this->fd, UI_SET_EVBIT, EV_KEY);

    ioctl(this->fd, UI_SET_KEYBIT, PS4Button::CROSS);
    ioctl(this->fd, UI_SET_KEYBIT, PS4Button::CIRCLE);
    ioctl(this->fd, UI_SET_KEYBIT, PS4Button::SQUARE);
    ioctl(this->fd, UI_SET_KEYBIT, PS4Button::TRIANGLE);
    ioctl(this->fd, UI_SET_KEYBIT, PS4Button::R1);
    ioctl(this->fd, UI_SET_KEYBIT, PS4Button::L1);
    ioctl(this->fd, UI_SET_KEYBIT, PS4Button::R2);
    ioctl(this->fd, UI_SET_KEYBIT, PS4Button::L2);
    ioctl(this->fd, UI_SET_KEYBIT, PS4Button::SHARE);
    ioctl(this->fd, UI_SET_KEYBIT, PS4Button::OPTIONS);
    ioctl(this->fd, UI_SET_KEYBIT, PS4Button::HOME);
    ioctl(this->fd, UI_SET_KEYBIT, PS4Button::R3);
    ioctl(this->fd, UI_SET_KEYBIT, PS4Button::L3);

    ioctl(this->fd, UI_SET_EVBIT, EV_ABS);

    // Sticks
    ioctl(this->fd, UI_SET_ABSBIT, PS4Stick::LEFT_X);
    ioctl(this->fd, UI_SET_ABSBIT, PS4Stick::LEFT_Y);
    ioctl(this->fd, UI_SET_ABSBIT, PS4Stick::RIGHT_X);
    ioctl(this->fd, UI_SET_ABSBIT, PS4Stick::RIGHT_Y);

    setup.absmax[PS4Stick::LEFT_X] = 255;
    setup.absmin[PS4Stick::LEFT_X] = 0;
    setup.absmax[PS4Stick::LEFT_Y] = 255;
    setup.absmin[PS4Stick::LEFT_Y] = 0;

    setup.absmax[PS4Stick::RIGHT_X] = 255;
    setup.absmin[PS4Stick::RIGHT_X] = 0;
    setup.absmax[PS4Stick::RIGHT_Y] = 255;
    setup.absmin[PS4Stick::RIGHT_Y] = 0;

    // DPad
    ioctl(this->fd, UI_SET_ABSBIT, PS4DPad::X);
    ioctl(this->fd, UI_SET_ABSBIT, PS4DPad::Y);

    setup.absmax[PS4DPad::X] = 1;
    setup.absmin[PS4DPad::X] = -1;
    setup.absmax[PS4DPad::Y] = 1;
    setup.absmin[PS4DPad::Y] = -1;

    write(this->fd, &setup, sizeof(setup));
    ioctl(this->fd, UI_DEV_CREATE);

    // Initialize sticks to neutral position
    this->move_stick(PS4Stick::LEFT_X);
    this->move_stick(PS4Stick::LEFT_Y);
    this->move_stick(PS4Stick::RIGHT_X);
    this->move_stick(PS4Stick::RIGHT_Y);

    return error_code::OK;
  }

  // Needs to be called everytime an action is called
  void sync() const noexcept {
    emit(this->fd, EV_SYN, SYN_REPORT, 0);
  }

  void press(PS4Button button) noexcept {
    this->handle_button(button, true);
  }

  void release(PS4Button button) noexcept {
    this->handle_button(button, false);
  }

  void press_up() noexcept {
    this->handle_analog(PS4DPad::Y, -1);
  }

  void press_down() noexcept {
    this->handle_analog(PS4DPad::Y, 1);
  }

  void press_left() noexcept {
    this->handle_analog(PS4DPad::X, -1);
  }

  void press_right() noexcept {
    this->handle_analog(PS4DPad::X, 1);
  }

  void release_up() noexcept {
    this->handle_analog(PS4DPad::Y, 0);
  }

  void release_down() noexcept {
    this->handle_analog(PS4DPad::Y, 0);
  }

  void release_left() noexcept {
    this->handle_analog(PS4DPad::X, 0);
  }

  void release_right() noexcept {
    this->handle_analog(PS4DPad::X, 0);
  }

  void move_stick(PS4Stick stick, u8 value = 0x7f) noexcept {
    this->handle_analog(stick, value);
  }

  void move_stickf(PS4Stick stick, float value = 0.5F) noexcept {
    this->handle_analog(stick, 0xff * value);
  }

private:
  i32 fd = -1;

  void handle_button(u16 button, bool press) const noexcept {
    emit(this->fd, EV_KEY, button, press); // 1 for press, 0 for release
  }

  void handle_analog(u16 type, u8 value) const noexcept {
    emit(this->fd, EV_ABS, type, value);
  }
};

int main() noexcept {
  printf("Creating controller\n");
  PS4Controller controller{};
  error_code code = controller.init("Simulated PS4 Controller", true);
  if (code != error_code::OK) {
    printf("Could not initialize controller: %u\n", code);
    return 1;
  }

  printf("Creating sigterm\n");

  struct sigaction action{};
  action.sa_handler = sigint_callback;
  sigaction(SIGINT, &action, nullptr);

  printf("Simulated PS4 Controller created\n");

  // Test repeatedly press the cross button
  bool up = true;
  while (running) {
    if (up) {
      controller.press(PS4Button::CROSS);
    } else {
      controller.release(PS4Button::CROSS);
    }
    up = !up;
    controller.sync();
    usleep(1'000'000 / 60 * 5);
  }

  controller.release(PS4Button::CROSS);
  controller.sync();

  return 0;
}
