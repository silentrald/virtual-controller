#include "./types.hpp"
#include "controller/ps4.hpp"
#include <bits/types/struct_timeval.h>
#include <csignal>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <linux/input-event-codes.h>
#include <linux/joystick.h>
#include <linux/uinput.h>
#include <sys/ioctl.h>
#include <unistd.h>

static bool running = true; // NOLINT
// NOLINTNEXTLINE
static void sigint_callback(vc::i32 _signum) noexcept {
  printf("\nSIGINT received, exiting program\n");
  running = false;
}

int main() noexcept {
  printf("Creating controller\n");
  vc::PS4Controller controller{};
  vc::error_code code = controller.init("Simulated PS4 Controller", true);
  if (code != vc::error::OK) {
    printf("Could not initialize controller: %u\n", code);
    return 1;
  }

  printf("Creating sigint callback\n");
  struct sigaction action{};
  action.sa_handler = sigint_callback;
  sigaction(SIGINT, &action, nullptr);

  // NOTE: Remapping allowed since some games don't align their controls with
  //   real controllers
  controller.remap(vc::PS4Button::CROSS, BTN_EAST);
  controller.remap(vc::PS4Button::CIRCLE, BTN_NORTH);
  controller.remap(vc::PS4Button::TRIANGLE, BTN_WEST);
  controller.remap(vc::PS4Button::SQUARE, BTN_SOUTH);

  usleep(5'000'000);
  printf("Simulated PS4 Controller created\n");

  controller.move_stick(vc::PS4Stick::LEFT_X, 0xff);
  controller.sync();

  // Test repeatedly press the cross button
  vc::i32 state = 0;
  vc::PS4Button button = vc::PS4Button::CROSS;
  while (running) {
    switch (state) {
    case 0:
      controller.press_button(button);
      break;

    case 1:
      controller.release_button(button);
      break;

    default:
      state = 0;
      continue;
    }

    ++state;
    controller.sync();

    usleep(1'000'000 / 60 * 120);
  }

  controller.release_button(vc::PS4Button::CROSS);
  controller.sync();

  return 0;
}
