global Chunk_Manager *chunk_manager;

inline internal V3_S32 get_chunk_position(s32 x, s32 y, s32 z) {
    V3_S32 result;
    result.x = (int)floor_f32((f32)x / CHUNK_SIZE);
    result.y = (int)floor_f32((f32)y / CHUNK_HEIGHT);
    result.z = (int)floor_f32((f32)z / CHUNK_SIZE);
    return result;
}

inline internal V3_S32 get_chunk_position(V3_F64 position) {
    V3_S32 result;
    result.x = (int)floor_f64(position.x / CHUNK_SIZE);
    result.y = (int)floor_f64(position.y / CHUNK_HEIGHT);
    result.z = (int)floor_f64(position.z / CHUNK_SIZE);
    return result;
}

inline internal V3_S32 get_chunk_relative_position(Chunk *chunk, s32 x, s32 y, s32 z) {
    V3_S32 result;
    result.x = x - CHUNK_SIZE * chunk->position.x;
    result.y = y - CHUNK_HEIGHT * chunk->position.y;
    result.z = z - CHUNK_SIZE * chunk->position.z;
    return result;
}

inline internal u8 get_chunk_face_mask(Chunk *chunk, int x, int y, int z) {
    u8 face_mask = 0;
    if (y < CHUNK_SIZE - 1) face_mask |= FACE_MASK_TOP*block_is_opaque(*block_at(chunk, x, y + 1, z));
    if (y > 0)              face_mask |= FACE_MASK_BOTTOM*block_is_opaque(*block_at(chunk, x, y - 1, z));
    if (z < CHUNK_SIZE - 1) face_mask |= FACE_MASK_NORTH*block_is_opaque(*block_at(chunk, x, y, z + 1));
    if (z > 0)              face_mask |= FACE_MASK_SOUTH*block_is_opaque(*block_at(chunk, x, y, z - 1));
    if (x > 0)              face_mask |= FACE_MASK_WEST*block_is_opaque(*block_at(chunk, x - 1, y, z));
    if (x < CHUNK_SIZE - 1) face_mask |= FACE_MASK_EAST*block_is_opaque(*block_at(chunk, x + 1, y, z));
    return face_mask;
}

// 0-1 face orientation (TOP, BOT, NS, EW)
// 1-4 xyz (chunk relative)
// 4-5 tex index
// 5-6 color index
// 6-7 light level
inline internal u64 make_block_vertex_data(Face face, u8 x, u8 y, u8 z, u8 tex, u8 col, u8 l) {
    u64 result = (u64)face | (x<<8) | (y<<16) | ((u64)z<<24) | ((u64)tex<<32) | ((u64)col<<40) | ((u64)l << 48);
    return result;
}

