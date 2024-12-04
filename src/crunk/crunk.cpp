
global World_Generator *world_generator;
global Game_State *game_state;
global Chunk_Manager *chunk_manager;

global Texture_Atlas *g_block_atlas;
global Block g_basic_blocks[BLOCK_COUNT];

internal u64 djb2_hash_string(String8 string) {
    u64 result = 5381;
    for (u64 i = 0; i < string.count; i++) {
        result = ((result << 5) + result) + string.data[i];
    }
    return result;
}

inline internal bool block_active(Block_ID *block) {
    return *block != BLOCK_AIR;
}

internal v3_s32 get_chunk_position(v3 position) {
    v3_s32 result;
    result.x = (int)floorf(position.x / CHUNK_SIZE);
    result.y = (int)floorf(position.y / CHUNK_SIZE);
    result.z = (int)floorf(position.z / CHUNK_SIZE);
    return result;
}

internal v3_s32 get_chunk_relative_position(Chunk *chunk, v3 world_position) {
    v3_s32 result;
    result.x = (s32)floorf(world_position.x - CHUNK_SIZE * chunk->position.x);
    result.y = (s32)floorf(world_position.y - CHUNK_SIZE * chunk->position.y);
    result.z = (s32)floorf(world_position.z - CHUNK_SIZE * chunk->position.z);
    return result;
}

internal Chunk *get_chunk_from_position(Chunk_Manager *manager, v3_s32 chunk_position) {
    Chunk *result = NULL;
    for (Chunk *chunk = manager->loaded_chunks.first; chunk; chunk = chunk->next) {
        if (chunk->position == chunk_position) {
            result = chunk;
            break;
        }
    }
    return result;
}

internal Block_ID *get_block_from_position(Chunk_Manager *manager, v3 world_position) {
    Block_ID *result = NULL;

    v3_s32 chunk_p = get_chunk_position(world_position);
    Chunk *chunk = get_chunk_from_position(manager, chunk_p);
    if (chunk) {
        v3_s32 block_p = get_chunk_relative_position(chunk, world_position);
        result = BLOCK_AT(chunk, block_p.x, block_p.y, block_p.z);
    }
    return result;
}

internal bool is_block_active_at(Chunk_Manager *manager, v3 world_position) {
    Block_ID *block = get_block_from_position(manager, world_position);
    return block && *block != BLOCK_AIR;
}

internal s32 get_height_from_chunk_xz(Chunk *chunk, s32 x, s32 z) {
    s32 result = 0;
    for (int y = CHUNK_SIZE - 1; y >= 0; y--) {
        Block_ID *block = BLOCK_AT(chunk, x, y, z);
        if (block_active(block)) {
            result = y;
            break;
        }
    }
    return result;
}

internal Texture_Map load_texture_map(String8 file_name) {
    int x, y, n;
    u8 *data = stbi_load((char *)file_name.data, &x, &y, &n, 1);
    R_Handle tex = d3d11_create_texture(R_Tex2DFormat_R8, v2s32(x, y), data);

    Texture_Map result = {};
    result.texture = tex;
    result.size = v2s32(x, y);
    result.data = data;
    return result;
}

internal f32 get_height_map_value(Texture_Map *map, s32 x, s32 y) {
    Assert(x <= map->size.x);
    Assert(y <= map->size.y);
    f32 result = 0.f;
    result = (map->data[y * map->size.x + x]) / 255.0f;
    return result;
}

