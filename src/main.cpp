#include "Game.h"

// main.cpp is now just a thin wrapper that constructs the game object and
// forwards the Arduino setup()/loop() calls. All of the logic lives in
// SnakeGame (split across several source files).

SnakeGame game;

void setup() {
    game.begin();
}

void loop() {
    game.run();
}