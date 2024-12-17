#ifndef CRUNK_H
#define CRUNK_H

struct Ray {
    V3_F64 origin;
    V3_F32 direction;
};

struct Voxel_Raycast {
    Ray ray;
    V3_F64 hit;
    Block_ID *block;
    Face face;
};

struct Chunk_Manager {
    Arena *arena;
    V3_S32 chunk_position;
    Chunk_List free_chunks;
    Chunk_List loaded_chunks;
};

#define REGION_SIZE 32

#define INVENTORY_SLOTS 36
#define CRAFT_SLOTS 9
#define HOTBAR_SLOTS 9

enum Object_Type {
    OT_Nil,
    OT_Block,
    OT_Item,
    OT_COUNT
};

struct Object {
    Object_Type object_type;
};

struct Item : Object {
    R_Handle sprite;
};

struct Slot {
    Object *object;
};

struct Inventory {
    Slot slots[INVENTORY_SLOTS];
};

struct Crafting_State {
    Slot slots[CRAFT_SLOTS];
    Slot product;
};

struct Player {
    V3_F64 position;
    V3_F32 velocity;

    Voxel_Raycast raycast;
    Chunk *chunk;

    Block_ID *mining;
    f32 mining_t;
    f32 mining_target_t;

    Inventory *inventory;

    b32 grounded;
    b32 jumping;
    f32 jump_t;
};

struct Game_State {
    Arena *arena;
    V2_S32 client_dim;
    Camera camera;

    b32 creative_mode;
    Crafting_State *crafting_state;
    Player *player;

    int ticks_per_second;
    int ticks_per_day;

    int day_t;

    Frustum frustum;

    b32 mesh_debug;
};

#endif // CRUNK_H
