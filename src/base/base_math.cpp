inline internal Matrix4 identity_matrix4() {
    Matrix4 result{};
    result._00 = 1.0f;
    result._11 = 1.0f;
    result._22 = 1.0f;
    result._33 = 1.0f;
    return result;
}

inline internal Matrix4 transpose_matrix4(Matrix4 m) {
    Matrix4 result = m;
    result._01 = m._10;
    result._02 = m._20;
    result._03 = m._30;
    result._10 = m._01;
    result._12 = m._21;
    result._13 = m._31;
    result._20 = m._02;
    result._21 = m._12;
    result._23 = m._32;
    result._30 = m._03;
    result._31 = m._13;
    result._32 = m._23;
    return result;
}

inline internal Vector4 linear_combine_vector4_matrix4(Vector4 left, Matrix4 right) {
    Vector4 result;

    result.x = left.e[0] * right.columns[0].x;
    result.y = left.e[0] * right.columns[0].y;
    result.z = left.e[0] * right.columns[0].z;
    result.w = left.e[0] * right.columns[0].w;

    result.x += left.e[1] * right.columns[1].x;
    result.y += left.e[1] * right.columns[1].y;
    result.z += left.e[1] * right.columns[1].z;
    result.w += left.e[1] * right.columns[1].w;

    result.x += left.e[2] * right.columns[2].x;
    result.y += left.e[2] * right.columns[2].y;
    result.z += left.e[2] * right.columns[2].z;
    result.w += left.e[2] * right.columns[2].w;

    result.x += left.e[3] * right.columns[3].x;
    result.y += left.e[3] * right.columns[3].y;
    result.z += left.e[3] * right.columns[3].z;
    result.w += left.e[3] * right.columns[3].w;

    return result;
}

inline internal Matrix4 mul_matrix4(Matrix4 left, Matrix4 right) {
    Matrix4 result;
    result.columns[0] = linear_combine_vector4_matrix4(right.columns[0], left);
    result.columns[1] = linear_combine_vector4_matrix4(right.columns[1], left);
    result.columns[2] = linear_combine_vector4_matrix4(right.columns[2], left);
    result.columns[3] = linear_combine_vector4_matrix4(right.columns[3], left);
    return result;
}

inline internal Vector4 mul_matrix4_vector4(Matrix4 m, Vector4 v) {
    Vector4 result;
    result.e[0] = v.e[0] * m._00 + v.e[1] * m._01 + v.e[2] * m._02 + v.e[3] * m._03;
    result.e[1] = v.e[0] * m._10 + v.e[1] * m._11 + v.e[2] * m._12 + v.e[3] * m._13;
    result.e[2] = v.e[0] * m._20 + v.e[1] * m._21 + v.e[2] * m._22 + v.e[3] * m._23;
    result.e[3] = v.e[0] * m._30 + v.e[1] * m._31 + v.e[2] * m._32 + v.e[3] * m._33;
    return result;
}

inline internal Matrix4 translate_matrix4(f32 x, f32 y, f32 z) {
    Matrix4 result = identity_matrix4();
    result._30 = x;
    result._31 = y;
    result._32 = z;
    return result;
}

inline internal Matrix4 inv_translate_matrix4(f32 x, f32 y, f32 z) {
    Matrix4 result = translate_matrix4(-x, -y, -z);
    return result;
}

inline internal Matrix4 scale_matrix4(f32 x, f32 y, f32 z) {
    Matrix4 result{};
    result._00 = x;
    result._11 = y;
    result._22 = z;
    result._33 = 1.0f;
    return result;
}

inline internal Matrix4 rotate_rh(f32 angle, Vector3 axis) {
    Matrix4 result = identity_matrix4();
    axis = normalize_vector3(axis);

    f32 sin_t = sinf(angle);
    f32 cos_t = cosf(angle);
    f32 cos_inv = 1.0f - cos_t;

    result._00 = (cos_inv * axis.x * axis.x) + cos_t;
    result._01 = (cos_inv * axis.x * axis.y) + (axis.z * sin_t);
    result._02 = (cos_inv * axis.x * axis.z) - (axis.y * sin_t);

    result._10 = (cos_inv * axis.y * axis.x) - (axis.z * sin_t);
    result._11 = (cos_inv * axis.y * axis.y) + cos_t;
    result._12 = (cos_inv * axis.y * axis.z) + (axis.x * sin_t);

    result._20 = (cos_inv * axis.z * axis.x) + (axis.y * sin_t);
    result._21 = (cos_inv * axis.z * axis.y) - (axis.x * sin_t);
    result._22 = (cos_inv * axis.z * axis.z) + cos_t;

    return result; 
}

