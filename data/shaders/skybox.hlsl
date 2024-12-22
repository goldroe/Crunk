cbuffer Constants {
    matrix world_view_proj;
};

struct Vertex_Input {
    float3 pos_l : POSITION;
};

struct Vertex_Output {
    float4 pos_h : SV_POSITION;
    float3 pos_l : POSITION;
};

SamplerState skybox_sampler;
TextureCube skybox_tex;

Vertex_Output vs_main(Vertex_Input input) {
    Vertex_Output output;
    output.pos_l = input.pos_l;
    output.pos_h = mul(world_view_proj, float4(input.pos_l, 1.0)).xyww;
    return output;
}

float4 ps_main(Vertex_Output input) : SV_TARGET {
    float4 diffuse = skybox_tex.Sample(skybox_sampler, input.pos_l);
    float4 final_color = diffuse;
    return final_color;
}
