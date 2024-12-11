#ifndef BASE_MATH_H
#define BASE_MATH_H

#include <math.h>
#ifndef PI
    #define PI     3.14159265358979323846f
#endif
#ifndef PI2
    #define PI2    6.28318530717958647693f
#endif
#ifndef PIDIV2
    #define PIDIV2 1.5707963267948966192f
#endif
#ifndef EPSILON
    #define EPSILON 0.000001f
#endif

#define Rect_Zero make_rect(0.0f, 0.0f, 0.0f, 0.0f)
#define Rect_One make_rect(1.0f, 1.0f, 1.0f, 1.0f)

#define floor_f32(x)  floorf(x)
#define trunc_f32(x)  truncf(x)
#define round_f32(x)  roundf(x)
#define mod_f32(x)    modf(x)
#define ceil_f32(x)   ceilf(x)
#define sqrt_f32(x)   sqrtf(x)

#define DegToRad(deg) ((deg)*PI/180.f)
#define RadToDeg(rad) ((rad)*180.f/PI)

#define Vector2_Zero   make_vector2(0.0f, 0.0f)
#define Vector2_One    make_vector2(1.0f, 1.0f)
#define Vector2_Half   make_vector2(0.5f, 0.5f)
#define Vector2_Left   make_vector2(-1.0f, 0.0f)
#define Vector2_Right  make_vector2(1.0f, 0.0f)
#define Vector2_Up     make_vector2(0.0f, 1.0f)
#define Vector2_Down   make_vector2(0.0f, -1.0f)

#define Vector3_Zero   make_vector3(0.0f, 0.0f, 0.0f)
#define Vector3_One    make_vector3(1.0f, 1.0f, 1.0f)
#define Vector3_Half   make_vector3(0.5f, 0.5f, 0.5f)
#define Vector3_Left   make_vector3(-1.0f, 0.0f, 0.0f)
#define Vector3_Right  make_vector3(1.0f, 0.0f, 0.0f)
#define Vector3_Up     make_vector3(0.0f, 1.0f, 0.0f)
#define Vector3_Down   make_vector3(0.0f, -1.0f, 0.0f)

#define Vector4_Zero   make_vector4(0.0f, 0.0f, 0.0f, 0.0f)
#define Vector4_One    make_vector4(1.0f, 1.0f, 1.0f, 1.0f)
#define Vector4_Half   make_vector4(0.5f, 0.5f, 0.5f, 0.5f)
#define Vector4_Left   make_vector4(-1.0f, 0.0f, 0.0f, 0.0f)
#define Vector4_Right  make_vector4(1.0f, 0.0f, 0.0f, 0.0f)
#define Vector4_Up     make_vector4(0.0f, 1.0f, 0.0f, 0.0f)
#define Vector4_Down   make_vector4(0.0f, -1.0f, 0.0f, 0.0f)

#define Vector2Int_Zero   make_vector2int(0, 0)
#define Vector2Int_One    make_vector2int(1, 1)
#define Vector2Int_Left   make_vector2int(-1, 0)
#define Vector2Int_Right  make_vector2int(1, 0)
#define Vector2Int_Up     make_vector2int(0, 1)
#define Vector2Int_Down   make_vector2int(0, -1)

#define Vector3Int_Zero   make_vector3int(0, 0, 0)
#define Vector3Int_One    make_vector3int(1, 1, 1)
#define Vector3Int_Left   make_vector3int(-1, 0, 0)
#define Vector3Int_Right  make_vector3int(1, 0, 0)
#define Vector3Int_Up     make_vector3int(0, 1, 0)
#define Vector3Int_Down   make_vector3int(0, -1, 0)

#define Vector4Int_Zero   make_vector4int(0, 0, 0, 0)
#define Vector4Int_One    make_vector4int(1, 1, 1, )
#define Vector4Int_Left   make_vector4int(-1, 0, 0, 0)
#define Vector4Int_Right  make_vector4int(1, 0, 0, 0)
#define Vector4Int_Up     make_vector4int(0, 1, 0, 0)
#define Vector4Int_Down   make_vector4int(0, -1, 0, 0)

inline internal Vector2 make_vector2(f32 x, f32 y) { Vector2 result = {x, y}; return result; }
inline internal Vector3 make_vector3(f32 x, f32 y, f32 z) { Vector3 result = {x, y, z}; return result; }
inline internal Vector4 make_vector4(f32 x, f32 y, f32 z, f32 w) { Vector4 result = {x, y, z, w}; return result; }