inline internal Matrix4 ortho_rh_zo(f32 left, f32 right, f32 bottom, f32 top, f32 _near, f32 _far) {
    Matrix4 result = identity_matrix4();
    result._00 = 2.0f / (right - left);
    result._11 = 2.0f / (top - bottom);
    result._22 = 1.0f / (_far - _near);
    result._30 = - (right + left) / (right - left);
    result._31 = - (top + bottom) / (top - bottom);
    // result._32 = - near / (far - near);
    result._32 = -(_near + _far) / (_far - _near);
    return result;
}

inline internal Matrix4 perspective_projection_rh(f32 fov, f32 aspect_ratio, f32 _near, f32 _far) {
    Matrix4 result = identity_matrix4();
    f32 c = 1.0f / tanf(fov / 2.0f);
    result._00 = c / aspect_ratio;
    result._11 = c;
    result._23 = -1.0f;

    result._22 = (_far) / (_near - _far);
    result._32 = (_near * _far) / (_near - _far);
    return result;
}

inline internal Matrix4 look_at_rh(Vector3 eye, Vector3 target, Vector3 up) {
    Vector3 F = normalize_vector3(sub_vector3(target, eye));
    Vector3 R = normalize_vector3(cross_vector3(F, up));
    Vector3 U = cross_vector3(R, F);

    Matrix4 result;
    result._00 = R.x;
    result._01 = U.x;
    result._02 = -F.x;
    result._03 = 0.f;

    result._10 = R.y;
    result._11 = U.y;
    result._12 = -F.y;
    result._13 = 0.f;

    result._20 = R.z;
    result._21 = U.z;
    result._22 = -F.z;
    result._23 = 0.f;

    result._30 = -dot_vector3(R, eye);
    result._31 = -dot_vector3(U, eye);
    result._32 =  dot_vector3(F, eye);
    result._33 = 1.f;
    return result;
}

inline internal Matrix4 look_at_lh(Vector3 eye, Vector3 target, Vector3 up) {
    Vector3 F = normalize_vector3(sub_vector3(target, eye));
    Vector3 R = normalize_vector3(cross_vector3(up, F));
    Vector3 U = cross_vector3(F, R);

    Matrix4 result = identity_matrix4();
    result._00 = R.x;
    result._10 = R.y;
    result._20 = R.z;
    result._01 = U.x;
    result._11 = U.y;
    result._21 = U.z;
    result._02 = F.x;
    result._12 = F.y;
    result._22 = F.z;
    result._30 = -dot_vector3(R, eye);
    result._31 = -dot_vector3(U, eye);
    result._32 = -dot_vector3(F, eye);
    return result;
}

inline internal Vector2 floor_vector2(Vector2 v) {
    Vector2 result;
    result.x = floorf(v.x);
    result.y = floorf(v.y);
    return result;
}

inline internal Vector3 floor_vector3(Vector3 v) {
    Vector3 result;
    result.x = floorf(v.x);
    result.y = floorf(v.y);
    result.z = floorf(v.z);
    return result;
}

inline internal Vector4 floor_vector4(Vector4 v) {
    Vector4 result;
    result.x = floorf(v.x);
    result.y = floorf(v.y);
    result.z = floorf(v.z);
    result.w = floorf(v.w);
    return result;
}

inline internal f32 length_vector2(Vector2 v) {
    f32 result;
    result = sqrtf(v.x * v.x + v.y * v.y);
    return result;
}

inline internal f32 length_vector3(Vector3 v) {
    f32 result;
    result = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
    return result;
}

inline internal f32 length_vector4(Vector4 v) {
    f32 result;
    result = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);
    return result;
}

inline internal f32 length2_vector2(Vector2 v) {
    f32 result;
    result = v.x * v.x + v.y * v.y;
    return result;
}

inline internal f32 length2_vector3(Vector3 v) {
    f32 result = v.x * v.x + v.y * v.y + v.z * v.z;
    return result;
}

