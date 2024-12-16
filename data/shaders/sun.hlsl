
cbuffer Constants : register(b0) {
    matrix xform;
}

struct Vertex_Input {
    float4 pos_l : POSITION;
    float4 color : COLOR;
    float2 src : TEXCOORD;
};

struct Vertex_Output {
    float4 pos_h : SV_POSITION;
    float4 color : COLOR;
    float2 tex : TEXCOORD;
};

SamplerState tex_sampler : register(s0);
Texture2D main_tex : register(t0);

Vertex_Output vs_main(Vertex_Input input) {
    Vertex_Output output;
    output.pos_h = mul(xform, input.pos_l);
    output.color = input.color;
    output.tex = input.src;
    return output;
}

float4 ps_main(Vertex_Output input) : SV_TARGET {
    float4 tex_color = main_tex.Sample(tex_sampler, input.tex);
    float4 final_color = tex_color;
    final_color.rgb *= 1.5f;
    return final_color;
}
