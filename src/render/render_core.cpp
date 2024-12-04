global R_2D_Vertex g_r_2d_vertex_nil;
global R_3D_Vertex g_r_3d_vertex_nil;
global R_2D_Rect g_r_2d_rect_nil;

internal R_Handle r_handle_zero() {
    return (R_Handle)0;
}

internal R_2D_Vertex r_2d_vertex(f32 x, f32 y, f32 u, f32 v, v4 color) {
    R_2D_Vertex result = g_r_2d_vertex_nil;
    result.dst.x = x;
    result.dst.y = y;
    result.src.x = u;
    result.src.y = v;
    result.color = color;
    return result;
}

internal R_2D_Rect r_2d_rect(Rect dst, Rect src, v4 color, f32 border_thickness, f32 omit_tex) {
    R_2D_Rect result = g_r_2d_rect_nil;
    result.dst = dst;
    result.src = src;
    result.color = color;
    result.border_thickness = border_thickness;
    result.omit_tex = omit_tex;
    return result;
}

internal R_3D_Vertex r_3d_vertex(v3 pos, v4 color, v2 tex) {
    R_3D_Vertex result = g_r_3d_vertex_nil;
    result.pos.x = pos.x;
    result.pos.y = pos.y;
    result.pos.z = pos.z;
    result.color = color;
    result.tex = tex;
    return result;
}