inline internal f32 length2_vector4(Vector4 v) {
    f32 result = v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w;
    return result;
}

inline internal f32 angle_from_vector(Vector2 v) {
    f32 result = atan2f(v.y, v.x);
    return result;
}

inline internal Vector2 direction_from_angle(f32 angle) {
    Vector2 result;
    result.x = cosf(angle);
    result.y = sinf(angle);
    return result;
}

inline internal Vector2 normalize_vector2(Vector2 v) {
    Vector2 result;
    f32 length = length_vector2(v);
    result = (length == 0.f) ? Vector2_Zero : div_vector2f(v, length);
    return result;
}

inline internal Vector3 normalize_vector3(Vector3 v) {
    Vector3 result;
    f32 length = length_vector3(v);
    result = (length == 0.f) ? Vector3_Zero : div_vector3f(v, length);
    return result;
}

inline internal Vector4 normalize_vector4(Vector4 v) {
    Vector4 result;
    f32 length = length_vector4(v);
    result = (length == 0.f) ? Vector4_Zero : div_vector4f(v, length);
    return result;
}

inline internal f32 dot_vector2(Vector2 a, Vector2 b) {
    f32 result = a.x * b.x + a.y * b.y;
    return result;
}

inline internal f32 dot_vector3(Vector3 a, Vector3 b) {
    f32 result;
    result = a.x * b.x + a.y * b.y + a.z * b.z;
    return result; 
}

inline internal f32 dot_vector4(Vector4 a, Vector4 b) {
    f32 result;
    result = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
    return result; 
}

inline internal Vector3 cross_vector3(Vector3 a, Vector3 b) {
    Vector3 result;
    result.x = (a.y * b.z) - (a.z * b.y);
    result.y = (a.z * b.x) - (a.x * b.z);
    result.z = (a.x * b.y) - (a.y * b.x);
    return result;
}

inline internal f32 lerp(f32 a, f32 b, f32 t) {
    f32 result = (1.0f - t) * a + b * t;
    return result;
}

inline internal Vector2 lerp_vector2(Vector2 a, Vector2 b, f32 t) {
    Vector2 result;
    result = add_vector2(mul_vector2f(a, (1.0f - t)), mul_vector2f(b, t));
    return result;
}

inline internal Vector3 lerp_vector3(Vector3 a, Vector3 b, f32 t) {
    Vector3 result;
    result = add_vector3(mul_vector3f(a, (1.0f - t)), mul_vector3f(b, t));
    return result;
}

inline internal Vector4 lerp_vector4(Vector4 a, Vector4 b, f32 t) {
    Vector4 result;
    result = add_vector4(mul_vector4f(a, (1.0f - t)), mul_vector4f(b, t));
    return result;
}

inline internal Vector2Int add_vector2int(Vector2Int a, Vector2Int b) {
    Vector2Int result;
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    return result;
}

inline internal Vector3Int add_vector3int(Vector3Int a, Vector3Int b) {
    Vector3Int result;
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    result.z = a.z + b.z;
    return result;
}

inline internal Vector4Int add_vector4int(Vector4Int a, Vector4Int b) {
    Vector4Int result;
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    result.z = a.z + b.z;
    result.w = a.w + b.w;
    return result;
}

inline internal Vector2Int sub_vector2int(Vector2Int a, Vector2Int b) {
    Vector2Int result;
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    return result;
}

inline internal Vector3Int sub_vector3int(Vector3Int a, Vector3Int b) {
    Vector3Int result;
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    result.z = a.z - b.z;
    return result;
}

inline internal Vector4Int sub_vector4int(Vector4Int a, Vector4Int b) {
    Vector4Int result;
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    result.z = a.z - b.z;
    result.w = a.w - b.w;
    return result;
}

inline internal Vector2 add_vector2(Vector2 a, Vector2 b) {
    Vector2 result;
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    return result;
}

inline internal Vector3 add_vector3(Vector3 a, Vector3 b) {
    Vector3 result;
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    result.z = a.z + b.z;
    return result; 
}

inline internal Vector4 add_vector4(Vector4 a, Vector4 b) {
    Vector4 result;
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    result.z = a.z + b.z;
    result.w = a.w + b.w;
    return result;
}

inline internal Vector2 sub_vector2(Vector2 a, Vector2 b) {
    Vector2 result;
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    return result;
}

