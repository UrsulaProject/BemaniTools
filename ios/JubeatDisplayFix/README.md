# Jubeat Display Fix

This Theos tweak fixes the original iPad game's hard-coded display scale on
modern iOS devices.

IDA verification of the original binary established that:

- `-[GameViewController loadResources]` initializes `displayScale` to `1.0` and
  only recognizes the legacy `deviceType` values 3, 4, and 5.
- `-[GameViewController loop:]` obtains each touch with `locationInView:` and
  divides both coordinates by `displayScale` before testing the fixed
  `768 x 1024` Pad button grid.
- Rendering and UIKit overlays consume the same `displayScale`, so correcting
  the field keeps drawing and touch hit testing in one coordinate system.

The replacement scale is calculated from the short and long sides of the
actual EAGL view. This gives the same result for portrait and landscape bounds.
When the view itself exposes landscape coordinates, the `UITouch` hook converts
the point through `UIScreen.fixedCoordinateSpace` before the original game
divides it by the scale. The conversion is limited to the active Pad
`GameViewController` and its exact EAGL view.

Build with:

```sh
THEOS=/path/to/theos make package
```
