// file for handling game objects
// will handle colisions
#include "game.h"
#define UPDATE_RATE 500
#define INITIAL_BALL_POSITION ((Position){.X = 100, .Y = 100})
#define INITIAL_BALL_SPEED ((Velocity){.X = 0.5, .Y = 0.5})
#define INITIAL_BALL_RADIUS 10
#define INITIAL_PADDLE_LEFT ((Position){.X = 0, .Y = 0})
#define INITIAL_PADDLE_RIGHT ((Position){.X = DISPLAY_WIDTH - PADDLE_WIDTH, .Y = 0})


void *terminal_listening();
void *knobs_listening();
void *lcd_output();
void *led_output();
void init_paddle(Paddle *p, Position pos, unsigned short color);
void init_ball(Ball *b, Position pos, Velocity velocity, unsigned short color,
               int radius);
void init_shared_data();
int move_ball(Ball *ball, Paddle *left, Paddle *right);

int level;
unsigned short *frame_buff;

pthread_cond_t input_condvar;
pthread_mutex_t mtx;

struct shared {
    bool quit;
    bool pause;
    int move_left;
    int move_right;
    int pause_menu_selected;
    struct led {
        bool state;
        unsigned short color;
    } led1, led2;
    Ball *ball;
    Paddle *paddle_left, *paddle_right;
} shared_data;

void init_shared_data()
{
    shared_data.quit = false;
    shared_data.pause = false;
    shared_data.pause_menu_selected = 0;
    shared_data.move_left = 0;
    shared_data.move_right = 0;
    shared_data.led1.state = false;
    shared_data.led2.state = false;
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
    Position ball_pos = INITIAL_BALL_POSITION;
    Velocity ball_vel = INITIAL_BALL_SPEED;
    Position pad_l_pos = INITIAL_PADDLE_LEFT;
    Position pad_r_pos = INITIAL_PADDLE_RIGHT;
    init_ball(ball, ball_pos, ball_vel, WHITE, INITIAL_BALL_RADIUS);
    init_paddle(paddle_l, pad_l_pos, RED);
    init_paddle(paddle_r, pad_r_pos, BLUE);
    shared_data.ball = ball;
    shared_data.paddle_left = paddle_l;
    shared_data.paddle_right = paddle_r;
}

void start_game(unsigned short *fb, int lev)
{
    level = lev;
    frame_buff = fb;

    init_shared_data();
    printf("Selected level %d\n", lev);
    set_display_black(fb);
    countdown(115, 105, 5, fb);

    // threads
    enum { INPUT_TERMINAL, INPUT_KNOBS, OUTPUT_LCD, OUTPUT_LED, NUM_THREADS };
    void *(*thr_threads[])(void *) = {terminal_listening, knobs_listening,
                                      lcd_output, led_output};
    pthread_t threads[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; ++i) {
        printf("creating thread %d\n", i);
        pthread_create(&threads[i], NULL, thr_threads[i], NULL);
    }
    printf("Game threads created\n");

    game_loop();

    for (int i = 0; i < NUM_THREADS; ++i) {
        pthread_join(threads[i], NULL);
    }
    set_display_black(fb);
}

// returns 0 if no goal detected
// returns 1 if goal on right occured
// returns -1 if goal on left occured
int move_ball(Ball *ball, Paddle *left, Paddle *right){
    int ret = 0;
    
    pthread_mutex_lock(&mtx);
    int radius = ball->radius;
    Position new_pos = new_position(ball); // compute next position
    Velocity velocity = ball->velocity;
    pthread_mutex_unlock(&mtx);
    
    // bouncing on top and bottom
    if(new_pos.Y + radius >= DISPLAY_HEIGHT) { // bottom wall
        new_pos.Y = 2 * DISPLAY_HEIGHT - new_pos.Y - 2 * radius;
        velocity.Y = -velocity.Y;
    }else if(new_pos.Y - radius <= 0) { // upper wall
        new_pos.Y = -new_pos.Y + 2 * radius;
        velocity.Y = -velocity.Y;
    //bouncing on paddles
    }else if(new_pos.X - radius <= LEFT_BORDER){ // left wall
        if(paddle_touch(ball->pos.Y, left->pos.Y, left->height)){
            // paddle touches
            new_pos.X = 2 * LEFT_BORDER - new_pos.X + 2 * radius;
            velocity.X = -velocity.X;
        }else{
            ret = 1;
        }
    }else if(new_pos.X + radius >= RIGHT_BORDER){ // right wall
        if(paddle_touch(ball->pos.Y, right->pos.Y, right->height)){
            new_pos.X = 2 * RIGHT_BORDER - new_pos.X - 2 * radius;
            velocity.X = -velocity.X;
        }else{
            ret = -1;
        }
    }

    pthread_mutex_lock(&mtx);
    ball->pos = new_pos;
    ball->velocity = velocity;
    pthread_mutex_unlock(&mtx);
    return ret;
}

