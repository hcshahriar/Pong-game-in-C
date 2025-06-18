 #include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h> // for usleep

// For Linux/macOS (ncurses)
#include <ncurses.h>

// For Windows (uncomment below and replace ncurses functions)
// #include <conio.h>
// #include <windows.h>

#define GAME_WIDTH 80
#define GAME_HEIGHT 24
#define PADDLE_SIZE 4
#define BALL_DELAY 50000 // microseconds (lower = faster)

typedef struct {
    int x, y;
    int up, down;
    int score;
} Paddle;

typedef struct {
    int x, y;
    int dir_x, dir_y;
} Ball;

// Game state
Paddle player, computer;
Ball ball;
int game_over;
int score_limit = 10;

// Initialize game state
void init_game() {
    // Player paddle (left side)
    player.x = 2;
    player.y = GAME_HEIGHT / 2 - PADDLE_SIZE / 2;
    player.up = 'w';
    player.down = 's';
    player.score = 0;
    
    // Computer paddle (right side)
    computer.x = GAME_WIDTH - 3;
    computer.y = GAME_HEIGHT / 2 - PADDLE_SIZE / 2;
    computer.up = KEY_UP;
    computer.down = KEY_DOWN;
    computer.score = 0;
    
    // Ball in center
    ball.x = GAME_WIDTH / 2;
    ball.y = GAME_HEIGHT / 2;
    ball.dir_x = -1; // Start moving left
    ball.dir_y = 1; // Start moving down
    
    game_over = 0;
}

// Draw the game board
void draw() {
    clear(); // ncurses clear screen
    
    // Draw borders
    for (int i = 0; i < GAME_WIDTH; i++) {
        mvprintw(0, i, "-");
        mvprintw(GAME_HEIGHT-1, i, "-");
    }
    
    // Draw paddles
    for (int i = 0; i < PADDLE_SIZE; i++) {
        mvprintw(player.y + i, player.x, "|");
        mvprintw(computer.y + i, computer.x, "|");
    }
    
    // Draw ball
    mvprintw(ball.y, ball.x, "O");
    
    // Draw scores
    mvprintw(1, GAME_WIDTH / 2 - 5, "Player: %d  CPU: %d", player.score, computer.score);
    
    // Game over message
    if (game_over) {
        mvprintw(GAME_HEIGHT / 2, GAME_WIDTH / 2 - 10, "GAME OVER - %s WINS!", 
                player.score == score_limit ? "PLAYER" : "CPU");
        mvprintw(GAME_HEIGHT / 2 + 1, GAME_WIDTH / 2 - 15, "Press 'q' to quit or 'r' to restart");
    }
    
    refresh(); // ncurses refresh screen
}

// Update game state
void update() {
    if (game_over) return;
    
    // Move ball
    ball.x += ball.dir_x;
    ball.y += ball.dir_y;
    
    // Ball collision with top and bottom
    if (ball.y <= 1 || ball.y >= GAME_HEIGHT - 2) {
        ball.dir_y *= -1;
    }
    
    // Ball collision with paddles
    if ((ball.x == player.x + 1 && ball.y >= player.y && ball.y < player.y + PADDLE_SIZE) ||
        (ball.x == computer.x - 1 && ball.y >= computer.y && ball.y < computer.y + PADDLE_SIZE)) {
        ball.dir_x *= -1;
        
        // Add some randomness to the bounce
        ball.dir_y += rand() % 3 - 1; // -1, 0, or 1
        if (ball.dir_y == 0) ball.dir_y = 1; // Ensure it keeps moving
    }
    
    // Ball out of bounds (score)
    if (ball.x <= 0) {
        computer.score++;
        ball.x = GAME_WIDTH / 2;
        ball.y = GAME_HEIGHT / 2;
        ball.dir_x = 1;
        ball.dir_y = 1;
    } else if (ball.x >= GAME_WIDTH - 1) {
        player.score++;
        ball.x = GAME_WIDTH / 2;
        ball.y = GAME_HEIGHT / 2;
        ball.dir_x = -1;
        ball.dir_y = 1;
    }
    
    // Check for winner
    if (player.score == score_limit || computer.score == score_limit) {
        game_over = 1;
    }
}

// Simple AI for computer paddle
void computer_ai() {
    if (computer.y + PADDLE_SIZE/2 < ball.y && computer.y + PADDLE_SIZE < GAME_HEIGHT - 2) {
        computer.y++;
    } else if (computer.y + PADDLE_SIZE/2 > ball.y && computer.y > 1) {
        computer.y--;
    }
}

// Handle user input
void handle_input() {
    int ch = getch(); // ncurses get character
    
    switch(ch) {
        case 'w': // Player up
            if (player.y > 1) player.y--;
            break;
        case 's': // Player down
            if (player.y + PADDLE_SIZE < GAME_HEIGHT - 1) player.y++;
            break;
        case 'q': // Quit
            game_over = -1; // Special value to quit
            break;
        case 'r': // Restart
            if (game_over) init_game();
            break;
        case KEY_UP: // For two-player mode (uncomment below)
            if (computer.y > 1) computer.y--;
            break;
        case KEY_DOWN:
            if (computer.y + PADDLE_SIZE < GAME_HEIGHT - 1) computer.y++;
            break;
    }
}

int main() {
    // Initialize ncurses
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE); // Non-blocking input
    curs_set(0); // Hide cursor
    
    // Initialize random seed
    srand(time(NULL));
    
    // Initialize game
    init_game();
    
    // Game loop
    while (game_over != -1) {
        draw();
        handle_input();
        
        if (!game_over) {
            computer_ai(); // Comment this for two-player mode
            update();
        }
        
        usleep(BALL_DELAY); // Delay for ball movement
    }
    
    // Clean up ncurses
    endwin();
    
    return 0;
}
