#include "Game.h"

//--------------- Drawing and display ------------------------------------------------

void SnakeGame::drawEyes(int x, int y)
{
    if (dir == 0) {
        display.drawPixel(x + 1, y + 1, SSD1306_BLACK);
        display.drawPixel(x + 2, y + 1, SSD1306_BLACK);
    }
    if (dir == 1) {
        display.drawPixel(x + 2, y + 1, SSD1306_BLACK);
        display.drawPixel(x + 2, y + 2, SSD1306_BLACK);
    }
    if (dir == 2) {
        display.drawPixel(x + 1, y + 2, SSD1306_BLACK);
        display.drawPixel(x + 2, y + 2, SSD1306_BLACK);
    }
    if (dir == 3) {
        display.drawPixel(x + 1, y + 1, SSD1306_BLACK);
        display.drawPixel(x + 1, y + 2, SSD1306_BLACK);
    }
}

void SnakeGame::drawMouth(int x, int y)
{
    if (!mouthOpen) {
        return;
    }
    if (dir == 1) {
        display.drawPixel(x + 3, y + 1, SSD1306_BLACK);
    }
    if (dir == 3) {
        display.drawPixel(x, y + 1, SSD1306_BLACK);
    }
    if (dir == 0) {
        display.drawPixel(x + 1, y, SSD1306_BLACK);
    }
    if (dir == 2) {
        display.drawPixel(x + 1, y + 3, SSD1306_BLACK);
    }
}

void SnakeGame::drawSnake()
{
    for (int i = 1; i < snakeLength; i++) {
        int px = snake[i].x * CELL_SIZE;
        int py = snake[i].y * CELL_SIZE + SCORE_AREA;
        display.fillRect(px, py, CELL_SIZE, CELL_SIZE, SSD1306_WHITE);
    }
    int hx = snake[0].x * CELL_SIZE;
    int hy = snake[0].y * CELL_SIZE + SCORE_AREA;
    display.fillRect(hx, hy, CELL_SIZE, CELL_SIZE, SSD1306_WHITE);
    drawEyes(hx, hy);
    drawMouth(hx, hy);
}

void SnakeGame::drawObstacles()
{
    for (int i = 0; i < numObstacles; i++) {
        int px = obstacles[i].x * CELL_SIZE;
        int py = obstacles[i].y * CELL_SIZE + SCORE_AREA;
        display.drawRect(px, py, CELL_SIZE, CELL_SIZE, SSD1306_WHITE);
        for (int x = 1; x < CELL_SIZE - 1; x += 2) {
            for (int y = 1; y < CELL_SIZE - 1; y += 2) {
                display.drawPixel(px + x, py + y, SSD1306_WHITE);
            }
        }
    }
}

void SnakeGame::drawScore()
{
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.print("S:");
    display.print(snakeLength - 5);
    display.setCursor(60, 0);
    display.print("HI:");
    display.print(highScore);
}

void SnakeGame::drawFood()
{
    // Protect against invalid coordinates, which can otherwise flash at 0,0.
    if (!foodReady || food.x < 0 || food.y < 0) {
        return;
    }

    int fx = food.x * CELL_SIZE;
    int fy = food.y * CELL_SIZE + SCORE_AREA;

    if (millis() - lastFoodBlink > foodBlinkInterval) {
        foodOn = !foodOn;
        lastFoodBlink = millis();
    }

    display.fillRect(fx, fy, CELL_SIZE, CELL_SIZE,
                     foodOn ? SSD1306_WHITE : SSD1306_BLACK);
}

void SnakeGame::drawGame()
{
    display.clearDisplay();
    drawScore();
    drawObstacles();
    drawSnake();
    drawFood();
    display.display();
}

void SnakeGame::drawSplash()
{
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);

    if (showReleaseMessage) {
        display.setTextSize(1);
        display.setCursor(10, 20);
        display.print("Release to start");
        display.setCursor(10, 40);
        display.print("Hold both for reset");
    } else {
        // animation: snake falls from top, hi score rises from bottom
        unsigned long elapsed = millis() - splashStartTime;
        int snakeY = -5 + (elapsed / 20);  // descends from top
        int hiY = SCREEN_HEIGHT + 5 - (elapsed / 20);  // ascends from bottom

        // Constrain positions.
        if (snakeY > 15) {
            snakeY = 15;
        }
        if (hiY < 48) {
            hiY = 48;
        }

        // draw SNAKE title
        display.setTextSize(2);
        int16_t x1, y1;
        uint16_t w, h;
        display.getTextBounds("SNAKE", 0, 0, &x1, &y1, &w, &h);
        display.setCursor((SCREEN_WIDTH - w) / 2, snakeY);
        display.print("SNAKE");

        // draw HI score
        display.setTextSize(1);
        String hiText = "HI:" + String(highScore);
        display.getTextBounds(hiText, 0, 0, &x1, &y1, &w, &h);
        display.setCursor((SCREEN_WIDTH - w) / 2, hiY);
        display.print(hiText);
    }

    display.display();
}

void SnakeGame::drawCollision()
{
    unsigned long elapsed = millis() - collisionStartTime;
    if ((elapsed / 200) % 2 == 0) {
        drawGame();  // draw the game state
    } else {
        display.clearDisplay();
        display.display();
    }
}

void SnakeGame::drawGameOver()
{
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);

    // animation: GAME falls from top, OVER rises from bottom
    unsigned long elapsed = millis() - splashStartTime;
    int gameY = -10 + (elapsed / 20);   // falls from top
    int overY = SCREEN_HEIGHT + 10 - (elapsed / 20);  // rises from bottom
    
    // Constrain positions.
    if (gameY > 12) {
        gameY = 12;
    }
    if (overY < 28) {
        overY = 28;
    }

    display.setTextSize(2);
    int16_t x1, y1;
    uint16_t w, h;
    
    // draw GAME
    display.getTextBounds("GAME", 0, 0, &x1, &y1, &w, &h);
    display.setCursor((SCREEN_WIDTH - w) / 2, gameY);
    display.print("GAME");
    
    // draw OVER
    display.getTextBounds("OVER", 0, 0, &x1, &y1, &w, &h);
    display.setCursor((SCREEN_WIDTH - w) / 2, overY);
    display.print("OVER");
    
    // draw Score at the bottom
    display.setTextSize(1);
    String s = "Score:" + String(snakeLength - 5);
    display.getTextBounds(s, 0, 0, &x1, &y1, &w, &h);
    display.setCursor((SCREEN_WIDTH - w) / 2, SCREEN_HEIGHT - 8);
    display.print(s);
    
    display.display();
}
