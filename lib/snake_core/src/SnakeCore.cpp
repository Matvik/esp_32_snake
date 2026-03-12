#include "SnakeCore.h"

#include <algorithm>

namespace snakecore {

// Initializes gameplay core with grid dimensions and default runtime state.
SnakeCore::SnakeCore(int width, int height)
    : gridWidth(width),
      gridHeight(height),
            obstacleCount(8),
      food(-1, -1),
      dir(1),
      foodReady(false),
      gameOver(false),
            moveDelay(300.0f),
            randomState(0xA5A5A5A5u) {
    resetGame();
}

// Resets snake, direction, speed, obstacles, and initial food for a new run.
void SnakeCore::resetGame() {
    snake.clear();
    snake.push_back(GridPoint(10, 5));
    snake.push_back(GridPoint(9, 5));
    snake.push_back(GridPoint(8, 5));
    snake.push_back(GridPoint(7, 5));
    snake.push_back(GridPoint(6, 5));

    dir = 1;
    gameOver = false;
    foodReady = false;
    moveDelay = 300.0f;
    food = GridPoint(-1, -1);

    obstacles.clear();
    for (int i = 0; i < obstacleCount; ++i) {
        const int maxAttempts = gridWidth * gridHeight * 4;
        bool valid = false;
        for (int attempt = 0; attempt < maxAttempts && !valid; ++attempt) {
            GridPoint candidate(nextRandom(gridWidth), nextRandom(gridHeight - 1) + 1);
            valid = !collidesWithSnake(candidate) && !collidesWithObstacles(candidate);
            if (valid) {
                obstacles.push_back(candidate);
            }
        }
    }

    spawnFood();
}

// Rotates movement direction 90 degrees counter-clockwise.
void SnakeCore::turnLeft() {
    dir = (dir + 3) % 4;
}

// Rotates movement direction 90 degrees clockwise.
void SnakeCore::turnRight() {
    dir = (dir + 1) % 4;
}

// Injects random index generator used for obstacle and food placement.
void SnakeCore::setRandomSource(const std::function<int(int)>& randomIndex) {
    randomSource = randomIndex;
}

// Sets how many obstacles are generated on game reset.
void SnakeCore::setObstacleCount(int count) {
    obstacleCount = count < 0 ? 0 : count;
}

// Checks whether a point overlaps with any snake segment.
bool SnakeCore::collidesWithSnake(const GridPoint& p) const {
    return std::any_of(snake.begin(), snake.end(), [&](const GridPoint& s) {
        return s == p;
    });
}

// Checks whether a point overlaps with any obstacle.
bool SnakeCore::collidesWithObstacles(const GridPoint& p) const {
    return std::any_of(obstacles.begin(), obstacles.end(), [&](const GridPoint& o) {
        return o == p;
    });
}

// Returns bounded pseudo-random index using injected random source.
int SnakeCore::nextRandom(int maxVal) const {
    if (maxVal <= 0) {
        return 0;
    }
    if (randomSource) {
        int value = randomSource(maxVal);
        if (value < 0) {
            value = -value;
        }
        return value % maxVal;
    }

    // Deterministic fallback PRNG keeps core functional in native tests and
    // prevents reset/spawn from stalling if external random source is absent.
    randomState = randomState * 1664525u + 1013904223u;
    return static_cast<int>(randomState % static_cast<uint32_t>(maxVal));
}

// Spawns food using the configured random source.
bool SnakeCore::spawnFood() {
    return spawnFood([this](int maxVal) -> int {
        return nextRandom(maxVal);
    });
}

// Spawns food at a valid free cell not occupied by snake or obstacles.
bool SnakeCore::spawnFood(const std::function<int(int)>& randomIndex) {
    if (!randomIndex) {
        return false;
    }

    food = GridPoint(-1, -1);
    foodReady = false;

    const int maxAttempts = gridWidth * gridHeight * 4;
    for (int attempt = 0; attempt < maxAttempts; ++attempt) {
        GridPoint candidate(randomIndex(gridWidth), randomIndex(gridHeight - 1) + 1);
        if (!collidesWithSnake(candidate) && !collidesWithObstacles(candidate)) {
            food = candidate;
            foodReady = true;
            return true;
        }
    }

    return false;
}

// Applies world wrapping for snake head coordinates.
void SnakeCore::wrapHead() {
    if (snake[0].x < 0) {
        snake[0].x = gridWidth - 1;
    }
    if (snake[0].x >= gridWidth) {
        snake[0].x = 0;
    }
    if (snake[0].y < 0) {
        snake[0].y = gridHeight - 1;
    }
    if (snake[0].y >= gridHeight) {
        snake[0].y = 0;
    }
}

// Executes one gameplay tick: move, wrap, collision checks, and eat handling.
void SnakeCore::moveSnake() {
    if (snake.empty()) {
        return;
    }

    for (int i = static_cast<int>(snake.size()) - 1; i > 0; --i) {
        snake[i] = snake[i - 1];
    }

    if (dir == 0) {
        snake[0].y--;
    }
    if (dir == 1) {
        snake[0].x++;
    }
    if (dir == 2) {
        snake[0].y++;
    }
    if (dir == 3) {
        snake[0].x--;
    }

    wrapHead();

    for (size_t i = 1; i < snake.size(); ++i) {
        if (snake[i] == snake[0]) {
            gameOver = true;
        }
    }

    if (collidesWithObstacles(snake[0])) {
        gameOver = true;
    }

    if (foodReady && snake[0] == food) {
        snake.push_back(snake.back());
        moveDelay = std::max(120.0f, moveDelay * 0.98f);
        foodReady = false;
        food = GridPoint(-1, -1);
    }
}

// Replaces snake body; mainly used by tests.
void SnakeCore::setSnake(const std::vector<GridPoint>& newSnake) {
    snake = newSnake;
}

// Replaces obstacle set; mainly used by tests.
void SnakeCore::setObstacles(const std::vector<GridPoint>& newObstacles) {
    obstacles = newObstacles;
}

// Sets movement direction with normalization to [0..3].
void SnakeCore::setDirection(int newDirection) {
    dir = ((newDirection % 4) + 4) % 4;
}

// Places food explicitly and marks it as active; mainly used by tests.
void SnakeCore::setFood(const GridPoint& newFood) {
    food = newFood;
    foodReady = true;
}

// Returns current snake body.
const std::vector<GridPoint>& SnakeCore::getSnake() const {
    return snake;
}

// Returns current obstacle list.
const std::vector<GridPoint>& SnakeCore::getObstacles() const {
    return obstacles;
}

// Returns current food position.
GridPoint SnakeCore::getFood() const {
    return food;
}

// Returns current snake length.
int SnakeCore::getSnakeLength() const {
    return static_cast<int>(snake.size());
}

// Returns current movement direction.
int SnakeCore::getDirection() const {
    return dir;
}

// Returns current movement delay in milliseconds.
float SnakeCore::getMoveDelay() const {
    return moveDelay;
}

// Returns true when food is active on board.
bool SnakeCore::isFoodReady() const {
    return foodReady;
}

// Returns game-over flag set by collision checks.
bool SnakeCore::isGameOver() const {
    return gameOver;
}

}  // namespace snakecore