internal void draw_block(Block *block, v3 position, u8 face_mask) {
    f32 S = 0.5f;
    v3 p0 = make_v3(position.x - S, position.y - S, position.z + S);
    v3 p1 = make_v3(position.x + S, position.y - S, position.z + S);
    v3 p2 = make_v3(position.x + S, position.y + S, position.z + S);
    v3 p3 = make_v3(position.x - S, position.y + S, position.z + S);
    v3 p4 = make_v3(position.x - S, position.y - S, position.z - S);
    v3 p5 = make_v3(position.x + S, position.y - S, position.z - S);
    v3 p6 = make_v3(position.x + S, position.y + S, position.z - S);
    v3 p7 = make_v3(position.x - S, position.y + S, position.z - S);

    Texture_Atlas *atlas = g_block_atlas;

    if (!(face_mask & FACE_MASK_TOP)) {
        Block_Face *face = &block->faces[FACE_TOP];
        Rect src = make_rect(face->texture_region->offset.x / (f32)atlas->dim.x, face->texture_region->offset.y / (f32)atlas->dim.y, face->texture_region->dim.x / (f32)atlas->dim.x, face->texture_region->dim.y / (f32)atlas->dim.y);
        draw_3d_vertex(p3, face->color, make_v2(src.x0, src.y1));
        draw_3d_vertex(p2, face->color, make_v2(src.x1, src.y1));
        draw_3d_vertex(p6, face->color, make_v2(src.x1, src.y0));
        draw_3d_vertex(p7, face->color, make_v2(src.x0, src.y0));
    }
    if (!(face_mask & FACE_MASK_BOTTOM)) {
        Block_Face *face = &block->faces[FACE_BOTTOM];
        Rect src = make_rect(face->texture_region->offset.x / (f32)atlas->dim.x, face->texture_region->offset.y / (f32)atlas->dim.y, face->texture_region->dim.x / (f32)atlas->dim.x, face->texture_region->dim.y / (f32)atlas->dim.y);
        draw_3d_vertex(p1, face->color, make_v2(src.x0, src.y1));
        draw_3d_vertex(p0, face->color, make_v2(src.x1, src.y1));
        draw_3d_vertex(p4, face->color, make_v2(src.x1, src.y0));
        draw_3d_vertex(p5, face->color, make_v2(src.x0, src.y0));
    }
    if (!(face_mask & FACE_MASK_NORTH)) {
        Block_Face *face = &block->faces[FACE_NORTH];
        Rect src = make_rect(face->texture_region->offset.x / (f32)atlas->dim.x, face->texture_region->offset.y / (f32)atlas->dim.y, face->texture_region->dim.x / (f32)atlas->dim.x, face->texture_region->dim.y / (f32)atlas->dim.y);
        draw_3d_vertex(p0, face->color, make_v2(src.x0, src.y1));
        draw_3d_vertex(p1, face->color, make_v2(src.x1, src.y1));
        draw_3d_vertex(p2, face->color, make_v2(src.x1, src.y0));
        draw_3d_vertex(p3, face->color, make_v2(src.x0, src.y0));
    }
    if (!(face_mask & FACE_MASK_SOUTH)) {
        Block_Face *face = &block->faces[FACE_SOUTH];
        Rect src = make_rect(face->texture_region->offset.x / (f32)atlas->dim.x, face->texture_region->offset.y / (f32)atlas->dim.y, face->texture_region->dim.x / (f32)atlas->dim.x, face->texture_region->dim.y / (f32)atlas->dim.y);
        draw_3d_vertex(p5, face->color, make_v2(src.x0, src.y1));
        draw_3d_vertex(p4, face->color, make_v2(src.x1, src.y1));
        draw_3d_vertex(p7, face->color, make_v2(src.x1, src.y0));
        draw_3d_vertex(p6, face->color, make_v2(src.x0, src.y0));
    }
    if (!(face_mask & FACE_MASK_EAST)) {
        Block_Face *face = &block->faces[FACE_EAST];
        Rect src = make_rect(face->texture_region->offset.x / (f32)atlas->dim.x, face->texture_region->offset.y / (f32)atlas->dim.y, face->texture_region->dim.x / (f32)atlas->dim.x, face->texture_region->dim.y / (f32)atlas->dim.y);
        draw_3d_vertex(p1, face->color, make_v2(src.x0, src.y1));
        draw_3d_vertex(p5, face->color, make_v2(src.x1, src.y1));
        draw_3d_vertex(p6, face->color, make_v2(src.x1, src.y0));
        draw_3d_vertex(p2, face->color, make_v2(src.x0, src.y0));
    }
    if (!(face_mask & FACE_MASK_WEST)) {
        Block_Face *face = &block->faces[FACE_WEST];
        Rect src = make_rect(face->texture_region->offset.x / (f32)atlas->dim.x, face->texture_region->offset.y / (f32)atlas->dim.y, face->texture_region->dim.x / (f32)atlas->dim.x, face->texture_region->dim.y / (f32)atlas->dim.y);
        draw_3d_vertex(p4, face->color, make_v2(src.x0, src.y1));
        draw_3d_vertex(p0, face->color, make_v2(src.x1, src.y1));
        draw_3d_vertex(p3, face->color, make_v2(src.x1, src.y0));
        draw_3d_vertex(p7, face->color, make_v2(src.x0, src.y0));
    }
}

#define FACE_TEX_SIZE 1.0f
internal void draw_chunk(Chunk *chunk) {
    v4 color = make_v4(1.f, 1.f, 1.f, 1.f);
    for (int z = 0; z < CHUNK_SIZE; z++) {
        for (int y = 0; y < CHUNK_SIZE; y++) {
            for (int x = 0; x < CHUNK_SIZE; x++) {
                Block_ID *block = BLOCK_AT(chunk, x, y, z);
                v3 position = CHUNK_SIZE * make_v3((f32)chunk->position.x, (f32)chunk->position.y, (f32)chunk->position.z);
                position.x += x;
                position.y += y;
                position.z += z;
                if (block_active(block)) {
                    Block *block_type = &g_basic_blocks[*block];

                    //@Note Test for neighboring blocks
                    u8 face_mask = 0;
                    if (z < CHUNK_SIZE - 1) face_mask |= FACE_MASK_NORTH*block_active(BLOCK_AT(chunk, x, y, z + 1));
                    if (z > 0)              face_mask |= FACE_MASK_SOUTH*block_active(BLOCK_AT(chunk, x, y, z - 1));
                    if (x > 0)              face_mask |= FACE_MASK_WEST*block_active(BLOCK_AT(chunk, x - 1, y, z));
                    if (x < CHUNK_SIZE - 1) face_mask |= FACE_MASK_EAST*block_active(BLOCK_AT(chunk, x + 1, y, z));
                    if (y < CHUNK_SIZE - 1) face_mask |= FACE_MASK_TOP*block_active(BLOCK_AT(chunk, x, y + 1, z));
                    if (y > 0)              face_mask |= FACE_MASK_BOTTOM*block_active(BLOCK_AT(chunk, x, y - 1, z));

                    draw_block(block_type, position, face_mask);
                }
            }
        }
    }
}

