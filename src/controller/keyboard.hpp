/*=============================
 * Author/s: silentrald
 * Date Created: 2025-02-06
 *=============================*/

#ifndef KP_KEYBOARD_HPP
#define KP_KEYBOARD_HPP

#include "../types.hpp"
#include <array>
#include <cstring>
#include <fcntl.h>
#include <linux/input-event-codes.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <unistd.h>

namespace vc {

namespace Modifiers {
enum Modifiers : u16 {
  SHIFT = 0x8000,
  CTRL = 0x4000,
  ALT = 0x2000,
};
} // namespace Modifiers

class Keyboard {
public:
  Keyboard() noexcept = default;
  Keyboard(const Keyboard&) noexcept = delete;
  Keyboard& operator=(const Keyboard&) noexcept = delete;

  Keyboard(Keyboard&& other) noexcept;
  Keyboard& operator=(Keyboard&& rhs) noexcept;

  ~Keyboard() noexcept;

  [[nodiscard]] error_code init() noexcept;
  void set_delay(u32 delay) noexcept;

  /**
   * Tries to remap a character into a new codes
   * ie. Remap 'a' to KEY_B | Modifiers::SHIFT (code for 'B')
   */
  void remap(u8 key, u16 code) noexcept;

  // Converts a charater into a key press
  void key_press(c8 key) const noexcept;

private:
  i32 fd = -1;
  u32 delay = 1'000; // 1ms

  std::array<u16, 127> key_map{
      // Special key codes
      0, 0, 0, 0, 0, 0, 0, 0, 0,
      KEY_TAB,   // \t
      KEY_ENTER, // \n
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

      //

      KEY_SPACE,
      KEY_1 | Modifiers::SHIFT,          // !
      KEY_APOSTROPHE | Modifiers::SHIFT, // "
      KEY_3 | Modifiers::SHIFT,          // #
      KEY_4 | Modifiers::SHIFT,          // $
      KEY_5 | Modifiers::SHIFT,          // %
      KEY_7 | Modifiers::SHIFT,          // &
      KEY_APOSTROPHE,
      KEY_9 | Modifiers::SHIFT,     // (
      KEY_0 | Modifiers::SHIFT,     // )
      KEY_8 | Modifiers::SHIFT,     // *
      KEY_EQUAL | Modifiers::SHIFT, // +
      KEY_COMMA, KEY_MINUS, KEY_DOT, KEY_SLASH,

      // Numbers
      KEY_0, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9,
      //

      KEY_SEMICOLON | Modifiers::SHIFT, // :
      KEY_SEMICOLON,
      KEY_COMMA | Modifiers::SHIFT, // <
      KEY_EQUAL,
      KEY_DOT | Modifiers::SHIFT,   // >
      KEY_SLASH | Modifiers::SHIFT, // ?
      KEY_2 | Modifiers::SHIFT,     // @

      // Caps characters
      KEY_A | Modifiers::SHIFT, KEY_B | Modifiers::SHIFT,
      KEY_C | Modifiers::SHIFT, KEY_D | Modifiers::SHIFT,
      KEY_E | Modifiers::SHIFT, KEY_F | Modifiers::SHIFT,
      KEY_G | Modifiers::SHIFT, KEY_H | Modifiers::SHIFT,
      KEY_I | Modifiers::SHIFT, KEY_J | Modifiers::SHIFT,
      KEY_K | Modifiers::SHIFT, KEY_L | Modifiers::SHIFT,
      KEY_M | Modifiers::SHIFT, KEY_N | Modifiers::SHIFT,
      KEY_O | Modifiers::SHIFT, KEY_P | Modifiers::SHIFT,
      KEY_Q | Modifiers::SHIFT, KEY_R | Modifiers::SHIFT,
      KEY_S | Modifiers::SHIFT, KEY_T | Modifiers::SHIFT,
      KEY_U | Modifiers::SHIFT, KEY_V | Modifiers::SHIFT,
      KEY_W | Modifiers::SHIFT, KEY_X | Modifiers::SHIFT,
      KEY_Y | Modifiers::SHIFT, KEY_Z | Modifiers::SHIFT,
      //

      KEY_LEFTBRACE, KEY_BACKSLASH, KEY_RIGHTBRACE,
      KEY_6 | Modifiers::SHIFT,     // ^
      KEY_MINUS | Modifiers::SHIFT, // _
      KEY_GRAVE,

      // Normal characters
      KEY_A, KEY_B, KEY_C, KEY_D, KEY_E, KEY_F, KEY_G, KEY_H, KEY_I, KEY_J,
      KEY_K, KEY_L, KEY_M, KEY_N, KEY_O, KEY_P, KEY_Q, KEY_R, KEY_S, KEY_T,
      KEY_U, KEY_V, KEY_W, KEY_X, KEY_Y, KEY_Z,
      //

      KEY_LEFTBRACE | Modifiers::SHIFT,  // {
      KEY_BACKSLASH | Modifiers::SHIFT,  // |
      KEY_RIGHTBRACE | Modifiers::SHIFT, // }
      KEY_GRAVE | Modifiers::SHIFT,      // ~
  };
};

} // namespace vc

#endif
