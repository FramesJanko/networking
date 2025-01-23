#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#define TICK_RATE 50
#define MAX_PLAYERS 12

double get_current_time(){
    struct timeval time;
    gettimeofday(&time, NULL);
    return time.tv_sec + (time.tv_usec/1000000.0);
}

