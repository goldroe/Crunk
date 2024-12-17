#ifndef WORLD_GEN_H
#define WORLD_GEN_H

struct World_Generator {
    Arena *arena;

    s32 region_x;
    s32 region_z;

    FastNoiseLite height_noise;

    // noise::module::Perlin *height_noise;
    // noise::utils::NoiseMap *height_noise_map;
    // noise::utils::NoiseMapBuilderPlane *height_noise_map_builder;

    // noise::module::Perlin *tree_noise;
    // noise::utils::NoiseMap *tree_noise_map;
    // noise::utils::NoiseMapBuilderPlane *tree_noise_map_builder;
};

#endif // WORLD_GEN_H
