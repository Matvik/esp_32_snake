#include "SnakeRenderer.h"

namespace snakerender {

namespace {
constexpr uint16_t COLOR_BLACK = SSD1306_BLACK;
constexpr uint16_t COLOR_WHITE = SSD1306_WHITE;
}

// Creates renderer with fixed display geometry and animation timers.
SnakeRenderer::SnakeRenderer(int screenWidth, int screenHeight, int scoreAreaPixels, int cellPixels)
    : display(screenWidth, screenHeight, &Wire, -1),
      scoreArea(scoreAreaPixels),
      cellSize(cellPixels),
      foodOn(false),
      lastFoodBlink(0),
      foodBlinkInterval(500) {}

// Initializes underlying OLED display hardware.
void SnakeRenderer::begin()
{
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
}

// Draws eye pixels for the snake head according to current direction.
void SnakeRenderer::drawEyes(int x, int y, int dir)
{
    if (dir == 0) {
        display.drawPixel(x + 1, y + 1, COLOR_BLACK);
        display.drawPixel(x + 2, y + 1, COLOR_BLACK);
    }
    if (dir == 1) {
        display.drawPixel(x + 2, y + 1, COLOR_BLACK);
        display.drawPixel(x + 2, y + 2, COLOR_BLACK);
    }
    if (dir == 2) {
        display.drawPixel(x + 1, y + 2, COLOR_BLACK);
        display.drawPixel(x + 2, y + 2, COLOR_BLACK);
    }
    if (dir == 3) {
        display.drawPixel(x + 1, y + 1, COLOR_BLACK);
        display.drawPixel(x + 1, y + 2, COLOR_BLACK);
    }
}

// Draws animated mouth pixel when mouth is open.
void SnakeRenderer::drawMouth(int x, int y, int dir, bool mouthOpen)
{
    if (!mouthOpen) {
        return;
    }

    if (dir == 1) {
        display.drawPixel(x + 3, y + 1, COLOR_BLACK);
    }
    if (dir == 3) {
        display.drawPixel(x, y + 1, COLOR_BLACK);
    }
    if (dir == 0) {
        display.drawPixel(x + 1, y, COLOR_BLACK);
    }
    if (dir == 2) {
        display.drawPixel(x + 1, y + 3, COLOR_BLACK);
    }
}

// Draws full snake body and head with face details.
void SnakeRenderer::drawSnake(const snakecore::SnakeCore& core, bool mouthOpen)
{
    const auto& snake = core.getSnake();

    for (size_t i = 1; i < snake.size(); i++) {
        int px = snake[i].x * cellSize;
        int py = snake[i].y * cellSize + scoreArea;
        display.fillRect(px, py, cellSize, cellSize, COLOR_WHITE);
    }

    if (snake.empty()) {
        return;
    }

    int hx = snake[0].x * cellSize;
    int hy = snake[0].y * cellSize + scoreArea;
    int dir = core.getDirection();

    display.fillRect(hx, hy, cellSize, cellSize, COLOR_WHITE);
    drawEyes(hx, hy, dir);
    drawMouth(hx, hy, dir, mouthOpen);
}

// Draws patterned obstacle cells.
void SnakeRenderer::drawObstacles(const snakecore::SnakeCore& core)
{
    const auto& obstacles = core.getObstacles();
    for (const auto& obstacle : obstacles) {
        int px = obstacle.x * cellSize;
        int py = obstacle.y * cellSize + scoreArea;
        display.drawRect(px, py, cellSize, cellSize, COLOR_WHITE);
        for (int x = 1; x < cellSize - 1; x += 2) {
            for (int y = 1; y < cellSize - 1; y += 2) {
                display.drawPixel(px + x, py + y, COLOR_WHITE);
            }
        }
    }
}

// Draws current score and high score banner.
void SnakeRenderer::drawScore(const snakecore::SnakeCore& core, int highScore)
{
    display.setTextSize(1);
    display.setTextColor(COLOR_WHITE);
    display.setCursor(0, 0);
    display.print("S:");
    display.print(core.getSnakeLength() - snakecore::SnakeCore::INITIAL_SNAKE_LENGTH);
    display.setCursor(60, 0);
    display.print("HI:");
    display.print(highScore);
}

// Draws blinking food if food is currently active.
void SnakeRenderer::drawFood(const snakecore::SnakeCore& core)
{
    snakecore::GridPoint food = core.getFood();

    if (!core.isFoodReady() || food.x < 0 || food.y < 0) {
        return;
    }

    int fx = food.x * cellSize;
    int fy = food.y * cellSize + scoreArea;

    if (millis() - lastFoodBlink > foodBlinkInterval) {
        foodOn = !foodOn;
        lastFoodBlink = millis();
    }

    display.fillRect(fx, fy, cellSize, cellSize, foodOn ? COLOR_WHITE : COLOR_BLACK);
}

// Renders the main gameplay frame.
void SnakeRenderer::drawGame(const snakecore::SnakeCore& core, int highScore, bool mouthOpen)
{
    display.clearDisplay();
    drawScore(core, highScore);
    drawObstacles(core);
    drawSnake(core, mouthOpen);
    drawFood(core);
    display.display();
}

// Renders splash screen with optional release/reset helper text.
void SnakeRenderer::drawSplash(int screenWidth, int screenHeight, int highScore,
                               bool showReleaseMessage, unsigned long splashStartTime)
{
    display.clearDisplay();
    display.setTextColor(COLOR_WHITE);

    if (showReleaseMessage) {
        display.setTextSize(1);
        display.setCursor(10, 20);
        display.print("Release to start");
        display.setCursor(10, 40);
        display.print("Hold both for reset");
    } else {
        unsigned long elapsed = millis() - splashStartTime;
        int snakeY = -5 + (elapsed / 20);
        int hiY = screenHeight + 5 - (elapsed / 20);

        if (snakeY > 15) {
            snakeY = 15;
        }
        if (hiY < 48) {
            hiY = 48;
        }

        display.setTextSize(2);
        int16_t x1, y1;
        uint16_t w, h;
        display.getTextBounds("SNAKE", 0, 0, &x1, &y1, &w, &h);
        display.setCursor((screenWidth - w) / 2, snakeY);
        display.print("SNAKE");

        display.setTextSize(1);
        String hiText = "HI:" + String(highScore);
        display.getTextBounds(hiText, 0, 0, &x1, &y1, &w, &h);
        display.setCursor((screenWidth - w) / 2, hiY);
        display.print(hiText);
    }

    display.display();
}

// Renders collision blinking effect by alternating frame and blank screen.
void SnakeRenderer::drawCollision(const snakecore::SnakeCore& core, int highScore, bool mouthOpen,
                                  unsigned long collisionStartTime)
{
    unsigned long elapsed = millis() - collisionStartTime;
    if ((elapsed / 200) % 2 == 0) {
        drawGame(core, highScore, mouthOpen);
    } else {
        display.clearDisplay();
        display.display();
    }
}

// Renders animated game-over screen and final score.
void SnakeRenderer::drawGameOver(const snakecore::SnakeCore& core, int screenWidth, int screenHeight,
                                 unsigned long splashStartTime)
{
    display.clearDisplay();
    display.setTextColor(COLOR_WHITE);

    unsigned long elapsed = millis() - splashStartTime;
    int gameY = -10 + (elapsed / 20);
    int overY = screenHeight + 10 - (elapsed / 20);

    if (gameY > 12) {
        gameY = 12;
    }
    if (overY < 28) {
        overY = 28;
    }

    display.setTextSize(2);
    int16_t x1, y1;
    uint16_t w, h;

    display.getTextBounds("GAME", 0, 0, &x1, &y1, &w, &h);
    display.setCursor((screenWidth - w) / 2, gameY);
    display.print("GAME");

    display.getTextBounds("OVER", 0, 0, &x1, &y1, &w, &h);
    display.setCursor((screenWidth - w) / 2, overY);
    display.print("OVER");

    display.setTextSize(1);
    String scoreText = "Score:" + String(core.getSnakeLength() - snakecore::SnakeCore::INITIAL_SNAKE_LENGTH);
    display.getTextBounds(scoreText, 0, 0, &x1, &y1, &w, &h);
    display.setCursor((screenWidth - w) / 2, screenHeight - 8);
    display.print(scoreText);

    display.display();
}

}  // namespace snakerender