inline internal Matrix4 identity_matrix4();
inline internal Matrix4 mul_matrix4(Matrix4 b, Matrix4 a);
inline internal Vector4 mul_matrix4_vector4(Matrix4 m, Vector4 v);
inline internal Matrix4 transpose_matrix4(Matrix4 m);
inline internal Matrix4 translate_matrix4(f32 x, f32 y, f32 z);
inline internal Matrix4 inverse_translate_matrix4(f32 x, f32 y, f32 z);
inline internal Matrix4 scale_matrix4(f32 x, f32 y, f32 z);
inline internal Matrix4 rotate_rh(f32 angle, Vector3 axis);
inline internal Matrix4 ortho_rh_zo(f32 left, f32 right, f32 bottom, f32 top, f32 near, f32 far);
inline internal Matrix4 perspective_projection_rh(f32 fov, f32 aspect_ratio, f32 near, f32 far);
inline internal Matrix4 look_at_rh(Vector3 eye, Vector3 target, Vector3 up);
inline internal Matrix4 look_at_lh(Vector3 eye, Vector3 target, Vector3 up);

inline internal Vector2 add_vector2(Vector2 a, Vector2 b);
inline internal Vector3 add_vector3(Vector3 a, Vector3 b);
inline internal Vector4 add_vector4(Vector4 a, Vector4 b);
inline internal Vector2 sub_vector2(Vector2 a, Vector2 b);
inline internal Vector3 sub_vector3(Vector3 a, Vector3 b);
inline internal Vector4 sub_vector4(Vector4 a, Vector4 b);
inline internal Vector2 negate_vector2(Vector2 v);
inline internal Vector3 negate_vector3(Vector3 v);
inline internal Vector4 negate_vector4(Vector4 v);

inline internal Vector2 mul_vector2f(Vector2 v, f32 s);
inline internal Vector3 mul_vector3f(Vector3 v, f32 s);
inline internal Vector4 mul_vector4f(Vector4 v, f32 s);
inline internal Vector2 div_vector2f(Vector2 v, f32 s);
inline internal Vector3 div_vector3f(Vector3 v, f32 s);
inline internal Vector4 div_vector4f(Vector4 v, f32 s);

inline internal f32 length_vector2(Vector2 v);
inline internal f32 length_vector3(Vector3 v);
inline internal f32 length_vector4(Vector4 v);
inline internal f32 length2_vector2(Vector2 v);
inline internal f32 length2_vector3(Vector3 v);
inline internal f32 length2_vector4(Vector4 v);
inline internal Vector2 normalize_vector2(Vector2 v);
inline internal Vector3 normalize_vector3(Vector3 v);
inline internal Vector4 normalize_vector4(Vector4 v);

inline internal f32 lerp(f32 a, f32 b, f32 t);
inline internal Vector2 lerp_vector2(Vector2 a, Vector2 b, f32 t);
inline internal Vector3 lerp_vector3(Vector3 a, Vector3 b, f32 t);
inline internal Vector4 lerp_vector4(Vector4 a, Vector4 b, f32 t);

inline internal f32 dot_vector2(Vector2 a, Vector2 b);
inline internal f32 dot_vector3(Vector3 a, Vector3 b);
inline internal f32 dot_vector4(Vector4 a, Vector4 b);

inline internal Vector3 cross_vector3(Vector3 a, Vector3 b);

//@Note Function overloads
#ifdef __cplusplus
inline internal Vector2 normalize(Vector2 v);
inline internal Vector3 normalize(Vector3 v);
inline internal Vector4 normalize(Vector4 v);
inline internal f32 dot(Vector2 a, Vector2 b);
inline internal f32 dot(Vector3 a, Vector3 b);
inline internal f32 dot(Vector4 a, Vector4 b);
inline internal Vector2 lerp(Vector2 a, Vector2 b, f32 t);
inline internal Vector3 lerp(Vector3 a, Vector3 b, f32 t);
inline internal Vector4 lerp(Vector4 a, Vector4 b, f32 t);
inline internal f32 length(Vector2 v);
inline internal f32 length(Vector3 v);
inline internal f32 length(Vector4 v);
inline internal f32 length2(Vector2 v);
inline internal f32 length2(Vector3 v);
inline internal f32 length2(Vector4 v);

inline internal Vector2Int add_vector2int(Vector2Int a, Vector2Int b);
inline internal Vector3Int add_vector3int(Vector3Int a, Vector3Int b);
inline internal Vector4Int add_vector4int(Vector4Int a, Vector4Int b);

inline internal Vector2Int sub_vector2int(Vector2Int a, Vector2Int b);
inline internal Vector3Int sub_vector3int(Vector3Int a, Vector3Int b);
inline internal Vector4Int sub_vector4int(Vector4Int a, Vector4Int b);

//@Note Operator overloads
inline Vector2Int operator+(Vector2Int a, Vector2Int b) {Vector2Int result = add_vector2int(a, b); return result;}
inline Vector3Int operator+(Vector3Int a, Vector3Int b) {Vector3Int result = add_vector3int(a, b); return result;}
inline Vector4Int operator+(Vector4Int a, Vector4Int b) {Vector4Int result = add_vector4int(a, b); return result;}