internal Chunk *chunk_new(Chunk_Manager *manager) {
    Chunk *result = manager->free_chunks.first;
    if (result) {
        DLLRemove(manager->free_chunks.first, manager->free_chunks.last, result, next, prev);
    } else {
        result = push_array(manager->arena, Chunk, 1);
        result->blocks = (Block_ID *)arena_push(manager->arena, sizeof(Block_ID) * CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE);
    }
    DLLPushBack(manager->loaded_chunks.first, manager->loaded_chunks.last, result, next, prev);
    manager->loaded_chunks.count += 1;
    return result;
}

internal void generate_sphere(Chunk *chunk);

internal Chunk *load_chunk_at(Chunk_Manager *manager, s32 x, s32 y, s32 z) {
    v3_s32 position = {x, y, z};
    Chunk *result = NULL;
    for (Chunk *chunk = manager->free_chunks.first; chunk; chunk = chunk->next) {
        if (chunk->position == position) {
            result = chunk;
            DLLRemove(manager->free_chunks.first, manager->free_chunks.last, chunk, next, prev);
            DLLPushBack(manager->loaded_chunks.first, manager->loaded_chunks.last, chunk, next, prev);
            manager->free_chunks.count--;
            break;
        }
    }
    return result;
}

internal void load_new_chunk_at(Chunk_Manager *manager, s32 x, s32 y, s32 z) {
    v3_s32 position = {x, y, z};
    bool found = false;
    for (Chunk *chunk = manager->loaded_chunks.first; chunk; chunk = chunk->next) {
        if (chunk->position == position) {
            found = true;
            break;
        }
    }

    if (!found) {
        Chunk *chunk = chunk_new(manager);
        chunk->position = position;
        generate_chunk(manager, world_generator, chunk);
        // generate_sphere(chunk);
    }
}

internal void generate_sphere(Chunk *chunk) {
    for (int z = 0; z < CHUNK_SIZE; z++) {
        for (int y = 0; y < CHUNK_SIZE; y++) {
           for (int x = 0; x < CHUNK_SIZE; x++) {
               if (sqrt((float)(x - CHUNK_SIZE / 2) * (x - CHUNK_SIZE / 2) + (y - CHUNK_SIZE / 2) * (y - CHUNK_SIZE / 2) + (z - CHUNK_SIZE / 2) * (z - CHUNK_SIZE / 2)) <= CHUNK_SIZE / 2) {
                   Block_ID *block = BLOCK_AT(chunk, x, y, z);
                   *block = BLOCK_STONE;
               }
           }
        }
    }
}

internal void update_chunk_load_list(Chunk_Manager *manager, v3_s32 chunk_position) {
    //@Note Free chunks
    for (Chunk *chunk = manager->loaded_chunks.first, *next = NULL; chunk; chunk = next) {
        next = chunk->next;
        DLLRemove(manager->loaded_chunks.first, manager->loaded_chunks.last, chunk, next, prev);
        DLLPushBack(manager->free_chunks.first, manager->free_chunks.last, chunk, next, prev);
    }

    load_chunk_at(manager, chunk_position.x, chunk_position.y, chunk_position.z);
    load_chunk_at(manager, chunk_position.x - 1, chunk_position.y, chunk_position.z);
    load_chunk_at(manager, chunk_position.x + 1, chunk_position.y, chunk_position.z);
    load_chunk_at(manager, chunk_position.x, chunk_position.y, chunk_position.z - 1);
    load_chunk_at(manager, chunk_position.x, chunk_position.y, chunk_position.z + 1);
    load_chunk_at(manager, chunk_position.x - 1, chunk_position.y, chunk_position.z - 1);
    load_chunk_at(manager, chunk_position.x - 1, chunk_position.y, chunk_position.z + 1);
    load_chunk_at(manager, chunk_position.x + 1, chunk_position.y, chunk_position.z - 1);
    load_chunk_at(manager, chunk_position.x + 1, chunk_position.y, chunk_position.z + 1);

    load_new_chunk_at(manager, chunk_position.x, chunk_position.y, chunk_position.z);
    load_new_chunk_at(manager, chunk_position.x - 1, chunk_position.y, chunk_position.z);
    load_new_chunk_at(manager, chunk_position.x + 1, chunk_position.y, chunk_position.z);
    load_new_chunk_at(manager, chunk_position.x, chunk_position.y, chunk_position.z - 1);
    load_new_chunk_at(manager, chunk_position.x, chunk_position.y, chunk_position.z + 1);
    load_new_chunk_at(manager, chunk_position.x - 1, chunk_position.y, chunk_position.z - 1);
    load_new_chunk_at(manager, chunk_position.x - 1, chunk_position.y, chunk_position.z + 1);
    load_new_chunk_at(manager, chunk_position.x + 1, chunk_position.y, chunk_position.z - 1);
    load_new_chunk_at(manager, chunk_position.x + 1, chunk_position.y, chunk_position.z + 1);
}

