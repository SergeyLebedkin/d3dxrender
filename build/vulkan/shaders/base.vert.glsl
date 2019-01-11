#version 450
#extension GL_ARB_separate_shader_objects : enable

// attributes
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;

// outputs
layout(location = 0) out vec3 vNormal;
layout(location = 1) out vec2 vTexCoords;

// main
void main()
{
	// copy in to out
	vNormal    = aNormal;
	vTexCoords = vec2(aTexCoords.x, 1.0f - aTexCoords.y);

	// find position
	gl_Position = vec4(aPosition.xyz/16.0f, 1.0f);
}
