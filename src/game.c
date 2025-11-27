#include "snake.h"
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

void init_game_state(GameState *game) {
    // Initialize snake in the middle
    game->snake.length = 3;
    game->snake.direction = DIR_RIGHT;
    
    for (int i = 0; i < game->snake.length; i++) {
        game->snake.body[i].x = WIDTH / 2 - i;
        game->snake.body[i].y = HEIGHT / 2;
    }
    
    // Initialize food
    game->food.active = 0;
    game->food.type = FOOD_REGULAR;
    
    // Initialize obstacles
    game->obstacles.count = 0;
    
    // Initialize speed boost
    game->speed_boost.active = 0;
    
    // Initialize score and stats
    game->score = 0;
    game->state = GAME_RUNNING;
    game->apples_eaten = 0;
    for (int i = 0; i < 4; i++) {
        game->special_apples_eaten[i] = 0;
    }
}

void init_game(GameState *game) {
    srand(time(NULL));
    init_game_state(game);
}

void update_snake_position(Snake *snake) {
    // Move body segments
    for (int i = snake->length - 1; i > 0; i--) {
        snake->body[i] = snake->body[i - 1];
    }
    
    // Move head based on direction
    switch(snake->direction) {
        case DIR_UP:
            snake->body[0].y--;
            break;
        case DIR_RIGHT:
            snake->body[0].x++;
            break;
        case DIR_DOWN:
            snake->body[0].y++;
            break;
        case DIR_LEFT:
            snake->body[0].x--;
            break;
    }
}

int check_wall_collision(const Snake *snake) {
    return (snake->body[0].x <= 0 || snake->body[0].x >= WIDTH + 1 ||
            snake->body[0].y <= 0 || snake->body[0].y >= HEIGHT + 1);
}

int check_self_collision(const Snake *snake) {
    for (int i = 1; i < snake->length; i++) {
        if (snake->body[0].x == snake->body[i].x &&
            snake->body[0].y == snake->body[i].y) {
            return 1;
        }
    }
    return 0;
}

int check_obstacle_collision(const Snake *snake, const Obstacles *obstacles) {
    for (int i = 0; i < obstacles->count; i++) {
        if (snake->body[0].x == obstacles->obstacles[i].x &&
            snake->body[0].y == obstacles->obstacles[i].y) {
            return 1;
        }
    }
    return 0;
}

int check_collision(const Snake *snake, const Obstacles *obstacles) {
    return check_wall_collision(snake) || check_self_collision(snake) || 
           check_obstacle_collision(snake, obstacles);
}

int check_food_collision(const Snake *snake, const Food *food) {
    if (food->active &&
        snake->body[0].x == food->position.x &&
        snake->body[0].y == food->position.y) {
        return 1;
    }
    return 0;
}

int is_position_on_snake(const Snake *snake, int x, int y) {
    for (int i = 0; i < snake->length; i++) {
        if (snake->body[i].x == x && snake->body[i].y == y) {
            return 1;
        }
    }
    return 0;
}

int is_position_on_obstacle(const Obstacles *obstacles, int x, int y) {
    for (int i = 0; i < obstacles->count; i++) {
        if (obstacles->obstacles[i].x == x && obstacles->obstacles[i].y == y) {
            return 1;
        }
    }
    return 0;
}

void generate_food(const Snake *snake, const Obstacles *obstacles, Food *food) {
    int valid = 0;
    int attempts = 0;
    const int max_attempts = 1000;
    
    // Determine food type based on probability
    int r = rand() % 100;
    if (r < 60) {
        food->type = FOOD_REGULAR;  // 60% chance
    } else if (r < 75) {
        food->type = FOOD_GREEN;    // 15% chance
    } else if (r < 85) {
        food->type = FOOD_GOLD;     // 10% chance
    } else {
        food->type = FOOD_BLUE;     // 15% chance
    }
    
    while (!valid && attempts < max_attempts) {
        food->position.x = rand() % WIDTH + 1;
        food->position.y = rand() % HEIGHT + 1;
        
        // Make sure food doesn't spawn on snake or obstacles
        if (!is_position_on_snake(snake, food->position.x, food->position.y) &&
            !is_position_on_obstacle(obstacles, food->position.x, food->position.y)) {
            valid = 1;
        }
        attempts++;
    }
    
    food->active = 1;
}

