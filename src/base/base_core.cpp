
global Profile_Manager g_profile_manager;

internal void profile_scope_begin(char *name) {
    Profile_Scope *scope = &g_profile_manager.scopes[g_profile_manager.scope_count];
    scope->name = name;
    scope->start_clock = get_wall_clock();
}

internal void profile_scope_end() {
    Profile_Scope *scope = &g_profile_manager.scopes[g_profile_manager.scope_count];
    scope->ms_elapsed += get_ms_elapsed(scope->start_clock, get_wall_clock());
    g_profile_manager.scope_count++;
}

internal Vector2Int make_vector2int(s32 x, s32 y) {Vector2Int result; result.x = x; result.y = y; return result;}
internal Vector3Int make_vector3int(s32 x, s32 y, s32 z) {Vector3Int result; result.x = x; result.y = y; result.z = z; return result;}
internal Vector4Int make_vector4int(s32 x, s32 y, s32 z, s32 w) {Vector4Int result; result.x = x; result.y = y; result.z = z; result.w = w; return result;}

internal Vector2    vector2_from_vector2int(Vector2Int v) {Vector2 result; result.x = (f32)v.x; result.y = (f32)v.y; return result;}
internal Vector2Int vector2int_from_vector2(Vector2 v) {Vector2Int result; result.x = (s32)v.x; result.y = (s32)v.y; return result;}

internal Vector3    vector3_from_vector3int(Vector3Int v) {Vector3 result; result.x = (f32)v.x; result.y = (f32)v.y; result.z = (f32)v.z; return result;}
internal Vector3Int vector3int_from_vector3(Vector3 v) {Vector3Int result; result.x = (s32)v.x; result.y = (s32)v.y; result.z = (s32)v.z; return result;}

internal Rng_U64 rng_u64(u64 min, u64 max) { if (min > max) Swap(u64, min, max); Rng_U64 result; result.min = min; result.max = max; return result; }
internal u64 rng_u64_len(Rng_U64 rng) { u64 result = rng.max - rng.min; return result; }

internal Rng_S64 rng_s64(s64 min, s64 max) { if (min > max) Swap(s64, min, max); Rng_S64 result; result.min = min; result.max = max; return result; }
internal s64 rng_s64_len(Rng_S64 rng) { s64 result = rng.max - rng.min; return result; }

internal inline Rect make_rect(f32 x, f32 y, f32 w, f32 h) {Rect result = {x, y, x + w, y + h}; return result;}
internal inline Rect make_rect(Vector2 p, Vector2 dim) {Rect result = {p.x, p.y, p.x + dim.x, p.y + dim.y}; return result;}
internal inline Rect make_rect_center(Vector2 position, Vector2 size) {Rect result = make_rect(position.x - size.x/2.0f, position.y - size.y/2.0f, size.x, size.y); return result;}

internal void shift_rect(Rect *rect, f32 x, f32 y) { rect->x0 += x; rect->x1 += x; rect->y0 += y; rect->y1 += y; }

internal Vector2 rect_dim(Rect rect) {Vector2 result; result.x = rect.x1 - rect.x0; result.y = rect.y1 - rect.y0; return result;}
internal inline f32 rect_height(Rect rect) {f32 result = rect.y1 - rect.y0; return result;}
internal inline f32 rect_width(Rect rect) {f32 result = rect.x1 - rect.x0; return result;}

internal inline bool operator==(Rect a, Rect b) {
    return a.x0 == b.x0 && a.x1 == b.x1 && a.y0 == b.y0 && a.y1 == b.y1;
}
internal inline bool operator!=(Rect a, Rect b) {
    return !(a == b);
}

internal bool rect_contains(Rect rect, Vector2 v) {
    bool result = v.x >= rect.x0 &&
        v.x <= rect.x1 &&
        v.y >= rect.y0 &&
        v.y <= rect.y1;
    return result; 
}

internal Axis2 axis_flip(Axis2 axis) {Axis2 result; if (axis == Axis_X) result = Axis_Y; else result = Axis_X; return result;}

internal inline bool operator==(Vector3Int left, Vector3Int right) {return left.x == right.x && left.y == right.y && left.z == right.z;}
internal inline bool operator!=(Vector3Int left, Vector3Int right) {return !(left == right);}

internal inline RGBA make_rgba(u8 r, u8 g, u8 b, u8 a) {
    RGBA result;
    result.r = r;
    result.g = g;
    result.b = b;
    result.a = a;
    return result;
}
