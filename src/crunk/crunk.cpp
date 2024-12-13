global World_Generator *world_generator;

global Game_State *game_state;
global Chunk_Manager *chunk_manager;

global Texture_Atlas *g_block_atlas;
global R_Handle icon_tex;

inline internal f32 sign_f32(f32 x) {
    return x > 0 ? 1.0f : x < 0 ? -1.0f : 0.0f;
}

inline internal World_Position make_world_position(Vector3Int base, Vector3 off) {
    World_Position result;
    result.base = base;
    result.off = off;
    return result;
}

inline internal World_Position add_world_position(World_Position world, Vector3 distance) {
    World_Position result;
    Vector3 total_dp = distance + world.off;
    Vector3Int trunc_dp = make_vector3int((int)trunc_f32(total_dp.x), (int)trunc_f32(total_dp.y), (int)trunc_f32(total_dp.z));
    result.base = world.base + trunc_dp;
    result.off = make_vector3(mod_f32(total_dp.x, 1.0f), mod_f32(total_dp.y, 1.0f), mod_f32(total_dp.z, 1.0f));
    return result; 
}

internal World_Position lerp(World_Position a, World_Position b, f32 t) {
    World_Position result;
    result = add_world_position(a, t * b.off);
    result.base += b.base * t;
    return result;
}

inline internal Vector3Int get_chunk_position(Vector3Int position) {
    Vector3Int result;
    result.x = (int)floor_f32((f32)position.x / CHUNK_SIZE);
    result.y = (int)floor_f32((f32)position.y / CHUNK_HEIGHT);
    result.z = (int)floor_f32((f32)position.z / CHUNK_SIZE);
    return result;
}

inline internal Vector3Int get_chunk_position(Vector3 position) {
    Vector3Int result;
    result.x = (int)floor_f32(position.x / CHUNK_SIZE);
    result.y = (int)floor_f32(position.y / CHUNK_SIZE);
    result.z = (int)floor_f32(position.z / CHUNK_SIZE);
    return result;
}

inline internal Vector3Int get_chunk_relative_position(Chunk *chunk, Vector3Int world_position) {
    Vector3Int result;
    result.x = world_position.x - CHUNK_SIZE * chunk->position.x;
    result.y = world_position.y - CHUNK_SIZE * chunk->position.y;
    result.z = world_position.z - CHUNK_SIZE * chunk->position.z;
    return result;
}

internal Chunk *get_chunk_from_position(Chunk_Manager *manager, Vector3Int chunk_position) {
    Chunk *result = NULL;
    for (Chunk *chunk = manager->loaded_chunks.first; chunk; chunk = chunk->next) {
        if (chunk->position == chunk_position) {
            result = chunk;
            break;
        }
    }
    return result;
}

internal Block_ID *get_block_from_position(Chunk_Manager *manager, Vector3Int world_position) {
    Block_ID *result = NULL;

    Vector3Int chunk_p = get_chunk_position(world_position);
    Chunk *chunk = get_chunk_from_position(manager, chunk_p);
    if (chunk) {
        Vector3Int block_p = get_chunk_relative_position(chunk, world_position);
        result = BLOCK_AT(chunk, block_p.x, block_p.y, block_p.z);
    }
    return result;
}

