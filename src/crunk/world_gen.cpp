
V3_S32 tree_leaves_data[] = {
    v3_s32(0, 1, 0),
    v3_s32(0,  0, 0),

    v3_s32(-1, -1, 0),
    v3_s32(1,  -1, 0),
    v3_s32(0,  -1, 1),
    v3_s32(0,  -1, -1),

    v3_s32(-1, 0, 0),
    v3_s32(1,  0, 0),
    v3_s32(0,  0,  1),
    v3_s32(0,  0, -1),

    v3_s32(-1, -1, -1),
    v3_s32(-1, -1, 1),
    v3_s32(1, -1, -1),
    v3_s32(1, -1, 1),
};

internal void init_world_generator(World_Generator *generator, int seed) {
    FastNoiseLite *height_noise = &generator->height_noise;
    *height_noise = FastNoiseLite(seed);
    height_noise->SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2); 
    height_noise->SetFractalType(FastNoiseLite::FractalType_FBm);
    height_noise->SetFractalOctaves(5);
    height_noise->SetFractalLacunarity(1.75f);

    FastNoiseLite *vegetation_noise = &generator->vegetation_noise;
    *vegetation_noise = FastNoiseLite(seed);
    vegetation_noise->SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    vegetation_noise->SetFrequency(0.4f);
    vegetation_noise->SetFractalType(FastNoiseLite::FractalType_FBm);
    vegetation_noise->SetFractalOctaves(4);
    vegetation_noise->SetFractalGain(1.0f);
    vegetation_noise->SetFractalLacunarity(2.0f);
}

internal void generate_chunk(Chunk_Manager *manager, World_Generator *generator, Chunk *chunk) {
    chunk->flags |= CHUNK_FLAG_GENERATING;

    V3_S32 position = chunk->position;
    position.x *= CHUNK_SIZE;
    position.z *= CHUNK_SIZE;

    //@#ote First pass:
    //@Note Heightmap
    for (int x = 0; x < CHUNK_SIZE; x++) {
        for (int z = 0; z < CHUNK_SIZE; z++) {
            f32 noise_x = (position.x + (f32)x);
            f32 noise_z = (position.z + (f32)z);
            f32 noise_value = generator->height_noise.GetNoise(noise_x, noise_z) / 2.0f + 0.5f;
            s32 height = 45 + (s32)(noise_value * 45);

            for (int y = 0; y < height; y++) {
                Block_ID block_id = BLOCK_STONE;
                int dy = height - y;

                if (dy == 1) {
                    if (y <= 64 && y >= 63) {
                        block_id = BLOCK_SAND;
                    } else if (y < 63) {
                        block_id = BLOCK_STONE;
                    } else {
                        block_id = BLOCK_GRASS;
                    }
                } else if (dy < 5) {
                    if (dy < 64) {
                        block_id = BLOCK_STONE;
                    } else {
                        block_id = BLOCK_DIRT; 
                    }
                }
                *block_at(chunk, x, y, z) = block_id;
            }

            for (int y = 64; y >= height; y--) {
                *block_at(chunk, x, y, z) = BLOCK_WATER;
            }

            *block_at(chunk, x, 0, z) = BLOCK_BEDROCK;
        }
    }

    //@Note Vegetation
    for (int y = CHUNK_HEIGHT - 1; y >= 40; y--) {
    for (int x = 0; x <= CHUNK_SIZE; x++) {
        for (int z = 0; z < CHUNK_SIZE; z++) {
                Block_ID *block = block_at(chunk, x, y - 1, z);
                if (*block == BLOCK_GRASS) {
                    f32 noise_x = x + (f32)position.x;
                    f32 noise_z = z + (f32)position.z;
                    f32 noise_value = generator->vegetation_noise.GetNoise(noise_x, noise_z) / 1.8f + 0.1f;

                    bool gen_tree = noise_value >= 0.3f && noise_value <= 0.4f;
                    bool gen_grass = (noise_value > 0.2f && noise_value < 0.3f);
                    if (gen_tree) {
                        *block_at(chunk, x, y + 1, z) = BLOCK_LOG;
                        int tree_height = 3;
                        for (int log_y = 0; log_y < tree_height; log_y++) {
                        *block_at(chunk, x, y + log_y, z) = BLOCK_LOG;
                        }

                        V3_S32 base = v3_s32(x, y + tree_height, z);
                        for (int i = 0; i < ArrayCount(tree_leaves_data); i++) {
                            V3_S32 p = base + tree_leaves_data[i];
                            *block_at(chunk, p.x, p.y, p.z) = BLOCK_LEAVES;
                        }
                    }
                    else if (gen_grass) {
                        *block_at(chunk, x, y, z) = BLOCK_GRASS_PLANT;
                    }
                    break;
                }
            }
        }
    }

    chunk->flags |= CHUNK_FLAG_GENERATED;
}
