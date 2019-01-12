#version 450
#extension GL_ARB_separate_shader_objects : enable

// attributes
layout(location = 0) in vec4 aPosition;
layout(location = 1) in vec4 aColor;
layout(location = 2) in vec2 aTexCoords;

// outputs
layout(location = 0) out vec4 vColor;
layout(location = 1) out vec2 vTexCoords;

// uniforms
layout(binding = 1) uniform buffer0 {
	mat4 uWVP;
} matrices;

// main
void main()
{
	// copy in to out
	vColor = aColor;
	vTexCoords = aTexCoords;

	// find position
	//gl_Position = aPosition;
	gl_Position = matrices.uWVP * aPosition;
	//gl_Position = vec4(aPosition.xyz/16.0f, 1.0f);
}
