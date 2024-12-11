global Block g_basic_blocks[BLOCK_COUNT];

inline internal Block *get_basic_block(Block_ID block) {
    return &g_basic_blocks[block];
}

inline internal bool block_active(Block_ID *block) {
    return *block != BLOCK_AIR;
}

inline internal bool block_is_opaque(Block *block) {
    return !(block->flags & BLOCK_FLAG_TRANSPARENT);
}

inline internal bool block_is_opaque(Block_ID *block) {
    Block *basic_block = &g_basic_blocks[*block];
    return !(basic_block->flags & BLOCK_FLAG_TRANSPARENT);
}
