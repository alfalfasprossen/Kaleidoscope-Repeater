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

#include "Kaleidoscope-Repeater.h"
#include "kaleidoscope/layers.h"

namespace kaleidoscope {
namespace plugin {

bool Repeater::is_active_ = true;
Key Repeater::tracked_keys_[REPEATER_MAX_HELD_KEYS];
uint8_t Repeater::tap_timeout_ = 150;
uint8_t Repeater::num_registered_ = 0;
const Key(* Repeater::repeater_list_)[2 + REPEATER_MAX_CANCEL_KEYS] = nullptr;
uint16_t Repeater::tap_start_time_;
uint8_t Repeater::limited_to_layer = 255;
uint8_t Repeater::is_timer_bits_ = 0;


EventHandlerResult Repeater::onKeyswitchEvent(Key &mapped_key, KeyAddr key_addr, uint8_t key_state) {
  if (!is_active_) {
    return EventHandlerResult::OK;
  }

  if (limited_to_layer < 255 && limited_to_layer != Layer.top()) {
	return EventHandlerResult::OK;
  }

  // Not a real key, possibly injected by some plugin like this one,
  // ignore it.
  if (!key_addr.isValid() || (key_state & INJECTED)) {
    return EventHandlerResult::OK;
  }

  if (keyToggledOn(key_state)) {
    // Find all entries where this key triggers an action or cancels
    // a held key.
    for (uint8_t list_idx = 0; list_idx < num_registered_; list_idx++) {
      if (mapped_key == repeater_list_[list_idx][ACTION_KEY_IDX]) {
        if (!isRepeating(repeater_list_[list_idx][TARGET_KEY_IDX]) &&
            !isTiming(mapped_key)) {
          startTimer(mapped_key);
        }
      }
      if (isCancelKey(mapped_key, list_idx)) {
        stop(repeater_list_[list_idx][TARGET_KEY_IDX]);
      }
    }
  } else if (keyToggledOff(key_state)) {
    // If this is an action key whose tap timer is running, check if
    // this was a tap. A tap causes the target key to be marked for
    // repetition.
    for (uint8_t list_idx = 0; list_idx < num_registered_; list_idx++) {
      if (mapped_key != repeater_list_[list_idx][ACTION_KEY_IDX]) {
        continue;
      }
      for (uint8_t i = 0; i < REPEATER_MAX_HELD_KEYS; i++) {
        if (_REPEATER_IS_TIMER(i) && tracked_keys_[i] == mapped_key) {
          uint16_t held_time = Runtime.millisAtCycleStart() - tap_start_time_;
          if (held_time > tap_timeout_) {
            // This key was held down for too long, stop tracking it.
            tracked_keys_[i] = Key_NoKey;
            break;
          }
          // This key was tapped.
          if (isRepeating(repeater_list_[list_idx][TARGET_KEY_IDX])) {
            // In case the target key is already repeating, just clear
            // the space. This might happen when another action key
            // has the same target.
            tracked_keys_[i] = Key_NoKey;
          } else {
            tracked_keys_[i] = repeater_list_[list_idx][TARGET_KEY_IDX];
            _REPEATER_SET_TIMER_OFF(i);
          }
          // We assume only one timer can run per action key. Other
          // functions should've taken care of that.
          break;
        }
      }
    }
  }
  return EventHandlerResult::OK;
}

EventHandlerResult Repeater::beforeReportingState() {
  if (!is_active_) {
    return EventHandlerResult::OK;
  }

  if (limited_to_layer < 255 && limited_to_layer != Layer.top()) {
	return EventHandlerResult::OK;
  }

  for (uint8_t i = 0; i < REPEATER_MAX_HELD_KEYS; i++) {
    if (!_REPEATER_IS_TIMER(i) && tracked_keys_[i] != Key_NoKey) {
      handleKeyswitchEvent(tracked_keys_[i], UnknownKeyswitchLocation,
                           IS_PRESSED | WAS_PRESSED | INJECTED);
    }
  }
  return EventHandlerResult::OK;
}

}
}

kaleidoscope::plugin::Repeater Repeater;
