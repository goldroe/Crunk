#ifndef RENDER_CORE_H
#define RENDER_CORE_H

enum R_Rasterizer_Kind {
    R_RasterizerState_Default,
    R_RasterizerState_Wireframe,
    R_RasterizerState_Text,
    R_RasterizerState_COUNT
};

enum R_Blend_State_Kind {
    R_BlendState_Draw,
    R_BlendState_Mesh,
    R_BlendState_COUNT
};

enum R_Sampler_Kind {
    R_SamplerKind_Linear,
    R_SamplerKind_Point,
    R_SamplerKind_Block,
    R_SamplerKind_COUNT
};

enum R_Depth_State_Kind {
    R_DepthState_Default,
    R_DepthState_Wireframe,
    R_DepthState_Disabled,
    R_DepthState_COUNT
};

enum R_Tex2D_Format {
    R_Tex2DFormat_R8,
    R_Tex2DFormat_R8G8B8A8,
};

struct R_2D_Vertex {
    Vector2 dst;
    Vector2 src;
    Vector4 color;
    f32 omit_tex;
    Vector3 padding_;
};

struct R_2D_Rect {
    Rect dst;
    Rect src;
    Vector4 color;
    f32 border_thickness;
    f32 omit_tex;
    f32 _unused[2];
};

struct R_3D_Vertex {
    Vector4 pos;
    Vector4 color;
    Vector2 tex;
};

enum R_Params_Kind {
    R_ParamsKind_Nil,
    R_ParamsKind_UI,
    R_ParamsKind_Quad,
    R_ParamsKind_Mesh,
    R_ParamsKind_Blocks,
    R_ParamsKind_COUNT,
};

struct R_Params_UI {
    Matrix4 xform;
    Rect clip;
    R_Handle tex;
};

struct R_Params_Quad {
    Matrix4 xform;
    R_Handle tex;
    R_Sampler_Kind sampler;
};

struct R_Params_Mesh {
    Matrix4 projection;
    Matrix4 view;
    R_Handle tex;
    R_Rasterizer_Kind rasterizer;
};

struct R_Params_Blocks {
    Matrix4 projection;
    Matrix4 view;
    Texture_Atlas *atlas;
    R_Rasterizer_Kind rasterizer;
    Chunk_List chunks;
    Frustum frustum;
    World_Position position;
};

struct R_Params {
    R_Params_Kind kind;
    union {
        R_Params_UI *params_ui;
        R_Params_Quad *params_quad;
        R_Params_Mesh *params_mesh;
        R_Params_Blocks *params_blocks;
    };
};

struct R_Batch {
    R_Params params;
    u8 *v;
    int bytes;
};

struct R_Batch_Node {
    R_Batch_Node *next;
    R_Batch batch;
};

struct R_Batch_List {
    R_Batch_Node *first;
    R_Batch_Node *last;
    int count;
};

#endif // RENDER_CORE_H
