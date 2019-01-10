#version 450
#extension GL_ARB_separate_shader_objects : enable

// inputs
layout(location = 0) in vec4 vColor;

// outputs
layout(location = 0) out vec4 fragColor;

// main
void main()
{
	fragColor = vColor;
}

/*
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragColor;

layout(location = 0) out vec4 outColor;

void main() {
	outColor = vec4(fragColor, 1.0);
}
*/