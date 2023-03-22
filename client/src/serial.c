#include <serial.h>
#include <string.h>
#include <termios.h>
#include <fcntl.h>
#include <SDL2/SDL.h>
#include <signal.h>

pthread_t __sthread;
pthread_mutex_t __slock;
char __cmdbuf[64];
struct serial_head_t __scmd;

void serial_sig(int sig) {
    pthread_exit((void*)0);
}

static int serial_open_fd(const char* pathname) {
    int fd = open(pathname, O_RDWR | O_NOCTTY);

    if(fd<0) {
        perror("Could not open serial port");
        exit(1);
    }

    struct termios settings;

    cfmakeraw(&settings);
    settings.c_cflag |= (CLOCAL | CREAD);
    settings.c_iflag &= ~(IXOFF | IXANY);

    settings.c_cc[VMIN] = 0; 
    settings.c_cc[VTIME] = 5;

    cfsetispeed(&settings, B9600);
    cfsetospeed(&settings, B9600);

    tcsetattr(fd, TCSANOW, &settings);

    return fd;
}

void *serial_exec(void* serial_path) {

    signal(SIGINT, serial_sig);

    uint32_t serial_event = SDL_RegisterEvents(1);
    SDL_Event event;

    memset(&event, 0, sizeof(event));

    int fd = serial_open_fd((char*) serial_path);
    size_t cmd_idx = 0;

    while(1) {
        
        struct serial_node_t *node = malloc(
            sizeof(struct serial_node_t)
        );

        memset(node, 0, sizeof(struct serial_node_t));

        int nbytes;
        do {
            nbytes = read(fd, node->cmd_str+cmd_idx, 1);
            cmd_idx+=nbytes;
        } while(node->cmd_str[cmd_idx-nbytes]!='\n');

        if(cmd_idx>0) {

            node->cmd_str[cmd_idx-1]=0;

            pthread_mutex_lock(&__slock);
            STAILQ_INSERT_TAIL(&__scmd, node, next);
            pthread_mutex_unlock(&__slock);


            cmd_idx = 0;
            
            event.type = serial_event;
            SDL_PushEvent(&event);
        }
    }
    return NULL;
}

int serial_start(const char* serial_path) {
    STAILQ_INIT(&__scmd);
    pthread_mutex_init(&__slock, NULL);
    return pthread_create(&__sthread, NULL, serial_exec, (void*)serial_path);
}

int serial_stop() {
    STAILQ_INIT(&__scmd);
    pthread_mutex_destroy(&__slock);
    pthread_kill(__sthread, SIGINT);
    return pthread_join(__sthread, NULL);
}

int serial_pop(char* dst) {
    pthread_mutex_lock(&__slock);
    if(STAILQ_EMPTY(&__scmd)) {
        pthread_mutex_unlock(&__slock);
        return 1;
    }
    struct serial_node_t* node = STAILQ_FIRST(&__scmd);
    memcpy(dst, node->cmd_str, SERIAL_CMD_STR_LEN);
    STAILQ_REMOVE_HEAD(&__scmd, next);
    pthread_mutex_unlock(&__slock);
    
    free(node);

    return 0;
}
