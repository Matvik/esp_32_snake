#include <unity.h>

#include "SnakeCore.h"

using snakecore::GridPoint;
using snakecore::SnakeCore;

namespace {

// Validates that point does not exist in vector.
bool contains_point(const std::vector<GridPoint>& points, const GridPoint& p) {
    for (const auto& v : points) {
        if (v == p) {
            return true;
        }
    }
    return false;
}

// Validates that all snake segments are inside configured grid bounds.
bool snake_is_within_bounds(const SnakeCore& core) {
    for (const auto& p : core.getSnake()) {
        if (p.x < 0 || p.x >= SnakeCore::DEFAULT_GRID_WIDTH) {
            return false;
        }
        if (p.y < 0 || p.y >= SnakeCore::DEFAULT_GRID_HEIGHT) {
            return false;
        }
    }
    return true;
}

// Validates that snake body has no duplicate coordinates.
bool snake_has_unique_cells(const SnakeCore& core) {
    const auto& snake = core.getSnake();
    for (size_t i = 0; i < snake.size(); ++i) {
        for (size_t j = i + 1; j < snake.size(); ++j) {
            if (snake[i] == snake[j]) {
                return false;
            }
        }
    }
    return true;
}

// Verifies reset baseline state values.
void test_reset_initial_state() {
    SnakeCore core;

    TEST_ASSERT_EQUAL_INT(5, core.getSnakeLength());
    TEST_ASSERT_EQUAL_INT(1, core.getDirection());
    TEST_ASSERT_FALSE(core.isGameOver());
    TEST_ASSERT_TRUE(core.isFoodReady());
    TEST_ASSERT_TRUE(core.getObstacles().size() > 0);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 300.0f, core.getMoveDelay());
}

// Verifies left/right turns wrap direction correctly.
void test_turn_left_and_right_wrap() {
    SnakeCore core;

    core.setDirection(0);
    core.turnLeft();
    TEST_ASSERT_EQUAL_INT(3, core.getDirection());

    core.turnRight();
    TEST_ASSERT_EQUAL_INT(0, core.getDirection());
}

// Verifies one right movement advances head by one cell.
void test_move_right_advances_head() {
    SnakeCore core;
    core.setObstacles({});

    GridPoint start = core.getSnake()[0];
    core.setDirection(1);
    core.moveSnake();

    TEST_ASSERT_EQUAL_INT(start.x + 1, core.getSnake()[0].x);
    TEST_ASSERT_EQUAL_INT(start.y, core.getSnake()[0].y);
}

// Verifies wrapping on left border.
void test_wrap_left_edge() {
    SnakeCore core;
    core.setObstacles({});
    core.setSnake({GridPoint(0, 5), GridPoint(1, 5), GridPoint(2, 5)});
    core.setDirection(3);

    core.moveSnake();

    TEST_ASSERT_EQUAL_INT(SnakeCore::DEFAULT_GRID_WIDTH - 1, core.getSnake()[0].x);
    TEST_ASSERT_EQUAL_INT(5, core.getSnake()[0].y);
}

// Verifies wrapping on top border.
void test_wrap_top_edge() {
    SnakeCore core;
    core.setObstacles({});
    core.setSnake({GridPoint(4, 0), GridPoint(4, 1), GridPoint(4, 2)});
    core.setDirection(0);

    core.moveSnake();

    TEST_ASSERT_EQUAL_INT(4, core.getSnake()[0].x);
    TEST_ASSERT_EQUAL_INT(SnakeCore::DEFAULT_GRID_HEIGHT - 1, core.getSnake()[0].y);
}

// Verifies self-collision triggers game over.
void test_collision_with_body_sets_game_over() {
    SnakeCore core;
    core.setObstacles({});
    core.setSnake({GridPoint(5, 5), GridPoint(5, 4), GridPoint(4, 4), GridPoint(4, 5)});
    core.setDirection(0);

    core.moveSnake();

    TEST_ASSERT_TRUE(core.isGameOver());
}

// Verifies obstacle collision triggers game over.
void test_collision_with_obstacle_sets_game_over() {
    SnakeCore core;
    core.setSnake({GridPoint(5, 5), GridPoint(4, 5), GridPoint(3, 5)});
    core.setObstacles({GridPoint(6, 5)});
    core.setDirection(1);

    core.moveSnake();

    TEST_ASSERT_TRUE(core.isGameOver());
}

