#include "chap03.h"
#include "time_utils.h"
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    float x_pos;
    float y_pos;
    float z_pos;
} UnitPosition;
typedef struct {
    int id;
    int ownership;
    char unitType;
    int totalHealth;
    float movespeed;
    UnitPosition position;
} Unit;
typedef struct {
    int player_id;
    int unit_count;
    Unit units[32];
} PlayerUnits;
typedef struct {
    int response;
    UnitPosition location;
    Unit unit;
} SpawnResponse;
typedef struct {
    int numConnectedPlayers;
    //16 bits to track if the 16 players have connected yet. 1 is yes, 0 is no.
    uint16_t connectedPlayers;
    //16 bits to track if the 16 players have spawned yet. 1 is yes, 0 is no.
    uint16_t spawnedPlayers;
    UnitPosition player1spawn;
    UnitPosition player2spawn;
    UnitPosition player3spawn;
    UnitPosition player4spawn;
    UnitPosition player5spawn;
    UnitPosition player6spawn;
    UnitPosition player7spawn;
    UnitPosition player8spawn;
    UnitPosition player9spawn;
    UnitPosition player10spawn;
    UnitPosition player11spawn;
    UnitPosition player12spawn;
    UnitPosition player13spawn;
    UnitPosition player14spawn;
    UnitPosition player15spawn;
    UnitPosition player16spawn;
} Data;


void printBinaryRepresentation(void *data, size_t size) {
    for (int i = size - 1; i >= 0; i--) {
        for (int j = 7; j >= 0; j--) {
            // Access each byte, then each bit within that byte
            unsigned char byte = ((unsigned char*)data)[i];
            printf("%c", (byte & (1 << j)) ? '1' : '0');
        }
        printf(" "); // Separate bytes for readability
    }
    printf("\n");
}

char* configure_network_data(uint16_t send_to, int signifier_byte, char* buffer, size_t bufsize, void* data_to_send, size_t length_of_data){
    //Work on specific cases based on signifier_byte
    if(length_of_data < 4088 && length_of_data > 0){
        memset(buffer, 0, bufsize);
        memcpy(&buffer[0], &send_to, sizeof(send_to));
        buffer[2] = signifier_byte;        
        memcpy(&buffer[3], &length_of_data, sizeof(length_of_data));
        memcpy(&buffer[3 + sizeof(length_of_data)], data_to_send, length_of_data);
        char* temp = malloc(length_of_data + 1);
        memcpy(temp, data_to_send, length_of_data);
        temp[length_of_data] = '\0';
        printf("Length of data: %zu\n", length_of_data);
        printf("Configuring network data:\n");
        printf("send to: %u\n", send_to);
        printf("signifier: %d\n", signifier_byte);
        printf("message content: %s\n", temp);
    } else {

        //Handle case where length of data received is too much or too little
        char default_server_message[3] = {0xFF, 0x01, 0x00};
        memcpy(buffer, default_server_message, sizeof(default_server_message));
    }
    return buffer;
}

int intin(int num, int* list, size_t list_size){
    for (size_t i = 0; i < list_size; i++){
        if (num == list[i])
            return i+1;
    }
    return 0;
}