internal void deserialize_chunk(Chunk_Manager *manager, Chunk *chunk) {
    Arena *scratch = arena_alloc(get_malloc_allocator(), 128);
    String8 file_name = str8_pushf(scratch, "data/chunks/c.%d%d%d", chunk->position.x, chunk->position.y, chunk->position.z);
    OS_Handle file = os_open_file(file_name, OS_AccessFlag_Read);

    u8 *chunk_data = NULL;
    u64 data_size = os_read_entire_file(file, (void **)&chunk_data);
    Assert(data_size > 0);

    MemoryCopy((void *)&chunk->position, chunk_data, sizeof(chunk->position)); 
    MemoryCopy((void *)chunk->blocks, chunk_data + sizeof(chunk->position), sizeof(Block_ID) * CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE);

    arena_release(scratch);
}

internal void serialize_chunk(Chunk_Manager *manager, Chunk *chunk) {
    local_persist Arena *scratch = arena_alloc(get_virtual_allocator(), MB(1));
    
    String8 name = str8_pushf(scratch, "data/chunks/c.%d%d%d", chunk->position.x, chunk->position.y, chunk->position.z);
    OS_Handle file = os_open_file(name, OS_AccessFlag_Write);

    // arena_clear(scratch);

    u8 *buffer = (u8 *)scratch->current + scratch->current->pos;
    u8 *dst = buffer;

    for (int i = 0; i < 3; i++) {
        MemoryCopy(dst, (void *)&chunk->position.e[i], sizeof(chunk->position.e[i]));
        dst += sizeof(chunk->position.e[i]);
    }

    MemoryCopy(dst, chunk->blocks, sizeof(Block_ID) * CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE);
    dst += sizeof(Block_ID) * CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE;

    u64 size = dst - buffer;

    os_write_file(file, buffer, size);

    os_close_handle(file);
}

// internal void initialize_landscape(Chunk_Manager *manager) {
//     Chunk *chunk = chunk_new(manager);

//     deserialize_chunk(manager, chunk);

    // for (int z = 0; z < CHUNK_SIZE; z++) {
    //     for (int x = 0; x < CHUNK_SIZE; x++) {
    //         f64 height = (CHUNK_SIZE - 1) * get_height_map_value(&height_map, x, z);
    //         for (int y = 0; y < height; y++) {
    //             Block_ID *block = BLOCK_AT(chunk, x, y, z);
    //             if (y < 34) *block = BLOCK_STONE;
    //             else if (y < 60) *block = BLOCK_DIRT;
    //             else *block = BLOCK_GRASS;

    //             if (height - y <= 2) *block = BLOCK_GRASS;
    //         }
    //     }
    // }
    // serialize_chunk(manager, chunk);
// }

internal Texture_Region *find_texture_region(Texture_Atlas *atlas, String8 name) {
    Texture_Region *result = NULL;
    u64 hash = djb2_hash_string(name);
    Texture_Region_Bucket *region_bucket = atlas->region_hash_table + hash % atlas->region_hash_table_size;
    for (Texture_Region *region = region_bucket->first; region; region = region->hash_next) {
        if (str8_match(region->name, name, StringMatchFlag_Nil)) {
            result = region;
        }
    }
    return result;
}

internal void set_block_face(Block_Face *face, String8 texture_name, v4 color) {
    Texture_Region *region = find_texture_region(g_block_atlas, texture_name);
    face->texture_name = texture_name;
    face->texture_region = region;
    face->color = color;
}

internal void set_block_face_all(Block *block, String8 texture_name, v4 color) {
    Texture_Region *region = find_texture_region(g_block_atlas, texture_name);
    Assert(region);
    block->faces[FACE_TOP].texture_name = texture_name;
    block->faces[FACE_TOP].texture_region = region;
    block->faces[FACE_TOP].color = color;
    block->faces[FACE_BOTTOM] = block->faces[FACE_NORTH] = block->faces[FACE_SOUTH] = block->faces[FACE_EAST] = block->faces[FACE_WEST] = block->faces[FACE_TOP];
}

