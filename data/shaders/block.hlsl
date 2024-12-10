cbuffer Constants_Global : register(b0) {
    matrix view;
    matrix projection;
};

cbuffer Constants_Chunk : register(b1) {
    int4  world_position;
    float4 world_position_offset;
};

SamplerState tex_sampler : register(s0);

Texture2D tex : register(t0);
StructuredBuffer<float2> uv_buffer : register(t1);
StructuredBuffer<uint> color_table : register(t2);

// data
// ------------
// data.x
// 0-1 face orientation (TOP, BOT, NS, EW)
// 1-4 xyz (chunk relative)
// ------------
// data.y
// 4-5 tex index
// 5-6 color index

struct Vertex_Input {
    uint2 data : BLOCK_DATA;
};

struct Vertex_Output {
    float4 pos_w : SV_POSITION;
    float4 color : COL;
    float2 src : TEXCOORD;
};

uint get_face_orientation(uint data) {
    return (data >> 24) & 0xFF;
}

float3 get_vertex_position(uint data) {
    uint x = (data >> 8) & 0xFF;
    uint y = (data >> 16) & 0xFF;
    uint z = (data >> 24) & 0xFF;
    float3 position = float3(x, y, z);
    return position;
}

uint get_uv_index(uint data) {
    uint index = (data) & 0xFF;
    return index;
}

uint get_color_id(uint data) {
    return (data >> 8) & 0xFF;
}

float4 get_color(uint col_id) {
    uint C = color_table[col_id];
    float4 color;
    color.r = ((C >> 24) & 0xFF) / 255.0f;
    color.g = ((C >> 16) & 0xFF) / 255.0f;
    color.b = ((C >> 8)  & 0xFF) / 255.0f;
    color.a = (C & 0xFF) / 255.0f;
    // float4 color = float4(position.x / 255.0f, position.y / 255.0f, position.z / 255.0f, 1.0);
    return color;}


Vertex_Output vs_main(Vertex_Input input) {
    Vertex_Output output;
    float3 position = get_vertex_position(input.data.x);
    uint face = get_face_orientation(input.data.x);
    uint uv_index = input.data.y & 0xFF;

    float4x4 chunk_trans = {
        1, 0, 0, world_position.x,
        0, 1, 0, world_position.y,
        0, 0, 1, world_position.z,
        0, 0, 0, 1
    };
    float4x4 mvp = mul(projection, view);
    mvp = mul(mvp, chunk_trans);

    output.pos_w = mul(mvp, float4(position, 1));
    output.src = uv_buffer[uv_index];
    output.color = get_color(get_color_id(input.data.y));
    return output;
}

float4 ps_main(Vertex_Output input) : SV_TARGET {
    float4 final_color = tex.Sample(tex_sampler, input.src) * input.color;
    return final_color;
}
