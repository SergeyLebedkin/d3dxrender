#version 450

// inputs
layout (location = 0) in vec4 vColor;
layout (location = 1) in vec2 vTexCoord;

// outputs
layout (location = 0) out vec4 fragColor;

// main
void main()
{
	fragColor = vColor;
}
