#version 430

in vec2 gTexCoord;
in float gDensity;
in float gPressure;

uniform sampler2D uSpriteTex;
uniform float d0 = 1000;

out vec4 fFragColor;

float mapValue(in float value, in float istart, in float istop, in float ostart, in float ostop) {
	float val = ostart * (ostop - ostart) * ((value - istart)/(istop - istart));
	return val;
}

vec3 hsb2rgb( in vec3 c ){
    vec3 rgb = clamp(abs(mod(c.x*6.0+vec3(0.0,4.0,2.0), 6.0)-3.0)-1.0, 0.0, 1.0 );
    rgb = rgb*rgb*(3.0-2.0*rgb);
    return c.z * mix(vec3(1.0), rgb, c.y);
}

float maxCol = 0.9;
vec3 colors[] = { vec3(0.0,		0.0,		maxCol),
				  vec3(0.0,		maxCol,		0.0), 
				  vec3(maxCol,  maxCol,		0.0), 
				  vec3(maxCol,  maxCol/2,	0.0), 
				  vec3(maxCol,  0.0,		0.0)};

float top = 3.0;
float ranges[] = {top, top - top/5, top-2*top/5, top-3*top/5, top-4*top/5, 0.0};

float when_gt(float x, float y) {
  return max(sign(x - y), 0.0);
}

float when_le(float x, float y) {
  return 1.0 - when_gt(x, y);
}

float when_and(float a, float b) {
  return a * b;
}

void main()
{
	if ( length(gTexCoord - 0.5) > 0.38 ) discard;
    
	vec4 color = texture(uSpriteTex, gTexCoord);
	vec3 densCol = vec3(0.0);
	
	densCol += colors[0] * when_and(when_gt(gDensity, ranges[5]), when_le(gDensity, ranges[4]));
	densCol += mix(colors[0], colors[1], (gDensity-ranges[4])/(top/5)) * when_and(when_gt(gDensity, ranges[4]), when_le(gDensity, ranges[3]));
	densCol += mix(colors[1], colors[2], (gDensity-ranges[3])/(top/5)) * when_and(when_gt(gDensity, ranges[3]), when_le(gDensity, ranges[2]));
	densCol += mix(colors[2], colors[3], (gDensity-ranges[2])/(top/5)) * when_and(when_gt(gDensity, ranges[2]), when_le(gDensity, ranges[1]));
	densCol += mix(colors[3], colors[4], (gDensity-ranges[1])/(top/5)) * when_and(when_gt(gDensity, ranges[1]), when_le(gDensity, ranges[0]));
	densCol += colors[4] * when_gt(gDensity, ranges[0]);

	fFragColor = color * vec4(densCol, 1.0);
}