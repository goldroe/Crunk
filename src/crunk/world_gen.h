#ifndef WORLD_GEN_H
#define WORLD_GEN_H

struct World_Generator {
    Arena *arena;

    s32 region_x;
    s32 region_z;

    FastNoiseLite height_noise;
    FastNoiseLite vegetation_noise;
};

#endif // WORLD_GEN_H