// Verifies eating food increases length and reduces delay.
void test_eating_food_grows_snake_and_speeds_up() {
    SnakeCore core;
    core.setObstacles({});
    core.setSnake({GridPoint(5, 5), GridPoint(4, 5), GridPoint(3, 5)});
    core.setDirection(1);
    core.setFood(GridPoint(6, 5));

    float before = core.getMoveDelay();
    core.moveSnake();

    TEST_ASSERT_EQUAL_INT(4, core.getSnakeLength());
    TEST_ASSERT_TRUE(core.getMoveDelay() < before);
    TEST_ASSERT_FALSE(core.isFoodReady());
}

// Verifies speed reduction is clamped to minimum limit.
void test_speed_floor_is_120() {
    SnakeCore core;
    core.setObstacles({});
    core.setSnake({GridPoint(5, 5), GridPoint(4, 5), GridPoint(3, 5)});
    core.setDirection(1);

    for (int i = 0; i < 300; ++i) {
        GridPoint head = core.getSnake()[0];
        GridPoint next = head;
        next.x = (next.x + 1) % SnakeCore::DEFAULT_GRID_WIDTH;
        core.setFood(next);
        core.moveSnake();
    }

    TEST_ASSERT_FLOAT_WITHIN(0.001f, 120.0f, core.getMoveDelay());
}

// Verifies food spawn avoids occupied snake and obstacle cells.
void test_spawn_food_avoids_snake_and_obstacles() {
    SnakeCore core;
    core.setSnake({GridPoint(1, 1), GridPoint(2, 1), GridPoint(3, 1)});
    core.setObstacles({GridPoint(4, 1), GridPoint(5, 1)});

    int seq[] = {1, 0, 4, 0, 6, 0};
    int idx = 0;
    auto rng = [&](int maxVal) {
        int v = seq[idx++ % 6];
        if (maxVal <= 0) {
            return 0;
        }
        return v % maxVal;
    };

    bool ok = core.spawnFood(rng);

    TEST_ASSERT_TRUE(ok);
    GridPoint f = core.getFood();
    TEST_ASSERT_TRUE(core.isFoodReady());
    TEST_ASSERT_FALSE((f == GridPoint(1, 1)) || (f == GridPoint(2, 1)) || (f == GridPoint(3, 1)));
    TEST_ASSERT_FALSE((f == GridPoint(4, 1)) || (f == GridPoint(5, 1)));
}

// Verifies reset uses injected RNG and keeps entities non-overlapping.
void test_reset_with_injected_rng_generates_valid_layout() {
    SnakeCore core;

    int seq[] = {
        10, 5, 9, 5, 8, 5, 7, 5, 6, 5,  // ignored collisions against snake
        2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8,  // obstacles
        1, 1  // food
    };
    int idx = 0;
    auto rng = [&](int maxVal) {
        int v = seq[idx++ % (sizeof(seq) / sizeof(seq[0]))];
        if (maxVal <= 0) {
            return 0;
        }
        if (v < 0) {
            v = -v;
        }
        return v % maxVal;
    };

    core.setObstacleCount(6);
    core.setRandomSource(rng);
    core.resetGame();

    TEST_ASSERT_TRUE(core.isFoodReady());
    TEST_ASSERT_EQUAL_INT(6, static_cast<int>(core.getObstacles().size()));

    GridPoint food = core.getFood();
    TEST_ASSERT_FALSE(contains_point(core.getSnake(), food));
    TEST_ASSERT_FALSE(contains_point(core.getObstacles(), food));
}

// Verifies obstacle count is clamped for negative values.
void test_negative_obstacle_count_is_clamped_to_zero() {
    SnakeCore core;
    core.setObstacleCount(-5);
    core.resetGame();

    TEST_ASSERT_EQUAL_INT(0, static_cast<int>(core.getObstacles().size()));
    TEST_ASSERT_TRUE(core.isFoodReady());
}

// Verifies direction normalization for out-of-range values.
void test_set_direction_normalizes_value() {
    SnakeCore core;

    core.setDirection(-1);
    TEST_ASSERT_EQUAL_INT(3, core.getDirection());

    core.setDirection(6);
    TEST_ASSERT_EQUAL_INT(2, core.getDirection());
}

