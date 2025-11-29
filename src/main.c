#include "snake.h"
#include <ncurses.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>

// Fixed: Normalize movement speed to account for terminal character aspect ratio
// Terminal characters are typically 2:1 (height:width), so horizontal movement
// needs to be slower to match vertical movement visually
#define MOVE_DELAY_HORIZONTAL 100000  // 150ms for left/right
#define MOVE_DELAY_VERTICAL 170000    // 100ms for up/down

// Color pairs
#define COLOR_SNAKE 1
#define COLOR_FOOD_REGULAR 2
#define COLOR_BORDER 3
#define COLOR_INFO 4
#define COLOR_FOOD_GREEN 5
#define COLOR_FOOD_GOLD 6
#define COLOR_FOOD_BLUE 7
#define COLOR_OBSTACLE 8
#define COLOR_TITLE 9

void draw_border(void) {
    attron(COLOR_PAIR(COLOR_BORDER) | A_BOLD);
    
    // Double line border for better look
    for (int x = 0; x <= WIDTH + 1; x++) {
        mvaddch(0, x, '=');
        mvaddch(HEIGHT + 1, x, '=');
    }
    
    for (int y = 1; y <= HEIGHT; y++) {
        mvaddch(y, 0, '|');
        mvaddch(y, WIDTH + 1, '|');
    }
    
    // Corners
    mvaddch(0, 0, '+');
    mvaddch(0, WIDTH + 1, '+');
    mvaddch(HEIGHT + 1, 0, '+');
    mvaddch(HEIGHT + 1, WIDTH + 1, '+');
    
    attroff(COLOR_PAIR(COLOR_BORDER) | A_BOLD);
}

void draw_game(const GameState *game) {
    clear();
    
    // Draw border
    draw_border();
    
    // Draw title and stats
    attron(COLOR_PAIR(COLOR_TITLE) | A_BOLD);
    mvprintw(0, WIDTH + 5, "[ SNAKE GAME ]");
    attroff(COLOR_PAIR(COLOR_TITLE) | A_BOLD);
    
    // Draw score and stats
    attron(COLOR_PAIR(COLOR_INFO) | A_BOLD);
    mvprintw(2, WIDTH + 5, "SCORE: %d", game->score);
    mvprintw(3, WIDTH + 5, "LENGTH: %d/%d", game->snake.length, WIN_LENGTH);
    mvprintw(4, WIDTH + 5, "APPLES: %d", game->apples_eaten);
    attroff(COLOR_PAIR(COLOR_INFO) | A_BOLD);
    
    // Draw progress bar to win
    int progress = (game->snake.length * 20) / WIN_LENGTH;
    mvprintw(5, WIDTH + 5, "WIN:");
    attron(COLOR_PAIR(COLOR_TITLE));
    for (int i = 0; i < 20; i++) {
        mvaddch(5, WIDTH + 10 + i, i < progress ? '=' : '-');
    }
    attroff(COLOR_PAIR(COLOR_TITLE));
    
    // Draw speed boost indicator
    if (is_speed_boost_active(&game->speed_boost)) {
        attron(COLOR_PAIR(COLOR_FOOD_GOLD) | A_BOLD | A_BLINK);
        mvprintw(7, WIDTH + 5, ">>> SPEED x2 <<<");
        attroff(COLOR_PAIR(COLOR_FOOD_GOLD) | A_BOLD | A_BLINK);
    }
    
    // Draw legend
    mvprintw(9, WIDTH + 5, "--- APPLES ---");
    attron(COLOR_PAIR(COLOR_FOOD_REGULAR) | A_BOLD);
    mvprintw(10, WIDTH + 5, "* Red: +1 +10pts");
    attroff(COLOR_PAIR(COLOR_FOOD_REGULAR) | A_BOLD);
    
    attron(COLOR_PAIR(COLOR_FOOD_GREEN) | A_BOLD);
    mvprintw(11, WIDTH + 5, "$ Green: +2 +20pts");
    attroff(COLOR_PAIR(COLOR_FOOD_GREEN) | A_BOLD);
    
    attron(COLOR_PAIR(COLOR_FOOD_GOLD) | A_BOLD);
    mvprintw(12, WIDTH + 5, "@ Gold: Speed x2");
    attroff(COLOR_PAIR(COLOR_FOOD_GOLD) | A_BOLD);
    
    attron(COLOR_PAIR(COLOR_FOOD_BLUE) | A_BOLD);
    mvprintw(13, WIDTH + 5, "# Blue: +Wall");
    attroff(COLOR_PAIR(COLOR_FOOD_BLUE) | A_BOLD);
    
    // Draw snake
    attron(COLOR_PAIR(COLOR_SNAKE) | A_BOLD);
    for (int i = 0; i < game->snake.length; i++) {
        if (i == 0) {
            mvaddch(game->snake.body[i].y, game->snake.body[i].x, '@');
        } else {
            mvaddch(game->snake.body[i].y, game->snake.body[i].x, 'o');
        }
    }
    attroff(COLOR_PAIR(COLOR_SNAKE) | A_BOLD);
    
    // Draw food with different colors
    if (game->food.active) {
        int color_pair;
        char symbol;
        
        switch (game->food.type) {
            case FOOD_REGULAR:
                color_pair = COLOR_FOOD_REGULAR;
                symbol = '*';
                break;
            case FOOD_GREEN:
                color_pair = COLOR_FOOD_GREEN;
                symbol = '$';
                break;
            case FOOD_GOLD:
                color_pair = COLOR_FOOD_GOLD;
                symbol = '@';
                break;
            case FOOD_BLUE:
                color_pair = COLOR_FOOD_BLUE;
                symbol = '#';
                break;
            default:
                color_pair = COLOR_FOOD_REGULAR;
                symbol = '*';
        }
        
        attron(COLOR_PAIR(color_pair) | A_BOLD);
        mvaddch(game->food.position.y, game->food.position.x, symbol);
        attroff(COLOR_PAIR(color_pair) | A_BOLD);
    }
    
    // Draw obstacles
    attron(COLOR_PAIR(COLOR_OBSTACLE) | A_BOLD);
    for (int i = 0; i < game->obstacles.count; i++) {
        mvaddch(game->obstacles.obstacles[i].y, 
                game->obstacles.obstacles[i].x, 'X');
    }
    attroff(COLOR_PAIR(COLOR_OBSTACLE) | A_BOLD);
    
    // Instructions
    attron(COLOR_PAIR(COLOR_INFO));
    mvprintw(HEIGHT + 2, 0, "Arrow Keys: Move | Q: Quit | Get to %d length to WIN!", WIN_LENGTH);
    attroff(COLOR_PAIR(COLOR_INFO));
    
    refresh();
}

