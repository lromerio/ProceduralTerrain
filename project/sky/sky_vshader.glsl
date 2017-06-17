#version 330

in vec3 vpoint;
in vec2 vtexcoord;

out vec2 uv;
out float gl_ClipDistance[1];

uniform mat4 MVP;
uniform bool clip;

void main() {
    gl_Position =  MVP * vec4(vpoint.x, vpoint.y + 50.0, vpoint.z, 1);

    // TODO: pass terrain size as uniform
    if (clip) {
        gl_ClipDistance[0] = 10;
    }

    uv = vtexcoord;
}
