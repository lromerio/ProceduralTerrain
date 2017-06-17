#pragma once

// project
#define WINDOW_WIDTH 1600
#define WINDOW_HEIGHT 1200

// camera and navigation
#define START_CAM_YAW 90.0f
#define START_CAM_PITCH 0.0f
#define START_CAM_FOV 45.0f
#define MAX_CAM_PITCH 80.0f
#define MIN_CAM_PITCH -80.0f
#define MAX_CAM_SPEED 0.005f
#define MIN_CAM_SPEED -0.005f
#define CAM_NEAR 0.1f
#define CAM_FAR 100.0f
#define CAM_SPEED 0.001f
#define CAM_STRAIGHT_ACC_FACTOR 0.0001f
#define CAM_ANGLE_ACC_FACTOR 0.1f
#define CAM_DECELERATION_FACTOR 0.9f
#define VERTICAL_SPEED_MULT 250
#define MOUSE_SENSITIVITY 0.05f
static const glm::vec3 INITIAL_EYE = glm::vec3(0.0f, 20.0f, 0.0f);
static const glm::vec3 INITIAL_FRONT = glm::vec3(0.0f, 0.0f, 1.0f);
static const glm::vec3 INITIAL_UP = glm::vec3(0.0f, 1.0f, 0.0f);
static const glm::vec2 INITIAL_CENTER = glm::vec2(0.0, 0.0);

// world parameters
#define WORLD_SIZE 500.0f
#define RESOLUTION 500.0f
#define TERRAIN_HEIGHT_MULTIPLIER 20.0

// terrain parameters
#define INITIAL_SCALE 2
#define INITIAL_H 1.2f
#define INITIAL_LACUNARITY 2.45f
#define INITIAL_OCTAVES 6
#define INITIAL_CUT_COEFF 1.5f
#define INITIAL_OFFSET 1.0f

// water parameters
#define INITIAL_TRANSPARENCY 0.7f
#define INITIAL_REFLECTION 0.7f
#define INITIAL_REFRACTION 0.01f
#define INITIAL_ALPHA 25.0f
#define INITIAL_WAVE_SPEED 0.02f
static const glm::vec2 INITIAL_WAVE_DIR = glm::vec2(-1.0, -1.0);

static const int perlinPermutation[256] = { 151,160,137,91,90,15,
                                            131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
                                            190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
                                            88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
                                            77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
                                            102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
                                            135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
                                            5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
                                            223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
                                            129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
                                            251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
                                            49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
                                            138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
                                          };
