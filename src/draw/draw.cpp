#include "draw.h"

global Arena *draw_arena;
global Arena *draw_temp_arena;
global Draw_Bucket *draw_bucket;

internal void draw_begin(OS_Handle window_handle) {
    if (draw_arena == NULL) {
        draw_arena = arena_alloc(get_virtual_allocator(), MB(128));
    }

    if (draw_temp_arena == NULL) {
        draw_temp_arena = arena_alloc(get_virtual_allocator(), MB(4));
    }

    draw_bucket = push_array(draw_arena, Draw_Bucket, 1);
    *draw_bucket = {};

    Vector2 window_dim = os_get_window_dim(window_handle);
    draw_bucket->clip = make_rect(0.f, 0.f, window_dim.x, window_dim.y);
}

internal void draw_end() {
    arena_clear(draw_arena);
    arena_clear(draw_temp_arena);
    draw_bucket = nullptr;
}

internal void draw_push_batch_node(R_Batch_List *list, R_Batch_Node *node) {
    SLLQueuePush(list->first, list->last, node);
    list->count += 1;
}

internal void draw_set_texture(R_Handle tex) {
    // R_Batch_Node *node = draw_bucket->batches.last;
    // if (node == NULL || node->batch.bytes > 0) {
    //     node = push_array(draw_arena, R_Batch_Node, 1);
    //     draw_push_batch_node(&draw_bucket->batches, node);
    // }
    draw_bucket->tex = tex;
}

internal void draw_set_xform(Matrix4 xform) {
    // R_Batch_Node *node = draw_bucket->batches.last;
    // if (node == NULL || node->batch.bytes > 0) {
    //     node = push_array(draw_arena, R_Batch_Node, 1);
    //     draw_push_batch_node(&draw_bucket->batches, node);
    // }
    draw_bucket->xform = xform;
}

internal void draw_set_clip(Rect clip) {
    draw_bucket->clip = clip;
    R_Batch_Node *node = draw_bucket->batches.last;
    node = push_array(draw_arena, R_Batch_Node, 1);
    R_Params_UI *params_ui = push_array(draw_arena, R_Params_UI, 1);
    node->batch.params.kind = R_ParamsKind_UI;
    node->batch.params.params_ui = params_ui;
    params_ui->tex = draw_bucket->tex;
    params_ui->xform = draw_bucket->xform;
    node->batch.v = (u8 *)draw_arena->current + draw_arena->current->pos;
}

internal void draw_set_rasterizer(R_Rasterizer_Kind rasterizer) {
    R_Batch_Node *node = draw_bucket->batches.last;
    if (node == NULL || node->batch.bytes > 0) {
        node = push_array(draw_arena, R_Batch_Node, 1);
        draw_push_batch_node(&draw_bucket->batches, node);
    }
    draw_bucket->rasterizer = rasterizer;
}

internal void draw_set_sampler(R_Sampler_Kind sampler) {
    draw_bucket->sampler = sampler;
}

internal void draw_batch_push_vertex(R_Batch *batch, R_2D_Vertex src) {
    R_2D_Vertex *dst = push_array(draw_arena, R_2D_Vertex, 1);
    *dst = src;
    batch->bytes += sizeof(R_2D_Vertex);
}

inline internal void draw_batch_push_3d_vertex(R_Batch *batch, R_3D_Vertex src) {
    R_3D_Vertex *dst = push_array(draw_arena, R_3D_Vertex, 1);
    *dst = src;
    batch->bytes += sizeof(R_3D_Vertex);
}

internal void draw_batch_push_rect(R_Batch *batch, R_2D_Rect rect) {
    R_2D_Rect *dst = push_array(draw_arena, R_2D_Rect, 1);
    *dst = rect;
    batch->bytes += sizeof(R_2D_Rect);
}

