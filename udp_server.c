#include "chap03.h"
#include "time_utils.h"
#include "game_structs.h"
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// void printBinaryRepresentation(void *data, size_t size) {
//     for (int i = size - 1; i >= 0; i--) {
//         for (int j = 7; j >= 0; j--) {
//             // Access each byte, then each bit within that byte
//             unsigned char byte = ((unsigned char*)data)[i];
//             printf("%c", (byte & (1 << j)) ? '1' : '0');
//         }
//         printf(" "); // Separate bytes for readability
//     }
//     printf("\n");
// }

// int intin(SOCKET num, SOCKET* list, size_t list_size){
//     for (size_t i = 0; i < list_size; i++){
//         if (num == list[i])
//             return i+1;
//     }
//     return 0;
// }

typedef struct Lobby{
  int host_id;
  char lobby_name[16];
  struct sockaddr_storage client_list[4];
  socklen_t client_len[4];
  int client_count;
} Lobby;

void HandleNewHost(struct sockaddr_storage (*_sockaddr_host_list),
                   socklen_t (*_socklen_host_list),
                   struct sockaddr_storage _host_address,
                   socklen_t _host_len, 
                   int* _host_count,
                   char* _lobby_name, 
                   int _lobby_name_size, 
                   char (*_lobby_names)[16],
                   Lobby* _lobby_list)
{
  char temp_lobby_name[16] = {0};
  memcpy(temp_lobby_name, _lobby_name, _lobby_name_size);
  temp_lobby_name[15] = '\0';
  printf("Temp lobby name: %s\n", temp_lobby_name);
  Lobby lobby = {*_host_count};
  memcpy(&lobby.lobby_name, temp_lobby_name, 16);
  printf("Lobby name: %s\n", lobby.lobby_name);
  _lobby_list[*_host_count] = lobby;
  printf("Stored Lobby name: %s\n", _lobby_list->lobby_name);
  _sockaddr_host_list[*_host_count] = _host_address;
  _socklen_host_list[*_host_count] = _host_len;
  for (int x = 0; x < _lobby_name_size; x++)
  {
    _lobby_names[*_host_count][x] = _lobby_name[x];
  }
  *_host_count = *_host_count+1;

}