internal void push_mesh_geometry(Chunk *chunk, bool do_opaque, Auto_Array<u64> &geometry) {
#define push_vertex(F, P, T, C, L) (geometry.push(make_block_vertex_data(F, (u8)P.x, (u8)P.y, (u8)P.z, T, C, L)))

    for (int y = 0; y < CHUNK_HEIGHT; y++) {
        for (int z = 0; z < CHUNK_SIZE; z++) {
            for (int x = 0; x < CHUNK_SIZE; x++) {
                Block_ID *block = block_at(chunk, x, y, z);
                Block *basic_block = get_basic_block(*block);
                u8 l = chunk->light_map[x + y * CHUNK_SIZE * CHUNK_SIZE + z * CHUNK_SIZE];

                if (do_opaque != block_is_opaque(basic_block)) continue;

                if (block_is_active(*block)) {
                    u8 face_mask = get_chunk_face_mask(chunk, x, y, z);

                    const int S = 1;
                    V3_S32 p0 = v3_s32(x,     y,     z + S);
                    V3_S32 p1 = v3_s32(x + S, y,     z + S);
                    V3_S32 p2 = v3_s32(x + S, y + S, z + S);
                    V3_S32 p3 = v3_s32(x,     y + S, z + S);
                    V3_S32 p4 = v3_s32(x,     y,     z);
                    V3_S32 p5 = v3_s32(x + S, y,     z);
                    V3_S32 p6 = v3_s32(x + S, y + S, z);
                    V3_S32 p7 = v3_s32(x,     y + S, z);

                    if (!(face_mask & FACE_MASK_TOP)) {
                        Block_Face *face = &basic_block->faces[FACE_TOP];
                        u8 atlas_index = (u8)face->texture_region->atlas_index * 4;
                        push_vertex(FACE_TOP, p3, atlas_index + 0, face->color_id, l);
                        push_vertex(FACE_TOP, p2, atlas_index + 1, face->color_id, l);
                        push_vertex(FACE_TOP, p6, atlas_index + 2, face->color_id, l);
                        push_vertex(FACE_TOP, p6, atlas_index + 2, face->color_id, l);
                        push_vertex(FACE_TOP, p7, atlas_index + 3, face->color_id, l);
                        push_vertex(FACE_TOP, p3, atlas_index + 0, face->color_id, l);
                    }
                    if (!(face_mask & FACE_MASK_BOTTOM)) {
                        Block_Face *face = &basic_block->faces[FACE_BOTTOM];
                        u8 atlas_index = (u8)face->texture_region->atlas_index * 4;
                        push_vertex(FACE_BOTTOM, p1, atlas_index + 0, face->color_id, l);
                        push_vertex(FACE_BOTTOM, p0, atlas_index + 1, face->color_id, l);
                        push_vertex(FACE_BOTTOM, p4, atlas_index + 2, face->color_id, l);
                        push_vertex(FACE_BOTTOM, p4, atlas_index + 2, face->color_id, l);
                        push_vertex(FACE_BOTTOM, p5, atlas_index + 3, face->color_id, l);
                        push_vertex(FACE_BOTTOM, p1, atlas_index + 0, face->color_id, l);
                    }
                    if (!(face_mask & FACE_MASK_NORTH)) {
                        Block_Face *face = &basic_block->faces[FACE_NORTH];
                        u8 atlas_index = (u8)face->texture_region->atlas_index * 4;
                        push_vertex(FACE_NORTH, p0, atlas_index + 0, face->color_id, l);
                        push_vertex(FACE_NORTH, p1, atlas_index + 1, face->color_id, l);
                        push_vertex(FACE_NORTH, p2, atlas_index + 2, face->color_id, l);
                        push_vertex(FACE_NORTH, p2, atlas_index + 2, face->color_id, l);
                        push_vertex(FACE_NORTH, p3, atlas_index + 3, face->color_id, l);
                        push_vertex(FACE_NORTH, p0, atlas_index + 0, face->color_id, l);
                    }
                    if (!(face_mask & FACE_MASK_SOUTH)) {
                        Block_Face *face = &basic_block->faces[FACE_SOUTH];
                        u8 atlas_index = (u8)face->texture_region->atlas_index * 4;
                        push_vertex(FACE_SOUTH, p5, atlas_index + 0, face->color_id, l);
                        push_vertex(FACE_SOUTH, p4, atlas_index + 1, face->color_id, l);
                        push_vertex(FACE_SOUTH, p7, atlas_index + 2, face->color_id, l);
                        push_vertex(FACE_SOUTH, p7, atlas_index + 2, face->color_id, l);
                        push_vertex(FACE_SOUTH, p6, atlas_index + 3, face->color_id, l);
                        push_vertex(FACE_SOUTH, p5, atlas_index + 0, face->color_id, l);
                    }
                    if (!(face_mask & FACE_MASK_EAST)) {
                        Block_Face *face = &basic_block->faces[FACE_EAST];
                        u8 atlas_index = (u8)face->texture_region->atlas_index * 4;
                        push_vertex(FACE_EAST, p1, atlas_index + 0, face->color_id, l);
                        push_vertex(FACE_EAST, p5, atlas_index + 1, face->color_id, l);
                        push_vertex(FACE_EAST, p6, atlas_index + 2, face->color_id, l);
                        push_vertex(FACE_EAST, p6, atlas_index + 2, face->color_id, l);
                        push_vertex(FACE_EAST, p2, atlas_index + 3, face->color_id, l);
                        push_vertex(FACE_EAST, p1, atlas_index + 0, face->color_id, l);
                    }
                    if (!(face_mask & FACE_MASK_WEST)) {
                        Block_Face *face = &basic_block->faces[FACE_WEST];
                        u8 atlas_index = (u8)face->texture_region->atlas_index * 4;
                        push_vertex(FACE_WEST, p4, atlas_index + 0, face->color_id, l);
                        push_vertex(FACE_WEST, p0, atlas_index + 1, face->color_id, l);
                        push_vertex(FACE_WEST, p3, atlas_index + 2, face->color_id, l);
                        push_vertex(FACE_WEST, p3, atlas_index + 2, face->color_id, l);
                        push_vertex(FACE_WEST, p7, atlas_index + 3, face->color_id, l);
                        push_vertex(FACE_WEST, p4, atlas_index + 0, face->color_id, l);
                    }
                }
            }
        }
    }
}

