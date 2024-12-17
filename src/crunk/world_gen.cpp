
internal void init_world_generator(World_Generator *generator, int seed) {
    generator->height_noise = FastNoiseLite(seed);
    generator->height_noise.SetFractalLacunarity(1.75f);
    generator->height_noise.SetFractalOctaves(5);
    generator->height_noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2); 
    generator->height_noise.SetFractalType(FastNoiseLite::FractalType_FBm);

    // //@Note Tree Map
    // generator->tree_noise = new noise::module::Perlin();
    // generator->tree_noise->SetSeed(seed);
    // generator->tree_noise->SetOctaveCount(20);
    // generator->tree_noise->SetLacunarity(2.0);
    // generator->tree_noise->SetPersistence(1.0);
    // generator->tree_noise->SetSeed(seed);
    // generator->tree_noise->SetFrequency(0.5);
    // generator->tree_noise_map = new noise::utils::NoiseMap();

    // generator->tree_noise_map_builder = new noise::utils::NoiseMapBuilderPlane();
    // generator->tree_noise_map_builder->SetSourceModule(*generator->tree_noise);
    // generator->tree_noise_map_builder->SetDestNoiseMap(*generator->tree_noise_map);

}

// internal void update_world_generator(World_Generator *generator, s32 region_x, s32 region_z) {
//     generator->region_x = region_x;
//     generator->region_z = region_z;

//     generator->height_noise_map_builder->SetDestSize(256, 256);
//     generator->height_noise_map_builder->SetBounds(1.0, 8.0, 2.0, 10.0);
//     generator->height_noise_map_builder->Build();

//     generator->tree_noise_map_builder->SetDestSize(128, 128);
//     generator->tree_noise_map_builder->SetBounds(0.0, 128.0, 0.0, 128.0);
//     generator->tree_noise_map_builder->Build();
// }

internal void generate_chunk(Chunk_Manager *manager, World_Generator *generator, Chunk *chunk) {
    V3_S32 position = chunk->position;
    position.x *= CHUNK_SIZE;
    position.z *= CHUNK_SIZE;

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

    //     for (int y = 0; y < height; y++) {
    //         Block_ID block_type = BLOCK_STONE;
    //         if (y == height - 1) {
    //             block_type = BLOCK_GRASS;
    //         } else if (y >= 0.7f * height) {
    //             block_type = BLOCK_DIRT;
    //         }
    //         Block_ID *block = block_at(chunk, x, y, z);
    //         *block = block_type;
    //     }

    //     f32 tree_value = *generator->tree_noise_map->GetSlabPtr(x, z);
    //     bool generate_tree = false;
    //     if (tree_value > 0.7f && tree_value < 0.8f) {
    //         generate_tree = true;
    //     }

    //     if (generate_tree) {
    //         int max_tree_y = height + 4;
    //         for (int y = height; y < max_tree_y; y++) {
    //             Block_ID *block = block_at(chunk, x, y, z);
    //             *block = BLOCK_LOG;
    //         }
    //         Block_ID *block = block_at(chunk, x, max_tree_y, z);
    //         *block = BLOCK_LEAVES;
    //     }
    // }
}
