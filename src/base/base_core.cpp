
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

internal v2_s32 v2s32(s32 x, s32 y) {v2_s32 result; result.x = x; result.y = y; return result;}
internal v2 v2_v2s32(v2_s32 v) {v2 result; result.x = (f32)v.x; result.y = (f32)v.y; return result;}
internal v2_s32 v2s32_v2(v2 v) {v2_s32 result; result.x = (s32)v.x; result.y = (s32)v.y; return result;}

internal v3_s32 v3s32(s32 x, s32 y, s32 z) {v3_s32 result; result.x = x; result.y = y; result.z = z; return result;}

internal Rng_U64 rng_u64(u64 min, u64 max) { if (min > max) Swap(u64, min, max); Rng_U64 result; result.min = min; result.max = max; return result; }
internal u64 rng_u64_len(Rng_U64 rng) { u64 result = rng.max - rng.min; return result; }

internal Rng_S64 rng_s64(s64 min, s64 max) { if (min > max) Swap(s64, min, max); Rng_S64 result; result.min = min; result.max = max; return result; }
internal s64 rng_s64_len(Rng_S64 rng) { s64 result = rng.max - rng.min; return result; }

internal inline Rect make_rect(f32 x, f32 y, f32 w, f32 h) {Rect result = {x, y, x + w, y + h}; return result;}
internal inline Rect make_rect(v2 p, v2 dim) {Rect result = {p.x, p.y, p.x + dim.x, p.y + dim.y}; return result;}
internal inline Rect make_rect_center(v2 position, v2 size) {Rect result = make_rect(position.x - size.x/2.0f, position.y - size.y/2.0f, size.x, size.y); return result;}

internal void shift_rect(Rect *rect, f32 x, f32 y) { rect->x0 += x; rect->x1 += x; rect->y0 += y; rect->y1 += y; }

internal v2 rect_dim(Rect rect) {v2 result; result.x = rect.x1 - rect.x0; result.y = rect.y1 - rect.y0; return result;}
internal inline f32 rect_height(Rect rect) {f32 result = rect.y1 - rect.y0; return result;}
internal inline f32 rect_width(Rect rect) {f32 result = rect.x1 - rect.x0; return result;}

internal inline bool operator==(Rect a, Rect b) {
    return a.x0 == b.x0 && a.x1 == b.x1 && a.y0 == b.y0 && a.y1 == b.y1;
}
internal inline bool operator!=(Rect a, Rect b) {
    return !(a == b);
}

internal bool rect_contains(Rect rect, v2 v) {
    bool result = v.x >= rect.x0 &&
        v.x <= rect.x1 &&
        v.y >= rect.y0 &&
        v.y <= rect.y1;
    return result; 
}

internal Axis2 axis_flip(Axis2 axis) {Axis2 result; if (axis == Axis_X) result = Axis_Y; else result = Axis_X; return result;}

internal inline bool operator==(v3_s32 left, v3_s32 right) {return left.x == right.x && left.y == right.y && left.z == right.z;}
internal inline bool operator!=(v3_s32 left, v3_s32 right) {return !(left == right);}