internal void light_fill(Arena *arena, Light_Source_List *light_sources, u8 *light_map, u8 x, u8 y, u8 z, u8 level) {
    u8 *light_value = &light_map[x + y * CHUNK_SIZE * CHUNK_SIZE + z * CHUNK_SIZE];

    if (level > *light_value) {
        *light_value = level;
        Light_Source *light_source = push_array(arena, Light_Source, 1);
        light_source->x = x;
        light_source->y = y;
        light_source->z = z;
        light_source->level = level;
        DLLPushBack(light_sources->first, light_sources->last, light_source, next, prev);
        light_sources->count++;
    }
}

internal void load_chunk_mesh(Chunk *chunk, int hour) {
    chunk->opaque_geo.reset_count();
    chunk->transparent_geo.reset_count();

    Arena *arena = chunk_manager->arena;
    Arena_Temp temp = arena_temp_begin(arena);

    Light_Source_List light_sources = {};

    MemoryZero(chunk->light_map, sizeof(u8) * CHUNK_BLOCKS);

    //@Note Generate light map

    //@Note Surface lighting sources
    for (int x = 0; x < CHUNK_SIZE; x++) {
        for (int z = 0; z < CHUNK_SIZE; z++) {
            for (int y = CHUNK_HEIGHT - 1; y >= 0; y--) {
                Block_ID *block = block_at(chunk, x, y, z);
                if (block_is_active(*block)) {
                    chunk->light_map[x + y * CHUNK_SIZE * CHUNK_SIZE + z * CHUNK_SIZE] = MAX_LIGHT_LEVEL;
                    break;
                }
            }
        }
    }

    //@Note Block light sources
    for (int y = 0; y < CHUNK_HEIGHT; y++) {
        for (int x = 0; x < CHUNK_SIZE; x++) {
            for (int z = 0; z < CHUNK_SIZE; z++) {
                Block_ID *block = block_at(chunk, x, y, z);
                Block *basic = get_basic_block(*block);
                if (basic->flags & BLOCK_FLAG_EMITS_LIGHT) {
                    Light_Source *light_source = push_array(arena, Light_Source, 1);
                    light_source->x = (u8)x;
                    light_source->y = (u8)y;
                    light_source->z = (u8)z;
                    light_source->level = MAX_LIGHT_LEVEL;
                    DLLPushBack(light_sources.first, light_sources.last, light_source, next, prev);
                    light_sources.count++;
                    chunk->light_map[x + y * CHUNK_SIZE * CHUNK_SIZE + z * CHUNK_SIZE] = MAX_LIGHT_LEVEL;
                }
            }
        }
    }

    //@Note Light Flood Fill
    while (light_sources.count > 0) {
        Light_Source *node = light_sources.first;
        DLLRemove(light_sources.first, light_sources.last, node, next, prev);
        light_sources.count--;

        if (node->level - 1 == 0) continue;

        if (node->x < CHUNK_SIZE - 1) light_fill(arena, &light_sources, chunk->light_map, node->x + 1, node->y, node->z, node->level - 1);
        if (node->x > 0) light_fill(arena, &light_sources, chunk->light_map, node->x - 1, node->y, node->z, node->level - 1);
        if (node->y < CHUNK_HEIGHT - 1) light_fill(arena, &light_sources, chunk->light_map, node->x, node->y + 1, node->z, node->level - 1);
        if (node->y > 0) light_fill(arena, &light_sources, chunk->light_map, node->x, node->y - 1, node->z, node->level - 1);
        if (node->z < CHUNK_SIZE - 1) light_fill(arena, &light_sources, chunk->light_map, node->x, node->y, node->z + 1, node->level - 1);
        if (node->z > 0) light_fill(arena, &light_sources, chunk->light_map, node->x, node->y, node->z - 1, node->level - 1);
    }

    arena_temp_end(temp);

    //@Todo Sort??
    //@Note generate chunk mesh data
    push_mesh_geometry(chunk, true, chunk->opaque_geo);
    push_mesh_geometry(chunk, false, chunk->transparent_geo);

}
