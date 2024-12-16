#ifndef AABB_H
#define AABB_H

struct AABB {
    f32 x0;
    f32 x1;
    f32 y0;
    f32 y1;
    f32 z0;
    f32 z1;
};

inline internal AABB make_aabb(V3_F32 p, V3_F32 size) {
    AABB result;
    result.x0 = p.x;
    result.y0 = p.y;
    result.z0 = p.z;
    result.x1 = result.x0 + size.x;
    result.y1 = result.y0 + size.y;
    result.z1 = result.z0 + size.z;
    return result;
}



#endif // AABB_H
