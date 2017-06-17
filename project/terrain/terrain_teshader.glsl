#version 430 core
layout (quads, equal_spacing, ccw) in;

uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;
uniform float world_size;
uniform vec2 center;
uniform bool clip;
uniform float lightAngle;
uniform sampler2D tex;

out vec2 uv;
out vec4 pos3d;
out vec3 light_dir;
out vec3 view_dir;
out float gl_ClipDistance[1];
flat out int sum;

// prova homemade wireframe
in vec4 tVertexOut[];
patch in int tVertexCount[];

void main(void){

    // interpolate position
    vec4 p1 = mix(tVertexOut[1],tVertexOut[0],gl_TessCoord.x);
    vec4 p2 = mix(tVertexOut[2],tVertexOut[3],gl_TessCoord.x);
    vec4 bilinear = mix(p1, p2, gl_TessCoord.y);

    // map to texture coordinates
    uv = ( vec2(bilinear.x, -bilinear.z) + center + vec2(world_size/2, world_size/2))/world_size;

    // displace vertex based on texture
    //float height = texture(tex, vec2(0.5, 0.5)).r;
    float height = texture(tex, uv).r;

    // compute outer sum
    sum = tVertexCount[0] + tVertexCount[1] + tVertexCount[2] + tVertexCount[3];


    // Clip
    if (clip) {
        gl_ClipDistance[0] = height;
    }

    // compute position relative to model, view and projection
    pos3d = vec4(bilinear.x, 20*height, bilinear.z, 1.0);
    mat4 MV = view * model;
    vec4 vpoint_mv = MV * pos3d;
    gl_Position = projection * vpoint_mv;

    // compute light direction and view direction for shading purposes
    light_dir = normalize(vec4(250*cos(lightAngle),250*sin(lightAngle), 50.0, 0.0) - vec4(vpoint_mv.xyz, 0.0)).xyz;
    view_dir = normalize(vec4(0.0, 0.0, 0.0, 0.0) - vpoint_mv).xyz;

}
