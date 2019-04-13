#version 430

out vec4 fFragColor;

void main()
{
    vec4 col1 = vec4(1.0, 1.0, 1.0, 1.0);
    vec4 col2 = vec4(0.0, 0.0, 0.9, 1.0);

	fFragColor = col2;
}