void welcome_screen(void) {
    clear();
    
    // ASCII Art Title
    attron(COLOR_PAIR(COLOR_TITLE) | A_BOLD);
    mvprintw(3, WIDTH / 2 - 15, "   _____ _   _          _  _______ ");
    mvprintw(4, WIDTH / 2 - 15, "  / ____| \\ | |   /\\   | |/ /  ____|");
    mvprintw(5, WIDTH / 2 - 15, " | (___ |  \\| |  /  \\  | ' /| |__   ");
    mvprintw(6, WIDTH / 2 - 15, "  \\___ \\| . ` | / /\\ \\ |  < |  __|  ");
    mvprintw(7, WIDTH / 2 - 15, "  ____) | |\\  |/ ____ \\| . \\| |____ ");
    mvprintw(8, WIDTH / 2 - 15, " |_____/|_| \\_/_/    \\_\\_|\\_\\______|");
    attroff(COLOR_PAIR(COLOR_TITLE) | A_BOLD);
    
    // Instructions
    mvprintw(12, WIDTH / 2 - 18, "GOAL: Grow to %d length to WIN!", WIN_LENGTH);
    mvprintw(14, WIDTH / 2 - 15, "CONTROLS:");
    mvprintw(15, WIDTH / 2 - 15, "  Arrow Keys - Move");
    mvprintw(16, WIDTH / 2 - 15, "  Q - Quit");
    
    mvprintw(18, WIDTH / 2 - 15, "SPECIAL APPLES:");
    attron(COLOR_PAIR(COLOR_FOOD_REGULAR));
    mvprintw(19, WIDTH / 2 - 15, "  * Red");
    attroff(COLOR_PAIR(COLOR_FOOD_REGULAR));
    mvprintw(19, WIDTH / 2 - 6, "- Normal (+1, +10pts)");
    
    attron(COLOR_PAIR(COLOR_FOOD_GREEN));
    mvprintw(20, WIDTH / 2 - 15, "  $ Green");
    attroff(COLOR_PAIR(COLOR_FOOD_GREEN));
    mvprintw(20, WIDTH / 2 - 6, "- Big (+2, +20pts)");
    
    attron(COLOR_PAIR(COLOR_FOOD_GOLD));
    mvprintw(21, WIDTH / 2 - 15, "  @ Gold");
    attroff(COLOR_PAIR(COLOR_FOOD_GOLD));
    mvprintw(21, WIDTH / 2 - 6, "- Speed x2 for 3s (+50pts)");
    
    attron(COLOR_PAIR(COLOR_FOOD_BLUE));
    mvprintw(22, WIDTH / 2 - 15, "  # Blue");
    attroff(COLOR_PAIR(COLOR_FOOD_BLUE));
    mvprintw(22, WIDTH / 2 - 6, "- Adds obstacle (+15pts)");
    
    attron(COLOR_PAIR(COLOR_TITLE) | A_BOLD);
    mvprintw(HEIGHT - 3, WIDTH / 2 - 15, "Press ANY KEY to start...");
    attroff(COLOR_PAIR(COLOR_TITLE) | A_BOLD);
    
    refresh();
    nodelay(stdscr, FALSE);
    getch();
    nodelay(stdscr, TRUE);
}

