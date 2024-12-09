#ifndef CAMERA_H
#define CAMERA_H

struct Camera {
    v3 position;
    v3 forward;
    v3 up;
    v3 right;
    f32 yaw;
    f32 pitch;
    f32 fov;
    f32 aspect;
};

#endif // CAMERA_H