internal void load_blocks() {
    Arena *arena = arena_alloc(get_virtual_allocator(), MB(1));

    Arena *scratch = arena_alloc(get_malloc_allocator(), KB(64));

    // --------------------------------------
    //@Note Pack block/item textures
    // --------------------------------------
    g_block_atlas = push_array(arena, Texture_Atlas, 1);

    s32 texture_count = 0;
    Texture_Atlas *atlas = g_block_atlas;
    atlas->region_dim.x = MAX_ATLAS_X;
    atlas->region_dim.y = MAX_ATLAS_Y;
    atlas->dim.x = MAX_ATLAS_X * 64;
    atlas->dim.y = MAX_ATLAS_Y * 64;

    int max_texture_count = atlas->dim.x * atlas->dim.y;
    atlas->texture_regions = push_array(arena, Texture_Region, max_texture_count);
    atlas->region_hash_table_size = 128;
    atlas->region_hash_table = push_array(arena, Texture_Region_Bucket, atlas->region_hash_table_size);

    //@Note Blocks directory
    OS_File file;
    OS_File_List file_list = {};
    OS_Handle find_handle = os_find_first_file(scratch, str8_lit("data/assets/blocks"), &file);
    do {
        if (str8_match(file.file_name, str8_lit("."), StringMatchFlag_Nil) || str8_match(file.file_name, str8_lit(".."), StringMatchFlag_Nil)) continue;

        String8 ext = path_strip_extension(scratch, file.file_name);
        if (str8_match(ext, str8_lit("png"), StringMatchFlag_CaseInsensitive)) {
            file.file_name = path_join(scratch, str8_lit("data/assets/blocks"), file.file_name);
            OS_File_Node *node = push_array(scratch, OS_File_Node, 1);
            node->file = file;
            SLLQueuePush(file_list.first, file_list.last, node);
            file_list.count++;
        }
    } while (os_find_next_file(scratch, find_handle, &file));
    os_find_close(find_handle);

    //@Note Items directory
    find_handle = os_find_first_file(scratch, str8_lit("data/assets/items"), &file);
    do {
        if (str8_match(file.file_name, str8_lit("."), StringMatchFlag_Nil) || str8_match(file.file_name, str8_lit(".."), StringMatchFlag_Nil)) continue;

        String8 ext = path_strip_extension(scratch, file.file_name);
        if (str8_match(ext, str8_lit("png"), StringMatchFlag_CaseInsensitive)) {
            file.file_name = path_join(scratch, str8_lit("data/assets/items"), file.file_name);
            OS_File_Node *node = push_array(scratch, OS_File_Node, 1);
            node->file = file;
            SLLQueuePush(file_list.first, file_list.last, node);
            file_list.count++;
        }
    } while (os_find_next_file(scratch, find_handle, &file));
    os_find_close(find_handle);

    //@Note Load the textures
    for (OS_File_Node *file_node = file_list.first; file_node; file_node = file_node->next) {
        OS_File file = file_node->file;
        int x, y, n;
        u8 *data = stbi_load((char *)file.file_name.data, &x, &y, &n, 4);
        Assert(data);

        s32 atlas_x = texture_count % MAX_ATLAS_X;
        s32 atlas_y = texture_count / MAX_ATLAS_X;

        Texture_Region *region = &atlas->texture_regions[texture_count];
        region->name = path_strip_file_name(arena, file.file_name);
        region->offset = v2s32(atlas_x * 64, atlas_y * 64);
        region->dim = v2s32(64, 64);
        region->size = x * y * 4;
        region->data = data;

        u64 index = djb2_hash_string(region->name) % atlas->region_hash_table_size;
        Texture_Region_Bucket *bucket = atlas->region_hash_table + index;
        DLLPushBack(bucket->first, bucket->last, region, hash_next, hash_prev);

        texture_count++;
    }

    arena_release(scratch);

    //@Note Pack the textures into atlas
    const int bytes_per_pixel = 4;
    u8 *bitmap = push_array(arena, u8, atlas->dim.x * atlas->dim.y * bytes_per_pixel);
    for (int tex_idx = 0; tex_idx < texture_count; tex_idx++) {
        Texture_Region *region = &atlas->texture_regions[tex_idx];
        u8 *dst = bitmap + atlas->dim.x * region->offset.y * bytes_per_pixel + region->offset.x * bytes_per_pixel;
        u8 *src = region->data;

        for (int y = 0; y < region->dim.y; y++) {
            MemoryCopy(dst, src, region->dim.x * bytes_per_pixel);
            dst += atlas->dim.x * bytes_per_pixel;
            src += region->dim.x * bytes_per_pixel;
        }
    }
    atlas->texture_region_count = texture_count;
    atlas->tex_handle = d3d11_create_texture_mipmap(R_Tex2DFormat_R8G8B8A8, atlas->dim, bitmap);

    // --------------------------------------
    //@Note Configure all block types
    // --------------------------------------
    Block *blocks = g_basic_blocks;
    {
        Block *block = &blocks[BLOCK_STONE];
        set_block_face_all(block, str8_lit("stone.png"), make_v4(1.f, 1.f, 1.f, 1.f));
        block->step_type = STEP_STONE;
    }

    {
        Block *block = &blocks[BLOCK_DIRT];
        set_block_face_all(block, str8_lit("dirt.png"), make_v4(1.f, 1.f, 1.f, 1.f));
        block->step_type = STEP_DIRT;
    }

    {
        Block *block = &blocks[BLOCK_GRASS];
        set_block_face(block->faces + FACE_TOP, str8_lit("grass_block_top.png"), make_v4(0.62f, 0.95f, 0.47f, 1.f));
        set_block_face(block->faces + FACE_BOTTOM, str8_lit("dirt.png"), make_v4(1.f, 1.f, 1.f, 1.f));
        set_block_face(block->faces + FACE_NORTH, str8_lit("grass_block_side.png"), make_v4(1.f, 1.f, 1.f, 1.f));
        block->faces[FACE_SOUTH] = block->faces[FACE_EAST] = block->faces[FACE_WEST] = block->faces[FACE_NORTH];
        block->step_type = STEP_GRASS;
    }
}

