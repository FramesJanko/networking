#include "chap03.h"
#include "time_utils.h"
#include "game_structs.h"
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {

#if defined(_WIN32)
  WSADATA d;
  if (WSAStartup(MAKEWORD(2, 2), &d)) {
    fprintf(stderr, "Failed to initialize.\n");
    return 1;
  }
#endif

  printf("Configuring local address...\n");
  struct addrinfo hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags = AI_PASSIVE;

  struct addrinfo *bind_address;
  getaddrinfo("0.0.0.0", "12555", &hints, &bind_address);

  printf("Creating socket...\n");
  SOCKET socket_listen;
  socket_listen = socket(bind_address->ai_family, bind_address->ai_socktype,
                         bind_address->ai_protocol);
  if (!ISVALIDSOCKET(socket_listen)) {
    fprintf(stderr, "socket() failed. (%d)\n", GETSOCKETERRNO());
    return 1;
  }

  printf("Binding socket to local address...\n");
  if (bind(socket_listen, bind_address->ai_addr, bind_address->ai_addrlen)) {
    fprintf(stderr, "bind() failed. (%d)\n", GETSOCKETERRNO());
    return 1;
  }
  freeaddrinfo(bind_address);

  struct timeval timeout;
  timeout.tv_sec = 0;
  timeout.tv_usec = 10;
  fd_set master;
  FD_ZERO(&master);
  FD_SET(socket_listen, &master);
  SOCKET max_socket = socket_listen;

  printf("Waiting for connections...\n");

  uint64_t tick_interval = 1000000 / TICK_RATE;
  int game_running = 1;
  uint64_t last_time = get_current_time();
  int connection_count = 0;
  int first_run = 1;
  int spawn_count = 0;
  int current_tick = 0;
  struct sockaddr_storage sockaddr_list[16];
  socklen_t socklen_list[16];

  while (game_running) {
    uint64_t current_time = get_current_time();
    if (current_time - last_time >= tick_interval) {
      fd_set reads;
      reads = master;
      if (select(max_socket + 1, &reads, 0, 0, &timeout) < 0) {
        fprintf(stderr, "select() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
      }

      if(FD_ISSET(socket_listen, &reads)){

        struct sockaddr_storage client_address;
        socklen_t client_len = sizeof(client_address);
        char read[1024];
        int bytes_received = recvfrom(socket_listen, read, 1024, 0, (struct sockaddr *)&client_address, &client_len);
        if(bytes_received < 1){
          fprintf(stderr, "Connection closed (%d)\n", GETSOCKETERRNO());
          return 1;
        }

        sockaddr_list[connection_count] = client_address;
        socklen_list[connection_count] = client_len;
        connection_count++;
        for (int i = 0; i < connection_count; i++)
        {
          char addressbuffer[100];
          char portbuffer[100];
          printf("Client number %d: ", i);
          getnameinfo((struct sockaddr*)&sockaddr_list[i],socklen_list[i],addressbuffer, sizeof(addressbuffer), portbuffer, sizeof(portbuffer), NI_DGRAM | NI_NUMERICHOST);
          printf("%s %s\n", addressbuffer, portbuffer);
        }

      }

      first_run = 0;
      last_time += tick_interval;
      current_tick++;
    }
  }
  printf("Closing listening socket...\n");
  CLOSESOCKET(socket_listen);
#if defined(_WIN32)
  WSACleanup();
#endif
  printf("Finished.\n");
  return 0;
}
