#ifndef SNAKE_CORE_H
#define SNAKE_CORE_H

#include <cstdint>
#include <functional>
#include <vector>

namespace snakecore {

struct GridPoint {
    int x;
    int y;

    GridPoint() : x(-1), y(-1) {}
    GridPoint(int px, int py) : x(px), y(py) {}

    bool operator==(const GridPoint& other) const {
        return x == other.x && y == other.y;
    }
};

class SnakeCore {
public:
    static constexpr int DEFAULT_GRID_WIDTH = 32;
    static constexpr int DEFAULT_GRID_HEIGHT = 14;
    static constexpr int INITIAL_SNAKE_LENGTH = 5;

    // Creates core with configured grid dimensions.
    explicit SnakeCore(int gridWidth = DEFAULT_GRID_WIDTH, int gridHeight = DEFAULT_GRID_HEIGHT);

    // Resets snake, obstacles, and food for a new run.
    void resetGame();
    // Advances simulation by one movement tick.
    void moveSnake();
    // Rotates direction counter-clockwise.
    void turnLeft();
    // Rotates direction clockwise.
    void turnRight();

    // Injects random source used for obstacle/food placement.
    void setRandomSource(const std::function<int(int)>& randomIndex);
    // Sets obstacle count generated on reset.
    void setObstacleCount(int count);

    // Spawns food using configured random source.
    bool spawnFood();
    // Spawns food with one-shot random source.
    bool spawnFood(const std::function<int(int)>& randomIndex);

    // Replaces snake body (primarily for tests).
    void setSnake(const std::vector<GridPoint>& newSnake);
    // Replaces obstacles (primarily for tests).
    void setObstacles(const std::vector<GridPoint>& newObstacles);
    // Sets absolute direction with normalization.
    void setDirection(int newDirection);
    // Places active food at explicit coordinates.
    void setFood(const GridPoint& newFood);

    // Returns snake body vector.
    const std::vector<GridPoint>& getSnake() const;
    // Returns obstacle vector.
    const std::vector<GridPoint>& getObstacles() const;
    // Returns food coordinates.
    GridPoint getFood() const;

    // Returns current snake length.
    int getSnakeLength() const;
    // Returns movement direction.
    int getDirection() const;
    // Returns current move delay in ms.
    float getMoveDelay() const;

    // Indicates whether food is currently active.
    bool isFoodReady() const;
    // Indicates whether collision game-over has occurred.
    bool isGameOver() const;

private:
    // Checks point overlap with snake body.
    bool collidesWithSnake(const GridPoint& p) const;
    // Checks point overlap with obstacles.
    bool collidesWithObstacles(const GridPoint& p) const;
    // Wraps head coordinates at world edges.
    void wrapHead();
    // Returns bounded random index.
    int nextRandom(int maxVal) const;

    int gridWidth;
    int gridHeight;
    int obstacleCount;

    std::vector<GridPoint> snake;
    std::vector<GridPoint> obstacles;
    GridPoint food;

    int dir;
    bool foodReady;
    bool gameOver;
    float moveDelay;

    mutable uint32_t randomState;
    std::function<int(int)> randomSource;
};

}  // namespace snakecore

#endif  // SNAKE_CORE_H
