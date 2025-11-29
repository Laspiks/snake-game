#ifndef SNAKE_H
#define SNAKE_H

#include <sys/time.h>

#ifdef __cplusplus
extern "C" {
#endif

// КОНСТАНТИ НАЛАШТУВАНЬ ГРИ

#define WIDTH 40                // Ширина ігрового поля
#define HEIGHT 20               // Висота ігрового поля
#define MAX_SNAKE_LENGTH 51     // Максимальна довжина змійки
#define WIN_LENGTH 50           // Довжина для перемоги
#define MAX_OBSTACLES 20        // Максимальна кількість перешкод
#define SPEED_BOOST_DURATION 3000000  // Тривалість прискорення (3 сек в мкс)

// Напрямки руху
#define DIR_UP 0
#define DIR_RIGHT 1
#define DIR_DOWN 2
#define DIR_LEFT 3

// Стан гри
#define GAME_RUNNING 0
#define GAME_OVER 1
#define GAME_QUIT 2
#define GAME_WON 3

// Типи їжі (Яблука)
#define FOOD_REGULAR 0    // Червоне - звичайне (+1 довжина, +10 балів)
#define FOOD_GREEN 1      // Зелене - велике (+2 довжина, +20 балів)
#define FOOD_GOLD 2       // Золоте - прискорення (+1 довжина, +50 балів, x2 швидкість)
#define FOOD_BLUE 3       // Синє - перешкода (+1 довжина, +15 балів, додає стіну)

// --- СТРУКТУРИ ДАНИХ ---

/**
 * @brief Координати точки на полі (x, y).
 */
typedef struct {
    int x;
    int y;
} Point;

/**
 * @brief Структура змійки.
 * Зберігає масив координат тіла, поточну довжину та напрямок руху.
 */
typedef struct {
    Point body[MAX_SNAKE_LENGTH];
    int length;
    int direction;
} Snake;

/**
 * @brief Структура їжі.
 * Зберігає позицію, статус активності та тип ефекту.
 */
typedef struct {
    Point position;
    int active;
    int type;
} Food;

/**
 * @brief Масив перешкод на полі.
 */
typedef struct {
    Point obstacles[MAX_OBSTACLES];
    int count;
} Obstacles;

/**
 * @brief Таймер та статус для ефекту прискорення.
 */
typedef struct {
    int active;
    struct timeval start_time;
} SpeedBoost;

/**
 * @brief Головна структура, що зберігає повний стан гри.
 */
typedef struct {
    Snake snake;
    Food food;
    Obstacles obstacles;
    SpeedBoost speed_boost;
    int score;
    int state;                   ///< Поточний стан (гра йде/перемога/поразка)
    int apples_eaten;            ///< Загальна кількість з'їдених яблук
    int special_apples_eaten[4]; ///< Статистика по типах яблук
} GameState;

// --- ЛОГІКА ГРИ (Функції) ---

/**
 * @brief Ініціалізує змінні стану гри (встановлює нульові значення).
 * @param game Вказівник на структуру стану гри.
 */
void init_game_state(GameState *game);

/**
 * @brief Оновлює координати голови та тіла змійки (рух).
 * Зсуває всі елементи тіла та оновлює голову залежно від напрямку.
 */
void update_snake_position(Snake *snake);

/**
 * @brief Перевіряє зіткнення змійки зі стінами ігрового поля.
 * @return 1, якщо зіткнення є, інакше 0.
 */
int check_wall_collision(const Snake *snake);

/**
 * @brief Перевіряє, чи врізалася змійка сама в себе.
 * @return 1, якщо голова збігається з частиною тіла, інакше 0.
 */
int check_self_collision(const Snake *snake);

/**
 * @brief Перевіряє зіткнення змійки зі статичними перешкодами.
 * @return 1, якщо є зіткнення, інакше 0.
 */
int check_obstacle_collision(const Snake *snake, const Obstacles *obstacles);

/**
 * @brief Загальна перевірка всіх фатальних зіткнень (стіни, хвіст, перешкоди).
 * @return 1, якщо гра має закінчитися через зіткнення.
 */
int check_collision(const Snake *snake, const Obstacles *obstacles);

/**
 * @brief Перевіряє, чи знаходиться голова змійки на координатах їжі.
 * @return 1, якщо змійка "з'їла" їжу.
 */
int check_food_collision(const Snake *snake, const Food *food);

/**
 * @brief Генерує нову їжу у вільному місці.
 * Гарантує, що їжа не з'явиться на тілі змійки або на перешкоді.
 */
void generate_food(const Snake *snake, const Obstacles *obstacles, Food *food);

/**
 * @brief Перевіряє валідність зміни напрямку.
 * Забороняє миттєвий розворот на 180 градусів (наприклад, вліво під час руху вправо).
 */
int is_valid_direction_change(int current_dir, int new_dir);

/**
 * @brief Збільшує довжину змійки.
 * Додає нові сегменти до хвоста змійки.
 * @param amount Кількість сегментів для додавання.
 */
void grow_snake(Snake *snake, int amount);

/**
 * @brief Перевіряє, чи зайнята координата тілом змійки.
 */
int is_position_on_snake(const Snake *snake, int x, int y);

/**
 * @brief Перевіряє, чи зайнята координата перешкодою.
 */
int is_position_on_obstacle(const Obstacles *obstacles, int x, int y);

/**
 * @brief Додає нову перешкоду на поле.
 * Викликається при з'їданні синього яблука.
 */
void add_obstacle(GameState *game);

/**
 * @brief Перевіряє, чи ще діє ефект прискорення.
 */
int is_speed_boost_active(const SpeedBoost *boost);

/**
 * @brief Активує таймер прискорення (скидає час початку).
 */
void activate_speed_boost(SpeedBoost *boost);

/**
 * @brief Розрахунок різниці часу в мікросекундах.
 */
long get_time_diff_us(struct timeval start, struct timeval end);

// КЕРУВАННЯ ПОТОКОМ ГРИ

/**
 * @brief Повна ініціалізація гри перед стартом.
 * Скидає змійку, рахунок та генерує першу їжу.
 */
void init_game(GameState *game);

/**
 * @brief Основний крок ігрового циклу.
 * Виконує рух, перевірки колізій та оновлення стану.
 * @return Новий стан гри (наприклад, GAME_OVER або GAME_RUNNING).
 */
int update_game(GameState *game);

/**
 * @brief Обробляє наслідки поїдання їжі.
 * Нараховує бали, збільшує змійку та активує ефекти залежно від типу їжі.
 */
void handle_food_eaten(GameState *game);

// Інтерфейс

/**
 * @brief Відмальовує поточний кадр гри в консолі.
 */
void draw_game(const GameState *game);

/**
 * @brief Малює статичну рамку ігрового поля.
 */
void draw_border(void);

/**
 * @brief Показує екран поразки "Game Over".
 */
void game_over_screen(const GameState *game);

/**
 * @brief Показує екран перемоги.
 */
void game_won_screen(const GameState *game);
/**
 * @brief Показує початковий вітальний екран з інструкціями.
 */
void welcome_screen(void);

#ifdef __cplusplus
}
#endif

#endif // SNAKE_H