inline internal Vector3 sub_vector3(Vector3 a, Vector3 b) {
    Vector3 result;
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    result.z = a.z - b.z;
    return result; 
}

inline internal Vector4 sub_vector4(Vector4 a, Vector4 b) {
    Vector4 result;
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    result.z = a.z - b.z;
    result.w = a.w - b.w;
    return result;
}

inline internal Vector2 negate_vector2(Vector2 v) {
    Vector2 result;
    result.x = -v.x;
    result.y = -v.y;
    return result;
}

inline internal Vector3 negate_vector3(Vector3 v) {
    Vector3 result;
    result.x = -v.x;
    result.y = -v.y;
    result.z = -v.z;
    return result;
}

inline internal Vector4 negate_vector4(Vector4 v) {
    Vector4 result;
    result.x = -v.x;
    result.y = -v.y;
    result.z = -v.z;
    result.w = -v.w;
    return result;
}

inline internal Vector2 mul_vector2f(Vector2 v, f32 s) {
    Vector2 result;
    result.x = v.x * s;
    result.y = v.y * s;
    return result;
}

inline internal Vector3 mul_vector3f(Vector3 v, f32 s) {
    Vector3 result;
    result.x = v.x * s;
    result.y = v.y * s;
    result.z = v.z * s;
    return result;
}

inline internal Vector4 mul_vector4f(Vector4 v, f32 s) {
    Vector4 result;
    result.x = v.x * s;
    result.y = v.y * s;
    result.z = v.z * s;
    result.w = v.w * s;
    return result;
}

inline internal Vector2 div_vector2f(Vector2 v, f32 s) {
    Vector2 result;
    result.x = v.x / s;
    result.y = v.y / s;
    return result;
}

inline internal Vector3 div_vector3f(Vector3 v, f32 s) {
    Vector3 result;
    result.x = v.x / s;
    result.y = v.y / s;
    result.z = v.z / s;
    return result;
}

inline internal Vector4 div_vector4f(Vector4 v, f32 s) {
    Vector4 result;
    result.x = v.x / s;
    result.y = v.y / s;
    result.z = v.z / s;
    result.w = v.w / s;
    return result;
}

//@Note Function overloads
#ifdef __cplusplus
inline internal Vector4 mul(Matrix4 m, Vector4 v) {Vector4 result = mul_matrix4_vector4(m, v); return result;}
inline internal Matrix4 mul(Matrix4 a, Matrix4 b) {Matrix4 result = mul_matrix4(a, b); return result;}

inline internal Vector2 normalize(Vector2 v) {Vector2 result = normalize_vector2(v); return result;}
inline internal Vector3 normalize(Vector3 v) {Vector3 result = normalize_vector3(v); return result;}
inline internal Vector4 normalize(Vector4 v) {Vector4 result = normalize_vector4(v); return result;}

inline internal f32 dot(Vector2 a, Vector2 b) {f32 result = dot_vector2(a, b); return result;}
inline internal f32 dot(Vector3 a, Vector3 b) {f32 result = dot_vector3(a, b); return result;}
inline internal f32 dot(Vector4 a, Vector4 b) {f32 result = dot_vector4(a, b); return result;}

inline internal f32 length(Vector2 v) {f32 result = length_vector2(v); return result;}
inline internal f32 length(Vector3 v) {f32 result = length_vector3(v); return result;}
inline internal f32 length(Vector4 v) {f32 result = length_vector4(v); return result;}

inline internal f32 length2(Vector2 v) {f32 result = length2_vector2(v); return result;}
inline internal f32 length2(Vector3 v) {f32 result = length2_vector3(v); return result;}
inline internal f32 length2(Vector4 v) {f32 result = length2_vector4(v); return result;}

inline internal Vector2 lerp(Vector2 a, Vector2 b, f32 t) {Vector2 result = lerp_vector2(a, b, t); return result;}
inline internal Vector3 lerp(Vector3 a, Vector3 b, f32 t) {Vector3 result = lerp_vector3(a, b, t); return result;}
inline internal Vector4 lerp(Vector4 a, Vector4 b, f32 t) {Vector4 result = lerp_vector4(a, b, t); return result;}

inline internal Vector3 cross(Vector3 a, Vector3 b) {Vector3 result = cross_vector3(a, b); return result;}
#endif // __cplusplus
