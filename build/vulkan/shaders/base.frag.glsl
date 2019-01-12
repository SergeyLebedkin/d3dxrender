#version 450
#extension GL_ARB_separate_shader_objects : enable

// inputs
layout(location = 0) in vec4 vColor;
layout(location = 1) in vec2 vTexCoords;

// uniforms
layout(binding = 0) uniform sampler2D texSampler;

// outputs
layout(location = 0) out vec4 fragColor;

// main
void main()
{	
	fragColor = texture(texSampler, vTexCoords);
}
