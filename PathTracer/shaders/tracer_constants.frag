#version 150 core
#define EPSILON 0.00001

#define EMPTY 0
#define SPHERE 1
#define CUBOID 2
#define PLANE 3

uniform vec2 resolution;
uniform int MAX_BOUNCES;
uniform int SAMPLES;
uniform int OBJECT_COUNT;
uniform sampler2D randomTexture;
uniform int frame;
uniform vec3 cameraPosition;
uniform vec3 cameraRotation;
uniform sampler2D lastFrame;
uniform sampler2D textureMap;

in vec2 TexCoord;
out vec4 FragColor;

struct Material
{
    vec3 color;
    vec3 emission;
    float roughness;
    float indexOfRefraction;
};

struct Object
{
    int type;
    vec3 position;
    vec4 scale;
    Material material;
    int id;
};

uniform Object objects[30];

Object SKY;

float noise(float x)
{
    return fract(cos(x * 2831.0) * 3023.0);
}

vec3 random3dVector(vec3 s)
{
    vec3 v = fract(s.xyx * vec3(103.34, 278.27, 192.42));
    v += dot(v, v + 29.92);
    return fract(vec3(v.x * v.y, v.y * v.z, v.x * v.z));
}

void swap(inout float a, inout float b)
{
    float temp = a;
    a = b;
    b = temp;
}

float min3(vec3 v)
{
    return min(min(v.x, v.y), v.z);
}

float max3(vec3 v)
{
    return max(max(v.x, v.y), v.z);
}
