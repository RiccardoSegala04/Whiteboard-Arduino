#ifndef SERIAL_H
#define SERIAL_H

#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/queue.h>

#define SERIAL_CMD_STR_LEN 64

extern pthread_t __sthread;
extern pthread_mutex_t __slock;
extern char __cmdbuf[SERIAL_CMD_STR_LEN];
extern struct serial_head_t __scmd;

struct serial_node_t {
    char cmd_str[SERIAL_CMD_STR_LEN];
    STAILQ_ENTRY(serial_node_t) next;
};

STAILQ_HEAD(serial_head_t, serial_node_t);

int serial_start(const char* serial_path);
int serial_stop();

int serial_pop(char* dst);

#endif