void game_loop()
{
    pthread_mutex_lock(&mtx);
        Ball *ball = shared_data.ball;
        Paddle *paddle_left = shared_data.paddle_left;
        Paddle *paddle_right = shared_data.paddle_right;
        Position paddle_right_pos = paddle_right->pos;
        Position paddle_left_pos = paddle_left->pos;
        bool pause = shared_data.pause;
        bool quit = shared_data.quit;
        int pause_menu_selected = shared_data.pause_menu_selected;
    pthread_mutex_unlock(&mtx);


    while(!quit){
        bool goal = false;
        while (!goal && !quit) {
            if (move_ball(ball, paddle_left, paddle_right)){
                goal = true;
            }
            
            pthread_mutex_lock(&mtx);
                pause = shared_data.pause;
                quit = shared_data.quit;
                pause_menu_selected = shared_data.pause_menu_selected;
            pthread_mutex_unlock(&mtx);
            int r = usleep(UPDATE_RATE);
            if(r) printf("usleep error\n");
        }
        sleep(1);
    }
}

// terminal in game listening
void *terminal_listening()
{
    printf("Terminal thread running\n");
    pthread_mutex_lock(&mtx);
    bool quit = shared_data.quit;
    bool pause = shared_data.pause;
    int pause_menu_selected = shared_data.pause_menu_selected;
    int move_right = shared_data.move_right;
    int move_left = shared_data.move_right;
    pthread_mutex_unlock(&mtx);
    char c;
    while (!quit) {
        c = getchar();
        switch (c) {
        case 'r':
            move_left = -1;
            break;
        case 'f':
            move_left = 1;
            break;
        case 'i':
            move_right = -1;
            break;
        case 'k':
            move_right = 1;
            break;
        case 'w':
            if (pause && pause_menu_selected > 0)
                --pause_menu_selected;
            break;
        case 's':
            if (pause && pause_menu_selected < PAUSE_MENU_SELECTION)
                ++pause_menu_selected;
            break;
        case 'p':
            pause = true;
            break;
        case 'q':
            quit = true;
            break;
        }
        pthread_mutex_lock(&mtx);
        if (quit)
            shared_data.quit = quit;
        else
            quit = shared_data.quit;
        shared_data.pause = pause;
        shared_data.pause_menu_selected = pause_menu_selected;
        shared_data.move_right = move_right;
        shared_data.move_left = move_left;
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
    int move_right = shared_data.move_right;
    int move_left = shared_data.move_right;
    pthread_mutex_unlock(&mtx);
    char c;
    while (!quit) {
        // enter knobs listening

        pthread_mutex_lock(&mtx);
        if (quit)
            shared_data.quit = quit;
        else
            quit = shared_data.quit;
        shared_data.pause = pause;
        shared_data.move_right = move_right;
        shared_data.move_left = move_left;
        pthread_mutex_unlock(&mtx);
        // pthread_cond_broadcast(&condvar);
    }
    printf("Knobs thread exiting\n");
    return EXIT_SUCCESS;
}

void *lcd_output()
{
    pthread_mutex_lock(&mtx);
        Ball *ball = shared_data.ball;
        Paddle *paddle_left = shared_data.paddle_left;
        Paddle *paddle_right = shared_data.paddle_right;
        bool pause = shared_data.pause;
        bool quit = shared_data.quit;
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
    pthread_mutex_unlock(&mtx);

    Position old__ball_pos = ball_pos;
    int old_radius = radius;
    Position old__paddle_l_pos = paddle_l_pos;
    Position old__paddle_r_pos = paddle_r_pos;
    int old_paddle_l_length = paddle_l_length;
    int old_paddle_r_length = paddle_r_length;
    
    update_ball(ball_pos, old__ball_pos, radius, old_radius, ball_color, frame_buff);
    update_paddle(paddle_l_pos, old__paddle_l_pos, paddle_l_width, paddle_l_length, old_paddle_l_length, paddle_l_color, frame_buff);
    update_paddle(paddle_r_pos, old__paddle_r_pos, paddle_r_width, paddle_r_length, old_paddle_r_length, paddle_r_color, frame_buff);
    update_display(frame_buff);


    // update ball
    // update paddles
    // update pause menu if in menu state
    while(!quit){
        pthread_mutex_lock(&mtx);
            pause = shared_data.pause;
            quit = shared_data.quit;
            pause_menu_selected = shared_data.pause_menu_selected;
            ball_pos = ball->pos;
            radius = ball->radius;
            paddle_l_width = paddle_left->width;
            paddle_r_width = paddle_right->width;
            paddle_l_length = paddle_left->height;
            paddle_r_length = paddle_right->height;
        pthread_mutex_unlock(&mtx);
        
        if(!quit){
            if (pause) {
                print_pause_menu(PAUSE_MENU_OFFSET_X, PAUSE_MENU_OFFSET_Y, pause_menu_selected, frame_buff);
            }
            else{
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

void *led_output() {

    return EXIT_SUCCESS;
}