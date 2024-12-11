
internal void init_world_generator(World_Generator *generator, int seed) {
    //@Note Height Map
    generator->height_noise = new noise::module::Perlin();
    generator->height_noise->SetSeed(seed);
    generator->height_noise_map = new noise::utils::NoiseMap();

    generator->height_noise_map_builder = new noise::utils::NoiseMapBuilderPlane();
    generator->height_noise_map_builder->SetSourceModule(*generator->height_noise);
    generator->height_noise_map_builder->SetDestNoiseMap(*generator->height_noise_map);

    //@Note Tree Map
    generator->tree_noise = new noise::module::Perlin();
    generator->tree_noise->SetSeed(seed);
    generator->tree_noise->SetOctaveCount(20);
    generator->tree_noise->SetLacunarity(2.0);
    generator->tree_noise->SetPersistence(1.0);
    generator->tree_noise->SetSeed(seed);
    generator->tree_noise->SetFrequency(0.5);
    generator->tree_noise_map = new noise::utils::NoiseMap();

    generator->tree_noise_map_builder = new noise::utils::NoiseMapBuilderPlane();
    generator->tree_noise_map_builder->SetSourceModule(*generator->tree_noise);
    generator->tree_noise_map_builder->SetDestNoiseMap(*generator->tree_noise_map);

}

internal void update_world_generator(World_Generator *generator, s32 region_x, s32 region_z) {
    generator->region_x = region_x;
    generator->region_z = region_z;

    generator->height_noise_map_builder->SetDestSize(256, 256);
    generator->height_noise_map_builder->SetBounds(1.0, 8.0, 2.0, 10.0);
    generator->height_noise_map_builder->Build();

    generator->tree_noise_map_builder->SetDestSize(128, 128);
    generator->tree_noise_map_builder->SetBounds(0.0, 128.0, 0.0, 128.0);
    generator->tree_noise_map_builder->Build();
}

internal void generate_chunk(Chunk_Manager *manager, World_Generator *generator, Chunk *chunk) {
    s32 region_x = chunk->position.x >> 5;
    s32 region_z = chunk->position.z >> 5;
    if (region_x != generator->region_x || region_z != generator->region_z) {
        update_world_generator(generator, region_x, region_z);
    }

    for (int x = 0; x < CHUNK_SIZE; x++) {
        for (int z = 0; z < CHUNK_SIZE; z++) {
            f32 height_map_value = *generator->height_noise_map->GetSlabPtr(x, z);
            int height = (int)floorf(CHUNK_SIZE * Abs(height_map_value));
            height = Clamp(height, 4, 20);

            if (height < 8) {
                for (int y = 0; y < height; y++) {
                    Block_ID *block = BLOCK_AT(chunk, x, y, z);
                    *block = BLOCK_WATER;
                }
            } else {
                for (int y = 0; y < height; y++) {
                    Block_ID block_type = BLOCK_STONE;
                    if (y == height - 1) {
                        block_type = BLOCK_GRASS;
                    } else if (y >= 0.7f * height) {
                        block_type = BLOCK_DIRT;
                    }
                    Block_ID *block = BLOCK_AT(chunk, x, y, z);
                    *block = block_type;
                }

                f32 tree_value = *generator->tree_noise_map->GetSlabPtr(x, z);
                bool generate_tree = false;
                if (tree_value > 0.7f && tree_value < 0.8f) {
                    generate_tree = true;
                }

                if (generate_tree) {
                    int max_tree_y = height + 4;
                    for (int y = height; y < max_tree_y; y++) {
                        Block_ID *block = BLOCK_AT(chunk, x, y, z);
                        *block = BLOCK_LOG;
                    }
                    Block_ID *block = BLOCK_AT(chunk, x, max_tree_y, z);
                    *block = BLOCK_LEAVES;
                }
            }
        }
    }
}