// Property test: over many steps with no food/obstacles, bounds and length invariants hold.
void test_property_long_run_bounds_and_length_invariants() {
    SnakeCore core;
    core.setObstacles({});
    core.setSnake({GridPoint(10, 5), GridPoint(9, 5), GridPoint(8, 5), GridPoint(7, 5), GridPoint(6, 5)});
    core.setDirection(1);

    const int initialLength = core.getSnakeLength();
    for (int i = 0; i < 500; ++i) {
        // Deterministic turn pattern to exercise all directions and wraps.
        if (i % 11 == 0) {
            core.turnLeft();
        }
        if (i % 17 == 0) {
            core.turnRight();
        }

        core.moveSnake();

        TEST_ASSERT_TRUE(snake_is_within_bounds(core));
        TEST_ASSERT_EQUAL_INT(initialLength, core.getSnakeLength());

        if (core.isGameOver()) {
            break;
        }
    }
}

// Property test: repeated resets always produce valid food/obstacle/snake layout.
void test_property_repeated_reset_layout_validity() {
    SnakeCore core;
    core.setObstacleCount(8);

    for (int i = 0; i < 100; ++i) {
        core.resetGame();

        TEST_ASSERT_EQUAL_INT(SnakeCore::INITIAL_SNAKE_LENGTH, core.getSnakeLength());
        TEST_ASSERT_TRUE(core.isFoodReady());
        TEST_ASSERT_TRUE(snake_is_within_bounds(core));
        TEST_ASSERT_TRUE(snake_has_unique_cells(core));

        GridPoint food = core.getFood();
        TEST_ASSERT_TRUE(food.x >= 0 && food.x < SnakeCore::DEFAULT_GRID_WIDTH);
        TEST_ASSERT_TRUE(food.y >= 1 && food.y < SnakeCore::DEFAULT_GRID_HEIGHT);
        TEST_ASSERT_FALSE(contains_point(core.getSnake(), food));
        TEST_ASSERT_FALSE(contains_point(core.getObstacles(), food));

        const auto& obstacles = core.getObstacles();
        for (size_t k = 0; k < obstacles.size(); ++k) {
            TEST_ASSERT_TRUE(obstacles[k].x >= 0 && obstacles[k].x < SnakeCore::DEFAULT_GRID_WIDTH);
            TEST_ASSERT_TRUE(obstacles[k].y >= 1 && obstacles[k].y < SnakeCore::DEFAULT_GRID_HEIGHT);
            TEST_ASSERT_FALSE(contains_point(core.getSnake(), obstacles[k]));
            for (size_t m = k + 1; m < obstacles.size(); ++m) {
                TEST_ASSERT_FALSE(obstacles[k] == obstacles[m]);
            }
        }
    }
}

// Property test: repeated food consumption keeps speed clamped and monotonic non-increasing.
void test_property_speed_monotonic_and_clamped_on_repeated_eats() {
    SnakeCore core;
    core.setObstacles({});
    core.setSnake({GridPoint(5, 5), GridPoint(4, 5), GridPoint(3, 5)});
    core.setDirection(1);

    float prevDelay = core.getMoveDelay();
    for (int i = 0; i < 300; ++i) {
        GridPoint head = core.getSnake()[0];
        GridPoint next = head;
        next.x = (next.x + 1) % SnakeCore::DEFAULT_GRID_WIDTH;
        core.setFood(next);
        core.moveSnake();

        float d = core.getMoveDelay();
        TEST_ASSERT_TRUE(d <= prevDelay + 0.0001f);
        TEST_ASSERT_TRUE(d >= 120.0f - 0.0001f);
        prevDelay = d;
    }
}

}  // namespace

// Runs full SnakeCore unit test suite.
int main() {
    UNITY_BEGIN();

    RUN_TEST(test_reset_initial_state);
    RUN_TEST(test_turn_left_and_right_wrap);
    RUN_TEST(test_move_right_advances_head);
    RUN_TEST(test_wrap_left_edge);
    RUN_TEST(test_wrap_top_edge);
    RUN_TEST(test_collision_with_body_sets_game_over);
    RUN_TEST(test_collision_with_obstacle_sets_game_over);
    RUN_TEST(test_eating_food_grows_snake_and_speeds_up);
    RUN_TEST(test_speed_floor_is_120);
    RUN_TEST(test_spawn_food_avoids_snake_and_obstacles);
    RUN_TEST(test_reset_with_injected_rng_generates_valid_layout);
    RUN_TEST(test_negative_obstacle_count_is_clamped_to_zero);
    RUN_TEST(test_set_direction_normalizes_value);
    RUN_TEST(test_property_long_run_bounds_and_length_invariants);
    RUN_TEST(test_property_repeated_reset_layout_validity);
    RUN_TEST(test_property_speed_monotonic_and_clamped_on_repeated_eats);

    return UNITY_END();
}