internal void draw_string_truncated(String8 string, Font *font, Vector4 color, Vector2 offset, Rect bounds) {
    R_Handle tex = draw_bucket->tex;
    R_Params_Kind params_kind = draw_bucket->params_kind;
    R_Batch_Node *node = draw_bucket->batches.last;
    if (!node || tex != font->texture || params_kind != R_ParamsKind_UI) {
        node = push_array(draw_arena, R_Batch_Node, 1);
        node->batch.params.kind = R_ParamsKind_UI;
        R_Params_UI *params_ui = push_array(draw_arena, R_Params_UI, 1);
        node->batch.params.params_ui = params_ui;
        params_ui->tex = font->texture;
        params_ui->clip = draw_bucket->clip;
        params_ui->xform = draw_bucket->xform;
        draw_push_batch_node(&draw_bucket->batches, node);
        draw_bucket->tex = font->texture;
        draw_bucket->params_kind = R_ParamsKind_UI;
        node->batch.v = (u8 *)draw_arena->current + draw_arena->current->pos;
    }

    Vector2 cursor = offset;
    for (u64 i = 0; i < string.count; i++) {
        u8 c = string.data[i];
        if (c == '\n') {
            cursor.x = offset.x;
            cursor.y += font->glyph_height;
            continue;
        }
        Glyph g = font->glyphs[c];

        Rect dst;
        dst.x0 = cursor.x + g.bl;
        dst.x1 = dst.x0 + g.bx;
        dst.y0 = cursor.y - g.bt + font->ascend;
        dst.y1 = dst.y0 + g.by;

        cursor.x += g.ax;

        if (dst.y1 < bounds.y0 || dst.x1 < bounds.x0 || dst.y0 > bounds.y1 || dst.x0 > bounds.x1) {
            continue; 
        }

        Rect src;
        src.x0 = g.to;
        src.y0 = 0.f;
        src.x1 = src.x0 + (g.bx / (f32)font->width);
        src.y1 = src.y0 + (g.by / (f32)font->height);

        R_2D_Vertex tl = r_2d_vertex(dst.x0, dst.y0, src.x0, src.y0, color);
        R_2D_Vertex tr = r_2d_vertex(dst.x1, dst.y0, src.x1, src.y0, color);
        R_2D_Vertex bl = r_2d_vertex(dst.x0, dst.y1, src.x0, src.y1, color);
        R_2D_Vertex br = r_2d_vertex(dst.x1, dst.y1, src.x1, src.y1, color);

        draw_batch_push_vertex(&node->batch, bl);
        draw_batch_push_vertex(&node->batch, tl);
        draw_batch_push_vertex(&node->batch, tr);
        draw_batch_push_vertex(&node->batch, br);
    }
}

internal void draw_text(String8 text, Font *font, Vector4 color, Vector2 offset) {
    R_Handle tex = draw_bucket->tex;
    R_Params_Kind params_kind = draw_bucket->params_kind;
    R_Batch_Node *node = draw_bucket->batches.last;
    if (!node || tex != font->texture || params_kind != R_ParamsKind_UI) {
        node = push_array(draw_arena, R_Batch_Node, 1);
        R_Params_UI *params_ui = push_array(draw_arena, R_Params_UI, 1);
        node->batch.params.kind = R_ParamsKind_UI;
        node->batch.params.params_ui = params_ui;
        params_ui->tex = font->texture;
        params_ui->clip = draw_bucket->clip;
        params_ui->xform = draw_bucket->xform;
        draw_push_batch_node(&draw_bucket->batches, node);
        draw_bucket->tex = font->texture;
        draw_bucket->params_kind = R_ParamsKind_UI;
        node->batch.v = (u8 *)draw_arena->current + draw_arena->current->pos;
    }

    Vector2 cursor = offset;
    for (u64 i = 0; i < text.count; i++) {
        u8 c = text.data[i];
        if (c == '\n') {
            cursor.x = offset.x;
            cursor.y += font->glyph_height;
            continue;
        }
        Glyph g = font->glyphs[c];

        Rect dst;
        dst.x0 = cursor.x + g.bl;
        dst.x1 = dst.x0 + g.bx;
        dst.y0 = cursor.y - g.bt + font->ascend;
        dst.y1 = dst.y0 + g.by;

        Rect src;
        src.x0 = g.to;
        src.y0 = 0.f;
        src.x1 = src.x0 + (g.bx / (f32)font->width);
        src.y1 = src.y0 + (g.by / (f32)font->height);

        R_2D_Rect rect = r_2d_rect(dst, src, color, 0.f, 0.f);
        draw_batch_push_rect(&node->batch, rect);
        cursor.x += g.ax;
    }
}

