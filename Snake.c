#include "ripes_system.h"
#include <math.h>
#include <stdlib.h>
#include <time.h>

// global defines
#define RED_COLOR 0xff0000
#define GREEN_COLOR 0x00ff00
#define BLACK_COLOR 0x000000
#define BLUE_COLOR 0x0000ff
#define INITIAL_SNAKE_SIZE 3;
#define SNAKE_ALIVE 1
#define SNAKE_DEAD 2
#define SNAKE_WIN 3

// variables
volatile unsigned int *led_base = LED_MATRIX_0_BASE;
volatile unsigned int *pad_up = D_PAD_0_UP;
volatile unsigned int *pad_down = D_PAD_0_DOWN;
volatile unsigned int *pad_right = D_PAD_0_RIGHT;
volatile unsigned int *pad_left = D_PAD_0_LEFT;

// constants
const int led_width = LED_MATRIX_0_WIDTH;
const int led_height = LED_MATRIX_0_HEIGHT;
const int led_size = LED_MATRIX_0_SIZE; 

// offsets
const int right_offset = 1;
const int left_offset = -1;
const int up_offset = -led_width;
const int down_offset = led_width;

// simulating random
int rand_time = 50;

// tail index 
int tail_index;

// snake parts (stores led pointers)
int *snake_parts[LED_MATRIX_0_WIDTH * LED_MATRIX_0_HEIGHT];

// game state
int game_state;

// datatypes
typedef enum _my_bool { false, true } boolean;    // --> boolean datatype

// initialize the snake
void init_snake() {
    tail_index = INITIAL_SNAKE_SIZE - 1;
    int *head = led_base + (((led_width) * (led_height >> 1)) - (led_width >> 1));
    *head = GREEN_COLOR;
    snake_parts[0] = head;
    for (int i = 1; i <= tail_index; i++) {
        snake_parts[i] = snake_parts[i - 1] - 1;
    }
    for (int i = 0; i <= tail_index; i++) {
      int *ptr = snake_parts[i];
      *ptr = RED_COLOR;
    }
}

// move the snake
void move_snake(int offset) {
    int *old_part = snake_parts[0];
    snake_parts[0] += offset;
    int *head_ptr = snake_parts[0];
    
    // check if snake hits border or itself
    if (*head_ptr == BLUE_COLOR || *head_ptr == RED_COLOR) {
        game_state = SNAKE_DEAD;
        return;
    }
    
    // check if it eats apple
    if (*head_ptr == GREEN_COLOR) {
        tail_index++;
        snake_parts[tail_index] = snake_parts[tail_index - 1] + 1;
        int *tail = snake_parts[tail_index];
        *tail = RED_COLOR;
        create_apple();
    }
    
    // update array with new parts data, removes the tail
    for (int i = 1; i <= tail_index; i++) {
        if (i == tail_index) {
            int *tail = snake_parts[i];
            *tail = BLACK_COLOR;
        }
        int *curr_part = snake_parts[i];
        snake_parts[i] = old_part;
        old_part = curr_part;
    }
    
    // fill array
    for (int i = 0; i <= tail_index; i++) {
        int *ptr = snake_parts[i];
        *ptr = RED_COLOR;
    }
}


// generates a random apple
void create_apple() {
    boolean is_valid = false;    // check if position is valid (not a snake part or border)
    rand_time += 5;
    while (!is_valid) {
        srand(rand_time);
        int rand_x = rand() % (led_width - 2);
        int rand_y = rand() % (led_height - 2);
        int *rand_pos = led_base + (rand_y * led_width + rand_x);
        if (*rand_pos != BLUE_COLOR && *rand_pos != RED_COLOR) {
            is_valid = true;
            *rand_pos = GREEN_COLOR;
        }
    }
}

// initialize borders
void init_borders() {
    // init height
    int *init_ptr = led_base;
    for (int i = 0; i < led_height; i++) {
        *init_ptr = BLUE_COLOR;
        *(init_ptr + led_width - 1) = BLUE_COLOR;
        init_ptr += led_width;
    }
    
    // init width
    init_ptr = led_base;
    for (int i = 0; i < led_width; i++) {
        *(init_ptr + i) = BLUE_COLOR;
        *(init_ptr + (led_height * led_width - led_width) + i) = BLUE_COLOR;
    }
}

// checks if game is over
boolean is_gameover() {
    return (game_state == SNAKE_DEAD || game_state == SNAKE_WIN || tail_index == (led_width * led_height) - 1);
}


// main function / snake game
void main() {
    int offset = right_offset;    // snake moves to the left by default   
    game_state = SNAKE_ALIVE;
    init_snake();
    init_borders();
    create_apple();
    // gameloop
    for (int i = 0; i < 10000; i++){} // small delay
    while(!is_gameover()) {
        int prev_offset = offset;        // to ensure no oppose direction is done
        for (int i = 0; i < 2000; i++){} // ticks
        if (*pad_up) offset = up_offset;
        if (*pad_down) offset = down_offset;
        if (*pad_left) offset = left_offset;
        if (*pad_right) offset = right_offset;
        // check if offset an prev_offset are opposites, if true, retrieve
        if (offset + prev_offset == 0) offset = prev_offset;
        move_snake(offset);
    }
}