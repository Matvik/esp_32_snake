#ifndef GAME_H
#define GAME_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ezButton.h>
#include <Preferences.h>

// -----------------------------------------------------------------------------
// Struct for coordinates on the game grid
// -----------------------------------------------------------------------------
struct Point {
    int x, y;
    Point() : x(-1), y(-1) {}
    Point(int px, int py) : x(px), y(py) {}
};

// -----------------------------------------------------------------------------
// Main game class managing Snake game logic, rendering, and input
// -----------------------------------------------------------------------------
class SnakeGame {
public:
    SnakeGame();
    void begin();
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

    static constexpr int MAX_OBSTACLES = 12;          // Maximum number of obstacles
    static constexpr int BUTTON_LEFT_PIN = 4;         // GPIO pin for left button
    static constexpr int BUTTON_RIGHT_PIN = 18;       // GPIO pin for right button

    static constexpr unsigned long LONG_PRESS_DURATION_MS = 2000;  // Duration for long press (reset high score)
    static constexpr unsigned long COLLISION_BLINK_DURATION_MS = 2000;  // Duration of collision blink

    // -----------------------------------------------------------------------------
    // Hardware interfaces
    // -----------------------------------------------------------------------------
    Adafruit_SSD1306 display;   // OLED display object
    ezButton buttonLeft;        // Left button handler
    ezButton buttonRight;       // Right button handler
    Preferences prefs;          // Non-volatile storage for high score

    // -----------------------------------------------------------------------------
    // Game state
    // -----------------------------------------------------------------------------
    Point snake[200];
    Point food;
    bool foodReady;

    Point obstacles[MAX_OBSTACLES];
    int numObstacles;

    int snakeLength;
    int dir;
    bool gameOver;
    bool turned;
    unsigned long lastMove;
    float moveDelay;
    int highScore;
    bool mouthOpen;

    // -----------------------------------------------------------------------------
    // Food blinking
    // -----------------------------------------------------------------------------
    bool foodOn;
    unsigned long lastFoodBlink;
    unsigned long foodBlinkInterval;

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

    // -----------------------------------------------------------------------------
    // Game logic methods
    // -----------------------------------------------------------------------------
    void spawnFood();
    void resetGame();
    void turnLeft();
    void turnRight();
    void moveSnake();
    void saveHighScore();

    // -----------------------------------------------------------------------------
    // Rendering
    // -----------------------------------------------------------------------------
    void drawEyes(int x, int y);
    void drawMouth(int x, int y);
    void drawSnake();
    void drawObstacles();
    void drawScore();
    void drawFood();
    void drawGame();
    void drawSplash();
    void drawCollision();
    void drawGameOver();

    // -----------------------------------------------------------------------------
    // Input handling
    // -----------------------------------------------------------------------------
    void updateInput();  // Update button states (call in main loop)
};

#endif // GAME_H
