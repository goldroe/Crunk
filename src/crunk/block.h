#ifndef BLOCK_H
#define BLOCK_H

struct Chunk;

enum Block_Flags {
    BLOCK_FLAG_NIL          = 0,
    BLOCK_FLAG_GRAVITY      = (1<<0),
    BLOCK_FLAG_TRANSPARENT  = (1<<1),
};
EnumDefineFlagOperators(Block_Flags);

enum Block_ID : u16 {
    BLOCK_AIR,
    BLOCK_ERR,
    BLOCK_WATER,
    BLOCK_STONE,
    BLOCK_DIRT,
    BLOCK_GRASS,
    BLOCK_SAND,
    BLOCK_COBBLESTONE,
    BLOCK_BEDROCK,
    BLOCK_LOG,
    BLOCK_LEAVES,
    BLOCK_WOOD,
    BLOCK_BRICK,
    BLOCK_COUNT
};

struct Block_Face {
    String8 texture_name;
    Texture_Region *texture_region;
    u8 color_id;
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

struct Block {
    Block_Flags flags;
    Block_Face faces[FACE_COUNT];
    Step_Type step_type;

    // f32 gravity_coefficient;
    // f32 friction_coefficient;
};

inline internal Block_ID *block_id_zero();
inline internal char *block_to_string(Block_ID block);
inline internal Block_ID *block_at(Chunk *chunk, int x, int y, int z);
inline internal bool block_is_active(Block_ID block);
inline internal Block *get_basic_block(Block_ID block);
inline internal bool block_is_opaque(Block *block);
inline internal bool block_is_opaque(Block_ID block);
inline internal void block_place(Block_ID *block, Block_ID value);
#endif // BLOCK_H
