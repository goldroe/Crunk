#ifndef FRUSTUM_H
#define FRUSTUM_H

struct Frustum {
    V4_F32 planes[6];
};

inline internal bool point_behind_plane(V3_F32 p, V4_F32 plane);
internal Frustum make_frustum(Camera camera, f32 near_z, f32 far_z);
internal bool aabb_in_frustum(Frustum frustum, AABB box);

#endif // FRUSTUM_H
