
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
