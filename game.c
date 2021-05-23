// file for handling game objects
// will handle colisions
#include "game.h"
#include "headers.h"
#include "keyboard.h"
#include "knobs.h"
#include "LED.h"

#define UPDATE_RATE 150
#define INITIAL_BALL_POSITION ((Position){.X = DISPLAY_WIDTH / 2, .Y = DISPLAY_HEIGHT / 2})
#define INITIAL_BALL_SPEED 0.2
#define INITIAL_BALL_RADIUS 10
#define INITIAL_PADDLE_LEFT ((Position){.X = 0, .Y = DISPLAY_HEIGHT / 2 - PADDLE_LENGTH / 2 })
#define INITIAL_PADDLE_RIGHT ((Position){.X = DISPLAY_WIDTH - PADDLE_WIDTH, .Y = DISPLAY_HEIGHT / 2 - PADDLE_LENGTH / 2})
#define COUNTDOWN_X 115
#define COUNTDOWN_Y 105
#define COUNTDOWN_SCALE 5
#define BLICKING_PERIOD 50000
#define PADDLE_SPEED 5
#define BALL_SPEED 1
#define MAX_GOALS 2 // ALL_GOALS
#define PI 3.14159265358979323846
#define ANGLE (PI / 5)
#define EASY_ACCELERATION 1.05
#define HARD_ACCELERATION 1.10
#define GAMELOOP_SLEEP 500

enum{ NORMAL_LEVEL, HARD_LEVEL, NBR_LEVELS };

void *terminal_listening();
void *knobs_listening();
void *lcd_output();
void *led_output();
void init_paddle(Paddle *p, Position pos, unsigned short color);
void init_ball(Ball *b, Position pos, Velocity velocity, unsigned short color,
               int radius);
void init_shared_data();
int move_ball(Ball *ball, Paddle *left, Paddle *right);

static struct {
    int l, r;
} score = {
    .l = 0,
    .r = 0,
};

int level;
static double acceleration; 
unsigned short *frame_buff;
enum { RESUME, EXIT };

static pthread_cond_t input_condvar;
static pthread_cond_t output_condvar;
static pthread_cond_t knobs_condvar;
static pthread_mutex_t mtx;


struct shared {
    bool quit;
    bool pause;
    bool redraw;
    int pause_menu_selected;
    struct led {
        bool state;
        unsigned short color;
    } led1, led2;
    Ball *ball;
    Paddle *paddle_left, *paddle_right;
    struct {
        int left;
        int right;
    }paddle_move;
    int congrats;
    bool countdown;
} shared_data;

void init_shared_data() {
    shared_data.quit = false;
    shared_data.pause = false;
    shared_data.countdown = false;
    shared_data.pause_menu_selected = 0;
    shared_data.led1.state = false;
    shared_data.led2.state = false;
    acceleration = (level == NORMAL_LEVEL ? EASY_ACCELERATION : HARD_ACCELERATION);
    Ball *ball = malloc(sizeof(Ball));
    Paddle *paddle_l = malloc(sizeof(Paddle));
    Paddle *paddle_r = malloc(sizeof(Paddle));
    if (!ball || !paddle_l || !paddle_r) {
        if (ball)
            free(ball);
        if (paddle_l)
            free(paddle_l);
        if (paddle_r)
            free(paddle_r);
        fprintf(stderr, "ERROR: Not enough space for dynamic allocation!\n");
        call_stty(1);
        exit(EXIT_FAILURE);
    }
    init_ball(ball, INITIAL_BALL_POSITION, gen_rand_ball_vel(), WHITE, INITIAL_BALL_RADIUS);
    init_paddle(paddle_l, INITIAL_PADDLE_LEFT, RED);
    init_paddle(paddle_r, INITIAL_PADDLE_RIGHT, BLUE);
    shared_data.ball = ball;
    shared_data.paddle_left = paddle_l;
    shared_data.paddle_right = paddle_r;
    shared_data.paddle_move.right = 0;
    shared_data.paddle_move.left = 0;
    shared_data.congrats = 0;
    score.l = score.r = 0;
}

