#include "game.h"
#include "LED.h"
#include "bonus.h"
#include "compute.h"
#include "keyboard.h"
#include "knobs.h"

enum { NORMAL_LEVEL, HARD_LEVEL, NBR_LEVELS };

/* 
 * watches terminal for user activity
 * terminal control is described in the game documentation
 */
void *terminal_listening();

/* 
 * waits for a response from one of the three RGB knobs to play the game
 * reports the current state of knobs in the shared_data structure
 */
void *knobs_listening();

/* 
 * draws the frame buffer on the display
 * if there is request for printing countdown or pause menu, it prints it
 */
void *lcd_output();

/* intializes data for both paddles and the ball */
void init_shared_data();

/* frees allocated memory in shared data */
void clean_shared_data();

/*
 * changes ball position depending on its valocity
 * controls if ball hits paddle or hits the net
 * returns 0 if no goal detected
 * returns 1 if goal on right occured
 * returns -1 if goal on left occured
 */
int move_ball(Ball *ball, Paddle *left, Paddle *right);

static struct {
    int l, r;
} score = {
    .l = 0,
    .r = 0,
};

static int level;
static int last_hit;
static double acceleration;
pixel *frame_buff;
enum pause_menu { RESUME, EXIT };

static pthread_cond_t input_condvar;
static pthread_cond_t output_condvar;
static pthread_cond_t knobs_condvar;
static pthread_mutex_t mtx;

// data shared among threads
static struct shared {
    bool quit;
    bool pause;
    bool redraw;
    int pause_menu_selected;
    struct led {
        bool state;
        pixel color;
    } led1, led2;
    Ball *ball;
    struct {
        Bonus arr[MAX_BONUSES_NBR];
        int i;
    } bonuses;
    struct {
        Bonus bonus;  // lcd display shall print this black
        bool delete;
    } delete_bonus;
    Paddle *paddle_left, *paddle_right;
    struct {
        int left;
        int right;
    } paddle_move;
    int congrats;
    bool countdown;
    struct {
        bool left, right;
    } knob_activate;
} shared_data;