int health_pool[10] = {1, 10, 5, 6, 12, 15, 18, 20, 25, 30};
float movespeed_pool[10] = {.5f, 1.0f, 1.5f, 1.6f, 1.2f, 1.15f, 1.18f, 2.0f, 2.5f, 3.0f};

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
  int connection_count = 0;
  int connections[16] = {0};
  int first_run = 1;
  int spawn_count = 0;
  char data[4096] = "original";
  char last_data[4096] = "original";
  int current_tick = 0;
  int send_message = 1;
  Data game_state = {0};
  PlayerUnits* allPlayerUnits[16] = {0};
  int unitId;
    
  while (game_running) {
    uint64_t current_time = get_current_time();
    if (current_time - last_time >= tick_interval) {
      // DO THE SERVER STUFF EVERY TICK
      //
      // use select to find readable data (client input)
      fd_set reads;
      char read[4087];
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
            // printf("Size of connections divided by size of int: %zu\n", sizeof(connections)/sizeof(int));
            // printf("Is socket client in the connections list?: %d\n", intin(socket_client, connections, sizeof(connections)/sizeof(int)));
            if (intin(socket_client, connections, sizeof(connections)/sizeof(int)) < 1){
                // printf("Adding client [%d] to connections list\n", socket_client);
                connections[connection_count] = socket_client;
                // printf("1 << connection_count = %u\n", 1 << connection_count);
                // printf("connectedPlayers = %u\n", game_state.connectedPlayers);
                // printf("xor operator result: %d\n", game_state.connectedPlayers ^ (1 << connection_count));
                game_state.connectedPlayers = game_state.connectedPlayers ^ (1 << connection_count);
                // printf("connectedPlayers = %u\n", game_state.connectedPlayers);
                // printBinaryRepresentation((void*)&game_state.connectedPlayers, sizeof(game_state.connectedPlayers));
                // printf("Connection count was: %d\n", connection_count);
                PlayerUnits* units = calloc(1, sizeof(PlayerUnits));
                if (!units) {
                    printf("Error allocating player units for socket_client: %d\n", socket_client);
                }
                units->player_id = socket_client;
                allPlayerUnits[connection_count] = units;
                connection_count++;
                // printf("Connection count is: %d\n", connection_count);
            }

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
          } else { //Receive data from all connected clients
              int bytes_received = recv(i, read, 4087, 0);
              printf("receiving %d bytes\n", bytes_received);
              for (int byte = 0; byte < bytes_received; byte++){
                  printf("%x ", read[byte]);
              }
              printf("\n");
              if (bytes_received > 0) {
                int length = 0;
                if(bytes_received >= 5){
                    memcpy(&length, read + 1, sizeof(length));
                }
                else{
                    printf("not enough bytes received. received %d bytes, need at least 5.", bytes_received);
                }
                printf("Length of data remaining: %d\n", length);
                /* printf("received %d bytes\n", bytes_received); */
                switch (read[0]) {
                  case 0x00:
                    printf("acknowledge from %d\n", i);
                    break;
                  case 0x01:
                    printf("received 0x01\n");
                    configure_network_data(game_state.connectedPlayers, 1, data, sizeof(data), read + 5, bytes_received - 5);
                    send_message = 1;
                    break;
                  case 0x02:
                    printf("received 0x02\n");
                    char what_kind_of_spawn = 0;
                    memcpy(&what_kind_of_spawn, read + 5, 1);
                    if(what_kind_of_spawn == 1){
                        int list_location = intin(i, connections, sizeof(connections)/sizeof(int));
                        if(list_location > 0){
                            printf("Client in connections list: %d, %d\n", i, list_location);
                            int index = list_location - 1;
                            PlayerUnits* this_players_units = allPlayerUnits[index];
                            Unit spawnee = {unitId++, i, what_kind_of_spawn, health_pool[what_kind_of_spawn], movespeed_pool[what_kind_of_spawn], game_state.player1spawn};
                            Unit empty_unit = {0};
                            Unit desired_unit = this_players_units->units[this_players_units->unit_count];
                            if (memcmp(&desired_unit, &empty_unit, sizeof(Unit)) == 0) {
                                desired_unit = spawnee;
                                this_players_units->unit_count++;
                            } else {
                                printf("The unit's memory slot had something in it\n");
                            }
                            SpawnResponse spawn_response = {((game_state.spawnedPlayers >> index) & 1), {1.0f, 0.0f, 3.5f}, spawnee};

                            if(spawn_response.response)
                                configure_network_data(1 << index, 0, data, sizeof(data), &spawn_response, sizeof(spawn_response));
                            else {
                                printBinaryRepresentation((void *)&game_state.spawnedPlayers, sizeof(game_state.spawnedPlayers));
                                game_state.spawnedPlayers = game_state.spawnedPlayers ^ (1 << index);
                                printf("Flipping bit\n");
                                printBinaryRepresentation((void *)&game_state.spawnedPlayers, sizeof(game_state.spawnedPlayers));
                                configure_network_data(1 << index, 2, data, sizeof(data), &spawn_response, sizeof(spawn_response));

                            }
                        }
                    }
                    if(what_kind_of_spawn == 2){
                        int list_location = intin(i, connections, sizeof(connections)/sizeof(int));
                        if(list_location > 0){
                            spawn_count++;
                            printf("Client in connections list: %d, %d\n", i, list_location);
                            int index = list_location - 1;
                            SpawnResponse spawn_response = {((game_state.spawnedPlayers >> index) & 1), {++spawn_count * 2.0f, 0.0f, 3.5f}};
                            printBinaryRepresentation((void *)&game_state.spawnedPlayers, sizeof(game_state.spawnedPlayers));
                            game_state.spawnedPlayers = game_state.spawnedPlayers ^ (1 << index);
                            printf("Flipping bit\n");
                            printBinaryRepresentation((void *)&game_state.spawnedPlayers, sizeof(game_state.spawnedPlayers));
                            configure_network_data(1 << index, 2, data, sizeof(data), &spawn_response, sizeof(spawn_response));
                        }
                    }
                    send_message = 1;
                    break;
                } //end switch
              } // end if bytes received > 0
              if (bytes_received < 1) {
                FD_CLR(i, &master);
                CLOSESOCKET(i);
                continue;
              } //endif bytes_received < 1
          } //end else read data
        } //end if FD_ISSET 

      } // end for all numbers i to max_socket
      if(send_message){
          uint16_t* recipients = (uint16_t *)&data[0];
          printf("Number of connections: %d\n", connection_count);
          for (int i = 0; i < connection_count; i++) {
            printf("Data is: ");
            for (int j = 0; j < 20; j++){
                printf("%x ", data[j]);
            }
            printf("\n");
            if((1 << i) & *recipients){
                printf("Sending data to %d\n", i);
                send(connections[i], data, sizeof(data), 0);
            }
          }
          send_message = 0;
          /* memcpy(last_data, data, sizeof(data)); */
      }
      first_run = 0;
      last_time += tick_interval;
      current_tick++;
    } //end tick interval
  } //end while game running

  printf("Closing listening socket...\n");
  CLOSESOCKET(socket_listen);

#if defined(_WIN32)
  WSACleanup();
#endif

  printf("Finished.\n");

  return 0;
}
