global Block g_basic_blocks[BLOCK_COUNT];

inline internal Block *get_basic_block(Block_ID block) {
    return &g_basic_blocks[block];
}

inline internal bool block_active(Block_ID *block) {
    return *block != BLOCK_AIR;
}