internal void draw_textf(Font *font, Vector4 color, Vector2 offset, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    String8 string = str8_pushfv(draw_temp_arena, fmt, args);
    va_end(args);
    draw_text(string, font, color, offset);
}

internal void draw_ui_img(R_Handle img, Rect dst, Rect src, Vector4 color) {
    R_Handle tex = draw_bucket->tex;
    R_Params_Kind params_kind = draw_bucket->params_kind;
    R_Batch_Node *node = draw_bucket->batches.last;
    if (!node || tex != img || params_kind != R_ParamsKind_UI) {
        node = push_array(draw_arena, R_Batch_Node, 1);
        R_Params_UI *params_ui = push_array(draw_arena, R_Params_UI, 1);
        node->batch.params.kind = R_ParamsKind_UI;
        node->batch.params.params_ui = params_ui;
        params_ui->tex = img;
        params_ui->clip = draw_bucket->clip;
        params_ui->xform = draw_bucket->xform;
        draw_push_batch_node(&draw_bucket->batches, node);
        draw_bucket->params_kind = R_ParamsKind_UI;
        draw_bucket->tex = img;
        node->batch.v = (u8 *)draw_arena->current + draw_arena->current->pos;
    }

    R_2D_Rect rect = r_2d_rect(dst, src, color, 0.f, 0.f);
    draw_batch_push_rect(&node->batch, rect);
}

internal void draw_ui_rect(Rect dst, Vector4 color, f32 border_thickness) {
    R_Handle tex = draw_bucket->tex;
    R_Params_Kind params_kind = draw_bucket->params_kind;
    R_Batch_Node *node = draw_bucket->batches.last;
    if (!node || tex != 0 || params_kind != R_ParamsKind_UI) {
        node = push_array(draw_arena, R_Batch_Node, 1);
        R_Params_UI *params_ui = push_array(draw_arena, R_Params_UI, 1);
        node->batch.params.kind = R_ParamsKind_UI;
        node->batch.params.params_ui = params_ui;
        params_ui->tex = {0};
        params_ui->clip = draw_bucket->clip;
        params_ui->xform = draw_bucket->xform;
        draw_push_batch_node(&draw_bucket->batches, node);
        draw_bucket->params_kind = R_ParamsKind_UI;
        draw_bucket->tex = {0};
        node->batch.v = (u8 *)draw_arena->current + draw_arena->current->pos;
    }
    Assert(node);

    Rect src = {};
    R_2D_Rect rect = r_2d_rect(dst, src, color, border_thickness, 1.f);
    draw_batch_push_rect(&node->batch, rect);
}

internal void draw_ui_rect_outline(Rect rect, Vector4 color) {
    draw_ui_rect(make_rect(rect.x0, rect.y0, rect_width(rect), 1), color, 0.f);
    draw_ui_rect(make_rect(rect.x0, rect.y0, 1, rect_height(rect)), color, 0.f);
    draw_ui_rect(make_rect(rect.x1 - 1, rect.y0, 1, rect_height(rect)), color, 0.f);
    draw_ui_rect(make_rect(rect.x0, rect.y1 - 1, rect_width(rect), 1), color, 0.f);
}

