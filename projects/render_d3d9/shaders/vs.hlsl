// VS_INPUT
struct VS_INPUT
{
    float4 aPosition : POSITION;
    float4 aColor : COLOR;
    float4 aTexCoord0 : TEXCOORD0;
};

// VS_OUTPUT
struct VS_OUTPUT
{
    float4 vPosition : POSITION;
    float4 vColor0 : COLOR0;
    float4 vColor1 : COLOR1;
    float4 vTexCoord0 : TEXCOORD0;
};

// uniforms
uniform float4x4 uWVP : register(c0);

// main
VS_OUTPUT main(VS_INPUT input)
{
    // output
    VS_OUTPUT output;

    // setup output
    output.vPosition = mul(uWVP, input.aPosition);
    output.vColor0 = input.aColor;
    output.vColor1 = mul(uWVP, input.aPosition);
    output.vTexCoord0 = input.aTexCoord0;

    // return
    return output;
}