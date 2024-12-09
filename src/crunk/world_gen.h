#ifndef WORLD_GEN_H
#define WORLD_GEN_H

struct Perlin_Map {
    noise::module::Perlin perlin;
    noise::utils::NoiseMap *map;
    noise::utils::NoiseMapBuilderPlane *map_builder;
};

struct World_Generator {
    Arena *arena;

    s32 region_x;
    s32 region_z;

    // FastNoiseLite perlin_noise;
    // perlin_noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    // f32 *perlin_noise_map;

    noise::module::Perlin *height_noise;
    noise::utils::NoiseMap *height_noise_map;
    noise::utils::NoiseMapBuilderPlane *height_noise_map_builder;

    noise::module::Perlin *tree_noise;
    noise::utils::NoiseMap *tree_noise_map;
    noise::utils::NoiseMapBuilderPlane *tree_noise_map_builder;
};

#endif // WORLD_GEN_H