internal void update_and_render(OS_Event_List *event_list, OS_Handle window_handle, f32 dt) {
    local_persist bool first_call = true;
    if (first_call) {
        first_call = false;

        Arena *font_arena = arena_alloc(get_virtual_allocator(), MB(4));
        default_fonts[FONT_DEFAULT] = load_font(font_arena, str8_lit("data/assets/fonts/consolas.ttf"), 16);

        ui_set_state(ui_state_new());

        load_blocks();

        //@Note World Generator
        {
            Arena *arena = arena_alloc(get_virtual_allocator(), MB(1));
            world_generator = push_array(arena, World_Generator, 1);
            world_generator->arena = arena;
        }
        init_world_generator(world_generator, 1337);
        update_world_generator(world_generator, 0, 0);

        //@Note Game State
        {
            Arena *arena = arena_alloc(get_virtual_allocator(), MB(4));
            game_state = push_array(arena, Game_State, 1);
            game_state->arena = arena;
            game_state->player = push_array(game_state->arena, Player, 1);
            game_state->player->inventory = push_array(game_state->arena, Inventory, 1);
            game_state->crafting_state = push_array(game_state->arena, Crafting_State, 1);
        }

        //@Note Initialize Chunk Manager
        {
            Arena *arena = arena_alloc(get_virtual_allocator(), MB(8));
            chunk_manager = push_array(arena, Chunk_Manager, 1);
            chunk_manager->arena = arena;
        }

        update_chunk_load_list(chunk_manager, {0, 0, 0});

        game_state->player->position = make_v3(0.f, (f32)get_height_from_chunk_xz(get_chunk_from_position(chunk_manager, {0, 0, 0}), 0, 0) + 1.0f, 0.f);

        //@Note Camera
        game_state->camera.up = make_v3(0.f, 1.f, 0.f);
        game_state->camera.right = make_v3(1.f, 0.f, 0.f);
        game_state->camera.forward = make_v3(0.f, 0.f, -1.f);
        //@Note Toward -Z axis
        game_state->camera.yaw = -90.0f;
        game_state->camera.pitch =  0.0f;
        game_state->camera.fov = 60.f;
    }

    input_begin(window_handle, event_list);

    v2 dim = os_get_window_dim(window_handle);
    game_state->client_dim = v2s32((s32)dim.x, (s32)dim.y);

    ui_begin_build(dt, window_handle, event_list);

    //@Note Chunk update
    v3_s32 chunk_position = get_chunk_position(game_state->player->position);
    chunk_position.y = 0;
    if (chunk_position.x != chunk_manager->chunk_position.x || chunk_position.z != chunk_manager->chunk_position.z) {
        update_chunk_load_list(chunk_manager, chunk_position);
        chunk_manager->chunk_position = chunk_position;
    }

    f32 forward_dt = 0.0f;
    f32 right_dt = 0.0f;
    
    if (key_down(OS_KEY_A)) {
        right_dt -= 1.0f;
    }
    if (key_down(OS_KEY_D)) {
        right_dt += 1.0f;
    }
    if (key_down(OS_KEY_W)) {
        forward_dt += 1.0f;
    }
    if (key_down(OS_KEY_S)) {
        forward_dt -= 1.0f;
    }

    if (key_pressed(OS_KEY_F4)) {
        game_state->creative_mode = !game_state->creative_mode;
    }

    if (key_pressed(OS_KEY_LEFTMOUSE)) {
        if (game_state->raycast_hit) {
            Block_ID *block = get_block_from_position(chunk_manager, game_state->raycast_p);
            if (block && block_active(block)) {
                *block = BLOCK_AIR;
            }
        }
    }

    if (key_pressed(OS_KEY_RIGHTMOUSE)) {
        if (game_state->raycast_hit) {
            Block_ID *block = get_block_from_position(chunk_manager, game_state->raycast_p + make_v3(0.f, 1.f, 0.f));
            *block = BLOCK_STONE;
        }
    }

    //@Note Player camera
    {
        f32 rotation_dp = 7.f;
        game_state->camera.yaw += rotation_dp * get_mouse_delta().x * dt;
        game_state->camera.yaw = (f32)fmod(game_state->camera.yaw, 360.0f);
        game_state->camera.pitch -= rotation_dp * get_mouse_delta().y * dt;
        game_state->camera.pitch = Clamp(game_state->camera.pitch, -89.0f, 89.0f);

        v3 direction;
        direction.x = cosf(DegToRad(game_state->camera.yaw)) * cosf(DegToRad(game_state->camera.pitch));
        direction.y = sinf(DegToRad(game_state->camera.pitch));
        direction.z = sinf(DegToRad(game_state->camera.yaw)) * cosf(DegToRad(game_state->camera.pitch));

        game_state->camera.forward = normalize_v3(direction);
        game_state->camera.right = normalize_v3(cross_v3(game_state->camera.forward, game_state->camera.up));

        game_state->camera.fov -= 5.0f * g_input.scroll_delta.y;
        game_state->camera.fov = Clamp(game_state->camera.fov, 1.0f, 60.0f);
    }

    //@Note Player physics
    if (game_state->creative_mode) {
        Player *player = game_state->player;
        player->velocity = v3_zero();
        f32 speed = 10.0f;
        v3 distance = game_state->camera.forward * forward_dt + game_state->camera.right * right_dt;
        v3 direction = normalize_v3(distance);
        player->position += speed * direction * dt;
    } else {
        Player *player = game_state->player;
        f32 G = 9.8f;
        f32 J = 25.f;
        f32 jump_time = 0.3f;

        const v3 normals[6] = {
            {0.f, 1.f, 0.f},
            {0.f, -1.f, 0.f},
            {1.f, 0.f, 0.f},
            {-1.f, 0.f, 0.f},
            {0.f, 0.f, 1.f},
            {0.f, 0.f, -1.f},
        };

        //@Note Grounded
        player->grounded = false;
        {
            v3 velocity = make_v3(0.f, -0.1f, 0.f);
            v3 new_position = player->position + velocity;
            for (f32 t = 0.0f; t <= 1.0f; t += 1.0f/64.0f) {
                v3 position = lerp_v3(player->position, new_position, t);
                v3 block_position = floor_v3(position);

                b32 collides = is_block_active_at(chunk_manager, block_position);
                if (collides) {
                    player->grounded = true;
                    // player->position = position;
                    player->position.y = block_position.y + 1.001f;
                    player->velocity.y = 0.f;
                    break;
                }
            }
        }

        v3 new_velocity = player->velocity;
        new_velocity.x = 0.f;
        new_velocity.z = 0.f;
        v3 forward = make_v3(game_state->camera.forward.x, 0.f, game_state->camera.forward.z);
        new_velocity += 4.f * normalize(forward * forward_dt + game_state->camera.right * right_dt);

        v3 new_position = player->position + player->velocity * dt;
        for (f32 t = 0.0f; t <= 1.0f; t += 1.0f/64.0f) {
            v3 position = lerp_v3(player->position, new_position, t);
            v3 block_position = floor_v3(position);
            b32 collides = is_block_active_at(chunk_manager, block_position);
            if (collides) {
                v3 direction = normalize(block_position - position);

                f32 max_dot = 0.f;
                int normal_idx = 0;
                for (int i = 0; i < ArrayCount(normals); i++) {
                    v3 normal = normals[i];
                    f32 d = dot(direction, normal);
                    if (d > max_dot) {
                        max_dot = d;   
                        normal_idx = i;
                    }
                }

                Assert(max_dot != 0.f);

                v3 collision_normal = normals[normal_idx];
                // printf("NORM: %f %f %f\n", collision_normal.x, collision_normal.y, collision_normal.z);
                // printf("from %f,%f,%f", new_position.x, new_position.y, new_position.z);
                new_position += max_dot * collision_normal * 0.01f;
                // new_velocity -= max_dot * collision_normal * 0.01f;
                // printf(" to %f,%f,%f", new_position.x, new_position.y, new_position.z);
           }
        }

        if (player->jumping && player->grounded) {
            player->jumping = false;
        }

        if (player->grounded && key_down(OS_KEY_SPACE)) {
            player->jumping = true;
            player->jump_t = 0.f;
            new_velocity.y += J * dt;
        }

        if (player->jumping) {
            new_velocity.y += J * dt;

            if (player->jump_t >= jump_time) {
                player->jumping = false;
            }
            player->jump_t += dt;
        }

        if (player->grounded) {
            // player->velocity.y = 0.f;
        } else {
            new_velocity.y -= G * dt;
        }

        player->velocity = new_velocity;
        player->position = new_position;
    }

    game_state->camera.position = game_state->player->position + make_v3(0.f, 1.0f, 0.f); 

    {
        game_state->raycast_hit = false;
        v3 origin = game_state->camera.position;
        v3 direction = game_state->camera.forward;
        f32 t = 0.0f;
        f32 max_d = 12.0f;
        while (t <= max_d) {
            v3 p = origin + t * direction;
            Block_ID *block = get_block_from_position(chunk_manager, p);
            if (block && block_active(block)) {
                game_state->raycast_hit = true;
                game_state->raycast_p = floor_v3(p);
                break;
            }
            t += 1.0f;
        }
    }

    game_state->camera.view_matrix = look_at_rh(game_state->camera.position, game_state->camera.position + game_state->camera.forward, game_state->camera.up);

    draw_begin(window_handle);

    m4 ortho_projection = ortho_rh_zo(0.f, dim.x, 0.f, dim.y, -1.f, 1.f);
    m4 projection  = perspective_projection_rh(DegToRad(game_state->camera.fov), dim.x/dim.y, 0.1f, 1000.0f);
    m4 view        = game_state->camera.view_matrix;

    draw_3d_mesh_begin(projection, view, g_block_atlas->tex_handle, R_RasterizerState_Default);

    for (Chunk *chunk = chunk_manager->loaded_chunks.first; chunk; chunk = chunk->next) {
        draw_chunk(chunk);
    }

    //@Note draw raycast block
    draw_3d_mesh_begin(projection, view, r_handle_zero(), R_RasterizerState_Wireframe);
    if (game_state->raycast_hit) {
        draw_cube(game_state->raycast_p, make_rect(0, 0, 0, 0), make_v4(1, 1, 1, 1), FACE_MASK_NIL);
    }

    draw_set_xform(ortho_projection);
    draw_rect(make_rect(0.5f*dim.x - 5.f, 0.5f*dim.y - 5.f, 10.f, 10.f), make_v4(1.f, 1.f, 0.f, 1.f));

    //@Note Hot bar
    v2 hotbar_dim = make_v2(450.0f, 50.0f);
    Rect hotbar_rect = make_rect((dim.x - hotbar_dim.x) * 0.5f, 0.f, hotbar_dim.x, hotbar_dim.y);
    Rect src = make_rect(0.f, 5.f/16.f, 1.f/16.f, 1.f/16.f);
    draw_rect(hotbar_rect, make_v4(0.f, 0.f, 0.f, 1.f));
    v2 slot_dim = make_v2(hotbar_dim.x / (f32)HOTBAR_SLOTS, hotbar_dim.y);
    for (int i = 0; i < HOTBAR_SLOTS; i++) {
        Texture_Atlas *atlas = g_block_atlas;
        Block *block = &g_basic_blocks[BLOCK_DIRT];
        Block_Face *face = &block->faces[FACE_TOP];
        Rect src = make_rect(face->texture_region->offset.x / (f32)atlas->dim.x, face->texture_region->offset.y / (f32)atlas->dim.y, face->texture_region->dim.x / (f32)atlas->dim.x, face->texture_region->dim.y / (f32)atlas->dim.y);
        Rect rect = make_rect(hotbar_rect.x0 + slot_dim.x * i, hotbar_rect.y0, slot_dim.x, slot_dim.y);
        draw_quad(g_block_atlas->tex_handle, rect, src);
        draw_rect_outline(rect, make_v4(1.f, 1.f, 1.f, 1.f));
    }
    draw_rect_outline(hotbar_rect, make_v4(1.f, 1.f, 1.f, 1.f));

    //@DEBUG
    ui_labelf("delta: %fms", 1000.0 * dt);
    // ui_labelf("chunk position: %d, %d, %d", chunk_manager->chunk_position.x, chunk_manager->chunk_position.y, chunk_manager->chunk_position.z);
    ui_labelf("world position: %.2f, %.2f, %.2f", game_state->player->position.x, game_state->player->position.y, game_state->player->position.z);
    ui_labelf("%s", game_state->player->grounded ? "GROUNDED" : "NOT GROUNDED");
    ui_labelf("velocity: %.3f %.3f %.3f", game_state->player->velocity.x, game_state->player->velocity.y, game_state->player->velocity.z);

    ui_layout_apply(ui_root());

    draw_ui_layout(ui_root());

    d3d11_render(window_handle, draw_bucket);

    draw_end();

    ui_end_build();

    input_end(window_handle);
}