internal void draw_ui_box(UI_Box *box) {
    if (box->flags & UI_BoxFlag_DrawBackground) {
        draw_ui_rect(box->rect, box->background_color, box->border_thickness);
    }
    if (box->flags & UI_BoxFlag_DrawBorder) {
        // draw_ui_rect_outline(box->rect, box->border_color);
    }
    if (box->flags & UI_BoxFlag_DrawHotEffects) {
        Vector4 hot_color = box->hover_color;
        hot_color.w *= box->hot_t;
        draw_ui_rect(box->rect, hot_color, box->border_thickness);
    }
    // if (box->flags & UI_BoxFlag_DrawActiveEffects) {
    //     Vector4 active_color = VECTOR4(1.f, 1.f, 1.f, 1.f);
    //     active_color.w *= box->hot_t;
    //     draw_ui_rect(box->rect, active_color);
    // }
    if (box->flags & UI_BoxFlag_DrawText) {
        Vector2 text_position = ui_text_position(box);
        text_position += box->view_offset;
        draw_text(box->string, box->font, box->text_color, text_position);
    }

    if (box->custom_draw_proc) {
        box->custom_draw_proc(box, box->draw_data);
    }
}

internal void draw_ui_layout(UI_Box *box) {
    if (box != ui_root()) {
        draw_ui_box(box);
    }

    for (UI_Box *child = box->first; child != nullptr; child = child->next) {
        draw_ui_layout(child);
    }
}

internal void draw_quad_pro(R_Handle img, Rect src, Rect dst, Vector2 origin, f32 rotation, Vector4 color) {
    R_Handle tex = draw_bucket->tex;
    R_Params_Kind params_kind = draw_bucket->params_kind;
    R_Batch_Node *node = draw_bucket->batches.last;
    if (node == NULL || tex != img || params_kind != R_ParamsKind_Quad) {
        node = push_array(draw_arena, R_Batch_Node, 1);
        R_Params_Quad *params_quad = push_array(draw_arena, R_Params_Quad, 1);
        draw_push_batch_node(&draw_bucket->batches, node);
        node->batch.params.kind = R_ParamsKind_Quad;
        node->batch.params.params_quad = params_quad;
        params_quad->xform = draw_bucket->xform;
        params_quad->tex = img;
        node->batch.v = (u8 *)draw_arena->current + draw_arena->current->pos;
        draw_bucket->tex = img;
        draw_bucket->params_kind = R_ParamsKind_Quad;
        params_quad->sampler = draw_bucket->sampler;
    }

    Vector2 tl = Vector2_Zero;
    Vector2 tr = Vector2_Zero;
    Vector2 bl = Vector2_Zero;
    Vector2 br = Vector2_Zero;

    if (rotation == 0.f) {
        tl = make_vector2(dst.x0, dst.y1);
        tr = make_vector2(dst.x1, dst.y1);
        bl = make_vector2(dst.x0, dst.y0);
        br = make_vector2(dst.x1, dst.y0);
    } else {
        f32 S = sinf(DegToRad(rotation));
        f32 C = cosf(DegToRad(rotation));
        f32 x = dst.x0;
        f32 y = dst.y0;
        f32 dx = -origin.x;
        f32 dy = -origin.y;
        f32 dw = rect_width(dst);
        f32 dh = rect_height(dst);

        tl.x = x + dx * C - (dy + dh) * S;
        tl.y = y + dx * S + (dy + dh) * C;
        tr.x = x + (dx + dw) * C - (dy + dh) * S;
        tr.y = y + (dx + dw) * S + (dy + dh) * C;

        bl.x = x + dx * C - dy * S;
        bl.y = y + dx * S + dy * C;
        br.x = x + (dx + dw) * C - dy * S;
        br.y = y + (dx + dw) * S + dy * C;
    }

    R_2D_Vertex top_left     = r_2d_vertex(tl.x, tl.y, src.x0, src.y0, color);
    R_2D_Vertex top_right    = r_2d_vertex(tr.x, tr.y, src.x1, src.y0, color);
    R_2D_Vertex bottom_left  = r_2d_vertex(bl.x, bl.y, src.x0, src.y1, color);
    R_2D_Vertex bottom_right = r_2d_vertex(br.x, br.y, src.x1, src.y1, color);

    draw_batch_push_vertex(&node->batch, bottom_left);
    draw_batch_push_vertex(&node->batch, bottom_right);
    draw_batch_push_vertex(&node->batch, top_right);
    draw_batch_push_vertex(&node->batch, top_left);
}

