#include "Game.h"

//------------------------------------------------------------------------------
// constructor
//------------------------------------------------------------------------------
SnakeGame::SnakeGame()
    : display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1),
      buttonLeft(BUTTON_LEFT_PIN),
      buttonRight(BUTTON_RIGHT_PIN),
      gameState(SPLASH),
      foodReady(false),
      numObstacles(8),
      snakeLength(0),
      dir(1),
      gameOver(false),
      turned(false),
      lastMove(0),
      moveDelay(300),
      highScore(0),
      mouthOpen(true),
      foodOn(false),
      lastFoodBlink(0),
      foodBlinkInterval(500),
      splashStartTime(0),
      leftButtonPressStart(0),
      wasLongPress(false),
      collisionStartTime(0),
      bothPressed(false),
      anyPressed(false),
      showReleaseMessage(false)
{
    food.x = -1;
    food.y = -1;
}

//------------------------------------------------------------------------------
// public API
//------------------------------------------------------------------------------
void SnakeGame::begin()
{
    Serial.begin(115200);
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    buttonLeft.setDebounceTime(40);
    buttonRight.setDebounceTime(40);

    prefs.begin("snake", false);
    highScore = prefs.getInt("hi", 0);

    randomSeed(micros());

    gameState = SPLASH;
    splashStartTime = millis();
}

void SnakeGame::run()
{
    updateInput();

    // State machine for game logic.
    switch (gameState) {
        case SPLASH: {
            drawSplash();

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
                        resetGame();
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
                turnLeft();
                turned = true;
            }

            if (!turned && buttonRight.isPressed()) {
                turnRight();
                turned = true;
            }

            // Move the snake at the configured interval.
            if (millis() - lastMove > moveDelay) {
                lastMove = millis();
                moveSnake();
                drawGame();
                turned = false;
            }

            // Transition to collision animation when the run ends.
            if (gameOver) {
                gameState = COLLISION;
                collisionStartTime = millis();
            }
        }
            break;

        case COLLISION: {
            drawCollision();

            // After the blink animation, save score and go to game over.
            if (millis() - collisionStartTime > COLLISION_BLINK_DURATION_MS) {
                int currentScore = snakeLength - 5;
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
            drawGameOver();

            // Wait for a button press to return to splash.
            if (buttonLeft.isPressed() || buttonRight.isPressed()) {
                gameState = SPLASH;
                splashStartTime = millis();
            }
        }
            break;
    }
}

void SnakeGame::updateInput()
{
    buttonLeft.loop();
    buttonRight.loop();
}

void SnakeGame::saveHighScore()
{
    prefs.putInt("hi", highScore);
}
