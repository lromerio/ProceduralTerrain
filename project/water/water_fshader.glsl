#version 330

in vec2 uv;
in vec2 pos;
out vec4 color;

uniform sampler2D normal_tex;
uniform sampler2D normal_tex2;
uniform sampler2D tex_mirror;
uniform sampler2D tex_refraction;

uniform float time;
uniform vec2 waveDirection;
uniform float waveSpeed;

uniform float alpha;

uniform float transparency;
uniform float reflection;
uniform float refraction;
uniform vec2 center;
uniform float lightAngle;

in vec4 light_dir;
in vec4 view_dir;

float fresnel(vec4 normal) {
    float fresnelTerm = dot(view_dir, normal);
    fresnelTerm = 1 - fresnelTerm*1.3;

    fresnelTerm = fresnelTerm > 1.0 ? 1.0 : fresnelTerm;
    fresnelTerm = fresnelTerm < 0.0 ? 0.0 : fresnelTerm;

    return refraction*fresnelTerm;
}


void main() {

    vec3 kd = vec3(0.1f, 0.3f, 0.6f);
    vec3 ks = vec3(0.8f, 0.9f, 0.8f);


    vec2 bumpMapSampling = uv*10 + center*10 + waveDirection*waveSpeed*time;

    float waveBlend = 0.1*sin(time)+0.4;

    vec4 normal = vec4(normalize(mix(texture(normal_tex, bumpMapSampling).rgb, texture(normal_tex2, 2*bumpMapSampling).rgb, waveBlend)), 0.0);

    float bg = 1.0;
    float x = sin(lightAngle);

    if (x <= 0.5 && x >= -0.5) {
        bg = 1.0 - (0.5 - x)/0.5*0.7;
    } else if (x < -0.5 && x >= -0.8) {
        bg = 0.3 + (-0.5 - x)/0.3*0.7;
    }

    bg = (bg > 1.0) ? 1.0 : bg;
    bg = (bg < 0.3) ? 0.3 : bg;

    vec3 light_color = vec3(1.0, bg, bg);

    vec4 r = normalize(2*normal*(dot(normal, light_dir)) - light_dir);
    vec3 diffuse = kd*max(dot(normal, light_dir), 0)*light_color;
    vec3 specular = ks*pow(max(dot(r, view_dir), 0), alpha)*light_color;

    vec3 ambient = vec3(0.0, 0.0, 0.0);

    // Access reflection texture
    ivec2 window_size = textureSize(tex_mirror, 0);
    vec2 uv2 = vec2(1 - gl_FragCoord.x / window_size.x, gl_FragCoord.y / window_size.y);

    vec3 mirror = texture(tex_mirror, uv2 + fresnel(normal)*waveDirection).rgb;



    float dist = sqrt(pos.x*pos.x + pos.y*pos.y);
    float fogFactor = (dist - 250)/75.0;
    fogFactor = fogFactor > 1.0 ? 1.0 : fogFactor;
    fogFactor = fogFactor < 0.0 ? 0.0 : fogFactor;
    //color = colorWater;

    // Compute final color
    vec4 colorWater = vec4(mix(ambient + diffuse + specular, mirror, reflection), transparency);
    color = mix(colorWater, vec4(200.0f/255.0f, 187.0f/255.0f, 164.0f/255.0f, 1.0), fogFactor);
}
