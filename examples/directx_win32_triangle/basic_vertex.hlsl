#include "types.hlsl"

PS_INPUT main(VS_INPUT input) {
    PS_INPUT v;
    v.position = float4(input.position, 1.0);
    v.texcoord = input.texcoord;
    v.color = input.color;
    return v;
}