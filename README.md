# Vulkan Renderer

## Coordinates

- upward: +y
- forward: -z
- right: +x
- default camera looks at -z

## Vulkan fragment shader stage range

- x: [-1, 1] from left to right
- y: [-1, 1] from top to bottom
- z: [ 0, 1] from front to back

## Build setting
- The engine MUST be compiled by a 64 bit compiler. 
- For Visual Studio: x86_amd64 is good, but x86 does NOT work