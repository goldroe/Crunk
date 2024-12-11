#ifndef CAMERA_H
#define CAMERA_H

struct Camera {
    Vector3 position;
    Vector3 forward;
    Vector3 up;
    Vector3 right;
    f32 yaw;
    f32 pitch;
    f32 fov;
    f32 aspect;
};

#endif // CAMERA_H
