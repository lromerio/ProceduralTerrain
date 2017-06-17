#version 430 core

//specify the number of vertices per patch
layout (vertices = 4) out;

in vec4 vVertexOut[];

out vec4 tVertexOut[];
patch out int tVertexCount[];

uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;

float smootherstep(float edge0, float edge1, float x)
{
    // Scale, bias and saturate x to 0..1 range
    x = clamp((x - edge0)/(edge1 - edge0), 0.0, 1.0);
    // Evaluate polynomial
    return x*x*x*(x*(x*6 - 15) + 10);
}

float expInterpolate(float x, float k) {
    float x2 = clamp(x, 0.0, 1.0);

    return (exp(k*x2) - 1)/(exp(k) - 1);
}

float interpolate2(float x, float a) {
    float x2 = clamp(x, 0.0, 1.0);

    return x2/(1 + a*(1-x2));
}

float clamp(float x, float lowerlimit, float upperlimit)
{
    if (x < lowerlimit) x = lowerlimit;
    if (x > upperlimit) x = upperlimit;
    return x;
}

int outerTessellationFunction(vec4 vertex1, vec4 vertex2, vec4 camera) {

    //Bring vertexes to world coordinates
    vec4 v1 = projection*view*model*vertex1;
    vec4 v2 = projection*view*model*vertex2;

    float norm1 = length(v1 - camera);
    float norm2 = length(v2 - camera);

    float edgeDistance = (norm1 + norm2)/2.0;

    if(edgeDistance < 10) {
        return 8;
    } else if (edgeDistance < 25){
        return 7;
    } else if (edgeDistance < 100){
        return 6;
    } else if (edgeDistance < 150){
        return 5;
    } else if (edgeDistance < 250){
        return 4;
    } else {
        return 3;
    }
/*
    if(edgeDistance < 150) {
        return 9;
    } else {
        return 1;
    }
*/
    //int level = int(mix(3, 20, 1 - expInterpolate(edgeDistance/350.0, -10)));
    //return level;

}

void main(void){
    if (gl_InvocationID == 0){

        int outer0 = outerTessellationFunction(vVertexOut[0], vVertexOut[3], vec4(0, 0, 0, 1));
        int outer1 = outerTessellationFunction(vVertexOut[2], vVertexOut[3], vec4(0, 0, 0, 1));
        int outer2 = outerTessellationFunction(vVertexOut[1], vVertexOut[2], vec4(0, 0, 0, 1));
        int outer3 = outerTessellationFunction(vVertexOut[0], vVertexOut[1], vec4(0, 0, 0, 1));

        int inner = (outer0 + outer1 + outer2 + outer3)/4;

        gl_TessLevelInner[0] = inner;
        gl_TessLevelInner[1] = inner;

        gl_TessLevelOuter[0] = outer0;
        gl_TessLevelOuter[1] = outer1;
        gl_TessLevelOuter[2] = outer2;
        gl_TessLevelOuter[3] = outer3;

        tVertexCount[0] = outer0;
        tVertexCount[1] = outer1;
        tVertexCount[2] = outer2;
        tVertexCount[3] = outer3;

    }

    tVertexOut[gl_InvocationID] = vVertexOut[gl_InvocationID];
}

