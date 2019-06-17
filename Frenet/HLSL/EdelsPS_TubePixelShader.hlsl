



struct SOInputStruct
{
    float4  f4_position     : SV_POSITION;
    float3  f3_color        : COLOR0;
};









float4 ps_main(SOInputStruct input) : SV_TARGET
{

    return float4(input.f3_color, 1.0f);

}




