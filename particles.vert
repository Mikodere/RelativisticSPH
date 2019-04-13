#version 430  

layout (location = 0) in vec4 aPosition;
layout (location = 1) in float aDensity;
layout (location = 2) in float aPressure;

uniform mat4 uModelViewMatrix;

out float vDensity;
out float vPressure;

void main()
{
    vDensity = aDensity;
    vPressure = aPressure;

    gl_Position = uModelViewMatrix * aPosition;
}