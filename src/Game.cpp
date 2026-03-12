#include "Game.h"

//------------------------------------------------------------------------------
// constructor
//------------------------------------------------------------------------------
// Configures runtime components and initializes state machine flags.
SnakeGame::SnakeGame()
    : buttonLeft(BUTTON_LEFT_PIN),
      buttonRight(BUTTON_RIGHT_PIN),
    core(GRID_WIDTH, GRID_HEIGHT),
    renderer(SCREEN_WIDTH, SCREEN_HEIGHT, SCORE_AREA, CELL_SIZE),
      gameState(SPLASH),
      numObstacles(8),
      turned(false),
      lastMove(0),
      highScore(0),
      mouthOpen(true),
      splashStartTime(0),
      leftButtonPressStart(0),
      wasLongPress(false),
      collisionStartTime(0),
      bothPressed(false),
      anyPressed(false),
      showReleaseMessage(false)
{}

//------------------------------------------------------------------------------
// public API
//------------------------------------------------------------------------------
// Initializes hardware, persisted score, random source, and gameplay core.
void SnakeGame::begin()
{
    Serial.begin(115200);
    renderer.begin();
    buttonLeft.setDebounceTime(40);
    buttonRight.setDebounceTime(40);

    prefs.begin("snake", false);
    highScore = prefs.getInt("hi", 0);

    randomSeed(micros());
    core.setObstacleCount(numObstacles);
    core.setRandomSource([&](int maxVal) -> int {
        return maxVal > 0 ? random(maxVal) : 0;
    });

    gameState = SPLASH;
    splashStartTime = millis();
}

// Runs one application tick: input processing and state machine transitions.
void SnakeGame::run()
{
    updateInput();

    // State machine for game logic.
    switch (gameState) {
        case SPLASH: {
            renderer.drawSplash(SCREEN_WIDTH, SCREEN_HEIGHT, highScore, showReleaseMessage, splashStartTime);

            // Handle button presses for starting the game or resetting high score.
            bool leftPressed = buttonLeft.getState() == LOW;
            bool rightPressed = buttonRight.getState() == LOW;

            if (leftPressed || rightPressed) {
                if (!anyPressed) {
                    anyPressed = true;
                    leftButtonPressStart = millis();
                    wasLongPress = false;
                    showReleaseMessage = true;
                }
                if (leftPressed && rightPressed) {
                    bothPressed = true;
                }
                if (bothPressed && millis() - leftButtonPressStart > LONG_PRESS_DURATION_MS) {
                    highScore = 0;
                    saveHighScore();
                    wasLongPress = true;
                    bothPressed = false;
                    anyPressed = false;
                }
            } else {
                if (anyPressed) {
                    if (!wasLongPress && !bothPressed) {
                        gameState = PLAYING;
                        core.resetGame();
                        turned = false;
                        mouthOpen = true;
                    }
                    anyPressed = false;
                    bothPressed = false;
                    wasLongPress = false;
                    showReleaseMessage = false;
                }
            }
        }
            break;

        case PLAYING: {
            // Handle input for turning.
            if (!turned && buttonLeft.isPressed()) {
                core.turnLeft();
                turned = true;
            }

            if (!turned && buttonRight.isPressed()) {
                core.turnRight();
                turned = true;
            }

            // Move the snake at the configured interval.
            if (millis() - lastMove > core.getMoveDelay()) {
                lastMove = millis();
                int beforeLength = core.getSnakeLength();
                core.moveSnake();
                if (core.getSnakeLength() > beforeLength) {
                    core.spawnFood();
                }
                mouthOpen = !mouthOpen;
                renderer.drawGame(core, highScore, mouthOpen);
                turned = false;
            }

            // Transition to collision animation when the run ends.
            if (core.isGameOver()) {
                gameState = COLLISION;
                collisionStartTime = millis();
            }
        }
            break;

        case COLLISION: {
            renderer.drawCollision(core, highScore, mouthOpen, collisionStartTime);

            // After the blink animation, save score and go to game over.
            if (millis() - collisionStartTime > COLLISION_BLINK_DURATION_MS) {
                int currentScore = core.getSnakeLength() - snakecore::SnakeCore::INITIAL_SNAKE_LENGTH;
                if (currentScore > highScore) {
                    highScore = currentScore;
                    saveHighScore();
                }
                gameState = GAME_OVER;
                splashStartTime = millis();
            }
        }
            break;

        case GAME_OVER: {
            renderer.drawGameOver(core, SCREEN_WIDTH, SCREEN_HEIGHT, splashStartTime);

            // Wait for a button press to return to splash.
            if (buttonLeft.isPressed() || buttonRight.isPressed()) {
                gameState = SPLASH;
                splashStartTime = millis();
            }
        }
            break;
    }
}

// Polls both button handlers each loop iteration.
void SnakeGame::updateInput()
{
    buttonLeft.loop();
    buttonRight.loop();
}

// Persists high score in non-volatile storage.
void SnakeGame::saveHighScore()
{
    prefs.putInt("hi", highScore);
}
