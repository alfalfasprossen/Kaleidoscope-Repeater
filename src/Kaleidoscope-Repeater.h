#pragma once

#include <Kaleidoscope.h>
#include "kaleidoscope/Runtime.h"
#include "kaleidoscope/key_events.h"
#include "kaleidoscope/keyswitch_state.h"


// TODO: use defines to allow using multiple cancel keys

// TODO: we could use a limit-to-layers array or so

// TODO: when using multiple cancel keys, we need to add a blank key at the end
#define REPEATER_MAX_CANCEL_KEYS 1
// TODO: move to cpp
// #define REPEATER_ACTION_KEY_IDX 0
// #define REPEATER_TARGET_KEY_IDX 1

#ifndef REPEATER_MAX_HELD_KEYS
#define REPEATER_MAX_HELD_KEYS 4
#endif

namespace kaleidoscope {
namespace plugin {

struct TrackedKey {
  /** Either a key to be repeated or an action key to be timed. */
  Key key;
  /** Start time when this key was tapped if it is an action key. */
  uint16_t tap_start_time;
  /**
   * If the key is an action key that is currently timed for tap/hold.
   */
  // TODO: Maybe we can make this a bitfield that uses one bit
  // from the key or time field.
  bool is_timer;
};

class Repeater : public kaleidoscope::Plugin {

 public:
  static const Key repeater_list[1][3]; //[8][2 + REPEATER_MAX_CANCEL_KEYS];

  // static void setHoldTimeout(uint16_t hold_timeout) {
  // 	hold_timeout_ = hold_timeout;
  // }

