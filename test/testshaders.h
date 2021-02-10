const std::string bunnyVert =

R"STRING(

#version 410 core

// -- this shader is just here to create a shader the client can query for a bunch of different variable offsets for UBOs using std140 offset
// -- this is so we can test these offsets against the c++ client side variable alignments from the std140 utility header

precision highp float;

layout (location=0) in vec3 v_position;
//layout (location=1) in vec3 v_normal;

out vec3 position;
out vec3 normal;
out vec3 color;
flat out int instanceID;

uniform mat4 projectionMatrix;
uniform mat4 cameraMatrix;
uniform mat4 cameraProjectionMatrix;

layout (location=1) in mat4 objectMatrix;
layout (location=5) in mat3 normalMatrix;

void main()
{
    normal = normalMatrix * v_position;

    vec4 pos = objectMatrix * vec4(v_position, 1.0);

    position = pos.xyz;

    gl_Position = cameraProjectionMatrix * pos;

    color = vec3(1.0);

    instanceID = gl_InstanceID;
}

)STRING";


const std::string bunnyFrag =

R"STRING(

#version 440 core

precision highp float;

in vec3 normal;
in vec3 position; // world space position of fragment
in vec3 color;
flat in int instanceID;

out vec4 col;

struct PointLight
{
    vec3 location;
    vec3 color;
};

struct DirectionalLight
{
    vec3 direction;
    vec3 color;
};

#define MAX_DIRECTIONAL_LIGHTS 25
#define MAX_POINT_LIGHTS 25
#define MAX_SPHERES 144

layout(std140) uniform PointLightBlock
{
    int nPointLights;
    PointLight       pointLights[MAX_POINT_LIGHTS];
};

layout(std140) uniform DirectionalLightBlock
{
    int nDirectionalLights;
    DirectionalLight directionalLights[MAX_DIRECTIONAL_LIGHTS];
};

struct InstanceMaterial
{
    vec3  surfaceColor;
    float roughness;
    vec3  emissive;
    float metallic;
};

layout(std140) uniform SphereInstances
{
   InstanceMaterial instanceMaterials[MAX_SPHERES];
};

struct TestStruct
{
    float a;
    float b;
    float c;
};


struct TestStruct2
{
    float a;
    vec2 b;
};


struct TestStruct3
{
    float a;
    float b;
};

struct TestStruct4
{
    vec3 a;
    vec3 b;
    vec3 c[2];
    vec3 d[4];
    float w;
};

struct TestStruct5
{
    vec3 a;
    vec3 b;
    vec3 c[2];
    vec3 d[3];
    vec3 e;
    float w;
};


layout (std140) uniform TestInstancesUBO6
{
    float testInstances6[5];
    float testInstances6_b;
};


layout (std140) uniform TestInstancesUBO5
{
    TestStruct5 testInstances5[2];
};

layout (std140) uniform TestInstancesUBO4
{
    TestStruct4 testInstances4[2];
};

layout (std140) uniform TestInstancesUBO3
{
    TestStruct3 testInstances3[2];
};

layout (std140) uniform TestInstancesUBO3B
{
    TestStruct3 TestInstancesUBO3B_a;
    TestStruct3 TestInstancesUBO3B_b;
    float TestInstancesUBO3B_c;
};

layout (std140) uniform TestInstancesUBO2
{
    TestStruct2 testInstances2[2];
};

layout (std140) uniform TestInstancesUBO
{
    TestStruct testInstances[2];
};


struct MatrixStruct
{
    mat3 a;
    float b;
    mat3x2 c;
    mat2x3 d;
    float e;
};


layout (std140) uniform MatrixUBO
{
    MatrixStruct matStruct;
};

struct DoubleStruct
{
    dvec4 a;
    vec3 b;
};

layout (std140) uniform DoubleUBO
{
    DoubleStruct testDoubleStruct[2];
};


struct DoubleStruct2
{
    vec2 a;
    dvec3 b;
};

layout (std140) uniform DoubleUBO2
{
    DoubleStruct2 testDoubleStruct2[2];
};


void main(void)
{
// this is gibberish - just do a bunch of things that read the UBOs so that the compiler doesn't optimize them out.
    vec4 accum;
    accum.xyz += testInstances[0].a * testInstances2[0].a * testInstances4[0].a * testInstances5[0].a;
    accum.x *= testInstances3[0].a;
    accum.xyz += instanceMaterials[0].surfaceColor;
    accum.xyz += directionalLights[0].color;
    accum.xyz += pointLights[0].color;
    accum.x += testInstances6[0];
    accum.x *= TestInstancesUBO3B_c;
    accum.xyz *= matStruct.a * instanceMaterials[0].surfaceColor;
    accum.xyz += testDoubleStruct[0].b;
    accum.xy += testDoubleStruct2[0].a;
    col = accum;
}

)STRING";