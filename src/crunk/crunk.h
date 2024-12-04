#ifndef CRUNK_H
#define CRUNK_H

struct Texture_Atlas;

struct Texture_Region {
    Texture_Region *hash_next;
    Texture_Region *hash_prev;
    Texture_Atlas *atlas;
    String8 name;
    v2_s32 offset;
    v2_s32 dim;

    u64 size;
    u8 *data;
};

struct Texture_Region_Bucket {
    Texture_Region *first;
    Texture_Region *last;
};

#define MAX_ATLAS_X 64
#define MAX_ATLAS_Y 32
struct Texture_Atlas {
    R_Handle tex_handle;

    v2_s32 dim;
    v2_s32 region_dim;

    s32 texture_region_count;
    Texture_Region *texture_regions;

    s32 region_hash_table_size;
    Texture_Region_Bucket *region_hash_table;
};

struct Texture_Map {
    R_Handle texture;
    v2_s32 size;
    u8 *data;
};

struct Camera {
    v3 position;
    v3 forward;
    v3 up;
    v3 right;
    f32 yaw;
    f32 pitch;
    f32 fov;
    m4 view_matrix;
};

enum Block_ID : u16 {
    BLOCK_AIR,
    BLOCK_ERR,
    BLOCK_STONE,
    BLOCK_DIRT,
    BLOCK_GRASS,
    BLOCK_WOOD,
    BLOCK_BRICK,
    BLOCK_SAND,
    BLOCK_COBBLESTONE,
    BLOCK_LOG,
    BLOCK_COUNT
};

enum Face {
    FACE_TOP,
    FACE_BOTTOM,
    FACE_NORTH,
    FACE_SOUTH,
    FACE_EAST,
    FACE_WEST,
    FACE_COUNT
};

enum Sound_Type {
    SOUND_BREAKING,
    SOUND_STEP,
    SOUND_PLACE,
    SOUND_COUNT
};

enum Step_Type {
    STEP_NIL,
    STEP_SAND,
    STEP_WOOD,
    STEP_GRASS,
    STEP_DIRT,
    STEP_STONE,
    STEP_COUNT
};

enum Block_Flags {
    BLOCK_FLAG_NIL     = 0,
    BLOCK_FLAG_GRAVITY = (1<<0),
    BLOCK_FLAG_OPAQUE  = (1<<1),
};
EnumDefineFlagOperators(Block_Flags);

struct Block_Face {
    String8 texture_name;
    Texture_Region *texture_region;
    v4 color;
};

struct Block {
    Block_Flags flags;
    Block_Face faces[FACE_COUNT];
    Step_Type step_type;

    // f32 gravity_coefficient;
    // f32 friction_coefficient;
};

#define FACE_MASK_NIL     0
#define FACE_MASK_WEST   (1<<0)
#define FACE_MASK_EAST  (1<<1)
#define FACE_MASK_TOP    (1<<2)
#define FACE_MASK_BOTTOM (1<<3)
#define FACE_MASK_SOUTH  (1<<4)
#define FACE_MASK_NORTH   (1<<5)

struct Chunk;
struct Chunk_List {
    Chunk *first;
    Chunk *last;
    int count;
};

struct Chunk {
    Chunk *next;
    Chunk *prev;
    v3_s32 position;
    Block_ID *blocks;
};

struct Chunk_Manager {
    Arena *arena;
    v3_s32 chunk_position;
    Chunk_List free_chunks;
    Chunk_List loaded_chunks;
};

#define REGION_SIZE 32

#define CHUNK_SIZE 64
// #define CHUNK_HEIGHT 256
#define BLOCK_AT(Chunk, X, Y, Z) (&(Chunk)->blocks[(X) + (Y)*CHUNK_SIZE + (Z)*CHUNK_SIZE*CHUNK_SIZE])

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
    Inventory *inventory;
    v3 position;
    v3 velocity;
    b32 grounded;
    b32 jumping;
    f32 jump_t;
};

struct Game_State {
    Arena *arena;
    v2_s32 client_dim;
    Camera camera;

    b32 raycast_hit;
    v3 raycast_p;

    b32 creative_mode;
    Crafting_State *crafting_state;
    Player *player;
};

#endif // CRUNK_H
