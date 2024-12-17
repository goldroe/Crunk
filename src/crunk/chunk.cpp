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
inline internal u64 make_block_vertex_data(Face face, u8 x, u8 y, u8 z, u8 tex, u8 col) {
    u64 result = (u64)face | (x<<8) | (y<<16) | ((u64)z<<24) | ((u64)tex<<32) | ((u64)col<<40);
    return result;
}

internal void push_mesh_geometry(Chunk *chunk, bool do_opaque, Auto_Array<u64> &geometry) {
#define push_vertex(F, P, T, C) (geometry.push(make_block_vertex_data(F, (u8)P.x, (u8)P.y, (u8)P.z, T, C)))

    for (int y = 0; y < CHUNK_HEIGHT; y++) {
        for (int z = 0; z < CHUNK_SIZE; z++) {
            for (int x = 0; x < CHUNK_SIZE; x++) {
                Block_ID *block = block_at(chunk, x, y, z);
                Block *basic_block = get_basic_block(*block);

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
                        push_vertex(FACE_TOP, p3, atlas_index + 0, face->color_id);
                        push_vertex(FACE_TOP, p2, atlas_index + 1, face->color_id);
                        push_vertex(FACE_TOP, p6, atlas_index + 2, face->color_id);
                        push_vertex(FACE_TOP, p6, atlas_index + 2, face->color_id);
                        push_vertex(FACE_TOP, p7, atlas_index + 3, face->color_id);
                        push_vertex(FACE_TOP, p3, atlas_index + 0, face->color_id);
                    }
                    if (!(face_mask & FACE_MASK_BOTTOM)) {
                        Block_Face *face = &basic_block->faces[FACE_BOTTOM];
                        u8 atlas_index = (u8)face->texture_region->atlas_index * 4;
                        push_vertex(FACE_BOTTOM, p1, atlas_index + 0, face->color_id);
                        push_vertex(FACE_BOTTOM, p0, atlas_index + 1, face->color_id);
                        push_vertex(FACE_BOTTOM, p4, atlas_index + 2, face->color_id);
                        push_vertex(FACE_BOTTOM, p4, atlas_index + 2, face->color_id);
                        push_vertex(FACE_BOTTOM, p5, atlas_index + 3, face->color_id);
                        push_vertex(FACE_BOTTOM, p1, atlas_index + 0, face->color_id);
                    }
                    if (!(face_mask & FACE_MASK_NORTH)) {
                        Block_Face *face = &basic_block->faces[FACE_NORTH];
                        u8 atlas_index = (u8)face->texture_region->atlas_index * 4;
                        push_vertex(FACE_NORTH, p0, atlas_index + 0, face->color_id);
                        push_vertex(FACE_NORTH, p1, atlas_index + 1, face->color_id);
                        push_vertex(FACE_NORTH, p2, atlas_index + 2, face->color_id);
                        push_vertex(FACE_NORTH, p2, atlas_index + 2, face->color_id);
                        push_vertex(FACE_NORTH, p3, atlas_index + 3, face->color_id);
                        push_vertex(FACE_NORTH, p0, atlas_index + 0, face->color_id);
                    }
                    if (!(face_mask & FACE_MASK_SOUTH)) {
                        Block_Face *face = &basic_block->faces[FACE_SOUTH];
                        u8 atlas_index = (u8)face->texture_region->atlas_index * 4;
                        push_vertex(FACE_SOUTH, p5, atlas_index + 0, face->color_id);
                        push_vertex(FACE_SOUTH, p4, atlas_index + 1, face->color_id);
                        push_vertex(FACE_SOUTH, p7, atlas_index + 2, face->color_id);
                        push_vertex(FACE_SOUTH, p7, atlas_index + 2, face->color_id);
                        push_vertex(FACE_SOUTH, p6, atlas_index + 3, face->color_id);
                        push_vertex(FACE_SOUTH, p5, atlas_index + 0, face->color_id);
                    }
                    if (!(face_mask & FACE_MASK_EAST)) {
                        Block_Face *face = &basic_block->faces[FACE_EAST];
                        u8 atlas_index = (u8)face->texture_region->atlas_index * 4;
                        push_vertex(FACE_EAST, p1, atlas_index + 0, face->color_id);
                        push_vertex(FACE_EAST, p5, atlas_index + 1, face->color_id);
                        push_vertex(FACE_EAST, p6, atlas_index + 2, face->color_id);
                        push_vertex(FACE_EAST, p6, atlas_index + 2, face->color_id);
                        push_vertex(FACE_EAST, p2, atlas_index + 3, face->color_id);
                        push_vertex(FACE_EAST, p1, atlas_index + 0, face->color_id);
                    }
                    if (!(face_mask & FACE_MASK_WEST)) {
                        Block_Face *face = &basic_block->faces[FACE_WEST];
                        u8 atlas_index = (u8)face->texture_region->atlas_index * 4;
                        push_vertex(FACE_WEST, p4, atlas_index + 0, face->color_id);
                        push_vertex(FACE_WEST, p0, atlas_index + 1, face->color_id);
                        push_vertex(FACE_WEST, p3, atlas_index + 2, face->color_id);
                        push_vertex(FACE_WEST, p3, atlas_index + 2, face->color_id);
                        push_vertex(FACE_WEST, p7, atlas_index + 3, face->color_id);
                        push_vertex(FACE_WEST, p4, atlas_index + 0, face->color_id);
                    }
                }
            }
        }
    }
}

internal void load_chunk_mesh(Chunk *chunk) {
    chunk->opaque_geo.reset_count();
    chunk->transparent_geo.reset_count();
    push_mesh_geometry(chunk, true, chunk->opaque_geo);
    push_mesh_geometry(chunk, true, chunk->transparent_geo);
}
