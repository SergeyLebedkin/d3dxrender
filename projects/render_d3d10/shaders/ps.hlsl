// PS_INPUT
struct PS_INPUT
{
    float4 vPosition : SV_POSITION;
    float4 vColor0 : COLOR0;
    float4 vTexCoord0 : TEXCOORD0;
};

// PS_OUTPUT
struct PS_OUTPUT
{
    float4 oColor0 : SV_Target0;
    float4 oColor1 : SV_Target1;
    float4 oColor2 : SV_Target2;
};

// main
PS_OUTPUT main(PS_INPUT input)
{
    // output
    PS_OUTPUT output;

    // setup output
    //output.oColor0 = input.vColor0;
    output.oColor0 = input.vColor0;
    output.oColor1 = input.vColor0;
    output.oColor2 = input.vTexCoord0;

    // return
    return output;
}