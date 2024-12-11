
inline internal bool point_behind_plane(Vector3 p, Vector4 plane) {
    f32 dp = dot(make_vector3(plane.x, plane.y, plane.z), p);
    return dp < -plane.w;
}

internal bool aabb_in_frustum(Frustum frustum, AABB box) {
    for (int i = 0; i < 6; i++) {
        Vector4 plane = frustum.planes[i];
        int out = 0;
        out += point_behind_plane(make_vector3(box.x0, box.y0, box.z0), plane);
        out += point_behind_plane(make_vector3(box.x1, box.y0, box.z0), plane);
        out += point_behind_plane(make_vector3(box.x1, box.y1, box.z0), plane);
        out += point_behind_plane(make_vector3(box.x0, box.y1, box.z0), plane);
        out += point_behind_plane(make_vector3(box.x0, box.y0, box.z1), plane);
        out += point_behind_plane(make_vector3(box.x1, box.y0, box.z1), plane);
        out += point_behind_plane(make_vector3(box.x1, box.y1, box.z1), plane);
        out += point_behind_plane(make_vector3(box.x0, box.y1, box.z1), plane);
        if (out == 8) return false;
	}
    return true;
}

internal Frustum make_frustum(Camera camera, f32 near_z, f32 far_z) {
    Matrix4 projection = perspective_projection_rh(DegToRad(camera.fov), camera.aspect, near_z, far_z);
    Matrix4 view = look_at_rh(camera.position, camera.position + camera.forward, camera.up);
    Matrix4 mvp = projection * view;
    mvp = transpose_matrix4(mvp);

    Frustum frustum = {};
    frustum.planes[0].x = mvp.columns[0][0] + mvp.columns[3][0];
    frustum.planes[0].y = mvp.columns[0][1] + mvp.columns[3][1];
    frustum.planes[0].z = mvp.columns[0][2] + mvp.columns[3][2];
    frustum.planes[0].w = mvp.columns[0][3] + mvp.columns[3][3];

    frustum.planes[1].x = -mvp.columns[0][0] + mvp.columns[3][0];
    frustum.planes[1].y = -mvp.columns[0][1] + mvp.columns[3][1];
    frustum.planes[1].z = -mvp.columns[0][2] + mvp.columns[3][2];
    frustum.planes[1].w = -mvp.columns[0][3] + mvp.columns[3][3];

    frustum.planes[2].x = mvp.columns[1][0] + mvp.columns[3][0];
    frustum.planes[2].y = mvp.columns[1][1] + mvp.columns[3][1];
    frustum.planes[2].z = mvp.columns[1][2] + mvp.columns[3][2];
    frustum.planes[2].w = mvp.columns[1][3] + mvp.columns[3][3];

    frustum.planes[3].x = -mvp.columns[1][0] + mvp.columns[3][0];
    frustum.planes[3].y = -mvp.columns[1][1] + mvp.columns[3][1];
    frustum.planes[3].z = -mvp.columns[1][2] + mvp.columns[3][2];
    frustum.planes[3].w = -mvp.columns[1][3] + mvp.columns[3][3];

    frustum.planes[4].x = mvp.columns[2][0] + mvp.columns[3][0];
    frustum.planes[4].y = mvp.columns[2][1] + mvp.columns[3][1];
    frustum.planes[4].z = mvp.columns[2][2] + mvp.columns[3][2];
    frustum.planes[4].w = mvp.columns[2][3] + mvp.columns[3][3];

    frustum.planes[5].x = -mvp.columns[2][0] + mvp.columns[3][0];
    frustum.planes[5].y = -mvp.columns[2][1] + mvp.columns[3][1];
    frustum.planes[5].z = -mvp.columns[2][2] + mvp.columns[3][2];
    frustum.planes[5].w = -mvp.columns[2][3] + mvp.columns[3][3];

    for (int i = 0; i < 6; i++) {
        frustum.planes[i] = normalize(frustum.planes[i]);
    }

    return frustum;
}
