global World_Generator *world_generator;

global Game_State *game_state;

global Texture_Atlas *g_block_atlas;
global R_Handle icon_tex;
global R_Handle sun_tex;

global R_Handle mining_textures[10];
global R_Handle moon_tex;

global int max_worker_threads = 16;
global int worker_threads = 0;

internal Ray make_ray(V3_F64 origin, V3_F32 direction) {
    Ray result;
    result.origin = origin;
    result.direction = direction;
    return result;
}

inline internal V3_F32 v3_f32_from_face(Face face) {
    switch (face) {
    default:
        return V3_Zero;
    case FACE_TOP:
        return V3_Up;
    case FACE_BOTTOM:
        return V3_Down;
    case FACE_NORTH:
        return V3_Forward;
    case FACE_SOUTH:
        return V3_Back;
    case FACE_EAST:
        return V3_Right;
    case FACE_WEST:
        return V3_Left;
    }
}

inline internal char *face_to_string(Face face) {
    char *result = NULL;
    switch (face) {
    case FACE_WEST:
        result = "WEST";
        break;
    case FACE_EAST:
        result = "EAST";
        break;
    case FACE_NORTH:
        result = "NORTH";
        break;
    case FACE_SOUTH:
        result = "SOUTH";
        break;
    case FACE_TOP:
        result = "TOP";
        break;
    case FACE_BOTTOM:
        result = "BOTTOM";
        break;
    }
    return result;
}

internal Chunk *get_chunk_from_position(Chunk_Manager *manager, V3_S32 position) {
    Chunk *result = NULL;
    for (Chunk *chunk = manager->loaded_chunks.first; chunk; chunk = chunk->next) {
        if (chunk->position.x == position.x && chunk->position.z == position.z) {
            result = chunk;
            break;
        }
    }
    return result;
}

internal Block_ID *get_block_from_position(Chunk_Manager *manager, s32 x, s32 y, s32 z) {
    Block_ID *result = block_id_zero();

    V3_S32 chunk_p = get_chunk_position(x, y, z);
    Chunk *chunk = get_chunk_from_position(manager, chunk_p);
    if (chunk) {
        V3_S32 block_p = get_chunk_relative_position(chunk, x, y, z);
        result = block_at(chunk, block_p.x, block_p.y, block_p.z);
    }

    return result;
}

inline internal bool is_block_active_at(Chunk_Manager *manager, s32 x, s32 y, s32 z) {
    Block_ID *block = get_block_from_position(manager, x, y, z);
    return block_is_active(*block);
}

internal s32 get_height_from_chunk_xz(Chunk *chunk, s32 x, s32 z) {
    s32 result = 0;
    for (int y = CHUNK_HEIGHT - 1; y >= 0; y--) {
        Block_ID *block = block_at(chunk, x, y, z);
        if (block_is_active(*block)) {
            result = y;
            break;
        }
    }
    return result;
}

internal R_Handle load_texture(String8 file_name) {
    int x, y, n;
    u8 *data = stbi_load((char *)file_name.data, &x, &y, &n, 4);
    R_Handle tex = d3d11_create_texture(R_Tex2DFormat_R8G8B8A8, v2_s32(x, y), data);
    stbi_image_free(data);
    return tex;
}

