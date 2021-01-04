struct VS_INPUT {
    float3 position  : SV_POSITION;
    float3 texcoord  : TEXCOORD;
    float4 color     : COLOR;
    // float3 normal    : NORMAL;
    // float3 tangent   : TANGENT;
    // float3 bitangent : BITANGENT;
};

struct PS_INPUT {
    float4 position         : SV_POSITION;
    float3 texcoord         : TEXCOORD;
    float4 color            : COLOR;
    // float3 normal           : NORMAL;
    // float3 tangent          : TANGENT;
    // float3 bitangent        : BITANGENT;
    // matrix<float, 3, 3> tbn : TBN;
    // float3 world_position   : WORLDPOS;
};

struct PS_OUTPUT {
    float4 color       : SV_Target0;
    // float4 bloom_color : SV_Target1;
    // float4 albedo      : SV_Target2;
    // float4 position    : SV_Target3;
    // float4 normal      : SV_Target4;
    // float4 material    : SV_Target5;
};

// cbuffer CBUFFER_PASS : register(b0) {
//     float2 screen_dimensions;
//     matrix view_matrix;
//     matrix projection_matrix;
//     float3 camera_position;
// };

// cbuffer CBUFFER_MODEL : register(b1) {
//     matrix model_matrix;
//     float4 model_color;
// };