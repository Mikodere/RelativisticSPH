#version 430

layout(points) in;

layout(triangle_strip, max_vertices = 4) out;

uniform float uSize2;
uniform mat4 uProjectionMatrix;

in float vDensity[];
in float vPressure[];

out vec2 gTexCoord;
out vec4 gColor;
out float gDensity;
out float gPressure;

void main()
{	
	vec4 v0 = gl_in[0].gl_Position + vec4(-uSize2, -uSize2, 0.0, 0.0);
	gTexCoord = vec2(0.0 , 0.0);
	gDensity = vDensity[0];
	gPressure = vPressure[0];
	gl_Position = uProjectionMatrix * v0;
	EmitVertex();

	vec4 v1 = gl_in[0].gl_Position + vec4( uSize2, -uSize2, 0.0, 0.0);
	gTexCoord = vec2(1.0 , 0.0);
	gDensity = vDensity[0];
	gPressure = vPressure[0];
	gl_Position = uProjectionMatrix * v1;
	EmitVertex();

	vec4 v2 = gl_in[0].gl_Position + vec4(-uSize2,  uSize2, 0.0, 0.0);
	gTexCoord = vec2(0.0 , 1.0);
	gDensity = vDensity[0];
	gPressure = vPressure[0];
	gl_Position = uProjectionMatrix * v2;
	EmitVertex();

	vec4 v3 = gl_in[0].gl_Position + vec4( uSize2,  uSize2, 0.0, 0.0);
	gTexCoord = vec2(1.0 , 1.0);
	gDensity = vDensity[0];
	gPressure = vPressure[0];
	gl_Position = uProjectionMatrix * v3;
	EmitVertex();

}