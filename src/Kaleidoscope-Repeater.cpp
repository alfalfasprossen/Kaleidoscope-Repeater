#include "Kaleidoscope-Repeater.h"
#include "kaleidoscope/layers.h"

namespace kaleidoscope {
namespace plugin {

bool Repeater::is_active_ = true;
TrackedKey Repeater::tracked_keys_[REPEATER_MAX_HELD_KEYS];
uint8_t Repeater::tap_timeout_ = 150;
uint8_t Repeater::num_registered_ = 0;
const Key(* Repeater::repeater_list_)[2 + REPEATER_MAX_CANCEL_KEYS] = nullptr;
uint16_t Repeater::tap_start_time_;
uint8_t Repeater::limited_to_layer = 255;


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
      // Kaleidoscope.serialPort().print(list_idx);
      if (mapped_key == repeater_list_[list_idx][ACTION_KEY_IDX]) {
        Kaleidoscope.serialPort().println("Is action key");
        if (!isRepeating(repeater_list_[list_idx][TARGET_KEY_IDX]) &&
            !isTiming(mapped_key)) {
          startTimer(mapped_key);
        }
      }
      if (isCancelKey(mapped_key, list_idx)) {
        Kaleidoscope.serialPort().println("Is cancel key");
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
      Kaleidoscope.serialPort().println("action key released");
      for (uint8_t i = 0; i < REPEATER_MAX_HELD_KEYS; i++) {
        Kaleidoscope.serialPort().println(tracked_keys_[i].key.getKeyCode());
        Kaleidoscope.serialPort().println(mapped_key.getKeyCode());
        if (tracked_keys_[i].is_timer && tracked_keys_[i].key == mapped_key) {
          Kaleidoscope.serialPort().println("is timer");
          uint16_t held_time = Runtime.millisAtCycleStart() - tap_start_time_;
          Kaleidoscope.serialPort().print("was held for ");
          Kaleidoscope.serialPort().println(held_time);
          if (held_time > tap_timeout_) {
            // This key was held down for too long, stop tracking it.
            tracked_keys_[i].key = Key_NoKey;
            break;
          }
          // This key was tapped.
          Kaleidoscope.serialPort().println("key was tapped");
          if (isRepeating(repeater_list_[list_idx][TARGET_KEY_IDX])) {
            // In case the target key is already repeating, just clear
            // the space. This might happen when another action key
            // has the same target.
            Kaleidoscope.serialPort().println("is already repeating");
            tracked_keys_[i].key = Key_NoKey;
          } else {
            Kaleidoscope.serialPort().println("setting repeater");
            tracked_keys_[i].key = repeater_list_[list_idx][TARGET_KEY_IDX];
            tracked_keys_[i].is_timer = false;
          }
          // We assume only one timer can run per action key. Other
          // functions should've taken care of that.
          break;
        }
      }
    }
  }
  // TODO: maybe add a weak function to allow consuming key events.
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
    if (!tracked_keys_[i].is_timer && tracked_keys_[i].key != Key_NoKey) {
      Kaleidoscope.serialPort().println("would inject key");
      // handleKeyswitchEvent(tracked_keys_[i].key, UnknownKeyswitchLocation, IS_PRESSED | INJECTED);
    }
  }
  return EventHandlerResult::OK;
}

}
}

kaleidoscope::plugin::Repeater Repeater;