void game_over_screen(const GameState *game) {
    clear();
    
    // Title - Simplified and centered properly
    attron(COLOR_PAIR(COLOR_FOOD_REGULAR) | A_BOLD);
    mvprintw(HEIGHT / 2 - 4, WIDTH / 2 - 12, "   ____    _    __  __ _____ ");
    mvprintw(HEIGHT / 2 - 3, WIDTH / 2 - 12, "  / ___|  / \\  |  \\/  | ____|");
    mvprintw(HEIGHT / 2 - 2, WIDTH / 2 - 12, " | |  _  / _ \\ | |\\/| |  _|  ");
    mvprintw(HEIGHT / 2 - 1, WIDTH / 2 - 12, " | |_| |/ ___ \\| |  | | |___ ");
    mvprintw(HEIGHT / 2, WIDTH / 2 - 12,     "  \\____/_/   \\_\\_|  |_|_____|");
    mvprintw(HEIGHT / 2 + 1, WIDTH / 2 - 12, "   _____     _______ ____  _ ");
    mvprintw(HEIGHT / 2 + 2, WIDTH / 2 - 12, "  / _ \\ \\   / / ____|  _ \\| |");
    mvprintw(HEIGHT / 2 + 3, WIDTH / 2 - 12, " | | | \\ \\ / /|  _| | |_) | |");
    mvprintw(HEIGHT / 2 + 4, WIDTH / 2 - 12, " | |_| |\\ V / | |___|  _ <|_|");
    mvprintw(HEIGHT / 2 + 5, WIDTH / 2 - 12, "  \\___/  \\_/  |_____|_| \\_(_)");
    attroff(COLOR_PAIR(COLOR_FOOD_REGULAR) | A_BOLD);


    // Stats
    attron(COLOR_PAIR(COLOR_INFO) | A_BOLD);
    mvprintw(HEIGHT / 2 + 7, WIDTH / 2 - 10, "FINAL SCORE: %d", game->score);
    mvprintw(HEIGHT / 2 + 8, WIDTH / 2 - 10, "FINAL LENGTH: %d/%d", game->snake.length, WIN_LENGTH);
    mvprintw(HEIGHT / 2 + 9, WIDTH / 2 - 10, "APPLES EATEN: %d", game->apples_eaten);
    attroff(COLOR_PAIR(COLOR_INFO) | A_BOLD);
    
    // Apple breakdown
    attron(COLOR_PAIR(COLOR_INFO));
    mvprintw(HEIGHT / 2 + 11, WIDTH / 2 - 10, "Apple Breakdown:");
    attroff(COLOR_PAIR(COLOR_INFO));
    
    attron(COLOR_PAIR(COLOR_FOOD_REGULAR));
    mvprintw(HEIGHT / 2 + 12, WIDTH / 2 - 8, "Red: %d", game->special_apples_eaten[FOOD_REGULAR]);
    attroff(COLOR_PAIR(COLOR_FOOD_REGULAR));
    
    attron(COLOR_PAIR(COLOR_FOOD_GREEN));
    mvprintw(HEIGHT / 2 + 12, WIDTH / 2, "Green: %d", game->special_apples_eaten[FOOD_GREEN]);
    attroff(COLOR_PAIR(COLOR_FOOD_GREEN));
    
    attron(COLOR_PAIR(COLOR_FOOD_GOLD));
    mvprintw(HEIGHT / 2 + 13, WIDTH / 2 - 8, "Gold: %d", game->special_apples_eaten[FOOD_GOLD]);
    attroff(COLOR_PAIR(COLOR_FOOD_GOLD));
    
    attron(COLOR_PAIR(COLOR_FOOD_BLUE));
    mvprintw(HEIGHT / 2 + 13, WIDTH / 2, "Blue: %d", game->special_apples_eaten[FOOD_BLUE]);
    attroff(COLOR_PAIR(COLOR_FOOD_BLUE));
    
    attron(COLOR_PAIR(COLOR_BORDER) | A_BOLD);
    mvprintw(HEIGHT - 2, WIDTH / 2 - 15, "Press any key to exit...");
    attroff(COLOR_PAIR(COLOR_BORDER) | A_BOLD);
    
    refresh();
}

