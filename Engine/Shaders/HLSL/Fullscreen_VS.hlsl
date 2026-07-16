struct FullscreenVSOut
{
    float4 Position : SV_POSITION;
    float2 UV       : TEXCOORD;
};

FullscreenVSOut main(uint VertexID : SV_VertexID)
{
    FullscreenVSOut output;
    output.UV = float2((VertexID << 1) & 2, VertexID & 2);
    output.Position = float4(output.UV * float2(2.0, -2.0) + float2(-1.0, 1.0), 0.0, 1.0);
    return output;
}