internal void draw_quad(R_Handle img, Rect dst, Rect src) {
    draw_quad_pro(img, src, dst, Vector2_Zero, 0.f, Vector4_Zero);
}

internal void draw_rect(Rect dst, Vector4 color) {
    R_Handle tex = draw_bucket->tex;
    R_Params_Kind params_kind = draw_bucket->params_kind;
    R_Batch_Node *node = draw_bucket->batches.last;
    if (node == NULL || tex != r_handle_zero() || params_kind != R_ParamsKind_Quad) {
        node = push_array(draw_arena, R_Batch_Node, 1);
        R_Params_Quad *params_quad = push_array(draw_arena, R_Params_Quad, 1);
        draw_push_batch_node(&draw_bucket->batches, node);
        node->batch.params.kind = R_ParamsKind_Quad;
        node->batch.params.params_quad = params_quad;
        params_quad->xform = draw_bucket->xform;
        params_quad->tex = 0;
        node->batch.v = (u8 *)draw_arena->current + draw_arena->current->pos;
        draw_bucket->tex = 0;
        draw_bucket->params_kind = R_ParamsKind_Quad;
    }

    R_2D_Vertex tl = r_2d_vertex(dst.x0, dst.y0, 0.f, 0.f, color);
    R_2D_Vertex tr = r_2d_vertex(dst.x1, dst.y0, 0.f, 0.f, color);
    R_2D_Vertex bl = r_2d_vertex(dst.x0, dst.y1, 0.f, 0.f, color);
    R_2D_Vertex br = r_2d_vertex(dst.x1, dst.y1, 0.f, 0.f, color);

    draw_batch_push_vertex(&node->batch, bl);
    draw_batch_push_vertex(&node->batch, tl);
    draw_batch_push_vertex(&node->batch, tr);
    draw_batch_push_vertex(&node->batch, br);
}

internal void draw_rect_outline(Rect rect, Vector4 color) {
    draw_rect(make_rect(rect.x0, rect.y0, rect_width(rect), 1), color);
    draw_rect(make_rect(rect.x0, rect.y0, 1, rect_height(rect)), color);
    draw_rect(make_rect(rect.x1 - 1, rect.y0, 1, rect_height(rect)), color);
    draw_rect(make_rect(rect.x0, rect.y1 - 1, rect_width(rect), 1), color);
}

internal void draw_chunks(Chunk_List chunks, Vector3 position, Frustum frustum, Matrix4 projection, Matrix4 view, Texture_Atlas *atlas, R_Rasterizer_Kind rasterizer) {
    R_Batch_Node *node = draw_bucket->batches.last;
    node = push_array(draw_arena, R_Batch_Node, 1);
    draw_push_batch_node(&draw_bucket->batches, node);

    node->batch.params.kind = R_ParamsKind_Blocks;
    R_Params_Blocks *params_blocks = push_array(draw_arena, R_Params_Blocks, 1);
    node->batch.params.params_blocks = params_blocks;
    params_blocks->projection = projection;
    params_blocks->view = view;
    params_blocks->atlas = atlas;
    params_blocks->rasterizer = rasterizer;
    params_blocks->chunks = chunks;
    params_blocks->frustum = frustum;
    params_blocks->position = position;
}

internal void draw_3d_mesh_begin(Matrix4 projection, Matrix4 view, R_Handle tex, R_Rasterizer_Kind rasterizer) {
    R_Batch_Node *node = push_array(draw_arena, R_Batch_Node, 1);
    draw_push_batch_node(&draw_bucket->batches, node);
    node->batch.params.kind = R_ParamsKind_Mesh;
    node->batch.params.params_mesh = push_array(draw_arena, R_Params_Mesh, 1);
    R_Params_Mesh *params = node->batch.params.params_mesh;
    params->view = view;
    params->projection = projection;
    params->tex = tex;
    params->rasterizer = rasterizer;
    node->batch.v = (u8 *)draw_arena->current + draw_arena->current->pos;
}