void game_won_screen(const GameState *game) {
    clear();
    
    // Victory Title
    attron(COLOR_PAIR(COLOR_FOOD_GOLD) | A_BOLD);
    mvprintw(HEIGHT / 2 - 6, WIDTH / 2 - 15, "__   __ ___   _   _  __      __ ___  _  _ _ ");
    mvprintw(HEIGHT / 2 - 5, WIDTH / 2 - 15, "\\ \\ / // _ \\ | | | | \\ \\    / /|_ _|| \\| | |");
    mvprintw(HEIGHT / 2 - 4, WIDTH / 2 - 15, " \\ V /| (_) || |_| |  \\ \\/\\/ /  | | | .` |_|");
    mvprintw(HEIGHT / 2 - 3, WIDTH / 2 - 15, "  |_|  \\___/  \\___/    \\_/\\_/  |___||_|\\_(_)");
    attroff(COLOR_PAIR(COLOR_FOOD_GOLD) | A_BOLD);
    
    // Congratulations
    attron(COLOR_PAIR(COLOR_TITLE) | A_BOLD);
    mvprintw(HEIGHT / 2 - 1, WIDTH / 2 - 20, "CONGRATULATIONS! You reached %d length!", WIN_LENGTH);
    attroff(COLOR_PAIR(COLOR_TITLE) | A_BOLD);
    
    // Stats
    attron(COLOR_PAIR(COLOR_INFO) | A_BOLD);
    mvprintw(HEIGHT / 2 + 1, WIDTH / 2 - 10, "FINAL SCORE: %d", game->score);
    mvprintw(HEIGHT / 2 + 2, WIDTH / 2 - 10, "APPLES EATEN: %d", game->apples_eaten);
    mvprintw(HEIGHT / 2 + 3, WIDTH / 2 - 10, "OBSTACLES CREATED: %d", game->obstacles.count);
    attroff(COLOR_PAIR(COLOR_INFO) | A_BOLD);
    
    // Apple breakdown
    mvprintw(HEIGHT / 2 + 5, WIDTH / 2 - 10, "Apple Collection:");
    attron(COLOR_PAIR(COLOR_FOOD_REGULAR));
    mvprintw(HEIGHT / 2 + 6, WIDTH / 2 - 10, "  Red: %d", game->special_apples_eaten[FOOD_REGULAR]);
    attroff(COLOR_PAIR(COLOR_FOOD_REGULAR));
    
    attron(COLOR_PAIR(COLOR_FOOD_GREEN));
    mvprintw(HEIGHT / 2 + 7, WIDTH / 2 - 10, "  Green: %d", game->special_apples_eaten[FOOD_GREEN]);
    attroff(COLOR_PAIR(COLOR_FOOD_GREEN));
    
    attron(COLOR_PAIR(COLOR_FOOD_GOLD));
    mvprintw(HEIGHT / 2 + 8, WIDTH / 2 - 10, "  Gold: %d", game->special_apples_eaten[FOOD_GOLD]);
    attroff(COLOR_PAIR(COLOR_FOOD_GOLD));
    
    attron(COLOR_PAIR(COLOR_FOOD_BLUE));
    mvprintw(HEIGHT / 2 + 9, WIDTH / 2 - 10, "  Blue: %d", game->special_apples_eaten[FOOD_BLUE]);
    attroff(COLOR_PAIR(COLOR_FOOD_BLUE));
    
    attron(COLOR_PAIR(COLOR_FOOD_GOLD) | A_BOLD);
    mvprintw(HEIGHT - 2, WIDTH / 2 - 15, "Press any key to exit...");
    attroff(COLOR_PAIR(COLOR_FOOD_GOLD) | A_BOLD);
    
    refresh();
}

