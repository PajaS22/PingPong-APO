# Documentation for the Ping-Pong game

To see the full version click *[here](https://hackmd.io/@APOsem/rJz3nJkqO)*.

## Table of Contents

[TOC]

## Main
*files: main.c*  
``` C
int main(int argc, char *argv[]);
```
>main function of the whole game  
>click *[here](#Main-thread)* to see more  
---
``` C
void *terminal_listening_main();
```
>terminal listening thread function  
>click *[here](#Terminal-thread)* to see more  
---
``` C
void *knobs_listening_main();
```
>knobs listening thread function  
>click *[here](#Knobs-thread)* to see more  
---
``` C
void call_stty(int reset)
```
>sets terminal to raw mode  
---
## Game
*files: game.c, game.h*  

``` C
typedef struct {
    float X;
    float Y;
} Position;

typedef struct {
    float X;
    float Y;
} Velocity;

typedef unsigned short pixel; 
```
>structures for holding information about Position and Velocity.  
>pixel is represented by two bytes.  
---
``` C
void start_game();
``` 
>prepares variables for a new game  
>creates game threads  
>calls the game loop  
>after returning game loop, it frees allocated memory, joins threads  
---
``` C
void game_loop();
```
>main loop of the game  
---
``` C
void new_round();
```
>prepares for the next round  
>starts the countdown  
---
``` C
Velocity gen_rand_ball_vel();
```
>generates a random initial ball velocity  
---
``` C
void enlarge_paddle(Paddle *pad);
```
>changes paddle length and position if possible  
>new paddle length will be ENLARGE_CONST * previous length  
>if not possible, it does nothing  
---
``` C
void reduce_paddle(Paddle *pad);
```
>restore the original length of the paddle  
---
``` C
void maximal_angle(Velocity *vel);
```
>rotates the velocity vector so it deviates from paddle's normal vector maximally by the MAXIMAL_VELOCITY_ANGLE  
---


## Ball
*files: ball.c, ball.h*  

``` C
typedef struct {
    Position pos;
    int radius;
    Velocity velocity;
    pixel color;
} Ball;
```
>structure for holding information about ball  
---

``` C
void init_ball(Ball *b, Position pos, Velocity velocity, pixel color, int radius);
```

> initializes all data for the ball
---
``` C
void draw_ball(Position pos, int radius, pixel color, pixel *frame_buff);
```
>draw ball in its actual position in the frame buffer
---

``` C 
void update_ball(Position pos, Position old_pos, int radius, int old_radius, pixel color, pixel *frame_buff);
```
>draws the old ball in the color of the background  
>than draws the new ball in the color argument in the new position
---
``` C
Position new_position(Ball *ball);
```
>returns the position + velocity of the ball
---
``` C
bool paddle_touch(float ball_y, float paddle_y, int paddle_height, int radius);
```
>controls whether the ball is in given paddle's range on y-axis
---
``` C
double rebound_angle(float ball_y, float paddle_y, int paddle_height);
```
>computes the ball's rebound angle of the paddle's curved surface  
>returns double between -1 and 1
---


## Paddle
*files: paddle.c, paddle.h*  

``` C
typedef struct {
    Position pos;
    int width;
    int height;
    pixel color;
} Paddle;
```
>structure for holding information about paddle  
---

``` C
void init_paddle(Paddle *p, Position pos, pixel color);
```
>initializes all data for the paddle  
---
``` C
void draw_paddle(Position position, int paddle_width, int paddle_length, pixel color, pixel *frame_buff);
```
>draw paddle in its actual position in the frame buffer  
---
``` C
void update_paddle(Position position, Position old_position, int width, int length, int old_length, pixel color, pixel *frame_buff);
``` 
>draws the old paddle in the color of the background  
>than draws the new paddle in the color argument in the new position  
---
``` C 
void move_paddle(Paddle *p, int move);
``` 
>changes the y-coordinate of position depending on the move integer  
>controls if the new position is legal (in the display borders)  
---
## Bonus
*files: bonus.c, bonus.h*  

``` C
typedef struct {
    Ball ball;
    bonus_type bonus_mode;
} Bonus;
```
>structure for holding information about bonus  
---

``` C
void init_bonus(Bonus *b, int radius);
``` 
>sets random position within the specified area where the bonus ball can occure  
---
``` C 
void print_bonuses(Bonus *bonuses, int nbr_bonuses, pixel *frame_buff);
``` 
>draws all active bonuses in the frame buffer  
---
``` C
void print_bonus(Bonus *bonus, pixel *frame_buff);
``` 
>prints bonus in the frame buffer  
---
``` C
int bonus_hit(Bonus *bonuses, int nbr_bonuses, Ball *ball);
``` 
>returns the index of ball that was hit <0; MAX_BONUSES_NBR)  
---
``` C
bool bonus_touch(Bonus *bonus, Ball *ball);
``` 
>controls whether the game ball has touched the bonus ball  
---
``` C
Bonus remove_bonus(Bonus *bonuses, int idx, int nbr_bonuses);
``` 
>removes the bonus ball from the bonuses array and shifts all the remaining bonuses after a hit  
---

## LCD
*files: lcd_output.c, lcd_output.h*  

``` C
bool lcd_initialization(unsigned char **ret_lcd_mem_base);
```
>sets the display base memory adress and clears the display  
---
``` C
bool display_init();
```
>calls lcd_initialization() and sets fonts for the text printing  
---
``` C
void set_display_black(pixel *frame_buff);
```
>sets all pixels black  
---
``` C
void clear_buffer(pixel *frame_buff);
```
>sets all pixels in the buffer to zero  
---
``` C
void update_display(pixel *frame_buff);
```
>prints the frame buffer to the LCD display  
---
``` C
unsigned int hsv2rgb_lcd(int hue, int saturation, int value);
``` 
>returns an RGB value of the color given in hue syntax  
---
``` C
int char_width(int ch);
```
>returns the width of character in a set font in pixels  
---

``` C
void draw_char(int x, int y, char ch, pixel color, int scale, pixel *frame_buff);
```
>prints a character to the frame buffer  
---
``` C
void draw_pixel(int x, int y, pixel color, pixel *frame_buff);
```
>prints a pixel to the frame buffer  
---
``` C
void draw_pixel_big(int x, int y, int scale, pixel color, pixel *frame_buff);
```
>prints a pixel multiplied by a scale to the frame buffer  
>example: one pixel multiplied by scale = 4 would be represented as a square of four pixels  
---
``` C
void draw_string(int x, int y, pixel color, int scale, pixel *frame_buff, char *string);
```
>prints a string at the end to the frame buffer  
---
``` C
void draw_grounded_string(int x, int y, int padding_x, int padding_y, pixel color, pixel color_back, int scale, pixel *frame_buff, char *string);
```
>prints a string with a rectangle of the given color in the background  
---
``` C
void print_menu(int x, int y, int selected, pixel *frame_buff);
```
>prints the main menu  
>higlights the *selected*  option  
---
``` C
void print_pause_menu(int x, int y, int selected, pixel *frame_buff);
```
>prints the pause menu  
>higlights the 'selected' option  
---
``` C
void countdown(int x, int y, int scale, pixel *frame_buff);
```
>prints 3, 2, 1 with the COUNTDOWN_SLEEP period  
---
``` C
void goodbye(pixel *frame_buff);
```
>prints goodbye  
---
``` C
void print_congrats(pixel *frame_buff, pixel color);
``` 
>prints a congratulation or a draw  
---

## Knobs
*files: knobs.c, knobs.h*  
``` C
knobs_data knobs_value();
```
>returns actual knobs position  
---
``` C
bool knobs_init();
``` 
>sets the knobs memory base adress  
>find the actual knobs position for future relative computations  
---
``` C
knobs_data get_rel_knob_value();
```
>returns the relative position to the position found in the last call of this function  
---

## Keyboard
``` C
int keyboard_getc_timeout(unsigned char *c);
```
>wait KEYBOARD_TIMEOUT time for a character from stdin  
>returns 1 if a character has been read  
>returns 0 if no character has been read  
>returns -1 if an error occured  
---
## LED
*files: LED.c, LED.h*  
``` C
bool led_init();
```
>sets a LED memory base address  
>sets all LEDs to an off mode  
---
``` C
void goal_lights(int goal);
```
>lights up one the two RGB LED with color depending on the player that has scored the goal  
---
``` C
void led_line(int goals_l, int goals_r);
```
>shows the number of goals on the LED line  
---
``` C
void blick_n_times(int n, int period, int colour);
```
>blicks n times with LED of a given colour (side) ... red or blue  

---
``` C
void led_line_reset();
``` 
>sets all LEDs on the LED line off  

## General computations
*files: compute.c, compute.h*  
``` C
double my_abs(double n);
```
>return an absolute value of a double  
---
``` C
int my_sign(double n);
```
>returns signum(n)  
---
``` C
float max(float a, float b);
```
>returns the bigger value out of the two  
---
``` C
float min(float a, float b);
```
>returns the lower value out of the two  
---
``` C
void multiply_vel(Velocity *vel, float mult);
```
>multiplies both parts of the vector by a float  
---
``` C
Velocity rotate(Velocity old_vel, double angle);
```
>returns rotated vector by an angle angle  
---
## Mzapo templates
Mzapo templates are support files for work with mzapo board. It consists of:
* definitions of fonts (font_prop14x16.c, font_types.h)
* work with LCD (mzapo_parlcd.c mzapo_parlcd.h)
* support for memory adressing (mzpo_phys.c mzpo_phys.h mzpo_regs.h)
## Threads in main.c
### Main thread
**There are three threads in main.c.**
* Main thread listens to the terminal thread and the knobs thread and displays everything on LCD.
* Terminal thread listens to user's keyboard and reports requests to the main thread.
* Knobs thread listens to the three knobs and reports it to the main thread.
### Terminal thread
The thread listens to the keyboard and reports requests to the main thread.

| Key | Action                        |
| :-: | ------------------------------|
| q   | quit                          |
| w   | move up in menu               |
| s   | move down in menu             |
| d   | enter selected option in menu |

### Knobs thread
The thread listens to the three knobs and reports requests to the gameloop thread.
| Knob          | Action                |
| --------------| ----------------------|
| green rotate  | move in menu          |
| green button  | select option in menu |

## Threads in game.c
**There are four threads in game.c.**
* Main thread computes a new positions of the moving objects
* Terminal thread listens to user's keyboard and reports it to the main thread
* Knobs thread listens to the three knobs and reports it to main thread
* LCD thread shows objects in their actual positions when signaled from the main thread
### Gameloop thread
The thread runs through the game_loop function every 500 ms, where it:
1. moves the ball and paddles
2. controls collisions: ball & paddles, ball & bonuses
3. confirmes requests for enlarging the player's paddle
4. generates a new bonus every 5 seconds (up to 5 bonuses maximum can be displayed in the play field at the same time)
5. resizes the paddle to its original length after the player choses to enlarge its paddle for 5 seconds

### Terminal thread
The thread listens to user's keyboard and reports requests to the gameloop thread.

| Key | Action                              |
| :-: | ------------------------------------|
| q   | quit                                |
| r   | move red paddle up                  |
| f   | move red paddle down                |
| i   | move blue paddle up                 |
| k   | move blue paddle down               |
| p   | pause                               |
| w   | move up in menu                     |
| s   | move down in menu                   |
| d   | enter selected option in pause menu |

### Knobs thread
The thread listens to the three knobs and reports requests to the gameloop thread.
| Knob          | Action                               |
| --------------| -------------------------------------|
| green rotate  | move in pause menu                   |
| red rotate    | move red paddle                      |
| blue rotate   | move blue paddle                     |
| green button  | pause or (when in pause menu) select |
| red button    | activate bonus - enlarge paddle      |
| blue button   | activate bonus - enlarge paddle      |

### LCD thread
The thread displays all objects on the LCD when signaled from the gameloop thread.  
When an object is moving, the program prints the object on its previous position in the color of background and then it prints it in its actual color in the new position.  


###### tags: `PingPong` `Documentation`
