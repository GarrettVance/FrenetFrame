



cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
    matrix model;
    matrix view;
    matrix projection;
};




struct VertexShaderInput
{
    float3  f3_position     : POSITION;
    float3  f3_color        : COLOR0;
};




struct PSInputStruct
{
    float4  f4_position     : SV_POSITION;
    float3  f3_color        : COLOR0;
};





PSInputStruct vs_main(VertexShaderInput input)
{
    //  ghv : Don't apply view transform nor perspective projection;
    //  Those are now applied in the GS Geometry Shader.

    float4 pos4_model = float4(input.f3_position, 1.0f);

    float4 pos4_world = mul(pos4_model, model);

    PSInputStruct output;
    output.f4_position = pos4_world;
    output.f3_color = input.f3_color;
    return output;
}



