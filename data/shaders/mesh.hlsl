cbuffer Constants : register(b0) {
    matrix xform;
};

struct Vertex_In {
    float4 pos_l : POSITION;
    float4 color : COLOR;
    float2 src   : TEXCOORD;
};

struct Vertex_Out {
    float4 pos_h : SV_POSITION;
    float4 color : COLOR;
    float2 tex   : TEXCOORD;
};

Texture2D tex : register(t0);
SamplerState tex_sampler : register(s0);

Vertex_Out vs_main(Vertex_In input) {
    Vertex_Out output;
    output.pos_h = mul(xform, float4(input.pos_l.xyz, 1.0));
    output.color = input.color;
    output.tex = input.src;
    return output;
}

float4 ps_main(Vertex_Out input) : SV_TARGET {
    float4 tex_color = tex.Sample(tex_sampler, input.tex);
    float4 final_color;
    final_color = input.color * tex_color;
    return final_color;
}