int HandleNewClient(struct sockaddr_storage (*_sockaddr_client_list)[3],
                   socklen_t (*_socklen_client_list)[3],
                   struct sockaddr_storage _client_address,
                   socklen_t _client_len, 
                   int host,
                   int clientSpot,
                   Lobby* _lobby_list)
{
  if(host > 3 || clientSpot > 3)
    return -1;
  struct sockaddr_storage zero_address = {0};
  if(memcmp(&_lobby_list->client_list[clientSpot], &zero_address, sizeof(zero_address)) == 0){
    _lobby_list->client_list[clientSpot] = _client_address;
    _lobby_list->client_len[clientSpot] = _client_len;
    _lobby_list->client_count += 1;
    return 1;
  }
  return 0;
}
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
  int maxHosts = 4;
  Lobby lobby_list[4] = {0};
  struct sockaddr_storage sockaddr_host_list[4] = {0};
  struct sockaddr_storage sockaddr_client_list[4][3] = {0};
  socklen_t socklen_host_list[4] = {0};
  socklen_t socklen_client_list[4][3] = {0};
  int connectedClients[4][3] = {0};
  int serverIndex;
  int hostCount = 0;
  char lobbyNames[4][16] = {0};
  
    
  while (game_running) {
    uint64_t current_time = get_current_time();
    if (current_time - last_time >= tick_interval) {
      fd_set reads;
      reads = master;
      if (select(max_socket + 1, &reads, 0, 0, &timeout) < 0) {
        fprintf(stderr, "select() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
      } //end select

      if(FD_ISSET(socket_listen, &reads)){
        struct sockaddr_storage client_address;
        socklen_t client_len = sizeof(client_address);
        char read[1024];
        int bytes_received = recvfrom(socket_listen, read, 1024, 0, (struct sockaddr *)&client_address, &client_len);
        if(bytes_received < 1){
          fprintf(stderr, "Connection closed (%d)\n", GETSOCKETERRNO());
          return 1;
        } //end bytes received
        int code = 0;
        memcpy(&code, read, 2);
        switch (read[0]){
          //New Connection
          case 1: 
            switch (read[1]){
              //client connecting to lobby
              case 0:
              {
                int host = read[2]; //host bit is 3rd bit, the lobby they want to join
                int spot = lobby_list[host].client_count; //spot bit is 4th bit, spot in the lobby they want to join
                if(HandleNewClient(sockaddr_client_list, socklen_client_list, client_address, client_len, host, spot, &lobby_list[host]) > 0){
                  char send_buffer[20];
                  send_buffer[0] = 0;
                  send_buffer[1] = 0;
                  send_buffer[2] = host;
                  send_buffer[3] = spot;
                  printf("Connecting to host [%d] in spot [%d]\n", send_buffer[2], send_buffer[3]);
                  memcpy(&send_buffer[4], lobby_list[host].lobby_name, 16);
                  printf("Sending\n");
                  sendto(socket_listen, send_buffer, sizeof(send_buffer), 0, (struct sockaddr *)&lobby_list[host].client_list[spot], lobby_list[host].client_len[spot]);
                }
              }
                break;
              // Host is connecting
              case 1:
              {
                if(hostCount >= maxHosts)
                  break;
                int host = hostCount;
                HandleNewHost(sockaddr_host_list, socklen_host_list, client_address, client_len, &hostCount, read+2, bytes_received-2, lobbyNames, lobby_list);
                //Send Lobby Name back as an acknowledge
                char send_buffer[19];
                send_buffer[0] = 0;
                send_buffer[1] = 0;
                send_buffer[2] = host;
                memcpy(&send_buffer[3], lobby_list[host].lobby_name, 16);
                printf("Sending\n");
                sendto(socket_listen, send_buffer, sizeof(send_buffer), 0, (struct sockaddr *)&sockaddr_host_list[host], socklen_host_list[host]);
              }
                break;
              //Client requesting lobby list
              case 2:
              {
                char send_buffer[66];
                send_buffer[0] = 0;
                send_buffer[1] = 2;
                for(int i = 0; i < 4; i++){
                  memcpy(&send_buffer[(i * 16) + 2], &lobbyNames[i], 16);
                }
                send_buffer[65] = '\0';
                for (int i = 0; i < 4; i++)
                {
                  printf("Lobby name: %s\n", &send_buffer[(i*16)+2]);
                }
                sendto(socket_listen, send_buffer, sizeof(send_buffer), 0, (struct sockaddr *)&client_address, client_len);
                // printf("Send buffer: %s\n", send_buffer);
              }
                break;
              default:
                break;
            }
            break;
          case 2:
          {
            int host = read[1];
            Lobby lobby = lobby_list[host];
            int num_clients = lobby.client_count;
            for(int i = 0; i < num_clients; i++){
              char address_name[46];
              char port_name[46];
              getnameinfo((struct sockaddr *)&lobby.client_list[i], lobby.client_len[i], address_name, sizeof(address_name), port_name, sizeof(port_name), NI_NUMERICHOST | NI_NUMERICSERV);
              // printf("Sending to %s on port %s\n", address_name, port_name);
              sendto(socket_listen, read, bytes_received, 0, (struct sockaddr *)&lobby.client_list[i], lobby.client_len[i]);
            }
          }
            break;
          case 3:
          {
            int host = read[1];
            char address_name[46];
            char port_name[46];
            getnameinfo((struct sockaddr *)&sockaddr_host_list[host], socklen_host_list[host], address_name, sizeof(address_name), port_name, sizeof(port_name), NI_NUMERICHOST | NI_NUMERICSERV);
            // printf("Sending to %s on port %s\n", address_name, port_name);
            sendto(socket_listen, read, bytes_received, 0, (struct sockaddr *)&sockaddr_host_list[host], socklen_host_list[host]);
          }
            break;
          default:
            break;
        }
      } //end FD_ISSET
      first_run = 0;
      last_time += tick_interval;
      current_tick++;
    } //end tick interval
  } //end while
  printf("Closing listening socket...\n");
  CLOSESOCKET(socket_listen);

  #if defined(_WIN32)
  WSACleanup();
  #endif

  printf("Finished.\n");

  return 0;
} // main

