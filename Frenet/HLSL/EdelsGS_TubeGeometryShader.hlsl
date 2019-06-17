

//   TODO : set the default behavior of the culling to CullMode.None;


cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
    matrix model;
    matrix view;
    matrix projection;
};



    
static const float cross_section_edge = 0.20f;  // Use 0.17f; Range is from 0.02f to 0.34f;



static const float      geom_pi = 3.14159265f;


static const int        n_pol_divs = 5;   //  Use  6;




struct PSInputStruct
{
    float4  f4_position     : SV_POSITION;
    float3  f3_color        : COLOR0;
};







float4 gv_tnb(float3 p_curr, float p_alpha, float3 n_hat, float3 b_hat)
{
    float3 termA = p_curr;
    float3 termB = b_hat * cross_section_edge * cos(p_alpha);
    float3 termC = n_hat * cross_section_edge * sin(p_alpha);

    float3 pt3_new = termA + termB + termC;

    return float4(pt3_new, 1.f);
}












[maxvertexcount(36)]
void    gs_main(lineadj float4 vertices[4] : SV_POSITION, inout TriangleStream<PSInputStruct> triStream)
{
    PSInputStruct vstruct_a;
    PSInputStruct vstruct_b;
    PSInputStruct vstruct_c;
    PSInputStruct vstruct_d;

    float angle_alpha = 0.f;
    float delta_alpha = 2.f * geom_pi / n_pol_divs;
    float4 pos4;   
    int pol_idx = 0;

    float3 pt3_1         = (float3)vertices[1].xyz;  //  current; 
    float3 pt3_2         = (float3)vertices[2].xyz;  //  next; 
    float3 pt3_3         = (float3)vertices[3].xyz;  //  adjacency point after "next";

    float3 tangent_hat;     //   TNB (Frenet-Serret);
    float3 binormal_hat;    //   TNB (Frenet-Serret);
    float3 normal_hat;      //   TNB (Frenet-Serret);

    for (pol_idx = 0; pol_idx < 1 + n_pol_divs; pol_idx++)
    {
        angle_alpha = pol_idx * delta_alpha;

        tangent_hat = normalize(pt3_2 - pt3_1);  //  axis of rotation; 
        binormal_hat = normalize(cross(tangent_hat, pt3_2 + pt3_1));
        normal_hat = -normalize(cross(binormal_hat, tangent_hat));

        pos4 = gv_tnb(pt3_1, angle_alpha, normal_hat, binormal_hat);
        pos4 = mul(pos4, view);
        pos4 = mul(pos4, projection);
        vstruct_a.f4_position = pos4;
        vstruct_a.f3_color = float3(0.0f, 0.0f, 0.9f);  // blue;

        pos4 = gv_tnb(pt3_1, angle_alpha + delta_alpha, normal_hat, binormal_hat);
        pos4 = mul(pos4, view);
        pos4 = mul(pos4, projection);
        vstruct_b.f4_position = pos4;
        vstruct_b.f3_color = float3(0.9f, 0.9f, 0.9f);  // white;

        //      Advance axially to end of this segment:

        tangent_hat = normalize(pt3_3 - pt3_2);  //  axis of rotation; 
        binormal_hat = normalize(cross(tangent_hat, pt3_3 + pt3_2));
        normal_hat = -normalize(cross(binormal_hat, tangent_hat));

        pos4 = gv_tnb(pt3_2, angle_alpha, normal_hat, binormal_hat);
        pos4 = mul(pos4, view);
        pos4 = mul(pos4, projection);
        vstruct_c.f4_position = pos4;
        vstruct_c.f3_color = float3(0.9f, 0.0f, 0.0f);  // red;

        pos4 = gv_tnb(pt3_2, angle_alpha + delta_alpha, normal_hat, binormal_hat);
        pos4 = mul(pos4, view);
        pos4 = mul(pos4, projection);
        vstruct_d.f4_position = pos4;
        vstruct_d.f3_color = float3(0.0f, 0.5f, 0.8f);  // green-blue;

        float3 tnbColor = float3(
            tangent_hat.x * normal_hat.x,
            tangent_hat.y * normal_hat.y,
            tangent_hat.z * normal_hat.z
            );


        //  debug only:   tnbColor = float3(0.7f, 0.f, 0.3f); 


        vstruct_c.f3_color = tnbColor; // keep!!!
        vstruct_d.f3_color = tnbColor; // keep!!!


        //  Assemble points into triangle strip:

        triStream.Append(vstruct_a);
        triStream.Append(vstruct_b);
        triStream.Append(vstruct_c);
        
        triStream.Append(vstruct_b);
        triStream.Append(vstruct_d);
        triStream.Append(vstruct_c);

        triStream.RestartStrip();  // must restart;
    }
}


        
