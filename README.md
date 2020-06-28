# Kaleidoscope-Repeater

Tap keys to make keys act as being held down until cancelled.

![Hold that key](./stone.png)

Ever got annoyed at - or pain in your middle finger from - holding
down the `W` key when exploring open world games? Are you even one of
those that once put a weight onto a key to have it held down? Some old
games had something called *auto-walk*, a key you could press once to
move forward until you either moved forward or backward by pressing a
respective other key.

Repeater lets you define keys to cause the repetition (act as if being
held) of either the same or any other key when a defined action key is
tapped - holding a key will not trigger a repetition. A repeating key
can be cancelled by specifying cancelling keys for the repeatable key,
they can also be the same key.

## Installation

Download or clone into your Arduino library folder. Depending on your
OS this should be inside `$HOME/Documents/Arduino` or `$HOME/Arduino`.

## Using The Plugin

Include the header and initialize `Repeater`. The order should not
matter as Repeater generally ignores key events that might have been
emitted from other plugins.

Define the repeater key combinations and register them with Repeater
within the sketches `setup()` function.

The macro `REGISTER_REPEATERS` is provided for convenience.

```c++
#include <Kaleidoscope.h>
#include <Kaleidoscope-Repeater.h>

KALEIDOSCOPE_INIT_PLUGINS(Repeater);

void setup() {
  Kaleidoscope.setup();

  REGISTER_REPEATERS(
    {Key_W, Key_W, Key_W},
    {Key_NoKey, Key_W, Key_S}
  );
}
```

Each entry consists of three keys `{ActionKey, TargetKey, CancelKey}`.
When an ActionKey is tapped, the TargetKey will be emitted as being
pressed until a CancelKey that is associated with the same TargetKey
is pressed.

The example above would emit `Key_W` when `Key_W` is tapped. It will
stop, when `Key_W` is pressed again, or when `Key_S` is pressed.

Or in a gaming context: When I tap forward, keep moving forward
automatically. If I press forward again or press backward, stop moving
forward automatically.

You can use `Key_NoKey` to define a repeater without a real action or
cancel key - for example to define multiple cancel keys for the same
target.

*Note: Key events are not consumed by Repeater. Any keypress - action
key, cancel key or else - is forwarded as-is to the host or followup
plugins.*

### Limiting To A Specific Layer

Setting `Repeater.limited_to_layer` to a layer index will cause
Repeater to pause (not react to keypresses as well) while the set
layer is not the top active layer.

```c++
// Layers
enum {TYPING, GAMING};

KEYMAPS(
  [TYPING] = KEYMAP_STACKED(...),

  [GAMING] = KEYMAP_STACKED(...)
)

KALEIDOSCOPE_INIT_PLUGINS(Repeater);

void setup () {
  Repeater.limited_to_layer = GAMING;
}
```

### Using Synthetic Keys

You may not want to use actual keys but a synthetic key to use as an
action key. There are no pre-defined synthetic keys to use with
Repeater, but you can simply define some yourself and use them in the
repeater definitions.

``` c++
// Ranges required to define syntethic keys that don't interfere with
// other plugins.
#include <Kaleidoscope-Ranges.h>

// Define a synthetic key to use in the keymap. If you use multiple,
// increase the key value, e.g. `SAFE_START + 1`.
static constexpr Key Key_MyRepeaterKey{kaleidoscope::ranges::SAFE_START};

KEYMAPS(
  [PRIMARY] = KEYMAP_STACKED
  (// Add your key somewhere in the keymap.
   Key_MyRepeaterKey, ...)
)

void setup() {
  Kaleidoscope.setup();

  REGISTER_REPEATERS(
    {Key_MyRepeaterKey, Key_W, Key_MyRepeaterKey}
  );
}
```

## Plugin Properties

The plugin provides the `Repeater` object, which has the following
properties:

### `.limited_to_layer`

> If set to a layer index, Repeater will only operate while that layer
> is the top active layer. It will pause otherwise.


## Plugin Methods

### `.deactivate()`

> Stop processing and clear any active repeating keys.

### `.activate()`

> Re-activate the plugin if it was deactivated.

### `.isActive()`

> Get the current activity state - useful if you need to conditionally
> toggle the plugin.

### `.setTapTimeout(timeout)`

> Set the time in which a key must be released after being pressed, to
> count as a tap. Default is 150 ms, maximum is 255 ms.

### `.stop(key)`

> Stop repeating a specific key.

### `.stopAll()`

> Stop repeating (or tap-timing) all currently tracked keys.
