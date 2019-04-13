#version 430  

in vec4 aPosition;
in vec4 aColor;

uniform mat4 uModelViewProjMatrix;

out vec4 vColor;

void main()
{
	vColor = aColor;
	gl_Position = uModelViewProjMatrix * aPosition;
}