void new_round() {
    // countdown
    
    init_ball(shared_data.ball, INITIAL_BALL_POSITION, gen_rand_ball_vel(), WHITE, INITIAL_BALL_RADIUS);
    init_paddle(shared_data.paddle_left, INITIAL_PADDLE_LEFT, RED);
    init_paddle(shared_data.paddle_right, INITIAL_PADDLE_RIGHT, BLUE);

    pthread_mutex_lock(&mtx);
    shared_data.countdown = true;
    pthread_mutex_unlock(&mtx);
    pthread_cond_broadcast(&output_condvar);
}

void start_game(unsigned short *fb, int lev) {
    frame_buff = fb;
    level = lev;

    init_shared_data();
    led_line_reset();
    char *all_levels[] = { "Normal", "Hard" };
    printf("Selected %s level\n", all_levels[lev]);
    
    // threads
    enum { INPUT_TERMINAL, INPUT_KNOBS, OUTPUT_LCD, NUM_THREADS };
    char *all_threads[] = { "Terminal", "Knobs", "LCD" };
    void *(*thr_threads[])(void *) = {terminal_listening, knobs_listening,
                                      lcd_output};
    pthread_t threads[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; ++i) {
        printf("Creating %s thread\n", all_threads[i]);
        pthread_create(&threads[i], NULL, thr_threads[i], NULL);
    }
    game_loop();

    for (int i = 0; i < NUM_THREADS; ++i) {
        pthread_join(threads[i], NULL);
    }
    set_display_black(fb);
}

// returns 0 if no goal detected
// returns 1 if goal on right occured
// returns -1 if goal on left occured
int move_ball(Ball *ball, Paddle *left, Paddle *right) {
    int ret = 0;
    double new_angle;
    int radius = ball->radius;
    Position new_pos = new_position(ball); // compute next position
    Velocity velocity = ball->velocity;
    
    // bouncing on top and bottom
    if (new_pos.Y + radius >= DISPLAY_HEIGHT) { // bottom wall
        new_pos.Y = 2 * DISPLAY_HEIGHT - new_pos.Y - 2 * radius;
        velocity.Y = -velocity.Y;
    } else if(new_pos.Y - radius <= 0) { // upper wall
        new_pos.Y = -new_pos.Y + 2 * radius;
        velocity.Y = -velocity.Y;
    //bouncing on paddles
    } else if(new_pos.X - radius <= LEFT_BORDER){ // left wall
        if (paddle_touch(ball->pos.Y, left->pos.Y, left->height)){
            // paddle touches
            new_pos.X = 2 * LEFT_BORDER - new_pos.X + 2 * radius;
            new_angle = ANGLE * rebound_angle(new_pos.Y, left->pos.Y, PADDLE_LENGTH);
            velocity.X = -velocity.X;
            // rotate
            velocity.X = (cos(new_angle) * velocity.X) - (sin(new_angle) * velocity.Y);
            velocity.Y = (sin(new_angle) * velocity.X) + (cos(new_angle) * velocity.Y);
            
            multiply_vel(&velocity, acceleration);
        } else
            ret = LEFT_GOAL;
    } else if (new_pos.X + radius >= RIGHT_BORDER) { // right wall
        if (paddle_touch(ball->pos.Y, right->pos.Y, right->height)) {
            new_pos.X = 2 * RIGHT_BORDER - new_pos.X - 2 * radius;
            new_angle = - ANGLE * rebound_angle(new_pos.Y, right->pos.Y, PADDLE_LENGTH);
            velocity.X = -velocity.X;
            // rotate
            velocity.X = (cos(new_angle) * velocity.X) - (sin(new_angle) * velocity.Y);
            velocity.Y = (sin(new_angle) * velocity.X) + (cos(new_angle) * velocity.Y);
            multiply_vel(&velocity, acceleration);
        } else
            ret = RIGHT_GOAL;
    }
    ball->pos = new_pos;
    ball->velocity = velocity;
    return ret;
}

