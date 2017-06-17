#version 330

in vec2 position;
out vec2 pos;
out vec2 uv;

uniform float time;
uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;
uniform float world_size;
uniform float lightAngle;
uniform vec2 center;


out vec4 light_dir;
out vec4 view_dir;

void main() {

    uv = (position + vec2(world_size/2, world_size/2))/world_size;
    vec3 pos_3d = vec3(position.x, 0.0, -position.y);

    pos = vec2(position.x, -position.y);

    mat4 MV = view * model;
    vec4 vpoint_mv = MV * vec4(pos_3d, 1.0);
    gl_Position = projection * vpoint_mv;

    light_dir = normalize(vec4(250*cos(lightAngle),250*sin(lightAngle), 100.0, 0.0) - vec4(vpoint_mv.xyz, 0.0));
    view_dir = normalize(vec4(0.0, 0.0, 0.0, 0.0) - vpoint_mv);
}
