// VS_INPUT
struct VS_INPUT
{
	float4 aPosition : POSITION0;
	float4 aTexCoord0 : TEXCOORD0;
};

// VS_OUTPUT
struct VS_OUTPUT
{
	float4 vPosition : POSITION;
	float4 vTexCoord0 : TEXCOORD0;
};

// main
VS_OUTPUT main(VS_INPUT input)
{
	// output
	VS_OUTPUT output;

	// setup output
	output.vPosition = input.aPosition;
	output.vTexCoord0 = input.aTexCoord0;

	// return
	return output;
}

/*
// vertex shader input registers
v# Input Register               16 R Unlimited 4
c# Constant Float Register     256 R Unlimited 4
i# Constant Integer Register    16 R         1 4
b# Constant Boolean Register    16 R         1 1
s# Sampler (Direct3D 9 asm-vs)   4 R         1 4

// vertex shader output registers
o# Output Register              12 W Unlimited 4
*/