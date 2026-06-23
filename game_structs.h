#include <stdint.h>

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
    int hostPlayer;
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
