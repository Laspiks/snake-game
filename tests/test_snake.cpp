#include <gtest/gtest.h>

extern "C" {
    #include "snake.h"
}

// Test fixture for game state
class SnakeGameTest : public ::testing::Test {
protected:
    GameState game;
    
    void SetUp() override {
        init_game_state(&game);
    }
};

// ========== Initialization Tests ==========

TEST_F(SnakeGameTest, InitialSnakeLength) {
    EXPECT_EQ(game.snake.length, 3);
}

TEST_F(SnakeGameTest, InitialSnakeDirection) {
    EXPECT_EQ(game.snake.direction, DIR_RIGHT);
}

TEST_F(SnakeGameTest, InitialScore) {
    EXPECT_EQ(game.score, 0);
}

TEST_F(SnakeGameTest, InitialGameState) {
    EXPECT_EQ(game.state, GAME_RUNNING);
}

TEST_F(SnakeGameTest, InitialSnakePosition) {
    // Snake should be in the middle
    EXPECT_EQ(game.snake.body[0].x, WIDTH / 2);
    EXPECT_EQ(game.snake.body[0].y, HEIGHT / 2);
    
    // Body segments should be to the left
    EXPECT_EQ(game.snake.body[1].x, WIDTH / 2 - 1);
    EXPECT_EQ(game.snake.body[2].x, WIDTH / 2 - 2);
}

TEST_F(SnakeGameTest, InitialFoodInactive) {
    EXPECT_EQ(game.food.active, 0);
}

// ========== Movement Tests ==========

TEST_F(SnakeGameTest, MoveRight) {
    int initial_x = game.snake.body[0].x;
    update_snake_position(&game.snake);
    EXPECT_EQ(game.snake.body[0].x, initial_x + 1);
}

TEST_F(SnakeGameTest, MoveUp) {
    game.snake.direction = DIR_UP;
    int initial_y = game.snake.body[0].y;
    update_snake_position(&game.snake);
    EXPECT_EQ(game.snake.body[0].y, initial_y - 1);
}

TEST_F(SnakeGameTest, MoveDown) {
    game.snake.direction = DIR_DOWN;
    int initial_y = game.snake.body[0].y;
    update_snake_position(&game.snake);
    EXPECT_EQ(game.snake.body[0].y, initial_y + 1);
}

TEST_F(SnakeGameTest, MoveLeft) {
    game.snake.direction = DIR_LEFT;
    int initial_x = game.snake.body[0].x;
    update_snake_position(&game.snake);
    EXPECT_EQ(game.snake.body[0].x, initial_x - 1);
}

TEST_F(SnakeGameTest, BodyFollowsHead) {
    Point old_head = game.snake.body[0];
    update_snake_position(&game.snake);
    EXPECT_EQ(game.snake.body[1].x, old_head.x);
    EXPECT_EQ(game.snake.body[1].y, old_head.y);
}

// ========== Collision Tests ==========

TEST_F(SnakeGameTest, WallCollisionLeft) {
    game.snake.body[0].x = 0;
    EXPECT_TRUE(check_wall_collision(&game.snake));
}

TEST_F(SnakeGameTest, WallCollisionRight) {
    game.snake.body[0].x = WIDTH + 1;
    EXPECT_TRUE(check_wall_collision(&game.snake));
}

TEST_F(SnakeGameTest, WallCollisionTop) {
    game.snake.body[0].y = 0;
    EXPECT_TRUE(check_wall_collision(&game.snake));
}

TEST_F(SnakeGameTest, WallCollisionBottom) {
    game.snake.body[0].y = HEIGHT + 1;
    EXPECT_TRUE(check_wall_collision(&game.snake));
}

TEST_F(SnakeGameTest, SelfCollisionDetection) {
    // Create a snake that collides with itself
    game.snake.length = 5;
    game.snake.body[0].x = 10;
    game.snake.body[0].y = 10;
    game.snake.body[1].x = 11;
    game.snake.body[1].y = 10;
    game.snake.body[2].x = 11;
    game.snake.body[2].y = 11;
    game.snake.body[3].x = 10;
    game.snake.body[3].y = 11;
    game.snake.body[4].x = 10;
    game.snake.body[4].y = 10; // Same as head
    
    EXPECT_TRUE(check_self_collision(&game.snake));
}

TEST_F(SnakeGameTest, NoSelfCollisionWithShortSnake) {
    EXPECT_FALSE(check_self_collision(&game.snake));
}

// ========== Direction Change Tests ==========

TEST_F(SnakeGameTest, CannotReverseUpToDown) {
    EXPECT_FALSE(is_valid_direction_change(DIR_UP, DIR_DOWN));
}

TEST_F(SnakeGameTest, CannotReverseDownToUp) {
    EXPECT_FALSE(is_valid_direction_change(DIR_DOWN, DIR_UP));
}

TEST_F(SnakeGameTest, CannotReverseLeftToRight) {
    EXPECT_FALSE(is_valid_direction_change(DIR_LEFT, DIR_RIGHT));
}

