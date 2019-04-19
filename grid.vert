#version 430  

in vec4 aPosition;
uniform mat4 uModelViewProjMatrix;

void main()
{
	gl_Position = uModelViewProjMatrix * aPosition;
}