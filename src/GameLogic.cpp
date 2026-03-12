#include "Game.h"

//--------------- Game mechanics ------------------------------------------------

/**
 * Spawns food at a random valid position on the grid.
 * Ensures food does not overlap with snake or obstacles.
 */
void SnakeGame::spawnFood()
{
    // Invalidate current food to prevent drawing stale data
    food.x = -1;
    food.y = -1;
    foodReady = false;

    Point newFood;
    bool valid = false;

    // Find a valid position for food
    while (!valid) {
        valid = true;
        newFood.x = random(GRID_WIDTH);
        newFood.y = random(1, GRID_HEIGHT);

        // Check collision with snake.
        for (int i = 0; i < snakeLength; i++) {
            if (snake[i].x == newFood.x && snake[i].y == newFood.y) {
                valid = false;
            }
        }

        // Check collision with obstacles.
        for (int i = 0; i < numObstacles; i++) {
            if (obstacles[i].x == newFood.x && obstacles[i].y == newFood.y) {
                valid = false;
            }
        }
    }

    // Set the new food position
    food = newFood;
    foodReady = true;
}

/**
 * Resets the game state for a new game.
 * Initializes snake position, direction, obstacles, and spawns initial food.
 */
void SnakeGame::resetGame()
{
    snakeLength = 5;
    snake[0] = Point(10, 5);
    snake[1] = Point(9, 5);
    snake[2] = Point(8, 5);
    snake[3] = Point(7, 5);
    snake[4] = Point(6, 5);
    dir = 1;  // Moving right
    moveDelay = 300;  // Initial move delay in ms
    gameOver = false;
    turned = false;
    mouthOpen = true;

    // Generate random obstacles that don't collide with the starting snake
    for (int i = 0; i < numObstacles; i++) {
        bool valid = false;
        while (!valid) {
            valid = true;
            obstacles[i].x = random(GRID_WIDTH);
            obstacles[i].y = random(1, GRID_HEIGHT);
            for (int j = 0; j < snakeLength; j++) {
                if (obstacles[i].x == snake[j].x &&
                    obstacles[i].y == snake[j].y) {
                    valid = false;
                }
            }
        }
    }

    spawnFood();
}

void SnakeGame::turnLeft() { dir = (dir + 3) % 4; }
void SnakeGame::turnRight() { dir = (dir + 1) % 4; }

/**
 * Moves the snake one step in the current direction.
 * Handles wrapping around edges, collision detection, and food consumption.
 */
void SnakeGame::moveSnake()
{
    // Shift the body segments to follow the head.
    for (int i = snakeLength - 1; i > 0; i--) {
        snake[i] = snake[i - 1];
    }

    // Update the head position based on direction.
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

    // Wrap around the edges of the grid.
    if (snake[0].x < 0) {
        snake[0].x = GRID_WIDTH - 1;
    }
    if (snake[0].x >= GRID_WIDTH) {
        snake[0].x = 0;
    }
    if (snake[0].y < 0) {
        snake[0].y = GRID_HEIGHT - 1;
    }
    if (snake[0].y >= GRID_HEIGHT) {
        snake[0].y = 0;
    }

    // Check collision with own body.
    for (int i = 1; i < snakeLength; i++) {
        if (snake[0].x == snake[i].x && snake[0].y == snake[i].y) {
            gameOver = true;
        }
    }

    // Check collision with obstacles.
    for (int i = 0; i < numObstacles; i++) {
        if (snake[0].x == obstacles[i].x && snake[0].y == obstacles[i].y) {
            gameOver = true;
        }
    }

    // Check if food is eaten.
    if (foodReady && snake[0].x == food.x && snake[0].y == food.y) {
        snakeLength++;
        moveDelay = max(120.0f, moveDelay * 0.98f);
        spawnFood();
    }

    mouthOpen = !mouthOpen;
}
