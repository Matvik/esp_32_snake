#ifndef SNAKE_RENDERER_H
#define SNAKE_RENDERER_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "SnakeCore.h"

namespace snakerender {

class SnakeRenderer {
public:
    // Creates renderer for given display geometry.
    SnakeRenderer(int screenWidth, int screenHeight, int scoreArea, int cellSize);

    // Initializes OLED display backend.
    void begin();

    // Draws full gameplay frame.
    void drawGame(const snakecore::SnakeCore& core, int highScore, bool mouthOpen);
    // Draws splash screen with optional helper prompt.
    void drawSplash(int screenWidth, int screenHeight, int highScore,
                    bool showReleaseMessage, unsigned long splashStartTime);
    // Draws collision blink animation frame.
    void drawCollision(const snakecore::SnakeCore& core, int highScore, bool mouthOpen,
                       unsigned long collisionStartTime);
    // Draws game-over animation and final score.
    void drawGameOver(const snakecore::SnakeCore& core, int screenWidth, int screenHeight,
                      unsigned long splashStartTime);

private:
    // Draws snake eye pixels based on direction.
    void drawEyes(int x, int y, int dir);
    // Draws optional snake mouth pixel.
    void drawMouth(int x, int y, int dir, bool mouthOpen);
    // Draws snake body and head.
    void drawSnake(const snakecore::SnakeCore& core, bool mouthOpen);
    // Draws all obstacles.
    void drawObstacles(const snakecore::SnakeCore& core);
    // Draws score HUD.
    void drawScore(const snakecore::SnakeCore& core, int highScore);
    // Draws blinking food item.
    void drawFood(const snakecore::SnakeCore& core);

    Adafruit_SSD1306 display;
    int scoreArea;
    int cellSize;

    bool foodOn;
    unsigned long lastFoodBlink;
    unsigned long foodBlinkInterval;
};

}  // namespace snakerender

#endif  // SNAKE_RENDERER_H
