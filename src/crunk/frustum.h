#ifndef FRUSTUM_H
#define FRUSTUM_H

struct Frustum {
    Vector4 planes[6];
};

inline internal bool point_behind_plane(Vector3 p, Vector4 plane);
internal Frustum make_frustum(Camera camera, f32 near_z, f32 far_z);
internal bool aabb_in_frustum(Frustum frustum, AABB box);

#endif // FRUSTUM_H