  EventHandlerResult onKeyswitchEvent(Key &mapped_key, KeyAddr key_addr, uint8_t key_state) {
    // each entry like (action_key, target_key, cancel_keys...)
    // for key in registered keys
    //   if key is held - ignore
    //   if key is pressed
    //     if is action key
    //       if not process for target key running - start timer for action key
    //     if is cancel key - cancel related action key process
    //   if key is released
    //     if not action key - ignore
    //     if no timer for action key running - ignore
    //     if timer for key > threshold - this is a tap, set process for target key

    // Not a real key, possibly injected by some plugin like
    // this one, ignore it.
    if (!key_addr.isValid() || (key_state & INJECTED)) {
      return EventHandlerResult::OK;
    }

    // if (keyIsPressed(key_state)) {
    // 	return EventHandlerResult::OK;
    // }

    if (keyToggledOn(key_state)) {
      // Find all entries where this key triggers an action or cancels
      // a held key.
      for (uint8_t list_idx = 0; list_idx < num_registered_; list_idx++) {
        // Kaleidoscope.serialPort().print(list_idx);
        if (mapped_key == repeater_list[list_idx][ACTION_KEY_IDX]) {
          Kaleidoscope.serialPort().println("Is action key");
          if (!isRepeating(repeater_list[list_idx][TARGET_KEY_IDX]) &&
              !isTiming(mapped_key)) {
            startTimer(mapped_key);
          }
        }
        if (isCancelKey(mapped_key, list_idx)) {
          Kaleidoscope.serialPort().println("Is cancel key");
          stop(repeater_list[list_idx][TARGET_KEY_IDX]);
        }
      }
    } else if (keyToggledOff(key_state)) {
      // If this is an action key whose tap timer is running, check if
      // this was a tap. A tap causes the target key to be marked for
      // repetition.
      for (uint8_t list_idx = 0; list_idx < num_registered_; list_idx++) {
        if (mapped_key != repeater_list[list_idx][ACTION_KEY_IDX]) {
          continue;
        }
        Kaleidoscope.serialPort().println("action key released");
        for (uint8_t i = 0; i < REPEATER_MAX_HELD_KEYS; i++) {
          Kaleidoscope.serialPort().println(tracked_keys_[i].key.getKeyCode());
          Kaleidoscope.serialPort().println(mapped_key.getKeyCode());
          if (tracked_keys_[i].is_timer && tracked_keys_[i].key == mapped_key) {
            Kaleidoscope.serialPort().println("is timer");
            uint16_t held_time = Runtime.millisAtCycleStart() - tracked_keys_[i].tap_start_time;
            Kaleidoscope.serialPort().print("was held for ");
            Kaleidoscope.serialPort().println(held_time);
            if (held_time > tap_timeout) {
              // This key was held down for too long, stop tracking it.
              tracked_keys_[i].key = Key_NoKey;
              break;
            }
            // This key was tapped.
            Kaleidoscope.serialPort().println("key was tapped");
            if (isRepeating(repeater_list[list_idx][TARGET_KEY_IDX])) {
              // In case the target key is already repeating, just
              // clear the space. This might happen when another
              // action key has the same target.
              Kaleidoscope.serialPort().println("is already repeating");
              tracked_keys_[i].key = Key_NoKey;
            } else {
              Kaleidoscope.serialPort().println("setting repeater");
              tracked_keys_[i].key = repeater_list[list_idx][TARGET_KEY_IDX];
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

  EventHandlerResult beforeReportingState() {
    for (uint8_t i = 0; i < REPEATER_MAX_HELD_KEYS; i++) {
      if (!tracked_keys_[i].is_timer && tracked_keys_[i].key != Key_NoKey) {
        Kaleidoscope.serialPort().println("would inject key");
        // handleKeyswitchEvent(tracked_keys_[i].key, UnknownKeyswitchLocation, IS_PRESSED | INJECTED);
      }
    }
    return EventHandlerResult::OK;
  }


  /** Stop repeating the key. */
  static void stop(Key key) {
    for (uint8_t i = 0; i < REPEATER_MAX_HELD_KEYS; i++) {
      if (!tracked_keys_[i].is_timer && tracked_keys_[i].key == key) {
        Kaleidoscope.serialPort().print("stopping key ");
        Kaleidoscope.serialPort().println(key.getKeyCode());
        tracked_keys_[i].key = Key_NoKey;
      }
    }
  }


 private:
  static const uint8_t num_registered_ = 1; // TODO: dynamicallize, see qukeys plugin
  // static Key repeating_keys_[REPEATER_MAX_HELD_KEYS];
  // static uint16_t tap_start_times_[REPEATER_MAX_HELD_KEYS];
  static TrackedKey tracked_keys_[REPEATER_MAX_HELD_KEYS];
  static const uint8_t tap_timeout = 150;

  static constexpr uint8_t ACTION_KEY_IDX = 0;
  static constexpr uint8_t TARGET_KEY_IDX = 1;

  static bool isRepeating(Key key) {
    for (uint8_t i = 0; i < REPEATER_MAX_HELD_KEYS; i++) {
      if (!tracked_keys_[i].is_timer && tracked_keys_[i].key == key) {
        return true;
      }
    }
    return false;
  }

  static bool isTiming(Key key) {
    for (uint8_t i = 0; i < REPEATER_MAX_HELD_KEYS; i++) {
      if (tracked_keys_[i].is_timer && tracked_keys_[i].key == key) {
        return true;
      }
    }
    return false;
  }

  /**
   * Set the tap start time for action key `key`.
   *
   * No check for duplicates is performed, the caller has to make sure
   * to not call this for already timing keys. If there is no free
   * slot to track the key, because there are already the maximum
   * number of keys being tracked, the call has no effect.
   */
  static void startTimer(Key key) {
    Kaleidoscope.serialPort().println("starting timer");
    for (uint8_t i = 0; i < REPEATER_MAX_HELD_KEYS; i++) {
      if (tracked_keys_[i].key == Key_NoKey) {
        tracked_keys_[i].key = key;
        tracked_keys_[i].tap_start_time = Runtime.millisAtCycleStart();
        tracked_keys_[i].is_timer = true;
        Kaleidoscope.serialPort().print("registered timer for ");
        Kaleidoscope.serialPort().println(tracked_keys_[i].key.getKeyCode());
        return;
      }
    }
  }

  static bool isCancelKey(Key key, uint8_t list_idx) {
    for (uint8_t ckey_idx = 0; ckey_idx < REPEATER_MAX_CANCEL_KEYS; ckey_idx++) {
      if (key == repeater_list[list_idx][ckey_idx + TARGET_KEY_IDX + 1]) {
        return true;
      }
    }
    return false;
  }
};
}
}

extern kaleidoscope::plugin::Repeater Repeater;