void game_loop() {
    pthread_mutex_lock(&mtx);
    Ball *ball = shared_data.ball;
    Paddle *paddle_left = shared_data.paddle_left;
    Paddle *paddle_right = shared_data.paddle_right;
    bool pause = shared_data.pause;
    bool quit = shared_data.quit;
    int congrats = shared_data.congrats;
    bool is_coundown = shared_data.countdown;
    pthread_mutex_unlock(&mtx);

    int goal = NO_GOAL;
    while(!quit){
        goal = NO_GOAL;

        if (score.l < MAX_GOALS && score.r < MAX_GOALS)
            new_round();
        else
            quit = true;

        while (!goal && !quit) {
            if (!pause && !congrats && !is_coundown){
                pthread_mutex_lock(&mtx);
                move_paddle(paddle_left, PADDLE_SPEED * shared_data.paddle_move.left);
                move_paddle(paddle_right, -PADDLE_SPEED * shared_data.paddle_move.right);
                if (goal = move_ball(ball, paddle_left, paddle_right)){ // goal
                    if(goal == LEFT_GOAL){
                        score.r += 1;
                        blick_n_times(10, BLICKING_PERIOD, RIGHT_GOAL);  
                    }
                    else if (goal == RIGHT_GOAL){
                        score.l += 1;
                        blick_n_times(10, BLICKING_PERIOD, LEFT_GOAL);
                    }
                    led_line(score.l, score.r);
                    
                    if (score.l == MAX_GOALS || score.r == MAX_GOALS){
                        if(score.l == MAX_GOALS)
                            congrats = LEFT_PLAYER;
                        else
                            congrats = RIGHT_PLAYER;
                    }
                    
                }
                pthread_mutex_unlock(&mtx);
            }
            pthread_mutex_lock(&mtx);
            pause = shared_data.pause;
            quit = shared_data.quit || quit;
            shared_data.congrats = congrats;
            is_coundown = shared_data.countdown;
            pthread_mutex_unlock(&mtx);
            pthread_cond_broadcast(&output_condvar);
            pthread_cond_broadcast(&knobs_condvar);
            usleep(GAMELOOP_SLEEP);
        }
        pthread_mutex_lock(&mtx);
        shared_data.quit = shared_data.quit || quit;
        pthread_mutex_unlock(&mtx);
        pthread_cond_broadcast(&output_condvar);
        pthread_cond_broadcast(&knobs_condvar);
    }
}

// terminal in game listening
void *terminal_listening() {
    printf("Terminal thread running\n");
    pthread_mutex_lock(&mtx);
    bool quit = shared_data.quit;
    bool pause = shared_data.pause;
    int pause_menu_selected = shared_data.pause_menu_selected;
    int move_left = shared_data.paddle_move.left;
    int move_right = shared_data.paddle_move.right;
    pthread_mutex_unlock(&mtx);
    bool pause_menu_selected_changed;
    unsigned char c;
    int r;
    
    while (!quit) {
        r = keyboard_getc_timeout(&c);
        pause_menu_selected_changed = false;
        if(r > 0){
            move_right = 0;
            move_left = 0;
            switch (c) {
                case 'r':
                    move_left = -5;
                    break;
                case 'f':
                    move_left = 5;
                    break;
                case 'i':
                    move_right = -5;
                    break;
                case 'k':
                    move_right = 5;
                    break;
                case 'w':
                    if (pause && pause_menu_selected > 0) {
                        --pause_menu_selected;
                        pause_menu_selected_changed = true;
                    }
                    break;
                case 's':
                    if (pause && pause_menu_selected < PAUSE_MENU_SELECTION) {
                        ++pause_menu_selected;
                        pause_menu_selected_changed = true;
                    }
                    break;
                case 'd':
                    if (pause){
                        switch(pause_menu_selected){
                            case RESUME:
                                pthread_mutex_lock(&mtx);
                                shared_data.pause = false;
                                pthread_mutex_unlock(&mtx);
                                break;
                            case EXIT:
                                quit = true;
                                break;
                        }
                    }
                    break;
                case 'p':
                    pthread_mutex_lock(&mtx);
                    shared_data.pause = true;
                    pthread_mutex_unlock(&mtx);
                    break;
                case 'q':
                    quit = true;
                    break;
            }//end switch
        } else if (r < 0){
            fprintf(stderr, "ERROR: Keyboard reading error");
            quit = true;
        }
        
        pthread_mutex_lock(&mtx);
            if (quit)
                shared_data.quit = quit;
            else
                quit = shared_data.quit;
            pause = shared_data.pause;
            if(pause_menu_selected_changed)
                shared_data.pause_menu_selected = pause_menu_selected;
            else
                pause_menu_selected = shared_data.pause_menu_selected;
            if(move_left)
                shared_data.paddle_move.left = move_left;
            if (move_right)
                shared_data.paddle_move.right = move_right;
            pthread_mutex_unlock(&mtx);
        pthread_cond_broadcast(&input_condvar);
    }
    printf("Terminal thread exiting\n");
    return EXIT_SUCCESS;
}

