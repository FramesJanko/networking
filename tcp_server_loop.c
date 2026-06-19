/*
 * MIT License
 *
 * Copyright (c) 2018 Lewis Van Winkle
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "chap03.h"
#include "time_utils.h"
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  char name[20];
  int age;
} Data;

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
  hints.ai_socktype = SOCK_STREAM;
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

  // listen for connections
  printf("Listening...\n");
  if (listen(socket_listen, 10) < 0) {
    fprintf(stderr, "listen() failed. (%d)\n", GETSOCKETERRNO());
    return 1;
  }

  // Create a list and put youself (server socket) into it
  struct timeval timeout;
  timeout.tv_sec = 0;
  timeout.tv_usec = 10;
  fd_set master;
  FD_ZERO(&master);
  FD_SET(socket_listen, &master);
  SOCKET max_socket = socket_listen;

  printf("Waiting for connections...\n");

  // use tick rate to synch communication

  uint64_t tick_interval = 1000000 / TICK_RATE;
  int game_running = 1;
  uint64_t last_time = get_current_time();
  int count = 0;
  int first_run = 1;
  char data[4096] = "original";
  char last_data[4096] = "original";
  int current_tick = 0;

  while (game_running) {
    uint64_t current_time = get_current_time();
    if (current_time - last_time >= tick_interval) {
      // DO THE SERVER STUFF EVERY TICK
      //
      // use select to find readable data (client input)
      fd_set reads;
      char read[4096];
      char *message;
      reads = master;
      if (select(max_socket + 1, &reads, 0, 0, &timeout) < 0) {
        fprintf(stderr, "select() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
      }

      // reply to each readable data (accept connection or
      SOCKET i;
      for (i = 1; i <= max_socket; ++i) {
        if (FD_ISSET(i, &reads)) {

          if (i == socket_listen) {
            struct sockaddr_storage client_address;
            socklen_t client_len = sizeof(client_address);
            SOCKET socket_client = accept(
                socket_listen, (struct sockaddr *)&client_address, &client_len);
            if (!ISVALIDSOCKET(socket_client)) {
              fprintf(stderr, "accept() failed. (%d)\n", GETSOCKETERRNO());
              return 1;
            }

            int optval;
            socklen_t optlen = sizeof(optval);
            getsockopt(socket_client, SOL_SOCKET, SO_ERROR, (char *)&optval, &optlen);
            if (optval != 0) {
                printf("Client socket error: %d\n", optval);
            }

            FD_SET(socket_client, &master);
            if (socket_client > max_socket)
              max_socket = socket_client;

            char address_buffer[100];
            char spawn_message[32];
            memset(spawn_message, 0, sizeof(spawn_message));
            spawn_message[0] = current_tick % 256;
            spawn_message[1] = socket_client;
            spawn_message[2] = 0x01;
            spawn_message[3] = 0x01;
            spawn_message[4] = 0x01;
            spawn_message[5] = 0xFF;
            getnameinfo((struct sockaddr *)&client_address, client_len,
                        address_buffer, sizeof(address_buffer), 0, 0,
                        NI_NUMERICHOST);
            printf("New connection from %s\n", address_buffer);

            /* struct timeval timeout; */
            /* timeout.tv_sec = 2;  // Wait up to 2 seconds */
            /* timeout.tv_usec = 0; */

            /* fd_set read_fds; */
            /* FD_ZERO(&read_fds); */
            /* FD_SET(socket_client, &read_fds); */
            /* printf("waiting...\n"); */
            /* printf("waiting...\n"); */
            /* printf("waiting...\n"); */

            /* if (!ISVALIDSOCKET(socket_client)) { */
            /*     fprintf(stderr, "accept() failed. (%d)\n", GETSOCKETERRNO()); */
            /*     return 1; */
            /* } */
            /* printf("Client connected! Socket: %d\n", socket_client); */
            /* int ready = select(socket_client + 1, &read_fds, 0, NULL, &timeout); */
            /* if (ready > 0) { */
            /*     printf("ready\n"); */
            /*     if (FD_ISSET(socket_client, &read_fds)){ */
            /*         char buf[4096]; */
            /*         int bytes_read = recv(i, buf, 4096, 0); */

            /*         if (bytes_read == -1) { */
            /*             int error_code = GETSOCKETERRNO(); // On Windows, use WSAGetLastError() */
            /*             printf("recv() failed on socket %d, error code: %d\n", socket_client, error_code); */

            /*             if (error_code == EWOULDBLOCK || error_code == EAGAIN) { */
            /*                 printf("Non-blocking socket: No data available yet, try again later.\n"); */
            /*             } else if (error_code == ECONNRESET) { */
            /*                 printf("Client forcibly closed the connection.\n"); */
            /*                 CLOSESOCKET(socket_client); */
            /*                 FD_CLR(socket_client, &read_fds); */
            /*             } else { */
            /*                 printf("Unexpected recv() error, closing socket.\n"); */
            /*                 CLOSESOCKET(socket_client); */
            /*                 FD_CLR(socket_client, &read_fds); */
            /*             } */
            /*         } */

            /*         int message_size = send(i, spawn_message, sizeof(spawn_message), 0); */
            /*         printf("Received %d bytes: ", bytes_read); */
            /*         for (int character = 0; character < bytes_read; character++){ */
            /*             printf("%x ", buf[character]); */
            /*         } */
            /*         printf("\n"); */
            /*         printf("sending welcome message of size %d\n", message_size); */
            /*         printf("content: %u (hex: %x)\n", (unsigned char)spawn_message[0], (unsigned char)spawn_message[0]); */

            /*         if (message_size == -1) { */
            /*             int error_code = GETSOCKETERRNO(); */
            /*             printf("send() failed on socket %d, error code: %d\n", i, error_code); */

            /*             if (error_code == ECONNRESET) { */
            /*                 printf("Client %d disconnected before data was sent.\n", i); */
            /*             } else if (error_code == EWOULDBLOCK || error_code == EAGAIN) { */
            /*                 printf("Socket %d send() would block. Retrying...\n", i); */
            /*             } */
            /*         } */
                    /* while(message_size < 0){ */
                    /*     message_size = send(i, spawn_message, sizeof(spawn_message), 0); */
                    /*     printf("sending welcome message of size %d\n", message_size); */
                    /*     printf("content: %u (hex: %x)\n", (unsigned char)spawn_message[0], (unsigned char)spawn_message[0]); */
                    /* } */
            } else {
              int bytes_received = recv(i, read, 4096, 0);
              printf("receiving\n");
              if (bytes_received > 0) {
                printf("received %d bytes\n", bytes_received);
                switch (read[0]) {
                  case 0x01:
                    read[bytes_received] = '\0';
                    message = malloc(sizeof(char) * 4096);
                    strcpy(message, (read + 1));
                    printf("%s\n", message);
                    strcpy(data, message);
                    free(message);
                    break;
                  case 0x00:
                    printf("acknowledge from %d\n", i);
                    break;
                }
              }
              if (bytes_received < 1) {
                FD_CLR(i, &master);
                CLOSESOCKET(i);
                continue;
              }
            }
          } 

        }

      /* for (int i = 0; i <= max_socket; i++) { */
      /*   /1* char *data_to_send = malloc(sizeof(char) * strlen(read) + 2); *1/ */
      /*   /1* strcpy(data_to_send, read); *1/ */
      /*   /1* strcat(data_to_send, "\n"); *1/ */
      /*   /1* if (strcmp(data, last_data) == 0) { *1/ */
      /*   /1*   /2* printf("comparing: %s %s\n", data, last_data); *2/ *1/ */
      /*   /1*   continue; *1/ */
      /*   /1* } *1/ */
      /*   if (i != socket_listen) { */
      /*     send(i, data, strlen(data), 0); */
      /*   } */
      /*   /1* free(data_to_send); *1/ */
      /* } */
      /* strcpy(last_data, data); */
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
