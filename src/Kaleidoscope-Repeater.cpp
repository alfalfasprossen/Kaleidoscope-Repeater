#include "Kaleidoscope-Repeater.h"

namespace kaleidoscope {
namespace plugin {

TrackedKey Repeater::tracked_keys_[REPEATER_MAX_HELD_KEYS];
uint8_t Repeater::tap_timeout_ = 150;
uint8_t Repeater::num_registered_ = 0;
const Key (* Repeater::repeater_list_)[2 + REPEATER_MAX_CANCEL_KEYS] = nullptr;

}
}

kaleidoscope::plugin::Repeater Repeater;