internal Texture_Map load_texture_map(String8 file_name) {
    int x, y, n;
    u8 *data = stbi_load((char *)file_name.data, &x, &y, &n, 1);
    R_Handle tex = d3d11_create_texture(R_Tex2DFormat_R8, v2_s32(x, y), data);

    Texture_Map result = {};
    result.texture = tex;
    result.size = v2_s32(x, y);
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

DWORD WINAPI update_chunk_thread(LPVOID lpParam) {
    Chunk *chunk = (Chunk *)lpParam;
    load_chunk_mesh(chunk);
    // printf("DONE LOADING MESH");
    return 0;
}

DWORD WINAPI generate_chunk_thread(LPVOID lpParam) {
    s64 start = get_wall_clock();
    Chunk *chunk = (Chunk *)lpParam;
    generate_chunk(chunk_manager, world_generator, chunk);
    s64 end = get_wall_clock();

    worker_threads--;
    printf("GENERATED CHUNK: %fms\n", get_ms_elapsed(start, end));
    return 0;
}

internal Chunk *chunk_new(Chunk_Manager *manager) {
    Chunk *result = manager->free_chunks.first;
    if (result) {
        DLLRemove(manager->free_chunks.first, manager->free_chunks.last, result, next, prev);
        manager->free_chunks.count--;
    } else {
        result = push_array(manager->arena, Chunk, 1);
        result->blocks = (Block_ID *)arena_push(manager->arena, sizeof(Block_ID) * CHUNK_BLOCKS);
        result->light_map = (u8 *)arena_push(manager->arena, sizeof(u8) * CHUNK_BLOCKS);
    }
    DLLPushBack(manager->loaded_chunks.first, manager->loaded_chunks.last, result, next, prev);
    manager->loaded_chunks.count++;
    return result;
}

internal Chunk *load_chunk_at(Chunk_Manager *manager, s32 x, s32 y, s32 z) {
    V3_S32 position = {x, y, z};
    Chunk *result = NULL;
    for (Chunk *chunk = manager->free_chunks.first; chunk; chunk = chunk->next) {
        if (chunk->position.x == x && chunk->position.z == z) {
            result = chunk;
            DLLRemove(manager->free_chunks.first, manager->free_chunks.last, chunk, next, prev);
            DLLPushBack(manager->loaded_chunks.first, manager->loaded_chunks.last, chunk, next, prev);
            manager->free_chunks.count--;
            manager->loaded_chunks.count++;
            break;
        }
    }
    return result;
}

internal void load_new_chunk_at(Chunk_Manager *manager, s32 x, s32 y, s32 z) {
    bool found = false;
    for (Chunk *chunk = manager->loaded_chunks.first; chunk; chunk = chunk->next) {
        if (chunk->position.x == x && chunk->position.z == z) {
            found = true;
            break;
        }
    }

    if (!found) {
        Chunk *chunk = chunk_new(manager);
        MemoryZero(chunk->blocks, CHUNK_BLOCKS * sizeof(Block_ID));
        chunk->position = v3_s32(x, 0, z);
        // chunk->opaque_geo.reserve(CHUNK_BLOCKS * sizeof(u64));
        // chunk->transparent_geo.reserve(CHUNK_BLOCKS * sizeof(u64));
        chunk->flags = CHUNK_FLAG_NIL;
        chunk->flags |= CHUNK_FLAG_DIRTY;
    }
}


internal void update_chunk_load_list(Chunk_Manager *manager, V3_S32 chunk_position) {
    //@Note Free chunks
    for (Chunk *chunk = manager->loaded_chunks.first, *next = NULL; chunk; chunk = next) {
        next = chunk->next;
        DLLRemove(manager->loaded_chunks.first, manager->loaded_chunks.last, chunk, next, prev);
        DLLPushBack(manager->free_chunks.first, manager->free_chunks.last, chunk, next, prev);
        manager->loaded_chunks.count--;
        manager->free_chunks.count++;
    }

    // load_chunk_at(manager, chunk_position.x,  0, chunk_position.z);
    // load_new_chunk_at(manager, chunk_position.x, 0, chunk_position.z);
    V3_S32 dim = v3_s32(4, 0, 4);
    s32 min_x = chunk_position.x - (s32)(0.5f * dim.x);
    s32 max_x = chunk_position.x + (s32)(0.5f * dim.x);
    s32 min_z = chunk_position.z - (s32)(0.5f * dim.z);
    s32 max_z = chunk_position.z + (s32)(0.5f * dim.z);
    for (s32 x = min_x; x < max_x; x++) {
        for (s32 z = min_z; z < max_z; z++) {
            load_chunk_at(manager, x, 0, z);
            load_new_chunk_at(manager, x, 0, z);
        }
    }
}

internal void deserialize_chunk(Chunk_Manager *manager, Chunk *chunk) {
    Arena *scratch = arena_alloc(get_malloc_allocator(), 128);
    String8 file_name = str8_pushf(scratch, "data/chunks/c.%d%d%d", chunk->position.x, chunk->position.y, chunk->position.z);
    OS_Handle file = os_open_file(file_name, OS_AccessFlag_Read);

    u8 *chunk_data = NULL;
    u64 data_size = os_read_entire_file(file, (void **)&chunk_data);
    Assert(data_size > 0);

    MemoryCopy((void *)&chunk->position, chunk_data, sizeof(chunk->position)); 
    MemoryCopy((void *)chunk->blocks, chunk_data + sizeof(chunk->position), sizeof(Block_ID) * CHUNK_HEIGHT * CHUNK_DEPTH);

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

    MemoryCopy(dst, chunk->blocks, sizeof(Block_ID) * CHUNK_HEIGHT * CHUNK_DEPTH);
    dst += sizeof(Block_ID) * CHUNK_HEIGHT * CHUNK_DEPTH;

    u64 size = dst - buffer;

    os_write_file(file, buffer, size);

    os_close_handle(file);
}

internal void set_block_face(Block_Face *face, String8 texture_name, u8 color_id) {
    Texture_Region *region = find_texture_region(g_block_atlas, texture_name);
    face->texture_name = texture_name;
    face->texture_region = region;
    face->color_id = color_id;
}

internal void set_block_face_all(Block *block, String8 texture_name, u8 color_id) {
    Texture_Region *region = find_texture_region(g_block_atlas, texture_name);
    Assert(region);
    block->faces[FACE_TOP].texture_name = texture_name;
    block->faces[FACE_TOP].texture_region = region;
    block->faces[FACE_TOP].color_id = color_id;
    block->faces[FACE_BOTTOM] = block->faces[FACE_NORTH] = block->faces[FACE_SOUTH] = block->faces[FACE_EAST] = block->faces[FACE_WEST] = block->faces[FACE_TOP];
}

internal void load_blocks() {
    Arena *arena = arena_alloc(get_virtual_allocator(), KB(256));

    Arena *scratch = arena_alloc(get_virtual_allocator(), MB(1));

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

    int max_texture_count = atlas->region_dim.x * atlas->region_dim.y;
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
        region->offset = v2_f32(atlas_x / (f32)MAX_ATLAS_X, atlas_y / (f32)MAX_ATLAS_Y);
        region->dim = v2_f32(1.0f / MAX_ATLAS_X, 1.0f / MAX_ATLAS_Y);
        region->size = x * y * 4;
        region->data = data;
        region->atlas_index = texture_count;

        u64 index = djb2_hash_string(region->name) % atlas->region_hash_table_size;
        Texture_Region_Bucket *bucket = atlas->region_hash_table + index;
        DLLPushBack(bucket->first, bucket->last, region, hash_next, hash_prev);

        texture_count++;
    }

    //@Note Pack the textures into atlas
    const int bytes_per_pixel = 4;
    u8 *bitmap = push_array(scratch, u8, atlas->dim.x * atlas->dim.y * bytes_per_pixel);
    for (int tex_idx = 0; tex_idx < texture_count; tex_idx++) {
        Texture_Region *region = &atlas->texture_regions[tex_idx];
        int src_pitch = atlas->region_dim.x * bytes_per_pixel;
        int dst_pitch = atlas->dim.x * bytes_per_pixel;

        u8 *dst = bitmap + (int)(region->offset.y * atlas->dim.x * bytes_per_pixel) + (int)(region->offset.x * atlas->dim.x * bytes_per_pixel);
        u8 *src = region->data;

        for (int y = 0; y < (int)atlas->dim.y * region->dim.y; y++) {
            MemoryCopy(dst, src, src_pitch);
            dst += dst_pitch;
            src += src_pitch;
        }

        stbi_image_free(region->data);
    }
    atlas->texture_region_count = texture_count;
    atlas->tex_handle = d3d11_create_texture_mipmap(R_Tex2DFormat_R8G8B8A8, atlas->dim, bitmap);

    d3d11_upload_block_atlas(atlas);
    d3d11_upload_color_table();

    arena_release(scratch);
    // arena_release(arena);

    // --------------------------------------
    //@Note Configure all block types
    // --------------------------------------
    Block *blocks = g_basic_blocks;

    {
        Block *block = &blocks[BLOCK_AIR];
        block->flags |= BLOCK_FLAG_TRANSPARENT;
    }

    {
        Block *block = &blocks[BLOCK_STONE];
        set_block_face_all(block, str8_lit("stone.png"), 0);
        block->step_type = STEP_STONE;
    }

    {
        Block *block = &blocks[BLOCK_DIRT];
        set_block_face_all(block, str8_lit("dirt.png"), 0);
        block->step_type = STEP_DIRT;
    }

    {
        Block *block = &blocks[BLOCK_GRASS];
        set_block_face(block->faces + FACE_TOP, str8_lit("grass_block_top.png"), 5);
        set_block_face(block->faces + FACE_BOTTOM, str8_lit("dirt.png"), 0);
        set_block_face(block->faces + FACE_NORTH, str8_lit("grass_block_side.png"), 0);
        block->faces[FACE_SOUTH] = block->faces[FACE_EAST] = block->faces[FACE_WEST] = block->faces[FACE_NORTH];
        block->step_type = STEP_GRASS;
    }

    {
        Block *block = &blocks[BLOCK_GRASS_PLANT];
        set_block_face_all(block, str8_lit("grass.png"), 5) ; 
        block->flags |= BLOCK_FLAG_TRANSPARENT;
    }

    {
        Block *block = &blocks[BLOCK_SAND];
        set_block_face_all(block, str8_lit("sand.png"), 0);
        block->step_type = STEP_SAND;
    }

    {
        Block *block = &blocks[BLOCK_LOG];
        set_block_face(block->faces + FACE_TOP, str8_lit("oak_log_top.png"), 0);
        set_block_face(block->faces + FACE_NORTH, str8_lit("oak_log.png"), 0);
        block->faces[FACE_SOUTH] = block->faces[FACE_EAST] = block->faces[FACE_WEST] = block->faces[FACE_NORTH];
        block->faces[FACE_BOTTOM] = block->faces[FACE_TOP];
    }

    {
        Block *block = &blocks[BLOCK_WOOD];
        set_block_face_all(block, str8_lit("oak_planks.png"), 0);
    }

    {
        Block *block = &blocks[BLOCK_LEAVES];
        set_block_face_all(block, str8_lit("oak_leaves.png"), 3);
        block->flags |= BLOCK_FLAG_TRANSPARENT;
    }

    {
        Block *block = &blocks[BLOCK_WATER];
        set_block_face_all(block, str8_lit("underwater.png"), 6);
        block->flags |= BLOCK_FLAG_TRANSPARENT;
    }

    {
        Block *block = &blocks[BLOCK_BEDROCK];
        set_block_face_all(block, str8_lit("bedrock.png"), 0);
    }

    {
        Block *block = &blocks[BLOCK_TORCH];
        set_block_face_all(block, str8_lit("torch.png"), 0);
        block->flags |= BLOCK_FLAG_EMITS_LIGHT;
    }

    {
        Block *block = &blocks[BLOCK_GLOWSTONE];
        set_block_face_all(block, str8_lit("glowstone.png"), 0);
        block->flags |= BLOCK_FLAG_EMITS_LIGHT;
    }


}

