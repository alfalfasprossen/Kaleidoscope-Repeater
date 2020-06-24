#include "Kaleidoscope-Repeater.h"

namespace kaleidoscope {
namespace plugin {

TrackedKey Repeater::tracked_keys_[REPEATER_MAX_HELD_KEYS];

}
}

kaleidoscope::plugin::Repeater Repeater;
