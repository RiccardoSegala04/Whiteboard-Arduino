#include <stdio.h>
#include <whiteboard.h>
#include <serial.h>
#include <signal.h>

#define MAX_DRAW 5
#define FPS 60
#define FRAME_DELAY 1000/FPS

struct options_t {
    int scale;
    char path[64];
};

struct whiteboard_t board;

void handle_sigint(int sig) {
    serial_stop();
    whiteboard_destroy(&board);
    exit(0);
}

int handle_whiteboard_command(char* cmd, struct whiteboard_t *board);
int get_cli_options(struct options_t* opts, int argc, char* argv[]);
int print_help();

int main(int argc, char* argv[]) {

    struct options_t opts;
    memset(&opts, 0, sizeof(opts));
    opts.scale = 2;

    int rc;
    if((rc=get_cli_options(&opts, argc, argv))<0) {
        return rc;
    }

    if(opts.path[0]==0) {
        printf("You need to specify a path to a serial interface\n");
        print_help();
        return -1;
    }

    signal(SIGINT, handle_sigint);

    if(whiteboard_init(&board, opts.scale)) {
        printf("SDL Error: %s\n", SDL_GetError());
        exit(1);
    }

    whiteboard_set_color(&board, 0xF800);

    serial_start(opts.path);

    int running = 1;

    while(running) {

        int f_start = SDL_GetTicks();
        
        SDL_Event event;
        while(SDL_PollEvent(&event)) {
            if(event.type==SDL_QUIT) {
                running=0;
            }
        }

        char buf[SERIAL_CMD_STR_LEN];
        for(int i=0; i<MAX_DRAW && !serial_pop(buf); i++) {
            handle_whiteboard_command(buf, &board);
        }

        whiteboard_display(&board);

        int f_time = SDL_GetTicks() - f_start;
        if(FRAME_DELAY>f_time) {
            SDL_Delay(FRAME_DELAY-f_time);
        }

    }

    serial_stop();

    whiteboard_destroy(&board);
    
    return 0;
}

int handle_whiteboard_command(char* cmd, struct whiteboard_t *board) {

    char* token = strtok(cmd, " ");

    if(!token)
        return -1;

    if(!strcmp(token, "point")) {

        token = strtok(NULL, " ");

        if(!token)
            return -1;
        
        char* tmp;
        int x = strtol(token, &tmp, 10);

        if(*tmp)
            return -1;

        token = strtok(NULL, " ");

        int y = strtol(token, &tmp, 10);

        if(*tmp)
            return -1;

        whiteboard_point(board, x, y);
        
    } else if(!strcmp(token, "line")) {
        token = strtok(NULL, " ");

        if(!token)
            return -1;
        
        char* tmp;
        int x = strtol(token, &tmp, 10);

        if(*tmp)
            return -1;

        token = strtok(NULL, " ");

        int y = strtol(token, &tmp, 10);

        if(*tmp)
            return -1;
        
        whiteboard_line_to(board, x, y);
        
    } else if(!strcmp(token, "color")) {
        token = strtok(NULL, " ");
        
        whiteboard_set_color(
                board, strtol(token, NULL, 16)
        );


    } else if(!strcmp(token, "size")) {
        token = strtok(NULL, " ");
        
        if(!token)
            return -1;

        int radius = strtol(token, NULL, 10);
        if(radius==0)
            return -1;

        whiteboard_set_size(board, radius);

    } else if(!strcmp(token, "clear")) {

        whiteboard_clear(board);
        
    } else {
        return -1;
    }


    return 0;
}

int print_help() {
    printf("Whiteboard:\n");
    printf("    Use -i <path> to specify the path to the serial interface\n");
    printf("    Use -s <scale> to specify the scale of the window (default is 2)");
}

int get_cli_options(struct options_t* opts, int argc, char* argv[]) {
    
    int option;
    
    while((option = getopt(argc, argv, ":i:s:h")) != -1) {
        switch(option) {
            case 'i':
                if(optarg!=NULL)
                    strcat(opts->path, optarg);
                else {
                    printf("Option -i needs an argument!\n");
                    return -1;
                }
                break;
            case 's':
                if(optarg!=NULL)
                    opts->scale = strtol(optarg, NULL, 10);
                else {
                    printf("Option -s needs an argument!\n");
                    return -1;
                }
                break;
            case 'h':
                print_help();
                break;
            case ':':
                return -1;
            default:
                printf("Invalid option.\nTry wb -h\n");
                return -1;
        }
    }

    return 0;
}
