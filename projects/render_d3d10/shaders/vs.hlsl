// VS_INPUT
struct VS_INPUT
{
    float4 aPosition : POSITION;
    float4 aColor : COLOR0;
    float4 aTexCoord0 : TEXCOORD0;
};

// VS_OUTPUT
struct VS_OUTPUT
{
    float4 vPosition : SV_POSITION;
    float4 vColor0 : COLOR0;
    float4 vTexCoord0 : TEXCOORD0;
};

// cbMatrices
cbuffer cbMatrices: register(b0)
{
    float4x4 uWVP;
};

// main
VS_OUTPUT main(VS_INPUT input)
{
    // output
    VS_OUTPUT output;
    
    // setup output
    output.vPosition = mul(uWVP, input.aPosition);
    output.vColor0 = input.aColor;
    output.vTexCoord0 = input.aTexCoord0;

    // return
    return output;
}