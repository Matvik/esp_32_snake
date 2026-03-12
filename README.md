# ESP32 Snake Game

A classic Snake game for ESP32 with an SSD1306 OLED display, two buttons, persistent high score storage, and a refactored architecture built around a testable gameplay core.

## Overview

The project targets ESP32 using PlatformIO and Arduino framework. The codebase is split into three clear layers:

- `SnakeGame`: application shell with input handling, state machine, and persistence.
- `snake_core`: pure gameplay logic used both by firmware and unit tests.
- `snake_render`: OLED rendering and screen animations.

This separation keeps the firmware simple while allowing most game rules to be tested on a desktop machine without ESP32 hardware.

## Gameplay

### Features
- Game field: `32 x 14` cells on a `128 x 64` OLED.
- Two-button control: left turn and right turn.
- Random food placement with collision-safe spawning.
- Random obstacles generated on reset.
- Animated splash, collision blink, and game-over screens.
- High score storage in ESP32 NVS using `Preferences`.
- Long-press reset of high score on splash screen.

### Controls
- Left button: `GPIO 4`
- Right button: `GPIO 18`
- Start game: press and release any button on splash.
- Reset high score: hold both buttons for 2 seconds on splash.
- Return from Game Over: press any button.

## Hardware

### Required parts
- 1 x ESP32 DOIT DEVKIT V1
- 1 x SSD1306 OLED `128x64` I2C display
- 2 x momentary push buttons
- Breadboard or prototyping board
- Jumper wires
- USB cable

### Wiring

![ESP32 Snake Wiring](assets/esp32-snake-wiring.svg)

Vector source: `assets/esp32-snake-wiring.svg`

#### OLED

| OLED | ESP32 |
|---|---|
| VCC | 3.3V |
| GND | GND |
| SDA | GPIO 21 |
| SCL | GPIO 22 |

#### Buttons
- Left button signal -> `GPIO 4`
- Right button signal -> `GPIO 18`
- Second leg of each button -> `GND`
- Button handling is active-low

### Assembly notes
- Do not connect `5V` directly to ESP32 GPIO pins.
- Power the OLED from `3.3V` unless your module explicitly supports another setup.
- Keep button wires short to reduce false triggering.
- Make sure ESP32, OLED, and both buttons share ground.

## Setup

### Firmware build and upload
1. Install VS Code.
2. Install the PlatformIO IDE extension.
3. Clone the repository:

```bash
git clone https://github.com/Matvik/esp_32_snake.git
```

4. Open the project in VS Code.
5. Wait for PlatformIO to install dependencies from `platformio.ini`.
6. Connect the ESP32 board.
7. Build firmware:

```bash
pio run
```

8. Upload firmware:

```bash
pio run --target upload
```

9. Open serial monitor if needed:

```bash
pio device monitor -b 115200
```

### Native unit tests

The project also defines a `native` PlatformIO environment for desktop unit tests of the gameplay core.

Run tests with:

```bash
pio test -e native
```

If `pio` is not available in your shell, run the command from a PlatformIO-enabled terminal inside VS Code.

## Architecture

### Runtime flow
- `src/main.cpp`: Arduino `setup()` / `loop()` entrypoint.
- `src/Game.cpp`: state machine, button handling, high score storage, and integration of core + renderer.
- `lib/snake_core`: pure gameplay logic.
- `lib/snake_render`: OLED drawing and animations.

### Core module

Files:
- `lib/snake_core/include/SnakeCore.h`
- `lib/snake_core/src/SnakeCore.cpp`

Responsibilities:
- snake movement
- wrap-around behavior
- body and obstacle collision checks
- obstacle generation on reset
- food spawning
- speed progression
- deterministic behavior suitable for tests

### Render module

Files:
- `lib/snake_render/include/SnakeRenderer.h`
- `lib/snake_render/src/SnakeRenderer.cpp`

Responsibilities:
- gameplay frame rendering
- splash screen animation
- collision blink effect
- game-over animation
- food blinking
- HUD drawing

### Application shell

Files:
- `include/Game.h`
- `src/Game.cpp`

Responsibilities:
- state transitions: `SPLASH`, `PLAYING`, `COLLISION`, `GAME_OVER`
- button polling with `ezButton`
- long-press handling
- persistent high score storage with `Preferences`
- wiring together core and renderer

## Tests

Test file:
- `test/test_snake_core/test_main.cpp`

Current unit tests cover:
- initial reset state
- direction changes
- movement and wrap-around
- self-collision and obstacle collision
- food consumption and speed-up
- minimum speed clamp
- safe food spawning
- reset layout validity
- long-run movement invariants
- repeated reset invariants

The tests are focused on `SnakeCore`, because it contains the gameplay rules and has no dependency on ESP32 display or button hardware.

## Project structure

```text
include/
  Game.h
lib/
  snake_core/
    include/
      SnakeCore.h
    src/
      SnakeCore.cpp
  snake_render/
    include/
      SnakeRenderer.h
    src/
      SnakeRenderer.cpp
src/
  Game.cpp
  main.cpp
test/
  test_snake_core/
    test_main.cpp
assets/
  esp32-snake-wiring.svg
platformio.ini
README.md
README.uk.md
```

## Notes

- The old split into `GameLogic.cpp` and `GameRender.cpp` has been removed.
- Gameplay logic now lives in `snake_core`.
- Rendering logic now lives in `snake_render`.
- Method-level comments were added across the project to make navigation easier.

---

## Ukrainian version

Full Ukrainian documentation is available in [README.uk.md](README.uk.md).
