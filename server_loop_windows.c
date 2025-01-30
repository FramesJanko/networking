#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#define TICK_RATE 50
#define MAX_PLAYERS 12

/* double get_current_time() { */
/*   struct timeval time; */
/*   gettimeofday(&time, NULL); */
/*   return time.tv_sec + (time.tv_usec / 1000000.0); */
/* } */

int main() {
  /* double time; */
  /* time = get_current_time(); */
  /* printf("The time is %lf\n", time); */
  FILETIME ft;
  ULARGE_INTEGER large_int;
  __int64 time, time2;

  GetSystemTimePreciseAsFileTime(&ft);

  large_int.LowPart = ft.dwLowDateTime;
  large_int.HighPart = ft.dwHighDateTime;
  time = large_int.QuadPart;

  time /= 10;
  Sleep(1000);
  GetSystemTimePreciseAsFileTime(&ft);

  large_int.LowPart = ft.dwLowDateTime;
  large_int.HighPart = ft.dwHighDateTime;
  time2 = large_int.QuadPart;

  time2 /= 10;
  printf("The time in microseconds is %lld\n", time2 - time);
}
