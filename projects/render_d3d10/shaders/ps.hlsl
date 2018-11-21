// PS_INPUT
struct PS_INPUT
{
    float4 vPosition : POSITION;
    float4 vColor0 : COLOR0;
    float4 vColor1 : COLOR1;
    float4 vTexCoord0 : TEXCOORD0;
};

// PS_OUTPUT
struct PS_OUTPUT
{
    float4 oColor0 : SV_Target0;
    float4 oColor1 : SV_Target1;
    float4 oColor2 : SV_Target2;
    float oDepth   : SV_DEPTH;
};

// uniforms
uniform float4 uColor0 : register(c0);
uniform float4 uColor1 : register(c1);

// textures
uniform sampler sBaseTexture : register(s0);

// main
PS_OUTPUT main(PS_INPUT input)
{
    // output
    PS_OUTPUT output;

    // setup output
    //output.oColor0 = tex2D(sBaseTexture, float2(input.vTexCoord0.x, 1 - input.vTexCoord0.y));
    output.oColor0 = input.vColor0;
    output.oColor1 = input.vColor0 + input.vColor1;
    output.oColor2 = input.vTexCoord0;
    output.oDepth = 0.0f;

    // return
    return output;
}