#ifndef CRUNK_H
#define CRUNK_H

#define FACE_MASK_NIL     0
#define FACE_MASK_WEST   (1<<0)
#define FACE_MASK_EAST  (1<<1)
#define FACE_MASK_TOP    (1<<2)
#define FACE_MASK_BOTTOM (1<<3)
#define FACE_MASK_SOUTH  (1<<4)
#define FACE_MASK_NORTH   (1<<5)

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

struct Chunk_Node;
struct Chunk;

struct Chunk_Node_List {
    Chunk_Node *first;
    Chunk_Node *last;
    int count;
};

struct Chunk_Node {
    Chunk_Node *prev;
    Chunk_Node *next;
    Chunk *chunk;
};

struct Chunk_List {
    Chunk *first;
    Chunk *last;
    int count;
};

struct Chunk {
    Chunk *next;
    Chunk *prev;
    V3_S32 position;
    Block_ID *blocks;
};

struct Chunk_Manager {
    Arena *arena;
    V3_S32 chunk_position;
    Chunk_List free_chunks;
    Chunk_List loaded_chunks;
};

#define REGION_SIZE 32

#define CHUNK_SIZE 64
#define CHUNK_HEIGHT 64

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

inline internal V3_S32 get_chunk_position(V3_S32 position);

#endif // CRUNK_H
