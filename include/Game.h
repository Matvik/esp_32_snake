#ifndef GAME_H
#define GAME_H

#include <Arduino.h>
#include <ezButton.h>
#include <Preferences.h>
#include "SnakeCore.h"
#include "SnakeRenderer.h"

// -----------------------------------------------------------------------------
// Main game class managing Snake game logic, rendering, and input
// -----------------------------------------------------------------------------
class SnakeGame {
public:
    // Constructs game controller and owned runtime components.
    SnakeGame();
    // Initializes hardware and persisted state.
    void begin();
    // Executes one main-loop tick.
    void run();

private:
    // -----------------------------------------------------------------------------
    // Game states enumeration
    // -----------------------------------------------------------------------------
    enum GameState {
        SPLASH,     // Initial screen with animation and high score
        PLAYING,    // Active gameplay
        COLLISION,  // Collision animation (blinking snake)
        GAME_OVER   // Game over screen
    };
    GameState gameState;

    // -----------------------------------------------------------------------------
    // Constants for display and game grid
    // -----------------------------------------------------------------------------
    static constexpr int SCREEN_WIDTH = 128;          // OLED display width in pixels
    static constexpr int SCREEN_HEIGHT = 64;          // OLED display height in pixels
    static constexpr int SCORE_AREA = 8;              // Height of score area at bottom
    static constexpr int CELL_SIZE = 4;               // Size of each grid cell in pixels
    static constexpr int GRID_WIDTH = SCREEN_WIDTH / CELL_SIZE;  // Number of cells horizontally
    static constexpr int GRID_HEIGHT = (SCREEN_HEIGHT - SCORE_AREA) / CELL_SIZE;  // Number of cells vertically

    static constexpr int BUTTON_LEFT_PIN = 4;         // GPIO pin for left button
    static constexpr int BUTTON_RIGHT_PIN = 18;       // GPIO pin for right button

    static constexpr unsigned long LONG_PRESS_DURATION_MS = 2000;  // Duration for long press (reset high score)
    static constexpr unsigned long COLLISION_BLINK_DURATION_MS = 2000;  // Duration of collision blink

    // -----------------------------------------------------------------------------
    // Hardware interfaces
    // -----------------------------------------------------------------------------
    ezButton buttonLeft;        // Left button handler
    ezButton buttonRight;       // Right button handler
    Preferences prefs;          // Non-volatile storage for high score
    snakecore::SnakeCore core;  // Pure gameplay core used by runtime and tests
    snakerender::SnakeRenderer renderer;

    // Number of random obstacles generated per run.
    int numObstacles;

    bool turned;
    unsigned long lastMove;
    int highScore;
    bool mouthOpen;

    // -----------------------------------------------------------------------------
    // Splash screen animation
    // -----------------------------------------------------------------------------
    unsigned long splashStartTime;

    // -----------------------------------------------------------------------------
    // Button press detection variables
    // -----------------------------------------------------------------------------
    unsigned long leftButtonPressStart;   // Timestamp when left button was pressed
    bool wasLongPress;          // Flag if a long press was detected
    bool bothPressed;           // Flag if both buttons are pressed
    bool anyPressed;            // Flag if any button is pressed
    bool showReleaseMessage;    // Flag to show "Release to start" message

    // -----------------------------------------------------------------------------
    // Collision animation variables
    // -----------------------------------------------------------------------------
    unsigned long collisionStartTime;  // Timestamp when collision started

    // Persists current high score to non-volatile storage.
    void saveHighScore();

    // -----------------------------------------------------------------------------
    // Input handling
    // -----------------------------------------------------------------------------
    // Updates debounced button states.
    void updateInput();
};

#endif // GAME_H