inline Vector2 operator+(Vector2 a, Vector2 b) {Vector2 result = add_vector2(a, b); return result;}
inline Vector3 operator+(Vector3 a, Vector3 b) {Vector3 result = add_vector3(a, b); return result;}
inline Vector4 operator+(Vector4 a, Vector4 b) {Vector4 result = add_vector4(a, b); return result;}

inline Vector2 operator-(Vector2 v) {Vector2 result = negate_vector2(v); return result;}
inline Vector3 operator-(Vector3 v) {Vector3 result = negate_vector3(v); return result;}
inline Vector4 operator-(Vector4 v) {Vector4 result = negate_vector4(v); return result;}

inline Vector2Int operator-(Vector2Int a, Vector2Int b) {Vector2Int result = sub_vector2int(a, b); return result;}
inline Vector3Int operator-(Vector3Int a, Vector3Int b) {Vector3Int result = sub_vector3int(a, b); return result;}
inline Vector4Int operator-(Vector4Int a, Vector4Int b) {Vector4Int result = sub_vector4int(a, b); return result;}

inline Vector2 operator-(Vector2 a, Vector2 b) {Vector2 result = sub_vector2(a, b); return result;}
inline Vector3 operator-(Vector3 a, Vector3 b) {Vector3 result = sub_vector3(a, b); return result;}
inline Vector4 operator-(Vector4 a, Vector4 b) {Vector4 result = sub_vector4(a, b); return result;}

inline Vector2Int operator+=(Vector2Int &a, Vector2Int b) {a = a + b; return a;}
inline Vector3Int operator+=(Vector3Int &a, Vector3Int b) {a = a + b; return a;}
inline Vector4Int operator+=(Vector4Int &a, Vector4Int b) {a = a + b; return a;}

inline Vector2 operator+=(Vector2 &a, Vector2 b) {a = a + b; return a;}
inline Vector3 operator+=(Vector3 &a, Vector3 b) {a = a + b; return a;}
inline Vector4 operator+=(Vector4 &a, Vector4 b) {a = a + b; return a;}

inline Vector2 operator-=(Vector2 &a, Vector2 b) {a = a - b; return a;}
inline Vector3 operator-=(Vector3 &a, Vector3 b) {a = a - b; return a;}
inline Vector4 operator-=(Vector4 &a, Vector4 b) {a = a - b; return a;}

inline Vector2 operator*(Vector2 v, f32 s) {Vector2 result = mul_vector2f(v, s); return result;}
inline Vector3 operator*(Vector3 v, f32 s) {Vector3 result = mul_vector3f(v, s); return result;}
inline Vector4 operator*(Vector4 v, f32 s) {Vector4 result = mul_vector4f(v, s); return result;}

inline Vector2 operator*(f32 s, Vector2 v) {Vector2 result = mul_vector2f(v, s); return result;}
inline Vector3 operator*(f32 s, Vector3 v) {Vector3 result = mul_vector3f(v, s); return result;}
inline Vector4 operator*(f32 s, Vector4 v) {Vector4 result = mul_vector4f(v, s); return result;}

inline Vector2 operator/(Vector2 v, f32 s) {Vector2 result = div_vector2f(v, s); return result;}
inline Vector3 operator/(Vector3 v, f32 s) {Vector3 result = div_vector3f(v, s); return result;}
inline Vector4 operator/(Vector4 v, f32 s) {Vector4 result = div_vector4f(v, s); return result;}

inline Vector2 operator*=(Vector2 &v, f32 s) {v = v * s; return v;}
inline Vector3 operator*=(Vector3 &v, f32 s) {v = v * s; return v;}
inline Vector4 operator*=(Vector4 &v, f32 s) {v = v * s; return v;}

inline Vector2 operator/=(Vector2 &v, f32 s) {v = v / s; return v;}
inline Vector3 operator/=(Vector3 &v, f32 s) {v = v / s; return v;}
inline Vector4 operator/=(Vector4 &v, f32 s) {v = v / s; return v;}

inline bool operator==(Vector2 a, Vector2 b) {bool result = a.x == b.x && a.y == b.y; return result;}
inline bool operator==(Vector3 a, Vector3 b) {bool result = a.x == b.x && a.y == b.y && a.z == b.z; return result;}
inline bool operator==(Vector4 a, Vector4 b) {bool result = a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w; return result;}

inline bool operator!=(Vector2 a, Vector2 b) {return !(a == b);}
inline bool operator!=(Vector3 a, Vector3 b) {return !(a == b);}
inline bool operator!=(Vector4 a, Vector4 b) {return !(a == b);}

inline Matrix4 operator*(Matrix4 a, Matrix4 b) {Matrix4 result = mul_matrix4(a, b); return result;}
#endif // __cplusplus

#endif // BASE_MATH_H
