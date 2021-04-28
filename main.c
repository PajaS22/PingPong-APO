#define _POSIX_C_SOURCE 200112L

#include "headers.h"
#include "LCD_output.h"
#include "game.h"

void *terminal_listening_main();
void *knobs_listening_main();


pthread_cond_t condvar;
pthread_mutex_t mtx;

struct shared {
    bool quit;
    bool start;
    int move;
} shared_data_main;

int main(int argc, char *argv[])
{
    if (display_init()) {
        unsigned short *frame_buff = (unsigned short *)malloc(DISPLAY_HEIGHT * DISPLAY_WIDTH *
                                              sizeof(unsigned short));
        if (frame_buff == NULL) {
            fprintf(stderr, "ERROR: Memory could not be allocated!\n");
        } else {
            call_stty(0);

            // threads
            shared_data_main.quit = false;
            shared_data_main.start = false;
            shared_data_main.move = 0;
            bool quit = false;

            pthread_mutex_init(&mtx, NULL);
            pthread_cond_init(&condvar, NULL);

            int num_of_threads = 2;
            pthread_t thrs[num_of_threads];
            pthread_create(&thrs[0], NULL, terminal_listening_main, NULL);
            pthread_create(&thrs[1], NULL, knobs_listening_main, NULL);

            int move = 0;
            int selected = 0;
            bool start = false;
            printf("printing menu\n");
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
                    pthread_mutex_lock(&mtx);
                    shared_data_main.move = 0;
                    pthread_mutex_unlock(&mtx);

                    print_menu(MENU_OFFSET_X, MENU_OFFSET_Y, selected, frame_buff);
                    update_display(frame_buff);
                }
                if (start) {
                    for (int i = 0; i < num_of_threads; ++i) {
                        pthread_join(thrs[i], NULL);
                    }
                    printf("All threads joined, starting game\n");
                    if(selected + 1 == MENU_SELECTION){
                        printf("exit selected\n");
                        // exit selected
                        pthread_mutex_lock(&mtx);
                        shared_data_main.quit = true;
                        pthread_mutex_unlock(&mtx);
                    }
                    else{
                        start_game(frame_buff, selected);

                        //game ended
                        start = false;
                        pthread_mutex_lock(&mtx);
                        shared_data_main.start = false;
                        pthread_mutex_unlock(&mtx);
                        pthread_create(&thrs[0], NULL, terminal_listening_main, NULL);
                        pthread_create(&thrs[1], NULL, knobs_listening_main, NULL);

                        print_menu(MENU_OFFSET_X, MENU_OFFSET_Y, selected, frame_buff);
                        update_display(frame_buff);
                    }
                }
                pthread_mutex_lock(&mtx);
                quit = shared_data_main.quit;
                if(!start && !quit)
                    pthread_cond_wait(&condvar, &mtx);
                quit = shared_data_main.quit;
                start = shared_data_main.start;
                move = shared_data_main.move;
                pthread_mutex_unlock(&mtx);
            }
            fprintf(stderr, "Main finished!\n");
            goodbye(frame_buff);
            if(!start){
                for (int i = 0; i < num_of_threads; ++i) {
                    pthread_join(thrs[i], NULL);
                }
            }
            fprintf(stderr, "All threads finished!\n");
            call_stty(1);
        }
    }
    return EXIT_SUCCESS;
}

void *terminal_listening_main() {
    printf("Terminal thread running\n");
    pthread_mutex_lock(&mtx);
    bool quit = shared_data_main.quit;
    bool start = shared_data_main.start;
    pthread_mutex_unlock(&mtx);
    char c;
    while (!quit && !start) {
        c = getchar();
        switch (c) {
        case 'w':
            pthread_mutex_lock(&mtx);
            shared_data_main.move = -1; // move up in the menu
            pthread_mutex_unlock(&mtx);
            break;
        case 's':
            pthread_mutex_lock(&mtx);
            shared_data_main.move = 1; // move down in the menu
            pthread_mutex_unlock(&mtx);
            break;
        case 'd':
            pthread_mutex_lock(&mtx);
            shared_data_main.start = true;
            pthread_mutex_unlock(&mtx);
            break;
        case 'q':
            quit = true;
            break;
        }
        pthread_mutex_lock(&mtx);
        if (quit)
            shared_data_main.quit = quit;
        else
            quit = shared_data_main.quit;
        start = shared_data_main.start;
        pthread_cond_broadcast(&condvar);
        pthread_mutex_unlock(&mtx);
    }
    printf("Terminal thread exiting\n");
    return EXIT_SUCCESS;
}

void *knobs_listening_main() {
    printf("Knobs thread running\n");
    pthread_mutex_lock(&mtx);
    bool quit = shared_data_main.quit;
    bool start = shared_data_main.start;
    pthread_mutex_unlock(&mtx);
    while (!quit && !start) { // get input from knobs
        pthread_mutex_lock(&mtx);
        if (quit)
            shared_data_main.quit = quit;
        else
            quit = shared_data_main.quit;
        if (start)
            shared_data_main.start = start;
        else
            start = shared_data_main.start;
        // pthread_cond_broadcast(&condvar);
        pthread_mutex_unlock(&mtx);
    }
    printf("Knobs thread exiting\n");
    return EXIT_SUCCESS;
}

void call_stty(int reset)
{
    if (reset) {
        system("stty -raw opost echo");
    } else {
        system("stty raw opost -echo");
    }
}