#define SIGN(x) (x > 0 ? 1 : (x < 0 ? -1 : 0))
#define FRAC0(x) (x - floor(x))
#define FRAC1(x) (1 - x + floor(x))

internal bool voxel_raycast(Chunk_Manager *manager, V3_F64 origin, V3_F32 direction, f32 max_d, Voxel_Raycast *result) {
    result->ray = make_ray(origin, direction);
    result->hit = V3_Zero;
    result->block = block_id_zero();
    result->face = FACE_TOP;

    f64 tMaxX, tMaxY, tMaxZ, tDeltaX, tDeltaY, tDeltaZ;
    V3_S32 voxel;

    f64 x1, y1, z1; // start point   
    f64 x2, y2, z2; // end point   

    V3_F64 end_p = origin;
    end_p.x += (f64)(direction.x * max_d);
    end_p.y += (f64)(direction.y * max_d);
    end_p.z += (f64)(direction.z * max_d);

    x1 = origin.x;
    y1 = origin.y;
    z1 = origin.z;

    x2 = end_p.x;
    y2 = end_p.y;
    z2 = end_p.z;

    int dx = SIGN(x2 - x1);
    if (dx != 0) tDeltaX = fmin(dx / (x2 - x1), 10000000.0); else tDeltaX = 10000000.0;
    if (dx > 0) tMaxX = tDeltaX * FRAC1(x1); else tMaxX = tDeltaX * FRAC0(x1);
    voxel.x = (int)floor(x1);

    int dy = SIGN(y2 - y1);
    if (dy != 0) tDeltaY = fmin(dy / (y2 - y1), 10000000.0); else tDeltaY = 10000000.0;
    if (dy > 0) tMaxY = tDeltaY * FRAC1(y1); else tMaxY = tDeltaY * FRAC0(y1);
    voxel.y = (int)floor(y1);

    int dz = SIGN(z2 - z1);
    if (dz != 0) tDeltaZ = fmin(dz / (z2 - z1), 10000000.0); else tDeltaZ = 10000000.0;
    if (dz > 0) tMaxZ = tDeltaZ * FRAC1(z1); else tMaxZ = tDeltaZ * FRAC0(z1);
    voxel.z = (int)floor(z1);

    while (true) {
        if (tMaxX < tMaxY) {
            if (tMaxX < tMaxZ) {
                voxel.x += dx;
                tMaxX += tDeltaX;
                result->face = (dx < 0) ? FACE_EAST : FACE_WEST;
            } else {
                voxel.z += dz;
                tMaxZ += tDeltaZ;
                result->face = (dz < 0) ? FACE_NORTH : FACE_SOUTH;
            }
        } else {
            if (tMaxY < tMaxZ) {
                voxel.y += dy;
                tMaxY += tDeltaY;
                result->face = (dy < 0) ? FACE_TOP : FACE_BOTTOM;
            } else {
                voxel.z += dz;
                tMaxZ += tDeltaZ;
                result->face = (dz < 0) ? FACE_NORTH : FACE_SOUTH;
            }
        }
        if (tMaxX > 1 && tMaxY > 1 && tMaxZ > 1) break;

        // process voxel here
        Block_ID *block = get_block_from_position(manager, voxel.x, voxel.y, voxel.z);
        if (block_is_active(*block)) {
            result->block = block;
            result->hit.x = (f64)voxel.x;
            result->hit.y = (f64)voxel.y;
            result->hit.z = (f64)voxel.z;
            return true;
        }
    }

    return false;
}

