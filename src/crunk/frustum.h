#ifndef FRUSTUM_H
#define FRUSTUM_H

struct Frustum {
    v4 planes[6];
};

inline internal bool point_behind_plane(v3 p, v4 plane);
internal Frustum make_frustum(Camera camera, f32 near_z, f32 far_z);
internal bool aabb_in_frustum(Frustum frustum, AABB box);

#endif // FRUSTUM_H
