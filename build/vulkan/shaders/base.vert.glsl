#version 450
#extension GL_ARB_separate_shader_objects : enable

// attributes
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;

// outputs
layout(location = 0) out vec4 vColor;

// main
void main()
{
	// copy in to out
	vColor = vec4(aNormal*0.5 + 0.5, 1.0);

	// find position
	gl_Position = vec4(aPosition.xyz/32.0, 1.0);
}

/*
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec3 fragColor;

vec2 positions[3] = vec2[](
	vec2(0.0, -0.5),
	vec2(0.5, 0.5),
	vec2(-0.5, 0.5)
	);

vec3 colors[3] = vec3[](
	vec3(1.0, 0.0, 0.0),
	vec3(0.0, 1.0, 0.0),
	vec3(0.0, 0.0, 1.0)
	);

void main() {
	gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
	fragColor = colors[gl_VertexIndex];
}
*/