void *knobs_listening()
{
    printf("Knobs thread running\n");
    pthread_mutex_lock(&mtx);
    bool quit = shared_data.quit;
    bool pause = shared_data.pause;
    int move_left = shared_data.paddle_move.left;
    int move_right = shared_data.paddle_move.right;
    int pause_menu_selected = shared_data.pause_menu_selected;
    pthread_mutex_unlock(&mtx);
    bool pause_menu_selected_changed = false;
    knobs_data kd;

    while (!quit) { // enter knobs listening
        kd = get_rel_knob_value();
        pause_menu_selected_changed = false;
        if (kd.gb){
            usleep(DEBOUNCE_GREEN_KNOB);
            if (pause){
                switch(pause_menu_selected){
                    case RESUME:
                        pthread_mutex_lock(&mtx);
                            shared_data.pause = false;
                        pthread_mutex_unlock(&mtx);
                        pause = false;
                        break;
                    case EXIT:
                        quit = true;
                        break;
                }
            }
            else
                pause = true;
        }
        if(kd.gk < 0 && pause_menu_selected > 0){
            --pause_menu_selected;
            pause_menu_selected_changed = true;
        } else if (kd.gk > 0 && pause_menu_selected < PAUSE_MENU_SELECTION) {
            ++pause_menu_selected;
            pause_menu_selected_changed = true;
        }

        if (!pause) {
            move_left = kd.rk;
            move_right = kd.bk;
        }
        pthread_mutex_lock(&mtx);
        if (quit)
            shared_data.quit = quit;
        else
            quit = shared_data.quit;
        shared_data.pause = pause || shared_data.pause;

        if (pause_menu_selected_changed)
            shared_data.pause_menu_selected = pause_menu_selected;
        else
            pause_menu_selected = shared_data.pause_menu_selected;
        
        shared_data.paddle_move.left = move_left;
        shared_data.paddle_move.right = move_right;
        if(!quit)
            pthread_cond_wait(&knobs_condvar, &mtx);
        pthread_mutex_unlock(&mtx);
    }
    printf("Knobs thread exiting\n");
    return EXIT_SUCCESS;
}