int get_movement_delay(int direction, int speed_boost) {
    int base_delay;
    
    if (direction == DIR_LEFT || direction == DIR_RIGHT) {
        base_delay = MOVE_DELAY_HORIZONTAL;
    } else {
        base_delay = MOVE_DELAY_VERTICAL;
    }
    
    // Apply speed boost (2x speed = half delay)
    if (speed_boost) {
        return base_delay / 2;
    }
    
    return base_delay;
}

int main(void) {
    GameState game;
    int ch;
    
    // Initialize ncurses
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);
    
    // Enable colors
    if (has_colors()) {
        start_color();
        init_pair(COLOR_SNAKE, COLOR_GREEN, COLOR_BLACK);
        init_pair(COLOR_FOOD_REGULAR, COLOR_RED, COLOR_BLACK);
        init_pair(COLOR_BORDER, COLOR_YELLOW, COLOR_BLACK);
        init_pair(COLOR_INFO, COLOR_CYAN, COLOR_BLACK);
        init_pair(COLOR_FOOD_GREEN, COLOR_GREEN, COLOR_BLACK);
        init_pair(COLOR_FOOD_GOLD, COLOR_YELLOW, COLOR_BLACK);
        init_pair(COLOR_FOOD_BLUE, COLOR_BLUE, COLOR_BLACK);
        init_pair(COLOR_OBSTACLE, COLOR_MAGENTA, COLOR_BLACK);
        init_pair(COLOR_TITLE, COLOR_WHITE, COLOR_BLACK);
    }
    
    // Initialize game
    init_game(&game);
    
    // Welcome screen
    welcome_screen();
    
    // Game loop
    while (game.state == GAME_RUNNING) {
        // Handle input
        ch = getch();
        switch(ch) {
            case KEY_UP:
            case 'w':
            case 'W':
                if (is_valid_direction_change(game.snake.direction, DIR_UP)) {
                    game.snake.direction = DIR_UP;
                }
                break;
            case KEY_RIGHT:
            case 'd':
            case 'D':
                if (is_valid_direction_change(game.snake.direction, DIR_RIGHT)) {
                    game.snake.direction = DIR_RIGHT;
                }
                break;
            case KEY_DOWN:
            case 's':
            case 'S':
                if (is_valid_direction_change(game.snake.direction, DIR_DOWN)) {
                    game.snake.direction = DIR_DOWN;
                }
                break;
            case KEY_LEFT:
            case 'a':
            case 'A':
                if (is_valid_direction_change(game.snake.direction, DIR_LEFT)) {
                    game.snake.direction = DIR_LEFT;
                }
                break;
            case 'q':
            case 'Q':
                game.state = GAME_QUIT;
                break;
        }
        
        if (game.state != GAME_RUNNING) {
            break;
        }
        
        // Update game state
        update_game(&game);
        
        // Draw everything
        draw_game(&game);
        
        // Control game speed based on direction and speed boost
        int delay = get_movement_delay(game.snake.direction, 
                                       is_speed_boost_active(&game.speed_boost));
        usleep(delay);
    }
    
    // Show appropriate end screen
    if (game.state == GAME_OVER) {
        game_over_screen(&game);
        nodelay(stdscr, FALSE);
        getch();
    } else if (game.state == GAME_WON) {
        game_won_screen(&game);
        nodelay(stdscr, FALSE);
        getch();
    }
    
    // Cleanup
    endwin();
    
    return 0;
}