TEST_F(SnakeGameTest, CannotReverseRightToLeft) {
    EXPECT_FALSE(is_valid_direction_change(DIR_RIGHT, DIR_LEFT));
}

TEST_F(SnakeGameTest, CanTurnUpFromRight) {
    EXPECT_TRUE(is_valid_direction_change(DIR_RIGHT, DIR_UP));
}

TEST_F(SnakeGameTest, CanTurnDownFromLeft) {
    EXPECT_TRUE(is_valid_direction_change(DIR_LEFT, DIR_DOWN));
}


TEST_F(SnakeGameTest, FoodCollisionDetection) {
    game.food.active = 1;
    game.food.position.x = game.snake.body[0].x;
    game.food.position.y = game.snake.body[0].y;
    
    EXPECT_TRUE(check_food_collision(&game.snake, &game.food));
}

TEST_F(SnakeGameTest, NoFoodCollisionWhenNotOnFood) {
    game.food.active = 1;
    game.food.position.x = 1;
    game.food.position.y = 1;
    
    EXPECT_FALSE(check_food_collision(&game.snake, &game.food));
}

TEST_F(SnakeGameTest, NoFoodCollisionWhenInactive) {
    game.food.active = 0;
    game.food.position.x = game.snake.body[0].x;
    game.food.position.y = game.snake.body[0].y;
    
    EXPECT_FALSE(check_food_collision(&game.snake, &game.food));
}

// ========== Snake Growth Tests ==========

TEST_F(SnakeGameTest, SnakeGrowsWhenEatingFood) {
    int initial_length = game.snake.length;
    grow_snake(&game.snake, 1);
    EXPECT_EQ(game.snake.length, initial_length + 1);
}

TEST_F(SnakeGameTest, SnakeDoesNotExceedMaxLength) {
    game.snake.length = MAX_SNAKE_LENGTH;
    grow_snake(&game.snake, 1);
    EXPECT_EQ(game.snake.length, MAX_SNAKE_LENGTH);
}

TEST_F(SnakeGameTest, ScoreIncreasesWhenEatingFood) {
    int initial_score = game.score;
    handle_food_eaten(&game);
    EXPECT_EQ(game.score, initial_score + 10);
}

TEST_F(SnakeGameTest, FoodDeactivatesWhenEaten) {
    game.food.active = 1;
    handle_food_eaten(&game);
    EXPECT_FALSE(game.food.active);
}

// ========== Position Helper Tests ==========

TEST_F(SnakeGameTest, PositionOnSnakeHead) {
    EXPECT_TRUE(is_position_on_snake(&game.snake, 
                                     game.snake.body[0].x, 
                                     game.snake.body[0].y));
}

TEST_F(SnakeGameTest, PositionOnSnakeTail) {
    EXPECT_TRUE(is_position_on_snake(&game.snake, 
                                     game.snake.body[2].x, 
                                     game.snake.body[2].y));
}

TEST_F(SnakeGameTest, PositionNotOnSnake) {
    EXPECT_FALSE(is_position_on_snake(&game.snake, 1, 1));
}

// ========== Integration Tests ==========

TEST_F(SnakeGameTest, GameUpdateMovesSnake) {
    int initial_x = game.snake.body[0].x;
    update_game(&game);
    EXPECT_EQ(game.snake.body[0].x, initial_x + 1);
}

TEST_F(SnakeGameTest, GameOverOnWallCollision) {
    // Move snake to wall
    game.snake.body[0].x = WIDTH;
    game.snake.direction = DIR_RIGHT;
    
    int result = update_game(&game);
    EXPECT_EQ(result, GAME_OVER);
    EXPECT_EQ(game.state, GAME_OVER);
}

TEST_F(SnakeGameTest, FoodGeneratedWhenInactive) {
    game.food.active = 0;
    update_game(&game);
    EXPECT_TRUE(game.food.active);
}

TEST_F(SnakeGameTest, CompleteEatFoodCycle) {
    // Setup: place food in front of snake
    game.food.active = 1;
    game.food.position.x = game.snake.body[0].x + 1;
    game.food.position.y = game.snake.body[0].y;
    game.snake.direction = DIR_RIGHT;
    
    int initial_length = game.snake.length;
    int initial_score = game.score;
    
    // Move snake to food
    update_game(&game);
    
    EXPECT_EQ(game.snake.length, initial_length + 1);
    EXPECT_EQ(game.score, initial_score + 10);
    EXPECT_TRUE(game.food.active); // New food should be generated
}

// ========== Edge Cases ==========

TEST_F(SnakeGameTest, SnakeLengthNeverNegative) {
    EXPECT_GT(game.snake.length, 0);
}

TEST_F(SnakeGameTest, ScoreNeverNegative) {
    EXPECT_GE(game.score, 0);
}

TEST_F(SnakeGameTest, DirectionAlwaysValid) {
    EXPECT_GE(game.snake.direction, DIR_UP);
    EXPECT_LE(game.snake.direction, DIR_LEFT);
}

// Main function for running tests
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
