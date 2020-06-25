#pragma once

#include <Kaleidoscope.h>
#include "kaleidoscope/Runtime.h"
#include "kaleidoscope/keyswitch_state.h"

#ifndef REPEATER_MAX_CANCEL_KEYS
#define REPEATER_MAX_CANCEL_KEYS 1
#endif

#ifndef REPEATER_MAX_HELD_KEYS
#define REPEATER_MAX_HELD_KEYS 4
#endif

// TODO: Would it make sense to move the repeater_list to progmem?
//
// Macro to be used in the sketch files `setup` function.
#define REGISTER_REPEATERS(repeater_defs...) {                         \
  static const Key _repeater_list[][2 + REPEATER_MAX_CANCEL_KEYS] = {  \
    repeater_defs                                                      \
  };                                                                   \
  Repeater.registerRepeaterList(_repeater_list);                       \
}


namespace kaleidoscope {
namespace plugin {

struct TrackedKey {
  /** Either a key to be repeated or an action key to be timed. */
  Key key;
  /**
   * If the key is an action key that is currently timed for tap/hold.
   */
  // TODO: Maybe we can make this a bitfield that uses one bit from
  // the key field. - or track this in a single bitfield outside the
  // tracked key, making the struct unnecessary. Even using a 16 bit
  // int to track this would be the same memory for 16 keys at is
  // currently for two.
  bool is_timer;
};

class Repeater : public kaleidoscope::Plugin {

 public:
  /** If this doesn't match the topmost active layer, pause repeating. */
  static uint8_t limited_to_layer;

  /** Set how fast a pressed key needs to be released to count as a tap. */
  static void setTapTimeout(uint8_t tap_timeout) {
   	tap_timeout_ = tap_timeout;
  }

  static void activate() {
	is_active_ = true;
  }

  static void deactivate() {
	is_active_ = false;
	stopAll();
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

  /** Stop repeating all keys and timers. */
  static void stopAll() {
	  for (uint8_t i = 0; i < REPEATER_MAX_HELD_KEYS; i++) {
		  tracked_keys_[i].key = Key_NoKey;
	  }
  }

  /**
   * Register the list of repeater key combinations.
   *
   * Using the `REGISTER_REPEATERS` macro takes care of this and is
   * the safest option. So unless you need to do something fancy, you
   * shouldn't call this yourself.
   */
  template <uint8_t num_entries>
  static void registerRepeaterList(Key const(&entries)[num_entries][2 + REPEATER_MAX_CANCEL_KEYS]) {
	  repeater_list_ = entries;
	  num_registered_ = num_entries;
  }

  EventHandlerResult onKeyswitchEvent(Key &mapped_key, KeyAddr key_addr, uint8_t key_state);
  EventHandlerResult beforeReportingState();

 private:
  static bool is_active_;
  static const Key (*repeater_list_)[2 + REPEATER_MAX_CANCEL_KEYS];
  static uint8_t num_registered_;
  static TrackedKey tracked_keys_[REPEATER_MAX_HELD_KEYS];
  static uint8_t tap_timeout_;
  // Time at which any action key got pressed. We use only one timer
  // to save memory - it is extremely unlikely that multiple keys
  // would require precise timing for something so short as a tap.
  static uint16_t tap_start_time_;

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
   * Start timing the key for tap/hold.
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
	    tap_start_time_ = Runtime.millisAtCycleStart();
        tracked_keys_[i].key = key;
        tracked_keys_[i].is_timer = true;
        Kaleidoscope.serialPort().print("registered timer for ");
        Kaleidoscope.serialPort().println(tracked_keys_[i].key.getKeyCode());
        return;
      }
    }
  }

  static bool isCancelKey(Key key, uint8_t list_idx) {
    for (uint8_t ckey_idx = 0; ckey_idx < REPEATER_MAX_CANCEL_KEYS; ckey_idx++) {
      if (key == repeater_list_[list_idx][ckey_idx + TARGET_KEY_IDX + 1]) {
        return true;
      }
    }
    return false;
  }
};

}
}

extern kaleidoscope::plugin::Repeater Repeater;