inline internal void draw_3d_vertex(Vector3 pos, Vector4 color, Vector2 tex) {
    R_Batch_Node *node = draw_bucket->batches.last;
    R_3D_Vertex vertex = r_3d_vertex(pos, color, tex);
    draw_batch_push_3d_vertex(&node->batch, vertex);
}

internal void draw_cube(Vector3 position, Rect src, Vector4 color, u8 face_mask) {
#if 0
    f32 S = 0.5f;
    Vector3 p0 = make_vector3(position.x - S, position.y - S, position.z + S);
    Vector3 p1 = make_vector3(position.x + S, position.y - S, position.z + S);
    Vector3 p2 = make_vector3(position.x + S, position.y + S, position.z + S);
    Vector3 p3 = make_vector3(position.x - S, position.y + S, position.z + S);
    Vector3 p4 = make_vector3(position.x - S, position.y - S, position.z - S);
    Vector3 p5 = make_vector3(position.x + S, position.y - S, position.z - S);
    Vector3 p6 = make_vector3(position.x + S, position.y + S, position.z - S);
    Vector3 p7 = make_vector3(position.x - S, position.y + S, position.z - S);
#else
    f32 S = 1.0f;
    Vector3 p0 = make_vector3(position.x,     position.y, position.z + S);
    Vector3 p1 = make_vector3(position.x + S, position.y, position.z + S);
    Vector3 p2 = make_vector3(position.x + S, position.y + S, position.z + S);
    Vector3 p3 = make_vector3(position.x, position.y + S, position.z + S);
    Vector3 p4 = make_vector3(position.x, position.y, position.z);
    Vector3 p5 = make_vector3(position.x + S, position.y, position.z);
    Vector3 p6 = make_vector3(position.x + S, position.y + S, position.z);
    Vector3 p7 = make_vector3(position.x, position.y + S, position.z);
#endif

    Vector2 tl = make_vector2(src.x0, src.y0);
    Vector2 br = make_vector2(src.x1, src.y1);
    Vector2 tr = make_vector2(src.x1, src.y0);
    Vector2 bl = make_vector2(src.x0, src.y1);

    if (!(face_mask & FACE_MASK_TOP)) {
        draw_3d_vertex(p1, color, bl);
        draw_3d_vertex(p0, color, br);
        draw_3d_vertex(p4, color, tr);
        draw_3d_vertex(p5, color, tl);
    }
    if (!(face_mask & FACE_MASK_BOTTOM)) {
        draw_3d_vertex(p3, color, bl);
        draw_3d_vertex(p2, color, br);
        draw_3d_vertex(p6, color, tr);
        draw_3d_vertex(p7, color, tl);
    }
    if (!(face_mask & FACE_MASK_NORTH)) {
        draw_3d_vertex(p0, color, bl);
        draw_3d_vertex(p1, color, br);
        draw_3d_vertex(p2, color, tr);
        draw_3d_vertex(p3, color, tl);
    }
    if (!(face_mask & FACE_MASK_SOUTH)) {
        draw_3d_vertex(p5, color, bl);
        draw_3d_vertex(p4, color, br);
        draw_3d_vertex(p7, color, tr);
        draw_3d_vertex(p6, color, tl);
    }
    if (!(face_mask & FACE_MASK_EAST)) {
        draw_3d_vertex(p1, color, bl);
        draw_3d_vertex(p5, color, br);
        draw_3d_vertex(p6, color, tr);
        draw_3d_vertex(p2, color, tl);
    }
    if (!(face_mask & FACE_MASK_WEST)) {
        draw_3d_vertex(p4, color, bl);
        draw_3d_vertex(p0, color, br);
        draw_3d_vertex(p3, color, tr);
        draw_3d_vertex(p7, color, tl);
    }
}

// internal void draw_plane_3d(Plane_3D plane, Vector4 color) {
//     Vector2 src = Vector2_Zero;
//     // draw_3d_vertex(plane.v[0], color, src);
//     // draw_3d_vertex(plane.v[1], color, src);
//     // draw_3d_vertex(plane.v[2], color, src);
//     // draw_3d_vertex(plane.v[3], color, src);
// }