void *lcd_output() {
    // shared values
    pthread_mutex_lock(&mtx);
    Ball *ball = shared_data.ball;
    Paddle *paddle_left = shared_data.paddle_left;
    Paddle *paddle_right = shared_data.paddle_right;
    bool pause = shared_data.pause;
    bool quit = shared_data.quit;
    bool is_countdown = shared_data.countdown;
    int pause_menu_selected = shared_data.pause_menu_selected;
    Position ball_pos = ball->pos;
    Position paddle_l_pos = paddle_left->pos;
    Position paddle_r_pos = paddle_right->pos;
    int radius = ball->radius;
    unsigned short ball_color = ball->color;
    unsigned short paddle_l_color = paddle_left->color;
    unsigned short paddle_r_color = paddle_right->color;
    int paddle_l_width = paddle_left->width;
    int paddle_r_width = paddle_right->width;
    int paddle_l_length = paddle_left->height;
    int paddle_r_length = paddle_right->height;
    int congrats = shared_data.congrats;
    pthread_mutex_unlock(&mtx);

    // old values
    Position old__ball_pos = ball_pos;
    int old_radius = radius;
    Position old__paddle_l_pos = paddle_l_pos;
    Position old__paddle_r_pos = paddle_r_pos;
    int old_paddle_l_length = paddle_l_length;
    int old_paddle_r_length = paddle_r_length;
    bool last_pause = pause;
    
    // update all
    update_ball(ball_pos, old__ball_pos, radius, old_radius, ball_color, frame_buff);
    update_paddle(paddle_l_pos, old__paddle_l_pos, paddle_l_width, paddle_l_length, old_paddle_l_length, paddle_l_color, frame_buff);
    update_paddle(paddle_r_pos, old__paddle_r_pos, paddle_r_width, paddle_r_length, old_paddle_r_length, paddle_r_color, frame_buff);
    update_display(frame_buff);

    // update ball
    // update paddles
    // update pause menu if in menu state
    while(!quit){
        pthread_mutex_lock(&mtx);
        quit = shared_data.quit;
        if (!quit){
            pthread_cond_wait(&output_condvar, &mtx);
            quit = shared_data.quit;
        }
        pause = shared_data.pause;
        is_countdown = shared_data.countdown;
        pause_menu_selected = shared_data.pause_menu_selected;
        ball_pos = ball->pos;
        radius = ball->radius;
        paddle_l_width = paddle_left->width;
        paddle_r_width = paddle_right->width;
        paddle_l_length = paddle_left->height;
        paddle_r_length = paddle_right->height;
        paddle_l_pos = paddle_left->pos;
        paddle_r_pos = paddle_right->pos;
        congrats = shared_data.congrats;
        pthread_mutex_unlock(&mtx);
        
        if(!pause && last_pause){
            // delete pause menu
            clear_buffer(frame_buff);
        }
        last_pause = pause;

        if ((!quit) || congrats) {
            if (pause)
                print_pause_menu(PAUSE_MENU_OFFSET_X, PAUSE_MENU_OFFSET_Y, pause_menu_selected, frame_buff);
            else if (congrats) {
                printf("congrats in LCD thread\n");
                unsigned short congr_color = (congrats == LEFT_PLAYER) ? RED : BLUE;
                print_congrats(frame_buff, congr_color);

                pthread_mutex_lock(&mtx);
                shared_data.congrats = 0;
                pthread_mutex_unlock(&mtx);
            }else if (is_countdown){
                clear_buffer(frame_buff);
                update_paddle(paddle_l_pos, old__paddle_l_pos, paddle_l_width, paddle_l_length, old_paddle_l_length, paddle_l_color, frame_buff);
                update_paddle(paddle_r_pos, old__paddle_r_pos, paddle_r_width, paddle_r_length, old_paddle_r_length, paddle_r_color, frame_buff);
                countdown(COUNTDOWN_X, COUNTDOWN_Y, COUNTDOWN_SCALE, frame_buff);

                pthread_mutex_lock(&mtx);
                shared_data.countdown = false;
                pthread_mutex_unlock(&mtx);
            } else {
                update_ball(ball_pos, old__ball_pos, radius, old_radius, ball_color, frame_buff);
                update_paddle(paddle_l_pos, old__paddle_l_pos, paddle_l_width, paddle_l_length, old_paddle_l_length, paddle_l_color, frame_buff);
                update_paddle(paddle_r_pos, old__paddle_r_pos, paddle_r_width, paddle_r_length, old_paddle_r_length, paddle_r_color, frame_buff);

                old__ball_pos = ball_pos;
                old_radius = radius;
                old__paddle_l_pos = paddle_l_pos;
                old__paddle_r_pos = paddle_r_pos;
                old_paddle_l_length = paddle_l_length;
                old_paddle_r_length = paddle_r_length;
            }
            update_display(frame_buff);
        }
    }
    printf("LCD output thread exiting\n");
    return EXIT_SUCCESS;
}

Velocity gen_rand_ball_vel() {
    double random_value;
    srand ( time ( NULL));
    random_value = (double)rand()/RAND_MAX*2.0-1.0; //float in range -1 to 1
    double r = 45. * (random_value / 100 - 1);
    double x = sin(r);
    double y = cos(r);
    random_value = (double)rand()/RAND_MAX*2.0-1.0; //float in range -1 to 1
    if (random_value < 0) {
        x *= -1;
        y *= -1;
    }
    return (Velocity){.X = INITIAL_BALL_SPEED * x, .Y = INITIAL_BALL_SPEED * y};
}

void multiply_vel(Velocity *vel, float mult){
    vel->X *= mult;
    vel->Y *= mult;
}
