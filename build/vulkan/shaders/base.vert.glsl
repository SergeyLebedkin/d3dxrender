#version 450

// attributes
layout (location = 0) in vec4 aPosition;
layout (location = 1) in vec4 aColor;
layout (location = 2) in vec2 aTexCoord;

// outputs
layout (location = 0) out vec4 vColor;
layout (location = 1) out vec2 vTexCoord;

// main
void main()
{
	// copy in to out
	vColor = aColor;
	vTexCoord = aTexCoord;

	// find position
	gl_Position = aPosition;
}