void init_shared_data() {
    shared_data.quit = false;
    shared_data.pause = false;
    shared_data.countdown = false;
    shared_data.pause_menu_selected = 0;
    shared_data.led1.state = false;
    shared_data.led2.state = false;
    shared_data.knob_activate.left = false;
    shared_data.knob_activate.right = false;
    acceleration = (level == NORMAL_LEVEL ? EASY_ACCELERATION : HARD_ACCELERATION);
    Ball *ball = malloc(sizeof(Ball));
    Paddle *paddle_l = malloc(sizeof(Paddle));
    Paddle *paddle_r = malloc(sizeof(Paddle));

    if (!ball || !paddle_l || !paddle_r) {
        if (ball) free(ball);
        if (paddle_l) free(paddle_l);
        if (paddle_r) free(paddle_r);

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
    last_hit = 0;
    score.l = score.r = 0;
    shared_data.paddle_move.right = 0;
    shared_data.paddle_move.left = 0;
    shared_data.congrats = 0;
    shared_data.bonuses.i = 0;
    shared_data.delete_bonus.delete = false;
}


void clean_shared_data() {
    if (shared_data.ball) free(shared_data.ball);
    if (shared_data.paddle_left) free(shared_data.paddle_left);
    if (shared_data.paddle_right) free(shared_data.paddle_right);
}

void new_round() {
    last_hit = 0;
    shared_data.paddle_move.right = 0;
    shared_data.paddle_move.left = 0;
    shared_data.bonuses.i = 0;
    shared_data.delete_bonus.delete = false;

    init_ball(shared_data.ball, INITIAL_BALL_POSITION, gen_rand_ball_vel(), WHITE, INITIAL_BALL_RADIUS);
    init_paddle(shared_data.paddle_left, INITIAL_PADDLE_LEFT, RED);
    init_paddle(shared_data.paddle_right, INITIAL_PADDLE_RIGHT, BLUE);

    pthread_mutex_lock(&mtx);
    shared_data.countdown = true;
    pthread_mutex_unlock(&mtx);
    pthread_cond_broadcast(&output_condvar);
}

void start_game(pixel *fb, int lev) {
    frame_buff = fb;
    level = lev;

    init_shared_data();
    led_line_reset();
    char *all_levels[] = {"Normal", "Hard"};
    printf("Selected %s level\n", all_levels[lev]);

    // threads
    enum { INPUT_TERMINAL, INPUT_KNOBS, OUTPUT_LCD, NUM_THREADS };
    char *all_threads[] = {"Terminal", "Knobs", "LCD"};
    void *(*thr_threads[])(void *) = {terminal_listening, knobs_listening, lcd_output};
    pthread_t threads[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; ++i) {
        printf("Creating %s thread\n", all_threads[i]);
        pthread_create(&threads[i], NULL, thr_threads[i], NULL);
    }
    game_loop();

    for (int i = 0; i < NUM_THREADS; ++i) {
        pthread_join(threads[i], NULL);
    }
    clean_shared_data();
    set_display_black(fb);
}

int move_ball(Ball *ball, Paddle *left, Paddle *right) {
    int ret = 0;
    double new_angle;
    int radius = ball->radius;
    Position new_pos = new_position(ball);  // compute next position
    Velocity velocity = ball->velocity;

    // bouncing on top and bottom
    if (new_pos.Y + radius >= DISPLAY_HEIGHT) {  // bottom wall
        new_pos.Y = 2 * DISPLAY_HEIGHT - new_pos.Y - 2 * radius;
        velocity.Y = -velocity.Y;
    } else if (new_pos.Y - radius <= 0) {  // upper wall
        new_pos.Y = -new_pos.Y + 2 * radius;
        velocity.Y = -velocity.Y;
        // bouncing on paddles
    } else if (new_pos.X - radius <= LEFT_BORDER) {  // left wall
        if (paddle_touch(ball->pos.Y, left->pos.Y, left->height, radius)) {
            last_hit = LEFT_PLAYER;
            new_pos.X = 2 * LEFT_BORDER - new_pos.X + 2 * radius;
            new_angle = ANGLE * rebound_angle(new_pos.Y, left->pos.Y, left->height);
            velocity.X = -velocity.X;
            // rotate
            velocity = rotate(velocity, new_angle);
            multiply_vel(&velocity, acceleration);
        } else
            ret = LEFT_GOAL;
    } else if (new_pos.X + radius >= RIGHT_BORDER) {  // right wall
        if (paddle_touch(ball->pos.Y, right->pos.Y, right->height, radius)) {
            last_hit = RIGHT_PLAYER;
            new_pos.X = 2 * RIGHT_BORDER - new_pos.X - 2 * radius;
            new_angle = -ANGLE * rebound_angle(new_pos.Y, right->pos.Y, right->height);
            velocity.X = -velocity.X;
            // rotate
            velocity = rotate(velocity, new_angle);
            multiply_vel(&velocity, acceleration);
        } else
            ret = RIGHT_GOAL;
    }
    ball->pos = new_pos;
    maximal_angle(&velocity);
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
    bool is_countdown = shared_data.countdown;
    bool knob_activate_left = shared_data.knob_activate.left;
    bool knob_activate_right = shared_data.knob_activate.right;
    pthread_mutex_unlock(&mtx);

    int t_spawn_bonus = 0;  // time
    int t_paddle_restore_left = 0;
    int t_paddle_restore_right = 0;
    bool enlarge_ability_left = false;
    bool enlarge_ability_right = false;
    int goal = NO_GOAL;
    int idx_hit;
    while (!quit) {
        goal = NO_GOAL;

        if (score.l < MAX_GOALS && score.r < MAX_GOALS)
            new_round();
        else
            quit = true;

        while (!goal && !quit) {
            if (!pause && !congrats && !is_countdown) {
                pthread_mutex_lock(&mtx);
                move_paddle(paddle_left, PADDLE_SPEED * shared_data.paddle_move.left);
                move_paddle(paddle_right, -PADDLE_SPEED * shared_data.paddle_move.right);
                if ((goal = move_ball(ball, paddle_left, paddle_right))) {  // goal
                    if (goal == LEFT_GOAL) {
                        if (++score.r >= MAX_GOALS) score.r = MAX_GOALS;
                        blick_n_times(10, BLICKING_PERIOD, RIGHT_GOAL);
                    } else if (goal == RIGHT_GOAL) {
                        if (++score.l >= MAX_GOALS) score.l = MAX_GOALS;
                        blick_n_times(10, BLICKING_PERIOD, LEFT_GOAL);
                    }
                    led_line(score.l, score.r);

                    // somebody got all points
                    if (score.l >= MAX_GOALS || score.r >= MAX_GOALS) {
                        if (score.l > score.r)
                            congrats = LEFT_PLAYER;
                        else if (score.r > score.l)
                            congrats = RIGHT_PLAYER;
                        else
                            congrats = DRAW;
                    }
                }
                if ((idx_hit = bonus_hit(shared_data.bonuses.arr, shared_data.bonuses.i, ball)) != NO_HIT) {
                    switch (shared_data.bonuses.arr[idx_hit].bonus_mode) {
                        case POINTS_BONUS:
                            if (last_hit == LEFT_PLAYER) {
                                if (++score.l >= MAX_GOALS) score.l = MAX_GOALS;
                                led_line(score.l, score.r);
                            } else if (last_hit == RIGHT_PLAYER) {
                                if (++score.r >= MAX_GOALS) score.r = MAX_GOALS;
                                led_line(score.l, score.r);
                            }
                            break;
                        case ENLARGE_BONUS:
                            if (last_hit == LEFT_PLAYER)
                                enlarge_ability_left = true;
                            else if (last_hit == RIGHT_PLAYER)
                                enlarge_ability_right = true;
                            break;
                    }
                    shared_data.delete_bonus.bonus = remove_bonus(shared_data.bonuses.arr, idx_hit, shared_data.bonuses.i);
                    shared_data.delete_bonus.delete = true;
                    shared_data.bonuses.i--;
                }
                pthread_mutex_unlock(&mtx);

                if (knob_activate_left && enlarge_ability_left) {  // enlarge left paddle
                    enlarge_ability_left = false;
                    t_paddle_restore_left = RESTORE_TIME / GAMELOOP_SLEEP;
                    pthread_mutex_lock(&mtx);
                    enlarge_paddle(paddle_left);
                    pthread_mutex_unlock(&mtx);
                }
                if (knob_activate_right && enlarge_ability_right) {  // enlarge left paddle
                    enlarge_ability_right = false;
                    t_paddle_restore_right = RESTORE_TIME / GAMELOOP_SLEEP;
                    pthread_mutex_lock(&mtx);
                    enlarge_paddle(paddle_right);
                    pthread_mutex_unlock(&mtx);
                }
                if (++t_spawn_bonus == SPAWN_TIME / GAMELOOP_SLEEP) {  // add bonus
                    t_spawn_bonus = 0;
                    pthread_mutex_lock(&mtx);
                    int i = shared_data.bonuses.i;
                    if (i < MAX_BONUSES_NBR) {
                        init_bonus(&(shared_data.bonuses.arr[i]), BONUS_RADIUS);
                        ++shared_data.bonuses.i;
                    }
                    pthread_mutex_unlock(&mtx);
                }
                if (t_paddle_restore_left > 0 && --t_paddle_restore_left == 0) {  // restore default paddle length
                    pthread_mutex_lock(&mtx);
                    reduce_paddle(paddle_left);
                    pthread_mutex_unlock(&mtx);
                }
                if (t_paddle_restore_right > 0 && --t_paddle_restore_right == 0) {  // restore default paddle length
                    pthread_mutex_lock(&mtx);
                    reduce_paddle(paddle_right);
                    pthread_mutex_unlock(&mtx);
                }
            }

            // update local copies of shared data
            pthread_mutex_lock(&mtx);
            pause = shared_data.pause;
            quit = shared_data.quit || quit;
            shared_data.congrats = congrats;
            is_countdown = shared_data.countdown;
            knob_activate_left = shared_data.knob_activate.left;
            knob_activate_right = shared_data.knob_activate.right;
            pthread_mutex_unlock(&mtx);

            // signal to threads
            pthread_cond_broadcast(&output_condvar);
            pthread_cond_broadcast(&knobs_condvar);

            usleep(GAMELOOP_SLEEP);
        }  // end while !goal && !quit
        pthread_mutex_lock(&mtx);
        shared_data.quit = shared_data.quit || quit;
        pthread_mutex_unlock(&mtx);
        pthread_cond_broadcast(&output_condvar);
        pthread_cond_broadcast(&knobs_condvar);
    }
}

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
        if (r > 0) {
            move_right = 0;
            move_left = 0;
            switch (c) {
                case 'r':
                    move_left = -KEYBOARD_PADDLE_CONTROL;
                    break;
                case 'f':
                    move_left = KEYBOARD_PADDLE_CONTROL;
                    break;
                case 'i':
                    move_right = -KEYBOARD_PADDLE_CONTROL;
                    break;
                case 'k':
                    move_right = KEYBOARD_PADDLE_CONTROL;
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
                    if (pause) {
                        switch (pause_menu_selected) {
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
            }  // end switch
        } else if (r < 0) {
            fprintf(stderr, "ERROR: Keyboard reading error");
            quit = true;
        }

        pthread_mutex_lock(&mtx);
        if (quit)
            shared_data.quit = quit;
        else
            quit = shared_data.quit;
        pause = shared_data.pause;
        if (pause_menu_selected_changed)
            shared_data.pause_menu_selected = pause_menu_selected;
        else
            pause_menu_selected = shared_data.pause_menu_selected;
        if (move_left) shared_data.paddle_move.left = move_left;
        if (move_right) shared_data.paddle_move.right = -move_right;
        pthread_mutex_unlock(&mtx);
        pthread_cond_broadcast(&input_condvar);
    }
    printf("Terminal thread exiting\n");
    return EXIT_SUCCESS;
}

void *knobs_listening() {
    printf("Knobs thread running\n");
    pthread_mutex_lock(&mtx);
    bool quit = shared_data.quit;
    bool pause = shared_data.pause;
    int move_left = shared_data.paddle_move.left;
    int move_right = shared_data.paddle_move.right;
    int pause_menu_selected = shared_data.pause_menu_selected;
    bool knob_activate_left = shared_data.knob_activate.left;
    bool knob_activate_right = shared_data.knob_activate.right;
    pthread_mutex_unlock(&mtx);
    bool pause_menu_selected_changed = false;
    knobs_data kd;

    while (!quit) {  // enter knobs listening
        kd = get_rel_knob_value();
        pause_menu_selected_changed = false;

        // GREEN KNOB BUTTON PRESSED
        if (kd.gb) {
            usleep(DEBOUNCE_GREEN_KNOB);
            if (pause) {
                switch (pause_menu_selected) {
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
            } else
                pause = true;
        }

        // GREEN KNOB ROTATED
        if (kd.gk < 0 && pause_menu_selected > 0) {
            --pause_menu_selected;
            pause_menu_selected_changed = true;
        } else if (kd.gk > 0 && pause_menu_selected < PAUSE_MENU_SELECTION) {
            ++pause_menu_selected;
            pause_menu_selected_changed = true;
        }

        knob_activate_left = kd.rb;
        knob_activate_right = kd.bb;

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
        shared_data.knob_activate.left = knob_activate_left;
        shared_data.knob_activate.right = knob_activate_right;
        if (!quit) pthread_cond_wait(&knobs_condvar, &mtx);
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
    pixel ball_color = ball->color;
    pixel paddle_l_color = paddle_left->color;
    pixel paddle_r_color = paddle_right->color;
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
    while (!quit) {
        pthread_mutex_lock(&mtx);
        quit = shared_data.quit;
        if (!quit) {
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

        if (!pause && last_pause) {
            // delete pause menu
            clear_buffer(frame_buff);
        }
        last_pause = pause;

        if ((!quit) || congrats) {
            if (pause)
                print_pause_menu(PAUSE_MENU_OFFSET_X, PAUSE_MENU_OFFSET_Y, pause_menu_selected, frame_buff);
            else if (congrats) {
                printf("congrats in LCD thread\n");
                pixel congr_color;
                if (congrats == LEFT_PLAYER)
                    congr_color = RED;
                else if (congrats == RIGHT_PLAYER)
                    congr_color = BLUE;
                else  // draw
                    congr_color = PURPLE;
                print_congrats(frame_buff, congr_color);

                pthread_mutex_lock(&mtx);
                shared_data.congrats = 0;
                pthread_mutex_unlock(&mtx);
            } else if (is_countdown) {
                clear_buffer(frame_buff);
                update_paddle(paddle_l_pos, old__paddle_l_pos, paddle_l_width, paddle_l_length, old_paddle_l_length, paddle_l_color, frame_buff);
                update_paddle(paddle_r_pos, old__paddle_r_pos, paddle_r_width, paddle_r_length, old_paddle_r_length, paddle_r_color, frame_buff);
                countdown(COUNTDOWN_X, COUNTDOWN_Y, COUNTDOWN_SCALE, frame_buff);

                pthread_mutex_lock(&mtx);
                shared_data.countdown = false;
                pthread_mutex_unlock(&mtx);
            } else {
                pthread_mutex_lock(&mtx);
                print_bonuses(shared_data.bonuses.arr, shared_data.bonuses.i, frame_buff);
                if (shared_data.delete_bonus.delete) {
                    Bonus del = shared_data.delete_bonus.bonus;
                    del.ball.color = BACKGROUND_COLOR;
                    print_bonus(&del, frame_buff);
                    shared_data.delete_bonus.delete = false;
                }
                pthread_mutex_unlock(&mtx);

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
    srand(time(NULL));
    random_value = ((double)rand() / RAND_MAX * 2.0) - 1.0;  // float in range -1 to 1
    double r = 45. * (random_value / 100 - 1);
    double x = sin(r);
    double y = cos(r);
    random_value = ((double)rand() / RAND_MAX * 2.0) - 1.0;  // float in range -1 to 1
    if (random_value < 0) {
        x *= -1;
        y *= -1;
    }
    return (Velocity){.X = INITIAL_BALL_SPEED * x, .Y = INITIAL_BALL_SPEED * y};
}

void enlarge_paddle(Paddle *pad) {
    if (pad->height * ENLARGE_CONST <= DISPLAY_HEIGHT) {
        float enl = pad->height * (ENLARGE_CONST - 1);
        float enl_up, enl_down;
        float dist_up, dist_down;
        dist_up = pad->pos.Y;
        dist_down = DISPLAY_HEIGHT - (pad->pos.Y + pad->height);
        enl_up = enl_down = enl / 2;
        if (dist_up >= enl_up) {  // adding to the top of the paddle
            pad->pos.Y -= enl_up;
            pad->height += enl_up;
        } else {
            enl_down += (enl_up - dist_up);  // there is not enought space
            pad->pos.Y -= dist_up;           // so the rest is added down
            pad->height += dist_up;
        }
        enl_up = 0;

        if (dist_down >= enl_down) {  // adding to the down part of the paddle
            pad->height += enl_down;
        } else {
            enl_up += (enl_down - dist_down);  // there is not enought space
            pad->pos.Y -= min(pad->pos.Y, enl_up);
            pad->height += min(pad->pos.Y, enl_up);  // so the rest is added up
        }
        enl_down = 0;
    }
}

void reduce_paddle(Paddle *pad) {
    double reduce = (pad->height - PADDLE_LENGTH) / 2;
    pad->height = PADDLE_LENGTH;
    pad->pos.Y += reduce;
}

void maximal_angle(Velocity *vel) {
    double x = vel->X;
    double y = vel->Y;
    double alpha = my_abs(atan(y / x));
    if (alpha > MAXIMAL_VELOCITY_ANGLE) { *vel = rotate(*vel, my_sign(x * y) * (MAXIMAL_VELOCITY_ANGLE - alpha)); }
}
