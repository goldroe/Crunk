#ifndef ATLAS_H
#define ATLAS_H

struct Texture_Atlas;

struct Texture_Region {
    Texture_Region *hash_next;
    Texture_Region *hash_prev;
    Texture_Atlas *atlas;
    String8 name;
    u32 atlas_index;
    Vector2 offset;
    Vector2 dim;
    u64 size;
    u8 *data;
};

struct Texture_Map {
    R_Handle texture;
    Vector2Int size;
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

    Vector2Int dim;
    Vector2Int region_dim;

    s32 texture_region_count;
    Texture_Region *texture_regions;

    s32 region_hash_table_size;
    Texture_Region_Bucket *region_hash_table;
};

#endif // ATLAS_H
