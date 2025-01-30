#ifndef TIME_UTILS_H
#define TIME_UTILS_H
#if defined(_WIN32)
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif
#include <windows.h>
#else
#include <sys/time.h>
#include <unistd.h>
#endif
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define TICK_RATE 50
#define MAX_PLAYERS 12

#if defined(_WIN32)
#define GET_TIME(time_snapshot) GetSystemTimePreciseAsFileTime(&time_snapshot)
#else
#define GET_TIME(time_snapshot) gettimeofday(&time_snapshot, NULL)
#endif

uint64_t get_current_time();
#endif
