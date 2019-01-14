// PS_INPUT
struct PS_INPUT
{
	float4 vPosition : POSITION;
	float4 vTexCoord0 : TEXCOORD0;
};

// PS_OUTPUT
struct PS_OUTPUT
{
	float4 oColor0 : COLOR0;
	float4 oColor1 : COLOR1;
	float4 oColor2 : COLOR2;
};

// textures
uniform sampler sBaseTexture : register(s0);

// main
PS_OUTPUT main(PS_INPUT input)
{
	// output
	PS_OUTPUT output;

	// setup output
	output.oColor0 = tex2D(sBaseTexture, float2(input.vTexCoord0.x, input.vTexCoord0.y));
	//output.oColor0 = float4(input.vTexCoord0.xy, 0.0f, 1.0f);
	output.oColor1 = float4(input.vTexCoord0.xy, 0.0f, 1.0f);
	output.oColor2 = float4(input.vTexCoord0.yx, 0.0f, 1.0f);

	// return
	return output;
}

/*
// pixel shader input registers
v# Input Register               10 R Unlimited 4
c# Constant Float Register     224 R Unlimited 4
i# Constant Integer Register    16 R         1 4
b# Constant Boolean Register    16 R         1 1
s# Sampler (Direct3D 9 asm-ps)  16 R         1 4

// pixel shader output registers
oC#    Output Color Register     4 W Unlimited 4 (COLOR0, COLOR1)
oDepth Output Depth Register     1 W Unlimited 1 (DEPTH)
*/