inline internal bool is_block_active_at(Chunk_Manager *manager, Vector3Int world_position) {
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

internal R_Handle load_texture(String8 file_name) {
    int x, y, n;
    u8 *data = stbi_load((char *)file_name.data, &x, &y, &n, 4);
    R_Handle tex = d3d11_create_texture(R_Tex2DFormat_R8G8B8A8, make_vector2int(x, y), data);
    return tex;
}

internal Texture_Map load_texture_map(String8 file_name) {
    int x, y, n;
    u8 *data = stbi_load((char *)file_name.data, &x, &y, &n, 1);
    R_Handle tex = d3d11_create_texture(R_Tex2DFormat_R8, make_vector2int(x, y), data);

    Texture_Map result = {};
    result.texture = tex;
    result.size = make_vector2int(x, y);
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

internal Chunk *load_chunk_at(Chunk_Manager *manager, s32 x, s32 y, s32 z) {
    Vector3Int position = {x, y, z};
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
    Vector3Int position = {x, y, z};
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
    }
}

internal void update_chunk_load_list(Chunk_Manager *manager, Vector3Int chunk_position) {
    //@Note Free chunks
    for (Chunk *chunk = manager->loaded_chunks.first, *next = NULL; chunk; chunk = next) {
        next = chunk->next;
        DLLRemove(manager->loaded_chunks.first, manager->loaded_chunks.last, chunk, next, prev);
        DLLPushBack(manager->free_chunks.first, manager->free_chunks.last, chunk, next, prev);
    }

    load_chunk_at(manager, chunk_position.x, chunk_position.y, chunk_position.z);
    load_new_chunk_at(manager, chunk_position.x, chunk_position.y, chunk_position.z);
    Vector3Int dim = make_vector3int(5, 0, 5);
    s32 min_x = chunk_position.x - (s32)(0.5f * dim.x);
    s32 max_x = chunk_position.x + (s32)(0.5f * dim.x);
    s32 min_z = chunk_position.z - (s32)(0.5f * dim.z);
    s32 max_z = chunk_position.z + (s32)(0.5f * dim.z);
    for (s32 x = min_x; x < max_x; x++) {
        for (s32 z = min_z; z < max_z; z++) {
            load_chunk_at(manager, x, chunk_position.y, z);
            load_new_chunk_at(manager, x, chunk_position.y, z);
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
        region->offset = make_vector2(atlas_x / (f32)MAX_ATLAS_X, atlas_y / (f32)MAX_ATLAS_Y);
        region->dim = make_vector2(1.0f / MAX_ATLAS_X, 1.0f / MAX_ATLAS_Y);
        region->size = x * y * 4;
        region->data = data;
        region->atlas_index = texture_count;

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
        int src_pitch = atlas->region_dim.x * bytes_per_pixel;
        int dst_pitch = atlas->dim.x * bytes_per_pixel;

        u8 *dst = bitmap + (int)(region->offset.y * atlas->dim.x * bytes_per_pixel) + (int)(region->offset.x * atlas->dim.x * bytes_per_pixel);
        u8 *src = region->data;

        for (int y = 0; y < (int)atlas->dim.y * region->dim.y; y++) {
            MemoryCopy(dst, src, src_pitch);
            dst += dst_pitch;
            src += src_pitch;
        }
    }
    atlas->texture_region_count = texture_count;
    atlas->tex_handle = d3d11_create_texture_mipmap(R_Tex2DFormat_R8G8B8A8, atlas->dim, bitmap);

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
        Block *block = &blocks[BLOCK_LOG];
        set_block_face(block->faces + FACE_TOP, str8_lit("oak_log_top.png"), 0);
        set_block_face(block->faces + FACE_NORTH, str8_lit("oak_log.png"), 0);
        block->faces[FACE_SOUTH] = block->faces[FACE_EAST] = block->faces[FACE_WEST] = block->faces[FACE_NORTH];
        block->faces[FACE_BOTTOM] = block->faces[FACE_TOP];
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

    d3d11_upload_block_atlas(atlas);
    d3d11_upload_color_table();
}

internal bool raycast(Chunk_Manager *manager, World_Position start, Vector3 direction, Raycast_Result *result) {
    int step_x = (int)sign_f32(direction.x);
    int step_y = (int)sign_f32(direction.y);
    int step_z = (int)sign_f32(direction.z);

    f32 next_bound_x = floor_f32(start.off.x + step_x);
    f32 next_bound_y = floor_f32(start.off.y + step_y);
    f32 next_bound_z = floor_f32(start.off.z + step_z);

    f32 t_max_x = (next_bound_x - start.off.x) / direction.x;
    f32 t_max_y = (next_bound_y - start.off.y) / direction.y;
    f32 t_max_z = (next_bound_z - start.off.z) / direction.z;

    f32 t_delta_x = direction.x != 0 ? (step_x / direction.x) : FLT_MAX;
    f32 t_delta_y = direction.y != 0 ? (step_y / direction.y) : FLT_MAX;
    f32 t_delta_z = direction.z != 0 ? (step_z / direction.z) : FLT_MAX;

    int x = 0;
    int y = 0;
    int z = 0;

    result->start = start;
    result->direction = direction;
    result->face = FACE_NORTH;

    for (int t = 0; t < 128; t++) {
        if (t_max_x < t_max_y) {
            if (t_max_x < t_max_z) {
                t_max_x += t_delta_x;
                x += step_x;
            } else {
                t_max_z += t_delta_z;
                z += step_z;
            }
        } else if (t_max_y < t_max_z) {
            t_max_y += t_delta_y;
            y += step_y;
        } else {
            t_max_z += t_delta_z;
            z += step_z;
        }

        World_Position world_p = add_world_position(start, make_vector3(x, y, z));
        Block_ID *block = get_block_from_position(manager, world_p.base);
        if (block && block_active(block)) {
            result->end = world_p;
            return true;
        }
    }

    return false;
}

internal void update_and_render(OS_Event_List *event_list, OS_Handle window_handle, f32 dt) {
    local_persist bool first_call = true;
    if (first_call) {
        first_call = false;

        Vector2 dim = os_get_window_dim(window_handle);

        Arena *font_arena = arena_alloc(get_virtual_allocator(), MB(4));
        default_fonts[FONT_DEFAULT] = load_font(font_arena, str8_lit("data/assets/fonts/consolas.ttf"), 16);

        ui_set_state(ui_state_new());

        load_blocks();

        icon_tex = load_texture(str8_lit("data/assets/icons.png"));

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

        update_chunk_load_list(chunk_manager, Vector3Int_Zero);

        game_state->player->position = make_world_position(make_vector3int(0, get_height_from_chunk_xz(get_chunk_from_position(chunk_manager, Vector3Int_Zero), 0, 0) + 1, 0), Vector3_Zero);

        //@Note Camera
        game_state->camera.up = make_vector3(0.f, 1.f, 0.f);
        game_state->camera.right = make_vector3(1.f, 0.f, 0.f);
        game_state->camera.forward = make_vector3(0.f, 0.f, -1.f);
        //@Note Toward -Z axis
        game_state->camera.yaw = -90.0f;
        game_state->camera.pitch =  0.0f;
        game_state->camera.fov = 70.f;
        game_state->camera.aspect = 1024.0f/ 768.0f;

        game_state->frustum = make_frustum(game_state->camera, 0.001f, 10000.0f);
    }

    input_begin(window_handle, event_list);

    Vector2 dim = os_get_window_dim(window_handle);
    game_state->client_dim = make_vector2int((s32)dim.x, (s32)dim.y);

    ui_begin_build(dt, window_handle, event_list);

    //@Note Chunk update
    Vector3Int chunk_position = get_chunk_position(game_state->player->position.base);
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

    if (key_pressed(OS_KEY_F1)) {
        game_state->mesh_debug = !game_state->mesh_debug;
    }
    if (key_pressed(OS_KEY_F4)) {
        game_state->creative_mode = !game_state->creative_mode;
    }

    if (key_pressed(OS_KEY_LEFTMOUSE)) {
        if (game_state->raycast_hit) {
            Block_ID *block = get_block_from_position(chunk_manager, game_state->raycast_result.end.base);
            if (block && block_active(block)) {
                *block = BLOCK_AIR;
            }
        }
    }

    if (key_pressed(OS_KEY_RIGHTMOUSE)) {
        if (game_state->raycast_hit) {
            Block_ID *block = get_block_from_position(chunk_manager, add_world_position(game_state->raycast_result.end, Vector3_Up).base);
            *block = BLOCK_LOG;
        }
    }

    //@Note Player camera
    {
        f32 rotation_dp = 7.f;
        game_state->camera.yaw += rotation_dp * get_mouse_delta().x * dt;
        game_state->camera.yaw = (f32)fmod(game_state->camera.yaw, 360.0f);
        game_state->camera.pitch -= rotation_dp * get_mouse_delta().y * dt;
        game_state->camera.pitch = Clamp(game_state->camera.pitch, -89.0f, 89.0f);

        Vector3 direction;
        direction.x = cosf(DegToRad(game_state->camera.yaw)) * cosf(DegToRad(game_state->camera.pitch));
        direction.y = sinf(DegToRad(game_state->camera.pitch));
        direction.z = sinf(DegToRad(game_state->camera.yaw)) * cosf(DegToRad(game_state->camera.pitch));

        game_state->camera.forward = normalize_vector3(direction);
        game_state->camera.right = normalize_vector3(cross_vector3(game_state->camera.forward, game_state->camera.up));

        game_state->camera.fov -= 5.0f * g_input.scroll_delta.y;
        game_state->camera.fov = Clamp(game_state->camera.fov, 1.0f, 60.0f);
    }

    //@Note Player physics
    if (true) {
        Player *player = game_state->player;
        player->velocity = Vector3_Zero;
        f32 speed = 10.0f;

        if (key_down(OS_KEY_SPACE)) {
            speed = 500.0f; 
        }

        Vector3 direction = normalize_vector3(game_state->camera.forward * forward_dt + game_state->camera.right * right_dt);
        Vector3 total_dp = speed * direction * dt;
        player->position = add_world_position(player->position, total_dp);
    }
#if 0
    else {
        Player *player = game_state->player;
        f32 G = 9.8f;
        f32 J = 25.f;
        f32 jump_time = 0.3f;

        const Vector3 normals[6] = {
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
            Vector3 velocity = make_vector3(0.f, -0.1f, 0.f);
            Vector3 new_position = player->position + velocity;
            for (f32 t = 0.0f; t <= 1.0f; t += 1.0f/64.0f) {
                Vector3 position = lerp_vector3(player->position, new_position, t);
                Vector3 block_position = floor_vector3(position);

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

        Vector3 new_velocity = player->velocity;
        new_velocity.x = 0.f;
        new_velocity.z = 0.f;
        Vector3 forward = make_vector3(game_state->camera.forward.x, 0.f, game_state->camera.forward.z);
        new_velocity += 4.f * normalize(forward * forward_dt + game_state->camera.right * right_dt);

        Vector3 new_position = player->position + player->velocity * dt;
        for (f32 t = 0.0f; t <= 1.0f; t += 1.0f/64.0f) {
            Vector3 position = lerp_vector3(player->position, new_position, t);
            Vector3 block_position = floor_vector3(position);
            b32 collides = is_block_active_at(chunk_manager, block_position);
            if (collides) {
                Vector3 direction = normalize(block_position - position);

                f32 max_dot = 0.f;
                int normal_idx = 0;
                for (int i = 0; i < ArrayCount(normals); i++) {
                    Vector3 normal = normals[i];
                    f32 d = dot(direction, normal);
                    if (d > max_dot) {
                        max_dot = d;   
                        normal_idx = i;
                    }
                }

                Assert(max_dot != 0.f);

                Vector3 collision_normal = normals[normal_idx];
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

    //@Todo Make camera position player position offset?
    // So that basically we pass player position relative to the chunk so everything is in world space
    game_state->camera.position = 0.5f * game_state->camera.forward + Vector3_Half;
    // game_state->camera.position = -Vector3_Half;
    game_state->camera.position.y += 1.f;

    //@Note Update Frustum
    game_state->frustum = make_frustum(game_state->camera, 0.001f, 1000.0f);

    //@Note Raycast
    game_state->raycast_hit = false;
    {
        Vector3 direction = game_state->camera.forward;
        // World_Position start = add_world_position(game_state->player->position, make_vector3(-0.5f, 0.5f, 0.0f));
        World_Position start = add_world_position(game_state->player->position, game_state->camera.position);

        Raycast_Result ray = {};
        if (raycast(chunk_manager, start, direction, &ray)) {
            game_state->raycast_hit = true;
            game_state->raycast_result = ray;
        }
    }

    Matrix4 ortho_projection = ortho_rh_zo(0.f, dim.x, 0.f, dim.y, -1.f, 1.f);
    Matrix4 projection  = perspective_projection_rh(DegToRad(game_state->camera.fov), dim.x/dim.y, 0.001f, 1000.0f);
    Matrix4 view = look_at_rh(game_state->camera.position, game_state->camera.position + game_state->camera.forward, game_state->camera.up);

    draw_begin(window_handle);

    draw_chunks(chunk_manager->loaded_chunks, game_state->player->position, game_state->frustum, projection, view, g_block_atlas, game_state->mesh_debug ? R_RasterizerState_Wireframe : R_RasterizerState_Default);

    //@Note draw raycast block
    draw_3d_mesh_begin(projection, view, r_handle_zero(), R_RasterizerState_Wireframe);
    if (game_state->raycast_hit) {
        draw_cube(vector3_from_vector3int(game_state->raycast_result.end.base - game_state->player->position.base) - game_state->player->position.off, Rect_Zero, Vector4_One, FACE_MASK_NIL);
    }

    draw_set_xform(ortho_projection);
    draw_quad(icon_tex, make_rect(0.5f*dim.x - 16.f, 0.5f*dim.y - 16.f, 32.f, 32.f), make_rect(0.0f, 0.0f, 64.0f/1024.0f, 64.0f/1024.0f));

    //@Note Hot bar
    // Vector2 hotbar_dim = make_vector2(450.0f, 50.0f);
    // Rect hotbar_rect = make_rect((dim.x - hotbar_dim.x) * 0.5f, 0.f, hotbar_dim.x, hotbar_dim.y);
    // Rect src = make_rect(0.f, 5.f/16.f, 1.f/16.f, 1.f/16.f);
    // draw_rect(hotbar_rect, make_vector4(0.f, 0.f, 0.f, 1.f));
    // Vector2 slot_dim = make_vector2(hotbar_dim.x / (f32)HOTBAR_SLOTS, hotbar_dim.y);
    // for (int i = 0; i < HOTBAR_SLOTS; i++) {
    //     Texture_Atlas *atlas = g_block_atlas;
    //     Block *block = &g_basic_blocks[BLOCK_DIRT];
    //     Block_Face *face = &block->faces[FACE_TOP];
    //     Rect src = make_rect(face->texture_region->offset.x / (f32)atlas->dim.x, face->texture_region->offset.y / (f32)atlas->dim.y, face->texture_region->dim.x / (f32)atlas->dim.x, face->texture_region->dim.y / (f32)atlas->dim.y);
    //     Rect rect = make_rect(hotbar_rect.x0 + slot_dim.x * i, hotbar_rect.y0, slot_dim.x, slot_dim.y);
    //     draw_quad(g_block_atlas->tex_handle, rect, src);
    //     draw_rect_outline(rect, make_vector4(1.f, 1.f, 1.f, 1.f));
    // }
    // draw_rect_outline(hotbar_rect, make_vector4(1.f, 1.f, 1.f, 1.f));

    //@DEBUG
    ui_labelf("delta: %fms", 1000.0 * dt);
    ui_labelf("world:%d %d %d offset: %f %f %f chunk:%d %d %d", game_state->player->position.base.x, game_state->player->position.base.y, game_state->player->position.base.z, game_state->player->position.off.x, game_state->player->position.off.y, game_state->player->position.off.z, chunk_manager->chunk_position.x, chunk_manager->chunk_position.y, chunk_manager->chunk_position.z);
    ui_labelf("forward:%.2f %.2f %.2f", game_state->camera.forward.x, game_state->camera.forward.y, game_state->camera.forward.z);
    // ui_labelf("vertices %d", mesh_vertices_this_frame);
    // ui_labelf("%s", game_state->player->grounded ? "GROUNDED" : "NOT GROUNDED");
    // ui_labelf("velocity: %.3f %.3f %.3f", game_state->player->velocity.x, game_state->player->velocity.y, game_state->player->velocity.z);

    ui_label(str8_lit("Profiler:"));
    for (int i = 0; i < g_profile_manager.scope_count; i++) {
        Profile_Scope *scope = &g_profile_manager.scopes[i];
        ui_labelf("- %s: %.3fms", scope->name, scope->ms_elapsed);
    }

    ui_layout_apply(ui_root());

    draw_ui_layout(ui_root());

    d3d11_render(window_handle, draw_bucket);

    r_d3d11_state->swap_chain->Present(1, 0);

    draw_end();

    ui_end_build();

    input_end(window_handle);
}
