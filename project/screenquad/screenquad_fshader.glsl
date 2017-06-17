#version 330

out vec3 color;
in vec2 uv;

uniform int permutation[256];

uniform int scaleFactor;
uniform float H;              // Fractal increment (1.0 relatively smoot, near 0.0 more withe noise)
uniform float lacunarity;     // Amplitude change between layer (octaves) ?!?
uniform int octaves;            // Number of octaves
uniform float cutoff_coef;    // Rescale final value (i.e. the height)
uniform float offset;
uniform vec2 center;

float fade(float t) {
    return t * t * t * (t * (t * 6 - 15) + 10);
}

float noise(vec2 coord) {

    vec2 pi = floor(coord);//*texUnit + texUnit/2.0;
    int xi = int(pi.x);
    int yi = int(pi.y);
    vec2 pf = fract(coord);

    int i1, i2, i3, i4;
    i1 = permutation[(permutation[xi%256] + yi)%256]%8;
    i2 = permutation[(permutation[xi%256] + yi + 1)%256]%8;
    i3 = permutation[(permutation[(xi + 1)%256] + yi + 1)%256]%8;
    i4 = permutation[(permutation[(xi + 1)%256] + yi)%256]%8;

    vec2 g[8];
    g[0] = vec2(1, 0);
    g[1] = vec2(0, 1);
    g[2] = vec2(-1, 0);
    g[3] = vec2(0, -1);
    g[4] = vec2(1, 1);
    g[5] = vec2(1, -1);
    g[6] = vec2(-1, 1);
    g[7] = vec2(-1, -1);

    float s = dot(g[i1], pf);
    float t = dot(g[i2], pf - vec2(0.0, 1.0));
    float u = dot(g[i3], pf - vec2(1.0, 1.0));
    float w = dot(g[i4], pf - vec2(1.0, 0.0));

    float st, uw, noise;
    st = mix(s, w, fade(pf.x));
    uw = mix(t, u, fade(pf.x));


    return mix(st, uw, fade(pf.y));
}

float fBm(vec2 coord, float H, float lacunarity, int octaves, float offset) {

    float value = 0.0;

    float signal;
    float weight = 1.0;
    float gain = 2.0;

    /* inner loop of fractal construction */
    for (int i = 0; i < octaves; i++) {
        signal =  offset - abs(noise(coord));
        signal *= signal;
        signal *= weight;

        weight = signal * gain;
        if (weight > 1.0) {
          weight = 1.0;
        }
        if (weight < 0.0) {
          weight = 0.0;
        }


        value += signal * pow(lacunarity, -H*i);
        coord *= lacunarity;
    }

    return value;
}


void main() {

    vec2 coord = (uv + center)*scaleFactor;
    float noise = fBm(coord, H, lacunarity, octaves, offset)*cutoff_coef-0.9;

    color = vec3(noise, noise, noise);
}
