global Block g_basic_blocks[BLOCK_COUNT];

global Block_ID g_block_id_nil;

inline internal Block *get_basic_block(Block_ID block) {
    return &g_basic_blocks[block];
}

inline internal f32 block_mining_time(Block_ID *block) {
    return 0.7f;
}

inline internal Block_ID *block_id_zero() {
    Block_ID *result = &g_block_id_nil;
    return result;
}

inline internal char *block_to_string(Block_ID block) {
    switch (block) {
    default:
        return "";
    case BLOCK_AIR:
        return "AIR";
    case BLOCK_ERR:
        return "ERR";
    case BLOCK_WATER:
        return "WATER";
    case BLOCK_STONE:
        return "STONE";
    case BLOCK_DIRT:
        return "DIRT";
    case BLOCK_GRASS:
        return "GRASS";
    case BLOCK_WOOD:
        return "WOOD";
    case BLOCK_BRICK:
        return "BRICK";
    case BLOCK_SAND:
        return "SAND";
    case BLOCK_COBBLESTONE:
        return "COBBLESTONE";
    case BLOCK_LOG:
        return "LOG";
    case BLOCK_LEAVES:
        return "LEAVES";
    }
}

inline internal Block_ID *block_at(Chunk *chunk, int x, int y, int z) {
    Assert(x < CHUNK_SIZE);
    Assert(y < CHUNK_HEIGHT);
    Assert(z < CHUNK_SIZE);
    Block_ID *result = block_id_zero();
    if (chunk) {
        result = &chunk->blocks[x + y*CHUNK_HEIGHT + z*CHUNK_SIZE*CHUNK_HEIGHT];
    }
    return result;
}

inline internal bool block_is_active(Block_ID block) {
    return block != BLOCK_AIR;
}

inline internal bool block_is_opaque(Block *block) {
    return !(block->flags & BLOCK_FLAG_TRANSPARENT);
}

inline internal bool block_is_opaque(Block_ID block) {
    Block *basic_block = get_basic_block(block);
    return !(basic_block->flags & BLOCK_FLAG_TRANSPARENT);
}

inline internal void block_place(Block_ID *block, Block_ID value) {
    if (block != block_id_zero()) {
        *block = value;
    }
}
