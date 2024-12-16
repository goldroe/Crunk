#ifndef CAMERA_H
#define CAMERA_H

struct Camera {
    V3_F64 position;
    V3_F32 forward;
    V3_F32 up;
    V3_F32 right;
    f32 yaw;
    f32 pitch;
    f32 fov;
    f32 aspect;
};

#endif // CAMERA_H
