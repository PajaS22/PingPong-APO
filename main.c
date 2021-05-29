#include "LCD_output.h"
#include "LED.h"
#include "game.h"
#include "headers.h"
#include "keyboard.h"
#include "knobs.h"

#define _POSIX_C_SOURCE 200112L
#define _XOPEN_SOURCE 500

/* threads */
void *terminal_listening_main();
void *knobs_listening_main();

/* sets terminal to raw mode */
void call_stty(int reset);

static pthread_cond_t condvar;
static pthread_mutex_t mtx_main;

struct shared {
    bool quit;
    bool start;
    int move;
} shared_data_main;

int main(int argc, char *argv[]) {
    if (display_init() && knobs_init() && led_init()) {
        ushort *frame_buff = (ushort *)malloc(DISPLAY_HEIGHT * DISPLAY_WIDTH * sizeof(ushort));
        if (frame_buff == NULL) {
            fprintf(stderr, "ERROR: Memory could not be allocated!\n");
        } else {
            call_stty(0);
            shared_data_main.quit = false;
            shared_data_main.start = false;
            shared_data_main.move = 0;
            bool quit = false;

            pthread_mutex_init(&mtx_main, NULL);
            pthread_cond_init(&condvar, NULL);

            int num_of_threads = 2;
            pthread_t thrs[num_of_threads];
            pthread_create(&thrs[0], NULL, terminal_listening_main, NULL);
            pthread_create(&thrs[1], NULL, knobs_listening_main, NULL);

            int move = 0;
            int selected = 0;
            bool start = false;
            print_menu(MENU_OFFSET_X, MENU_OFFSET_Y, selected, frame_buff);
            update_display(frame_buff);

            while (!quit) {
                if (move) {
                    selected += move;
                    if (selected >= MENU_SELECTION) {
                        selected = MENU_SELECTION - 1;
                    } else if (selected < 0) {
                        selected = 0;
                    }
                    pthread_mutex_lock(&mtx_main);
                    shared_data_main.move = 0;
                    pthread_mutex_unlock(&mtx_main);

                    print_menu(MENU_OFFSET_X, MENU_OFFSET_Y, selected, frame_buff);
                    update_display(frame_buff);
                }
                if (start) {
                    for (int i = 0; i < num_of_threads; ++i) {
                        pthread_join(thrs[i], NULL);
                    }
                    printf("All threads joined, starting game\n");
                    if (selected + 1 == MENU_SELECTION) { // exit selected
                        printf("Exit selected\n");
                        pthread_mutex_lock(&mtx_main);
                        shared_data_main.quit = true;
                        pthread_mutex_unlock(&mtx_main);
                    } else {
                        start_game(frame_buff, selected);
                        // game ended
                        start = false;
                        pthread_mutex_lock(&mtx_main);
                        shared_data_main.start = false;
                        pthread_mutex_unlock(&mtx_main);
                        pthread_create(&thrs[0], NULL, terminal_listening_main, NULL);
                        pthread_create(&thrs[1], NULL, knobs_listening_main, NULL);

                        print_menu(MENU_OFFSET_X, MENU_OFFSET_Y, selected, frame_buff);
                        update_display(frame_buff);
                    }
                }
                pthread_mutex_lock(&mtx_main);
                quit = shared_data_main.quit;
                if (!start && !quit) pthread_cond_wait(&condvar, &mtx_main);
                quit = shared_data_main.quit;
                start = shared_data_main.start;
                move = shared_data_main.move;
                pthread_mutex_unlock(&mtx_main);
            }
            fprintf(stderr, "Main finished!\n");
            goodbye(frame_buff);
            led_line_reset();
            if (!start) {
                for (int i = 0; i < num_of_threads; ++i) {
                    pthread_join(thrs[i], NULL);
                }
            }
            fprintf(stderr, "All threads finished!\n");
            call_stty(1);
            free(frame_buff);
        }
    }
    return EXIT_SUCCESS;
}

void *terminal_listening_main() {
    printf("Terminal thread running\n");
    pthread_mutex_lock(&mtx_main);
    bool quit = shared_data_main.quit;
    bool start = shared_data_main.start;
    pthread_mutex_unlock(&mtx_main);
    unsigned char c;
    int r;
    while (!quit && !start) {
        r = keyboard_getc_timeout(&c);
        if (r > 0) {  // something has been read
            switch (c) {
                case 'w':
                    pthread_mutex_lock(&mtx_main);
                    shared_data_main.move = -1;  // move up in the menu
                    pthread_mutex_unlock(&mtx_main);
                    break;
                case 's':
                    pthread_mutex_lock(&mtx_main);
                    shared_data_main.move = 1;  // move down in the menu
                    pthread_mutex_unlock(&mtx_main);
                    break;
                case 'd':
                    pthread_mutex_lock(&mtx_main);
                    shared_data_main.start = true;
                    pthread_mutex_unlock(&mtx_main);
                    break;
                case 'q':
                    quit = true;
                    break;
            }
        } else if (r < 0) {  // error
            fprintf(stderr, "ERROR: Keyboard reading error");
            quit = true;
        }

        pthread_mutex_lock(&mtx_main);
        if (quit)
            shared_data_main.quit = quit;
        else
            quit = shared_data_main.quit;
        start = shared_data_main.start;
        pthread_cond_broadcast(&condvar);
        pthread_mutex_unlock(&mtx_main);
    }
    printf("Terminal thread exiting\n");
    return EXIT_SUCCESS;
}

void *knobs_listening_main() {
    printf("Knobs thread running\n");
    pthread_mutex_lock(&mtx_main);
    bool quit = shared_data_main.quit;
    bool start = shared_data_main.start;
    int move_dir;
    knobs_data kd;

    pthread_mutex_unlock(&mtx_main);
    while (!quit && !start) {  // get input from knobs
        kd = get_rel_knob_value();

        if (kd.gk >= 1)
            move_dir = 1;
        else if (kd.gk <= -1)
            move_dir = -1;
        else
            move_dir = 0;

        if (move_dir) {
            pthread_mutex_lock(&mtx_main);
            shared_data_main.move = move_dir;
            pthread_mutex_unlock(&mtx_main);
        }
        if (kd.gb) {
            pthread_mutex_lock(&mtx_main);
            shared_data_main.start = true;
            pthread_mutex_unlock(&mtx_main);
        }

        usleep(DEBOUNCE_GREEN_KNOB);

        pthread_mutex_lock(&mtx_main);
        if (quit)
            shared_data_main.quit = quit;
        else
            quit = shared_data_main.quit;
        if (start)
            shared_data_main.start = start;
        else
            start = shared_data_main.start;
        pthread_cond_broadcast(&condvar);
        pthread_mutex_unlock(&mtx_main);
    }
    printf("Knobs thread exiting\n");
    return EXIT_SUCCESS;
}

void call_stty(int reset) {
    if (reset) {
        system("stty -raw opost echo");
    } else {
        system("stty raw opost -echo");
    }
}
