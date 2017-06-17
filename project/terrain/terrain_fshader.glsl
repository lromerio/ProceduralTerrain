#version 330

// inputs
in vec2 uv;
in vec3 light_dir;
in vec3 view_dir;
in vec4 pos3d;
flat in int sum;

// outputs
out vec3 color;

// textures
uniform sampler2D water_tex;
uniform sampler2D sand_tex;
uniform sampler2D grass_tex;
uniform sampler2D rock_tex;
uniform sampler2D main_tex;
uniform sampler2D snow_tex;
uniform sampler2D tex;
uniform sampler2D shore_tex;
uniform sampler2D grass_high_tex;
uniform bool wireframe;

// uniforms
uniform vec2 center;
uniform float snowHeight;
uniform float lightAngle;

// compute normal using finite differences
vec3 computeNormal() {

    // read neightbor heights using an arbitrary small offset relative to grid size
    float hL = textureOffset(tex, uv, ivec2(-1, 0)).r;
    float hR = textureOffset(tex, uv, ivec2(1, 0)).r;
    float hD = textureOffset(tex, uv, ivec2(0, -1)).r;
    float hU = textureOffset(tex, uv, ivec2(0, 1)).r;

    // deduce terrain normal
    vec3 normal;
    normal.x = hL - hR;
    normal.z = hD - hU;
    normal.y = 0.03;
    return normalize(normal);
}

vec3 computeNormal2() {

    float delta = 0.005;

    // read neightbor heights using an arbitrary small offset relative to grid size
    float hL = texture(tex, uv + vec2(-delta, 0)).r;
    float hR = texture(tex, uv + vec2(delta, 0)).r;
    float hD = texture(tex, uv + vec2(0, -delta)).r;
    float hU = texture(tex, uv + vec2(0, delta)).r;

    // deduce terrain normal
    vec3 normal;
    normal.x = hL - hR;
    normal.z = hD - hU;
    normal.y = 2*delta;
    return normalize(normal);

}

float gaussianDistribution(float x, float center, float std_dev) {

    return exp((-(x-center)*(x-center))/(2*std_dev*std_dev))/(sqrt(2*3.1415*std_dev*std_dev));
}

float mainWeight(float height, float steepness) {
    return 0.4;
}

float steepWeight(float height, float steepness) {
    return gaussianDistribution(steepness, 0.6, 0.2);
}

float flatWeight(float height, float steepness) {

    return gaussianDistribution(steepness, 1.2, 0.15)*2.0;
}

float rockWeight(float height, float steepness) {

    return gaussianDistribution(height, 0.4, 0.25)*gaussianDistribution(steepness, 0.45, 0.10);// + (1.0 - steepness);
}


// primer color
float shoreWeight(float height, float steepness) {
    return gaussianDistribution(height, 0.0, 0.1)/20.0f;//*gaussianDistribution(steepness, 1.0, 0.20)*gaussianDistribution(steepness, 1.0, 0.20);
}

float dirtWeight(float height, float steepness) {
    return gaussianDistribution(height, 0.5, 0.2);//*gaussianDistribution(steepness, 1.0, 0.20)*gaussianDistribution(steepness, 1.0, 0.20);
}


// flat color
float snowWeight(float height, float steepness) {
    return gaussianDistribution(height, 1.0, 0.1)*gaussianDistribution(steepness, 1.0, 0.20)*gaussianDistribution(steepness, 1.0, 0.20)/3.0f;
}

float grassWeight(float height, float steepness) {

    return gaussianDistribution(height, 0.25, 0.175)*20.0;
}

float sandWeight(float height, float steepness) {

    if(height >= 0.32) {
        return 0.0;
    } else {
        return (height - 0.32)*(height - 0.32)*15.0f;
    }
}

float grassHighWeight(float height, float steepness) {

    return gaussianDistribution(height, 0.35, 0.225);
}

vec3 colorScheme(float height, vec3 normal) {

    vec2 coord = fract(uv + center);

    vec3 sand = texture(sand_tex, coord*50).rgb;
    vec3 grass = texture(grass_tex, coord*50).rgb;
    vec3 rock = texture(rock_tex, coord*50).rgb;
    vec3 main = texture(main_tex, coord*50).rgb;
    vec3 snow = texture(snow_tex, coord*50).rgb;
    vec3 shore = texture(shore_tex, coord*50).rgb;
    vec3 grass_high = texture(grass_high_tex, coord*50).rgb;

    vec4 flatCoeff = normalize(vec4(grassWeight(height, normal.y), snowWeight(height+snowHeight, normal.y), grassHighWeight(height, normal.y), sandWeight(height, normal.y)));
    vec3 flatColor = flatCoeff[0]*grass + flatCoeff[1]*snow + flatCoeff[2]*grass_high + flatCoeff[3]*sand;

    vec3 primerCoeff = normalize(vec3(sandWeight(height, normal.y), shoreWeight(height, normal.y), dirtWeight(height, normal.y)));
    vec3 primerColor = primerCoeff[0]*sand + primerCoeff[1]*shore + primerCoeff[2]*main;

    vec3 coeff = normalize(vec3(flatWeight(height, normal.y), steepWeight(height, normal.y), mainWeight(height, normal.y)));

    return coeff[0]*flatColor + coeff[1]*rock + coeff[2]*primerColor;

    //return flatColor;
}


void main() {

    // compute normal
    vec3 normal = computeNormal();

    // color scheme depending on height
    vec3 baseColor = colorScheme(texture(tex, uv).r, normal);

    // compute diffuse component
    vec3 r = normalize(2*normal*(dot(normal, light_dir)) - light_dir);

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

    vec3 diffuse = baseColor*max(dot(normal, light_dir), 0)*light_color;

    float dist = sqrt(pos3d.x*pos3d.x + pos3d.z*pos3d.z);
    float fogFactor = (dist - 200)/50.0;
    fogFactor = fogFactor > 1.0 ? 1.0 : fogFactor;
    fogFactor = fogFactor < 0.0 ? 0.0 : fogFactor;

    if(wireframe) {

        const vec3 COLOR[6] = vec3[](
            vec3(0.0, 0.0, 1.0),
            vec3(0.0, 1.0, 0.0),
            vec3(0.0, 1.0, 1.0),
            vec3(1.0, 0.0, 0.0),
            vec3(1.0, 0.0, 1.0),
            vec3(1.0, 1.0, 0.0));
        //color = vec3(0.0, 0.0, 0.0);

        float normSum = (sum-12.0)/68.0;

       if(sum < 12) {
            color = COLOR[0];
        } else if(sum < 16) {
            color = COLOR[1];
        } else if(sum < 20) {
            color = COLOR[2];
        } else if(sum < 24) {
            color = COLOR[3];
        } else if(sum < 28) {
            color = COLOR[4];
        } else {
            color = COLOR[5];
        }

        if(sum%4 != 0) {
            //color = vec3(0.0, 0.0, 0.0);
        }
        //color = vec3(normSum, 0.0, 0.0);

        //color = COLOR[2];

    } else {
        color = mix(diffuse, vec3(200.0f/255.0f, 187.0f/255.0f, 164.0f/255.0f), fogFactor);
    }
}
