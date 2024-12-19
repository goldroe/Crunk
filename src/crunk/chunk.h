#ifndef CHUNK_H
#define CHUNK_H

#define FACE_MASK_NIL     0
#define FACE_MASK_WEST   (1<<0)
#define FACE_MASK_EAST  (1<<1)
#define FACE_MASK_TOP    (1<<2)
#define FACE_MASK_BOTTOM (1<<3)
#define FACE_MASK_SOUTH  (1<<4)
#define FACE_MASK_NORTH   (1<<5)

struct Chunk_Node;
struct Chunk;

#define CHUNK_SIZE 64
#define CHUNK_HEIGHT 256
#define CHUNK_DEPTH 4096
#define CHUNK_BLOCKS (CHUNK_HEIGHT*CHUNK_SIZE*CHUNK_SIZE)

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


#define MAX_LIGHT_LEVEL 15
struct Light_Source {
    Light_Source *next;
    Light_Source *prev;
    u8 x, y, z;
    u8 level;
};

struct Light_Source_List {
    Light_Source *first;
    Light_Source *last;
    int count;
};

enum Chunk_Flags {
    CHUNK_FLAG_NIL        = 0,
    CHUNK_FLAG_GENERATED  = (1<<0),
    CHUNK_FLAG_DIRTY      = (1<<1),
    CHUNK_FLAG_BAKED      = (1<<2),
    CHUNK_FLAG_CULLED     = (1<<3),
    CHUNK_FLAG_GENERATING  = (1<<4),
};
EnumDefineFlagOperators(Chunk_Flags);

struct Chunk {
    Chunk *next;
    Chunk *prev;
    V3_S32 position;

    Chunk_Flags flags;
    AABB box;
    Block_ID *blocks;

    Auto_Array<u64> opaque_geo;
    Auto_Array<u64> transparent_geo;
    u8 *light_map;
};

inline internal V3_S32 get_chunk_position(V3_S32 position);
internal void load_chunk_mesh(Chunk *chunk);
inline internal bool chunk_is_dirty(Chunk *chunk);
inline internal bool chunk_is_generated(Chunk *chunk);

#endif // CHUNK_H
