#version 330

in vec2 uv;

out vec3 color;

uniform sampler2D cubemap;
uniform float lightAngle;

void main() {
    vec3 colorTex = texture(cubemap, uv).rgb;

    float bg = 1.0;
    float x = sin(lightAngle);

    if (x <= 0.5 && x >= -0.5) {
        bg = 1.0 - (0.5 - x)/0.5*0.5;
    } else if (x < -0.5 && x >= -0.8) {
        bg = 0.5 + (-0.5 - x)/0.3*0.5;
    }

    bg = (bg > 1.0) ? 1.0 : bg;
    bg = (bg < 0.5) ? 0.5 : bg;

    vec3 light_color = vec3(1.0, bg, bg);

    color = mix(vec3(0.0, 0.0, 0.0), colorTex*light_color, 0.45*sin(lightAngle)+0.55);
    //color = colorTex;

}
