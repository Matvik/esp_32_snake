#include "Game.h"

// main.cpp is now just a thin wrapper that constructs the game object and
// forwards the Arduino setup()/loop() calls. All of the logic lives in
// SnakeGame (split across several source files).

SnakeGame game;

// Arduino setup hook initializes game runtime.
void setup() {
    game.begin();
}

// Arduino loop hook executes one game tick.
void loop() {
    game.run();
}