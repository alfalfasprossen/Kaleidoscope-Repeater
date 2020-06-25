# Kaleidoscope-Repeater

TODO

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

void setup () {
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
cancel key.

### Limiting To A Specific Layer

Setting `Repeater.limited_to_layer` to a layer index will cause
Repeater to pause (not react to keypresses as well) while the set
layer is not the top active layer.

```c++
// Layers
enum {TYPING, GAMING};

KEYMAPS(
  [TYPING] = (...),

  [GAMING] = (...)
)

KALEIDOSCOPE_INIT_PLUGINS(Repeater);

void setup () {
  Repeater.limited_to_layer = GAMING;
}
```

### TODO
advanced usage like a fake key that doesnt produce an actual keycode


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

### `.setTapTimeout(timeout)`

> Set the time in which a key must be released after being pressed, to
> count as a tap. Default is 150 ms, maximum is 255 ms.

### `.stop(key)`

> Stop repeating a specific key.

### `.stopAll()`

> Stop repeating (or tap-timing) all currently tracked keys.