internal void update_and_render(OS_Event_List *event_list, OS_Handle window_handle, f32 dt) {
    local_persist bool first_call = true;
    if (first_call) {
        first_call = false;

        V2_F32 dim = os_get_window_dim(window_handle);

        Arena *font_arena = arena_alloc(get_virtual_allocator(), MB(4));
        default_fonts[FONT_DEFAULT] = load_font(font_arena, str8_lit("data/assets/fonts/consolas.ttf"), 20);

        ui_set_state(ui_state_new());

        load_blocks();

        icon_tex = load_texture(str8_lit("data/assets/icons.png"));
        sun_tex = load_texture(str8_lit("data/assets/environment/sun.png"));
        moon_tex = load_texture(str8_lit("data/assets/environment/moon_phases.png"));

        mining_textures[0] = load_texture(str8_lit("data/assets/misc/destroy_stage_0.png"));
        mining_textures[1] = load_texture(str8_lit("data/assets/misc/destroy_stage_1.png"));
        mining_textures[2] = load_texture(str8_lit("data/assets/misc/destroy_stage_2.png"));
        mining_textures[3] = load_texture(str8_lit("data/assets/misc/destroy_stage_3.png"));
        mining_textures[4] = load_texture(str8_lit("data/assets/misc/destroy_stage_4.png"));
        mining_textures[5] = load_texture(str8_lit("data/assets/misc/destroy_stage_5.png"));
        mining_textures[6] = load_texture(str8_lit("data/assets/misc/destroy_stage_6.png"));
        mining_textures[7] = load_texture(str8_lit("data/assets/misc/destroy_stage_7.png"));
        mining_textures[8] = load_texture(str8_lit("data/assets/misc/destroy_stage_8.png"));
        mining_textures[9] = load_texture(str8_lit("data/assets/misc/destroy_stage_9.png"));

        //@Note World Generator
        {
            Arena *arena = arena_alloc(get_virtual_allocator(), MB(1));
            world_generator = push_array(arena, World_Generator, 1);
            world_generator->arena = arena;
        }

        srand((unsigned int)time(NULL));
        init_world_generator(world_generator, rand());

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
            Arena *arena = arena_alloc(get_virtual_allocator(), MB(64));
            chunk_manager = push_array(arena, Chunk_Manager, 1);
            chunk_manager->arena = arena;
        }

        update_chunk_load_list(chunk_manager, V3_Zero);

        game_state->player->position = V3_Zero;
        game_state->player->position.y = 100.0;
        game_state->player->mining = block_id_zero();

        //@Note Camera
        game_state->camera.up = V3_Up;
        game_state->camera.right = V3_Right;
        game_state->camera.forward = V3_Back;
        //@Note Toward -Z axis
        game_state->camera.yaw = -90.0f;
        game_state->camera.pitch =  0.0f;
        game_state->camera.fov = 70.f;
        game_state->camera.aspect = 1024.0f/ 768.0f;

        game_state->frustum = make_frustum(game_state->camera, 0.1f, 1200.0f);

        game_state->ticks_per_second = 10;
        game_state->ticks_per_day = SECONDS_PER_DAY * game_state->ticks_per_second;

        game_state->hour = 12;
        game_state->day_t = (int)(game_state->hour * (game_state->ticks_per_day / 24.0f));
    }

    input_begin(window_handle, event_list);

    V2_F32 dim = os_get_window_dim(window_handle);
    game_state->client_dim = v2_s32((s32)dim.x, (s32)dim.y);

    ui_begin_build(dt, window_handle, event_list);

    Player *player = game_state->player;

    f32 forward_dt = 0.0f;
    f32 right_dt = 0.0f;
    f32 up_dt = 0.0f;
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
    if (key_down(OS_KEY_Q)) {
        up_dt += 1.0f;
    }
    if (key_down(OS_KEY_E)) {
        up_dt -= 1.0f;
    }

    //@Note Player camera
    {
        f32 rotation_dp = 7.f;
        game_state->camera.yaw += rotation_dp * get_mouse_delta().x * dt;
        game_state->camera.yaw = (f32)fmod(game_state->camera.yaw, 360.0f);
        game_state->camera.pitch -= rotation_dp * get_mouse_delta().y * dt;
        game_state->camera.pitch = Clamp(game_state->camera.pitch, -89.0f, 89.0f);

        V3_F32 direction;
        direction.x = cosf(DegToRad(game_state->camera.yaw)) * cosf(DegToRad(game_state->camera.pitch));
        direction.y = sinf(DegToRad(game_state->camera.pitch));
        direction.z = sinf(DegToRad(game_state->camera.yaw)) * cosf(DegToRad(game_state->camera.pitch));

        game_state->camera.forward = normalize_v3_f32(direction);
        game_state->camera.right = normalize_v3_f32(cross_v3_f32(game_state->camera.forward, game_state->camera.up));

        game_state->camera.fov -= 5.0f * g_input.scroll_delta.y;
        game_state->camera.fov = Clamp(game_state->camera.fov, 1.0f, 60.0f);
    }

    //@Note Player physics
    if (true) {
        player->velocity = V3_Zero;
        f32 speed = 10.0f;

        if (key_down(OS_KEY_SPACE)) {
            speed = 40.0f; 
        }

        V3_F32 direction = normalize_v3_f32(game_state->camera.forward * forward_dt + game_state->camera.right * right_dt + game_state->camera.up * up_dt);
        V3_F32 distance = speed * direction * dt;
        player->position += v3_f64(distance.x, distance.y, distance.z);
    }
