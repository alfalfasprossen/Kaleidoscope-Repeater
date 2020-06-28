/* -*- mode: c++ -*-
 * Kaleidoscope-Repeater -- Define keys to be repeated when tapped.
 * Copyright (C) 2020  Johannes Becker
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <Kaleidoscope.h>
#include <Kaleidoscope-Ranges.h>
#include <Kaleidoscope-Repeater.h>

// Define a synthetic key to use in the keymap. If you use multiple,
// increase the key value, e.g. `SAFE_START + 1`.
static constexpr Key Key_MyRepeaterKey{kaleidoscope::ranges::SAFE_START};

// Layers
enum {TYPING, GAMING};

// *INDENT-OFF*
KEYMAPS(
  [TYPING] = KEYMAP_STACKED
  (Key_NoKey,    Key_1, Key_2, Key_3, Key_4, Key_5, Key_NoKey,
   Key_Backtick, Key_Q, Key_W, Key_E, Key_R, Key_T, Key_Tab,
   Key_PageUp,   Key_A, Key_S, Key_D, Key_F, Key_G,
   Key_PageDown, Key_Z, Key_X, Key_C, Key_V, Key_B, Key_Escape,
   Key_LeftControl, Key_Backspace, Key_LeftGui, Key_LeftShift,
   LockLayer(GAMING),

   Key_skip,  Key_6, Key_7, Key_8,     Key_9,      Key_0,         Key_skip,
   Key_Enter, Key_Y, Key_U, Key_I,     Key_O,      Key_P,         Key_Equals,
              Key_H, Key_J, Key_K,     Key_L,      Key_Semicolon, Key_Quote,
   Key_skip,  Key_N, Key_M, Key_Comma, Key_Period, Key_Slash,     Key_Minus,
   Key_RightShift, Key_RightAlt, Key_Spacebar, Key_RightControl,
   LockLayer(GAMING)),

  [GAMING] = KEYMAP_STACKED
  (___, ___,   ___,   ___,   ___,   ___,   ___,
   ___, Key_W, Key_W, Key_W, Key_W, Key_W, ___,
   ___, Key_S, Key_S, Key_S, Key_S, Key_S,
   ___, Key_Z, Key_X, Key_C, Key_V, Key_B, ___,
   ___, ___, ___, ___,
   UnlockLayer(GAMING),

   ___, ___, ___, ___, ___, ___, ___,
   ___, Key_MyRepeaterKey, ___, ___, ___, ___, ___,
        ___, ___, ___, ___, ___, ___,
   ___, ___, ___, ___, ___, ___, ___,
   ___, ___, ___, ___,
   UnlockLayer(GAMING))
)
// *INDENT-ON*

KALEIDOSCOPE_INIT_PLUGINS(Repeater);

void setup() {
  Kaleidoscope.setup();
  REGISTER_REPEATERS(
    // On tapping W, repeat W until either W or S is pressed.
    {Key_W, Key_W, Key_W},
    {Key_NoKey, Key_W, Key_S},
    // On tapping the MyRepeaterKey, keep LeftShift held, until
    // MyRepeaterKey is pressed again.
    {Key_MyRepeaterKey, Key_LeftShift, Key_MyRepeaterKey}
  );
  // Make Repeater only act while we are using the GAMING layer.
  Repeater.limited_to_layer = GAMING;
}

void loop() {
  Kaleidoscope.loop();
}
