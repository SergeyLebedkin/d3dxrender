#version 450
#extension GL_ARB_separate_shader_objects : enable

// inputs
layout(location = 0) in vec3 vNormal;
layout(location = 1) in vec2 vTexCoords;

// outputs
layout(location = 0) out vec4 fragColor;

// uniforms
layout(binding = 0) uniform sampler2D texSampler;

// main
void main()
{
	//fragColor = vec4(vNormal, 1.0f);
	fragColor = texture(texSampler, vTexCoords);
	//fragColor = vec4(vTexCoords, 0.0f, 1.0f);
}