int is_valid_direction_change(int current_dir, int new_dir) {
    // Can't reverse direction
    if ((current_dir == DIR_UP && new_dir == DIR_DOWN) ||
        (current_dir == DIR_DOWN && new_dir == DIR_UP) ||
        (current_dir == DIR_LEFT && new_dir == DIR_RIGHT) ||
        (current_dir == DIR_RIGHT && new_dir == DIR_LEFT)) {
        return 0;
    }
    return 1;
}

void grow_snake(Snake *snake, int amount) {
    for (int i = 0; i < amount; i++) {
        if (snake->length < MAX_SNAKE_LENGTH) {
            snake->length++;
        }
    }
}

void add_obstacle(GameState *game) {
    if (game->obstacles.count >= MAX_OBSTACLES) {
        return;
    }
    
    int valid = 0;
    int attempts = 0;
    const int max_attempts = 100;
    Point new_obstacle;
    
    while (!valid && attempts < max_attempts) {
        new_obstacle.x = rand() % WIDTH + 1;
        new_obstacle.y = rand() % HEIGHT + 1;
        
        // Don't place on snake, existing obstacles, or near food
        if (!is_position_on_snake(&game->snake, new_obstacle.x, new_obstacle.y) &&
            !is_position_on_obstacle(&game->obstacles, new_obstacle.x, new_obstacle.y) &&
            !(game->food.active && 
              abs(new_obstacle.x - game->food.position.x) < 3 &&
              abs(new_obstacle.y - game->food.position.y) < 3)) {
            valid = 1;
        }
        attempts++;
    }
    
    if (valid) {
        game->obstacles.obstacles[game->obstacles.count++] = new_obstacle;
    }
}

long get_time_diff_us(struct timeval start, struct timeval end) {
    return (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);
}

int is_speed_boost_active(const SpeedBoost *boost) {
    if (!boost->active) {
        return 0;
    }
    
    struct timeval now;
    gettimeofday(&now, NULL);
    long diff = get_time_diff_us(boost->start_time, now);
    
    return diff < SPEED_BOOST_DURATION;
}

void activate_speed_boost(SpeedBoost *boost) {
    boost->active = 1;
    gettimeofday(&boost->start_time, NULL);
}

void handle_food_eaten(GameState *game) {
    game->apples_eaten++;
    game->special_apples_eaten[game->food.type]++;
    
    switch (game->food.type) {
        case FOOD_REGULAR:
            game->score += 10;
            grow_snake(&game->snake, 1);
            break;
            
        case FOOD_GREEN:
            game->score += 20;
            grow_snake(&game->snake, 2);
            break;
            
        case FOOD_GOLD:
            game->score += 50;
            grow_snake(&game->snake, 1);
            activate_speed_boost(&game->speed_boost);
            break;
            
        case FOOD_BLUE:
            game->score += 15;
            grow_snake(&game->snake, 1);
            add_obstacle(game);
            break;
    }
    
    game->food.active = 0;
}

int update_game(GameState *game) {
    // Update snake position
    update_snake_position(&game->snake);
    
    // Check for collisions
    if (check_collision(&game->snake, &game->obstacles)) {
        game->state = GAME_OVER;
        return GAME_OVER;
    }
    
    // Check win condition
    if (game->snake.length >= WIN_LENGTH) {
        game->state = GAME_WON;
        return GAME_WON;
    }
    
    // Check if snake ate food
    if (check_food_collision(&game->snake, &game->food)) {
        handle_food_eaten(game);
    }
    
    // Generate new food if needed
    if (!game->food.active) {
        generate_food(&game->snake, &game->obstacles, &game->food);
    }
    
    // Update speed boost status
    if (game->speed_boost.active && !is_speed_boost_active(&game->speed_boost)) {
        game->speed_boost.active = 0;
    }
    
    return GAME_RUNNING;
}