#if 0
    else {
        Player *player = player;
        f32 G = 9.8f;
        f32 J = 25.f;
        f32 jump_time = 0.3f;

        const V3_F32 normals[6] = {
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
            V3_F32 velocity = make_v3_f32(0.f, -0.1f, 0.f);
            V3_F32 new_position = player->position + velocity;
            for (f32 t = 0.0f; t <= 1.0f; t += 1.0f/64.0f) {
                V3_F32 position = lerp_v3_f32(player->position, new_position, t);
                V3_F32 block_position = floor_v3_f32(position);

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

        V3_F32 new_velocity = player->velocity;
        new_velocity.x = 0.f;
        new_velocity.z = 0.f;
        V3_F32 forward = make_v3_f32(game_state->camera.forward.x, 0.f, game_state->camera.forward.z);
        new_velocity += 4.f * normalize(forward * forward_dt + game_state->camera.right * right_dt);

        V3_F32 new_position = player->position + player->velocity * dt;
        for (f32 t = 0.0f; t <= 1.0f; t += 1.0f/64.0f) {
            V3_F32 position = lerp_v3_f32(player->position, new_position, t);
            V3_F32 block_position = floor_v3_f32(position);
            b32 collides = is_block_active_at(chunk_manager, block_position);
            if (collides) {
                V3_F32 direction = normalize(block_position - position);

                f32 max_dot = 0.f;
                int normal_idx = 0;
                for (int i = 0; i < ArrayCount(normals); i++) {
                    V3_F32 normal = normals[i];
                    f32 d = dot(direction, normal);
                    if (d > max_dot) {
                        max_dot = d;   
                        normal_idx = i;
                    }
                }

                Assert(max_dot != 0.f);

                V3_F32 collision_normal = normals[normal_idx];
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
#endif

    //@Note Chunk update
    V3_S32 chunk_position = get_chunk_position(player->position);
    chunk_position.y = 0;
    if (chunk_position.x != chunk_manager->chunk_position.x || chunk_position.z != chunk_manager->chunk_position.z) {
        update_chunk_load_list(chunk_manager, chunk_position);
        chunk_manager->chunk_position = chunk_position;
    }


    for (Chunk *chunk = chunk_manager->loaded_chunks.first; chunk; chunk = chunk->next) {
        if (!(chunk->flags & CHUNK_FLAG_GENERATED) && !(chunk->flags & CHUNK_FLAG_GENERATING)) {
            // generate_chunk(chunk_manager, world_generator, chunk);
            if (worker_threads + 1 < max_worker_threads) {
                worker_threads++;
                DWORD thread_id;
                CreateThread(NULL, 0, generate_chunk_thread, (void *)chunk, 0, &thread_id);
                printf("GENERATING ON THREAD %d\n", thread_id);
            }
        }
    }

    voxel_raycast(chunk_manager, game_state->camera.position, game_state->camera.forward, 20.0f, &player->raycast);

    if (key_pressed(OS_KEY_F1)) {
        game_state->mesh_debug = !game_state->mesh_debug;
    }
    if (key_pressed(OS_KEY_F4)) {
        game_state->creative_mode = !game_state->creative_mode;
    }

    if (key_up(OS_KEY_LEFTMOUSE)) {
        player->mining_t = 0;
        player->mining = block_id_zero();
    }

    if (key_down(OS_KEY_LEFTMOUSE)) {
        if (player->raycast.block != block_id_zero()) {
            Assert(block_is_active(*player->raycast.block));
            if (player->mining != player->raycast.block) {
                player->mining = player->raycast.block;
                player->mining_t = 0;
                player->mining_target_t = block_mining_time(player->raycast.block);
            }

            player->mining_t += dt;
            if (player->mining_t >= player->mining_target_t) {
                player->mining_t = 0;
                block_place(player->mining, BLOCK_AIR);

                Chunk *chunk = get_chunk_from_position(chunk_manager, get_chunk_position(player->position));
                chunk->flags |= CHUNK_FLAG_DIRTY;
            }
        }
    }

    if (key_pressed(OS_KEY_RIGHTMOUSE)) {
        //@Note Interact
        V3_F32 face_normal = v3_f32_from_face(player->raycast.face);
        V3_F64 new_block_position = player->raycast.hit;
        new_block_position.x += face_normal.x;
        new_block_position.y += face_normal.y;
        new_block_position.z += face_normal.z;
        Block_ID *block = get_block_from_position(chunk_manager, (s32)new_block_position.x, (s32)new_block_position.y, (s32)new_block_position.z);
        block_place(block, BLOCK_GLOWSTONE);

        Chunk *chunk = get_chunk_from_position(chunk_manager, get_chunk_position(player->position));
        chunk->flags |= CHUNK_FLAG_DIRTY;
    }

    int ticks_elapsed = 10;
    game_state->ticks_per_day = SECONDS_PER_DAY * game_state->ticks_per_second;


    //@Note Day time
    game_state->day_t += ticks_elapsed;
    if (game_state->day_t > game_state->ticks_per_day) {
        game_state->day_t = 0;
    }

    {
        int hour = (int)(24 * game_state->day_t / (f32)game_state->ticks_per_day);
        //@Todo Update light map
        // if (hour != game_state->hour) {
        // }
        game_state->hour = hour;
    }


    if (player->raycast.block != player->mining) {
        player->mining_t = 0;
    }

    //@Todo Make camera position player position offset?
    // So that basically we pass player position relative to the chunk so everything is in world space
    game_state->camera.position = player->position + 1.5f * v3_f64(0, 1, 0);

    //@Note Update Frustum
    game_state->frustum = make_frustum(game_state->camera, 0.1f, 1000.0f);

    {
        for (Chunk *chunk = chunk_manager->loaded_chunks.first; chunk; chunk = chunk->next) {
            V3_F32 chunk_position = v3_f32_from_v3_s32(chunk->position);
            chunk_position.x *= CHUNK_SIZE;
            chunk_position.y *= CHUNK_HEIGHT;
            chunk_position.z *= CHUNK_SIZE;
            V3_F32 chunk_size = v3_f32(CHUNK_SIZE, CHUNK_HEIGHT, CHUNK_SIZE);
            AABB box = make_aabb(chunk_position, chunk_size);
            if (chunk_is_generated(chunk) && chunk_is_dirty(chunk) && aabb_in_frustum(game_state->frustum, box)) {
                load_chunk_mesh(chunk);
                // DWORD thread_id = 0;
                // CreateThread(NULL, 0, update_chunk_thread, (void *)chunk, 0, &thread_id);
                // printf("UPDATING CHUNK ON THREAD: %d\n", thread_id);
            }
        }
    }

    M4_F32 ortho_projection = ortho_rh_zo(0.f, dim.x, 0.f, dim.y, -1.f, 1.f);
    M4_F32 projection  = perspective_rh_zo(DegToRad(game_state->camera.fov), dim.x/dim.y, 0.1f, 1000.0f);
    M4_F32 view = look_at_rh_zo(v3_f32(game_state->camera.position), v3_f32(game_state->camera.position) + game_state->camera.forward, game_state->camera.up);

    draw_begin(window_handle);

    //@Note Draw Sun Texture
    R_Handle celestial_body = (game_state->hour >= 4  && game_state->hour <= 18) ? sun_tex : moon_tex;
    draw_sun(projection, view, celestial_body);
    {
        V3_F32 position = V3_Zero;
        position.x = 0.0f;
        position.y = sinf(PI * game_state->day_t / (f32)game_state->ticks_per_day);
        position.z = cosf(PI * game_state->day_t / (f32)game_state->ticks_per_day);
        position *= 1000.0f;
        position += v3_f32(player->position);

        V3_F32 right = v3_f32(view._00, view._10, view._20);
        V3_F32 up = v3_f32(view._01, view._11, view._21);
        f32 size = 400.0f;
        V3_F32 tl = position - 0.5f * size * right + 0.5f * size * up;
        V3_F32 bl = position - 0.5f * size * right - 0.5f * size * up;
        V3_F32 tr = position + 0.5f * size * right + 0.5f * size * up;
        V3_F32 br = position + 0.5f * size * right - 0.5f * size * up;

        Rect src = make_rect(0.0f, 0.0f, 1.0f, 1.0f);
        if (celestial_body == moon_tex) {
            src.x1 = 0.25f;
            src.y1 = 0.5f;
        }

        draw_3d_vertex(bl, V4_One, v2_f32(src.x0, src.y0));
        draw_3d_vertex(br, V4_One, v2_f32(src.x1, src.y0));
        draw_3d_vertex(tr, V4_One, v2_f32(src.x1, src.y1));

        draw_3d_vertex(tr, V4_One, v2_f32(src.x1, src.y1));
        draw_3d_vertex(tl, V4_One, v2_f32(src.x0, src.y1));
        draw_3d_vertex(bl, V4_One, v2_f32(src.x0, src.y0));
    }

    draw_chunks(chunk_manager->loaded_chunks, player->position, game_state->frustum, projection, view, g_block_atlas, game_state->mesh_debug ? R_RasterizerState_Wireframe : R_RasterizerState_Default);

    //@Note Draw block destroy decal
    if (player->mining != block_id_zero() && player->mining_t > 0.0f) {
        int index = (int)((player->mining_t / player->mining_target_t) * (ArrayCount(mining_textures) - 1));
        R_Handle tex = mining_textures[index];
        draw_3d_mesh_begin(projection, view, tex, R_RasterizerState_Default);
        V4_F32 color = v4_f32(0.14f, 0.20f, 0.13f, 0.8f);
        draw_cube(v3_f32(player->raycast.hit) - fill_v3_f32(0.01f), 1.02f, make_rect(0.0f, 0.0f, 1.0f, 1.0f), color, FACE_MASK_NIL);
    }

    draw_set_xform(ortho_projection);
    draw_quad(icon_tex, make_rect(0.5f*dim.x - 16.f, 0.5f*dim.y - 16.f, 32.f, 32.f), make_rect(0.0f, 0.0f, 64.0f/1024.0f, 64.0f/1024.0f));

    //@Note Hot bar
    // V2_F32 hotbar_dim = make_v2_f32(450.0f, 50.0f);
    // Rect hotbar_rect = make_rect((dim.x - hotbar_dim.x) * 0.5f, 0.f, hotbar_dim.x, hotbar_dim.y);
    // Rect src = make_rect(0.f, 5.f/16.f, 1.f/16.f, 1.f/16.f);
    // draw_rect(hotbar_rect, make_v4_f32(0.f, 0.f, 0.f, 1.f));
    // V2_F32 slot_dim = make_v2_f32(hotbar_dim.x / (f32)HOTBAR_SLOTS, hotbar_dim.y);
    // for (int i = 0; i < HOTBAR_SLOTS; i++) {
    //     Texture_Atlas *atlas = g_block_atlas;
    //     Block *block = &g_basic_blocks[BLOCK_DIRT];
    //     Block_Face *face = &block->faces[FACE_TOP];
    //     Rect src = make_rect(face->texture_region->offset.x / (f32)atlas->dim.x, face->texture_region->offset.y / (f32)atlas->dim.y, face->texture_region->dim.x / (f32)atlas->dim.x, face->texture_region->dim.y / (f32)atlas->dim.y);
    //     Rect rect = make_rect(hotbar_rect.x0 + slot_dim.x * i, hotbar_rect.y0, slot_dim.x, slot_dim.y);
    //     draw_quad(g_block_atlas->tex_handle, rect, src);
    //     draw_rect_outline(rect, make_v4_f32(1.f, 1.f, 1.f, 1.f));
    // }
    // draw_rect_outline(hotbar_rect, make_v4_f32(1.f, 1.f, 1.f, 1.f));

    //@DEBUG
    ui_labelf("delta: %.4fms", 1000.0 * dt);
    ui_labelf("world:%lld %lld %lld chunk:%d %d %d", (s64)player->position.x, (s64)player->position.y, (s64)player->position.z, chunk_manager->chunk_position.x, chunk_manager->chunk_position.y, chunk_manager->chunk_position.z);
    // ui_labelf("day: %d hour: %d", game_state->day_t, game_state->hour);

    // ui_labelf("forward:%.2f %.2f %.2f", game_state->camera.forward.x, game_state->camera.forward.y, game_state->camera.forward.z);

    {
        if (player->raycast.block != block_id_zero()) {
            Block_ID *block = get_block_from_position(chunk_manager, (s32)player->raycast.hit.x, (s32)player->raycast.hit.y, (s32)player->raycast.hit.z);
            ui_labelf("raycast: %lld %lld %.2f block:%s face: %s", (s64)player->raycast.hit.x, (s64)player->raycast.hit.y, (s64)player->raycast.hit.z, block_to_string(*block), face_to_string(player->raycast.face));
        }
    }

    // ui_labelf("vertices %d", mesh_vertices_this_frame);
    // ui_labelf("%s", player->grounded ? "GROUNDED" : "NOT GROUNDED");
    // ui_labelf("velocity: %.3f %.3f %.3f", player->velocity.x, player->velocity.y, player->velocity.z);

    // ui_label(str8_lit("Profiler:"));
    // for (int i = 0; i < g_profile_manager.scope_count; i++) {
    //     Profile_Scope *scope = &g_profile_manager.scopes[i];
    //     ui_labelf("- %s: %.3fms", scope->name, scope->ms_elapsed);
    // }

    ui_layout_apply(ui_root());

    draw_ui_layout(ui_root());

    d3d11_render(window_handle, draw_bucket);

    r_d3d11_state->swap_chain->Present(1, 0);

    draw_end();

    ui_end_build();

    input_end(window_handle);
}
