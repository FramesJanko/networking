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

uint64_t get_current_time() {
#ifdef _WIN32
  FILETIME ft;
  ULARGE_INTEGER large_int;
  __int64 time64, next_time64;
  GET_TIME(ft);
  large_int.LowPart = ft.dwLowDateTime;
  large_int.HighPart = ft.dwHighDateTime;
  time64 = large_int.QuadPart;
  time64 /= 10;
  return time64;
#else
  struct timeval time;
  GET_TIME(time);
  uint64_t microseconds = (time.tv_sec * 1000000ULL) + time.tv_usec;
  return microseconds;
#endif
}

/* int main() { */
/*   uint64_t the_time, the_time2; */
/*   the_time = get_current_time(); */
/* #ifdef _WIN32 */
/*   Sleep(100); */
/* #else */
/*   usleep(100000); */
/* #endif */
/*   the_time2 = get_current_time(); */
/*   printf("The time is %lu\n", the_time2 - the_time); */
/* } */
