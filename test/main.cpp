/// This file is a unit test for Std140.h
/// This creates a dummy shader with a bunch of different struct layouts for ubos
/// And makes client side C++ versions of those structs using Std140 namespace values
/// And tests if the offsets are the same
/// This is because it would be ideal to not have to query offsets and upload values piecemeal at runtime, 
/// But just define arrays of structs and do a memcpy


#define UBO_PATH
#define PBR_PATH

#define M_PI 3.14159

#include <cassert>
#include <inttypes.h>

#if 0
#define GLFW_INCLUDE_GLCOREARB
#define GLFW_INCLUDE_GLEXT
#else
#define GLFW_INCLUDE_NONE // < so we can use the default glalt path here...
#endif


#define GL_GLEXT_PROTOTYPES

#include <glhpp/legacy/glcorearb.h>
#include <glhpp/include/glhpp/OpenGL.hpp>


#include <GLFW/glfw3.h>
#include <glfwpp/include/GLFW.hpp>

#include <iostream>
#include <cmath>

#include <sstream>

#define VIRTUOSO_SHADERPROGRAMLIB_IMPLEMENTATION
#include "ShaderProgramLib.h"
#undef VIRTUOSO_SHADERPROGRAMLIB_IMPLEMENTATION

#include "../Std140.h"

bool verbose = false;

struct InstanceMaterial : public std140::UBOStruct<>
{
    std140::vec3  surfaceColor = { {1.0f,0.0f,0.0f} };
    std140::float32_t roughness = 1.f;
    std140::vec3  emissive = { {0.f, 0.f, 0.f} };
    std140::float32_t metallic = 0.f;
};

struct PointLight : public std140::UBOStruct<>
{
    std140::vec3 location;
    std140::vec3 color;
};

struct DirectionalLight : public std140::UBOStruct<>
{
    std140::vec3 direction;
    std140::vec3 color;
};

const std::size_t MAX_DIRECTIONAL_LIGHTS = 25u;

struct DirectionalLightUBO
{
    std140::int32_t nDirectionalLights = 0;
    std140::Array<DirectionalLight, MAX_DIRECTIONAL_LIGHTS> directionalLights;
};

const std::size_t MAX_POINT_LIGHTS = 25u;

struct PointLightUBO
{
    std140::int32_t nPointLights = 0;
    std140::Array<PointLight, MAX_POINT_LIGHTS> pointLights;
};

const std::size_t MAX_SPHERES = 5;

struct SphereUBO
{
    std140::Array<InstanceMaterial, MAX_SPHERES> instances;
};

// tests the structs pulled from the actual PBR demo application
void ActualAppTest(GLint program)
{
    SphereUBO sphereUBO;
    PointLightUBO plUBO;
    DirectionalLightUBO dlUBO;

    const std::size_t plBaseOff = (std::size_t) &plUBO.nPointLights;
    const std::size_t plOff2 = (std::size_t) & plUBO.pointLights[0].location - plBaseOff;
    const std::size_t plOff3 = (std::size_t) & plUBO.pointLights[1].location - plBaseOff;

    const std::size_t dlBaseOff = (std::size_t) & dlUBO.nDirectionalLights;
    const std::size_t dlOff2 = (std::size_t) & dlUBO.directionalLights[0].direction - dlBaseOff;
    const std::size_t dlOff3 = (std::size_t) & dlUBO.directionalLights[1].direction - dlBaseOff;

    const std::size_t sph1 = (std::size_t) & sphereUBO.instances[0].surfaceColor;
    const std::size_t sph2 = (std::size_t) &sphereUBO.instances[1].surfaceColor - sph1;

    const int testUniformCount = 8;

    GLuint clientOffsets[testUniformCount]
    {
        offsetof(PointLightUBO, nPointLights),
        plOff2,
        plOff3,
        offsetof(DirectionalLightUBO, nDirectionalLights),
        dlOff2,
        dlOff3,
        0,
        sph2
    };

    const GLchar* names[testUniformCount] =
    {
        "nPointLights",
        "pointLights[0].location",
        "pointLights[1].location",
        "nDirectionalLights",
        "directionalLights[0].direction",
        "directionalLights[1].direction",
        "instanceMaterials[0].surfaceColor",
        "instanceMaterials[1].surfaceColor",
    };

    GLuint rval[testUniformCount] = { 0u };
    GLint rval2[testUniformCount] = { 0u };

    glGetUniformIndices(program, testUniformCount, names, rval);
    glGetActiveUniformsiv(program, testUniformCount, rval, GL_UNIFORM_OFFSET, rval2);

    bool passed = true;
    for (int i = 0; i < testUniformCount; i++)
    {
        passed = (rval2[i] == clientOffsets[i]) && passed;
    }

    std::cout << "Test Result : " << (passed ? "PASSED" : "FAILED") << std::endl;

    if (!passed || verbose)
    {
        for (int i = 0; i < testUniformCount; i++)
        {
            std::cout << names[i] << " :: " << rval[i] << "\n\tGLSL offset : " << rval2[i] << "\n\tClient Offset : " << clientOffsets[i] << std::endl;
        }
    }
}

struct TestMatrixStruct : public std140::UBOStruct<>
{
    std140::mat3 a;
    std140::float32_t b;
    std140::mat3x2 c;
    std140::mat2x3 d;
    std140::float32_t e;

    static void uboOffsetTest(GLint program)
    {
        const GLint testUniformCount = 5;

        GLuint clientOffsets[testUniformCount]
        {
            offsetof(TestMatrixStruct, a),
            offsetof(TestMatrixStruct, b),
            offsetof(TestMatrixStruct, c),
            offsetof(TestMatrixStruct, d),
            offsetof(TestMatrixStruct, e)
        };

        const GLchar* names[testUniformCount] =
        {
            "matStruct.a",
            "matStruct.b",
            "matStruct.c", 
            "matStruct.d", 
            "matStruct.e", 
        };

        GLuint rval[testUniformCount] = { 0u };
        GLint rval2[testUniformCount] = { 0u };

        glGetUniformIndices(program, testUniformCount, names, rval);
        glGetActiveUniformsiv(program, testUniformCount, rval, GL_UNIFORM_OFFSET, rval2);

        bool passed = true;
        for (int i = 0; i < testUniformCount; i++)
        {
            passed = (rval2[i] == clientOffsets[i]) && passed;
        }

        std::cout << "Test Result : " << (passed ? "PASSED" : "FAILED") << std::endl;

        if (!passed || verbose)
        {
            for (int i = 0; i < testUniformCount; i++)
            {
                std::cout << names[i] << " :: " << rval[i] << "\n\tGLSL offset : " << rval2[i] << "\n\tClient Offset : " << clientOffsets[i] << std::endl;
            }
        }
    }
};



struct TestDoubleStruct : public std140::UBOStruct<std140::dvec4>
{
    std140::dvec4 a;
    std140::vec3 b;

    static void uboOffsetTest(GLint program)
    {
        std140::Array<TestDoubleStruct, 2> test;

        const GLint testUniformCount = 4;

        std::size_t arrayOff = ((std::size_t) & test[1].a - (std::size_t) & test[0].a);

        GLuint clientOffsets[testUniformCount]
        {
            offsetof(TestDoubleStruct, a),
            offsetof(TestDoubleStruct, b),
            offsetof(TestDoubleStruct, a) + arrayOff,
            offsetof(TestDoubleStruct, b) + arrayOff
        };

        const GLchar* names[testUniformCount] =
        {
            "testDoubleStruct[0].a",
            "testDoubleStruct[0].b",
            "testDoubleStruct[1].a",
            "testDoubleStruct[1].b",
        };

        GLuint rval[testUniformCount] = { 0u };
        GLint rval2[testUniformCount] = { 0u };

        glGetUniformIndices(program, testUniformCount, names, rval);
        glGetActiveUniformsiv(program, testUniformCount, rval, GL_UNIFORM_OFFSET, rval2);

        bool passed = true;
        for (int i = 0; i < testUniformCount; i++)
        {
            passed = (rval2[i] == clientOffsets[i]) && passed;
        }

        std::cout << "Test Result : " << (passed ? "PASSED" : "FAILED") << std::endl;

        if (!passed || verbose)
        {
            for (int i = 0; i < testUniformCount; i++)
            {
                std::cout << names[i] << " :: " << rval[i] << "\n\tGLSL offset : " << rval2[i] << "\n\tClient Offset : " << clientOffsets[i] << std::endl;
            }
        }
    }
};


// ...i can't actually seem to construct an example where the std140::dvec4 align type is actually necessary because of the other rules making it moot.
// but put it anyway.
struct TestDoubleStruct2 : public std140::UBOStruct<std140::dvec4>
{
    std140::vec2 a;
    std140::dvec3 b;
    //std140::vec2 c;

    static void uboOffsetTest(GLint program)
    {
        std140::Array<TestDoubleStruct2, 2> test;

        const GLint testUniformCount = 4;

        std::size_t arrayOff = ((std::size_t) & test[1].a - (std::size_t) & test[0].a);

        GLuint clientOffsets[testUniformCount]
        {
            offsetof(TestDoubleStruct2, a),
            offsetof(TestDoubleStruct2, b),
            //offsetof(TestDoubleStruct2, c),
            offsetof(TestDoubleStruct2, a) + arrayOff,
            offsetof(TestDoubleStruct2, b) + arrayOff,
            //offsetof(TestDoubleStruct2, c) + arrayOff
        };

        const GLchar* names[testUniformCount] =
        {
            "testDoubleStruct2[0].a",
            "testDoubleStruct2[0].b",
            //"testDoubleStruct2[0].c",
            "testDoubleStruct2[1].a",
            "testDoubleStruct2[1].b",
           // "testDoubleStruct2[1].c",
        };

        GLuint rval[testUniformCount] = { 0u };
        GLint rval2[testUniformCount] = { 0u };

        glGetUniformIndices(program, testUniformCount, names, rval);
        glGetActiveUniformsiv(program, testUniformCount, rval, GL_UNIFORM_OFFSET, rval2);

        bool passed = true;
        for (int i = 0; i < testUniformCount; i++)
        {
            passed = (rval2[i] == clientOffsets[i]) && passed;
        }

        std::cout << "Test Result : " << (passed ? "PASSED" : "FAILED") << std::endl;

        if (!passed || verbose)
        {
            for (int i = 0; i < testUniformCount; i++)
            {
                std::cout << names[i] << " :: " << rval[i] << "\n\tGLSL offset : " << rval2[i] << "\n\tClient Offset : " << clientOffsets[i] << std::endl;
            }
        }
    }
};

struct TestStruct : public std140::UBOStruct<>
{
    std140::float32_t a, b, c;

    static void uboOffsetTest(GLint program)
    {
        std140::Array<TestStruct,2> test;

        std::cout << "Alignment Value TestStruct Array : " << std140::ArrayAlignment<TestStruct>::AlignmentValue << std::endl;

        std::cout << "size of TestStruct : " << sizeof(TestStruct) << std::endl;
        std::cout << "size of TestStruct[2]: " << sizeof(test) << std::endl;

        std::size_t arrayOff = ((std::size_t) & test[1].a - (std::size_t)&test[0].a);

        std::cout << "Offset Between TestStruct[1] and TestStruct[0] " << arrayOff << std::endl;

        const GLint testUniformCount = 3 * 2;

        GLuint clientOffsets[testUniformCount]
        {
            offsetof(TestStruct, a),
            offsetof(TestStruct, b),
            offsetof(TestStruct, c),

            offsetof(TestStruct, a) + arrayOff,
            offsetof(TestStruct, b) + arrayOff,
            offsetof(TestStruct, c) + arrayOff,
        };

        const GLchar* names[testUniformCount] =
        {
            "testInstances[0].a",
            "testInstances[0].b",
            "testInstances[0].c",

            "testInstances[1].a",
            "testInstances[1].b",
            "testInstances[1].c",
        };

        GLuint rval[testUniformCount] = { 0u };
        GLint rval2[testUniformCount] = { 0u };

        glGetUniformIndices(program, testUniformCount, names, rval);
        glGetActiveUniformsiv(program, testUniformCount, rval, GL_UNIFORM_OFFSET, rval2);

        bool passed = true;
        for (int i = 0; i < testUniformCount; i++)
        {
            passed = (rval2[i] == clientOffsets[i]) && passed;
        }

        std::cout << "Test Result : " << (passed ? "PASSED" : "FAILED") << std::endl;

        if (!passed || verbose)
        {
            for (int i = 0; i < testUniformCount; i++)
            {
                std::cout << names[i] << " :: " << rval[i] << "\n\tGLSL offset : " << rval2[i] << "\n\tClient Offset : " << clientOffsets[i] << std::endl;
            }
        }
    }
};

// PASSES
struct TestStruct2 : public std140::UBOStruct<>
{
    std140::float32_t a;
    std140::vec2 b;

    static void uboOffsetTest(GLint program)
    {
        std140::Array<TestStruct2,2> test;

        std::cout << "size of TestStruct2 : " << sizeof(TestStruct2) << std::endl;
        std::cout << "size of TestStruct2[2]: " << sizeof(test) << std::endl;

        std::size_t arrayOff = ((std::size_t) & test[1] - (std::size_t)test.data());

        std::cout << "Offset Between TestStruct4[1] and TestStruct4[0] " << arrayOff << std::endl;

        const GLint testUniformCount = 2 * 2;

        GLuint clientOffsets[testUniformCount]
        {
            offsetof(TestStruct2, a),
            offsetof(TestStruct2, b),

            offsetof(TestStruct2, a) + arrayOff,
            offsetof(TestStruct2, b) + arrayOff,

        };

        const GLchar* names[testUniformCount] =
        {
            "testInstances2[0].a",
            "testInstances2[0].b",

            "testInstances2[1].a",
            "testInstances2[1].b",
        };

        GLuint rval[testUniformCount] = { 0u };
        GLint rval2[testUniformCount] = { 0u };

        glGetUniformIndices(program, testUniformCount, names, rval);
        glGetActiveUniformsiv(program, testUniformCount, rval, GL_UNIFORM_OFFSET, rval2);

        bool passed = true;
        for (int i = 0; i < testUniformCount; i++)
        {
            passed = (rval2[i] == clientOffsets[i]) && passed;
        }

        std::cout << "Test Result : " << (passed ? "PASSED" : "FAILED") << std::endl;

        if (!passed || verbose)
        {
            for (int i = 0; i < testUniformCount; i++)
            {
                std::cout << names[i] << " :: " << rval[i] << "\n\tGLSL offset : " << rval2[i] << "\n\tClient Offset : " << clientOffsets[i] << std::endl;
            }
        }
    }
};



struct TestStruct3 : public std140::UBOStruct<>
{
    std140::float32_t a, b;
 
    // do back to back structs instead of an array
    static void uboOffsetTest2(GLint program)
    {
        std::cout << std::endl;

        struct TestStruct3B
        {
            TestStruct3 a;
            TestStruct3 b;
            float c;
        };

        const GLint testUniformCount = 3;

        GLuint clientOffsets[testUniformCount]
        {
            offsetof(TestStruct3B, a),
            offsetof(TestStruct3B, b),
            offsetof(TestStruct3B, c)
        };

        const GLchar* names[testUniformCount] =
        {
            "TestInstancesUBO3B_a.a",
            "TestInstancesUBO3B_b.a",
            "TestInstancesUBO3B_c"
        };

        GLuint rval[testUniformCount] = { 0u };
        GLint rval2[testUniformCount] = { 0u };

        glGetUniformIndices(program, testUniformCount, names, rval);
        glGetActiveUniformsiv(program, testUniformCount, rval, GL_UNIFORM_OFFSET, rval2);

        bool passed = true;
        for (int i = 0; i < testUniformCount; i++)
        {
            passed = (rval2[i] == clientOffsets[i]) && passed;
        }

        std::cout << "Test Result : " << (passed ? "PASSED" : "FAILED") << std::endl;

        if (!passed || verbose)
        {
            for (int i = 0; i < testUniformCount; i++)
            {
                std::cout << names[i] << " :: " << rval[i] << "\n\tGLSL offset : " << rval2[i] << "\n\tClient Offset : " << clientOffsets[i] << std::endl;
            }
        }
    }


    static void uboOffsetTest(GLint program)
    {
        std140::Array<TestStruct3, 2> test;

        test[0].a = 2.0f;

        std::cout << "size of TestStruct3 : " << sizeof(TestStruct3) << std::endl;
        std::cout << "size of TestStruct3[2]: " << sizeof(test) << std::endl;

        std::size_t arrayOff = ((std::size_t) & test[1] - (std::size_t)&test[0]);

        std::cout << "Offset Between TestStruct3[1] and TestStruct3[0] " << arrayOff << std::endl;

        const GLint testUniformCount = 2 * 2;

        GLuint clientOffsets[testUniformCount]
        {
            offsetof(TestStruct3, a),
            offsetof(TestStruct3, b),

            offsetof(TestStruct3, a) + arrayOff,
            offsetof(TestStruct3, b) + arrayOff,

        };

        const GLchar* names[testUniformCount] =
        {
            "testInstances3[0].a",
            "testInstances3[0].b",
           
            "testInstances3[1].a",
            "testInstances3[1].b",
        };

        GLuint rval[testUniformCount] = { 0u };
        GLint rval2[testUniformCount] = { 0u };

        glGetUniformIndices(program, testUniformCount, names, rval);
        glGetActiveUniformsiv(program, testUniformCount, rval, GL_UNIFORM_OFFSET, rval2);

        bool passed = true;
        for (int i = 0; i < testUniformCount; i++)
        {
            passed = (rval2[i] == clientOffsets[i]) && passed;
        }

        std::cout << "Test Result : " << (passed ? "PASSED" : "FAILED") << std::endl;

        if (!passed || verbose)
        {
            for (int i = 0; i < testUniformCount; i++)
            {
                std::cout << names[i] << " :: " << rval[i] << "\n\tGLSL offset : " << rval2[i] << "\n\tClient Offset : " << clientOffsets[i] << std::endl;
            }
        }
    }
};



struct TestStruct4 : public std140::UBOStruct<>
{
    std140::vec3 a;
    std140::vec3 b;
    std140::Array<std140::vec3, 2> c;
    std140::Array<std140::vec3, 4> d;
    std140::float32_t w;

    static void uboOffsetTest(GLint program)
    {
        std140::Array<TestStruct4, 2> test;

        std::cout << "size of TestStruct4 : " << sizeof(TestStruct4) << std::endl;
        std::cout << "size of TestStruct4[2]: " << sizeof(test) << std::endl;

        std::size_t arrayOff = ((std::size_t) & test[1] - (std::size_t)test.data());

        std::cout << "Offset Between TestStruct4[1] and TestStruct4[0] " << arrayOff << std::endl;

        const GLint testUniformCount = 5 * 2;

        GLuint clientOffsets[testUniformCount]
        {
            offsetof(TestStruct4, a),
            offsetof(TestStruct4, b),
            offsetof(TestStruct4, c),
            offsetof(TestStruct4, d),
            offsetof(TestStruct4, w),

            offsetof(TestStruct4, a) + arrayOff,
            offsetof(TestStruct4, b) + arrayOff,
            offsetof(TestStruct4, c) + arrayOff,
            offsetof(TestStruct4, d) + arrayOff,
            offsetof(TestStruct4, w) + arrayOff,

        };

        const GLchar* names[testUniformCount] =
        {
            "testInstances4[0].a",
            "testInstances4[0].b",
            "testInstances4[0].c",
            "testInstances4[0].d",
            "testInstances4[0].w",
            "testInstances4[1].a",
            "testInstances4[1].b",
            "testInstances4[1].c",
            "testInstances4[1].d",
            "testInstances4[1].w",
        };

        GLuint rval[testUniformCount] = { 0u };
        GLint rval2[testUniformCount] = { 0u };

        glGetUniformIndices(program, testUniformCount, names, rval);
        glGetActiveUniformsiv(program, testUniformCount, rval, GL_UNIFORM_OFFSET, rval2);

        bool passed = true;
        for (int i = 0; i < testUniformCount; i++)
        {
            passed = (rval2[i] == clientOffsets[i]) && passed;
        }

        std::cout << "Test Result : " << (passed ? "PASSED" : "FAILED") << std::endl;

        if (!passed || verbose)
        {
            for (int i = 0; i < testUniformCount; i++)
            {
                std::cout << names[i] << " :: " << rval[i] << "\n\tGLSL offset : " << rval2[i] << "\n\tClient Offset : " << clientOffsets[i] << std::endl;
            }
        }
    }
};

// PASS
struct TestStruct5 : public std140::UBOStruct<>
{
    std140::vec3 a;
    std140::vec3 b;
    std140::Array<std140::vec3, 2> c;
    std140::Array<std140::vec3, 3> d;
    std140::vec3 e;
    std140::float32_t w; // the client has this aligned to a vec4 - when it should be packed after the vec3 e?

    static void uboOffsetTest(GLint program)
    {
        std140::Array<TestStruct5,2> test;

        std::cout << "size of TestStruct5 : " << sizeof(TestStruct5) << std::endl;
        std::cout << "size of TestStruct5[2]: " << sizeof(test) << std::endl;

        std::size_t arrayOff = ((std::size_t) & test[1] - (std::size_t)test.data());

        std::cout << "Offset Between TestStruct5[1] and TestStruct5[0] " << arrayOff << std::endl;

        const GLint testUniformCount = 6*2;

        GLuint clientOffsets[testUniformCount]
        {
            offsetof(TestStruct5, a),
            offsetof(TestStruct5, b),
            offsetof(TestStruct5, c),
            offsetof(TestStruct5, d),
            offsetof(TestStruct5, e),
            offsetof(TestStruct5, w),

            offsetof(TestStruct5, a) + arrayOff,
            offsetof(TestStruct5, b) + arrayOff,
            offsetof(TestStruct5, c) + arrayOff,
            offsetof(TestStruct5, d) + arrayOff,
            offsetof(TestStruct5, e) + arrayOff,
            offsetof(TestStruct5, w) + arrayOff,

        };

        const GLchar* names[testUniformCount] =
        {
            "testInstances5[0].a",
            "testInstances5[0].b",
            "testInstances5[0].c",
            "testInstances5[0].d",
            "testInstances5[0].e",
            "testInstances5[0].w",
            "testInstances5[1].a",
            "testInstances5[1].b",
            "testInstances5[1].c",
            "testInstances5[1].d",
            "testInstances5[1].e",
            "testInstances5[1].w",
        };

        GLuint rval[testUniformCount] = { 0u };
        GLint rval2[testUniformCount] = { 0u };

        glGetUniformIndices(program, testUniformCount, names, rval);
        glGetActiveUniformsiv(program, testUniformCount, rval, GL_UNIFORM_OFFSET, rval2);

        bool passed = true;
        for (int i = 0; i < testUniformCount; i++)
        {
            passed = (rval2[i] == clientOffsets[i]) && passed;
        }

        std::cout << "Test Result : " << (passed ? "PASSED" : "FAILED") << std::endl;

        if (!passed || verbose)
        {
            for (int i = 0; i < testUniformCount; i++)
            {
                std::cout << names[i] << " :: " << rval[i] << "\n\tGLSL offset : " << rval2[i] << "\n\tClient Offset : " << clientOffsets[i] << std::endl;
            }
        }
    }
};


// PASS
struct TestStruct6 : public std140::UBOStruct<>
{
    std140::Array<std140::float32_t, 5> a;
    float testInstances6_b;

    static void uboOffsetTest(GLint program)
    {
        TestStruct6 test;
        test.a[0] = 2.0f;
        float c = test.a[1];

        ///\todo what's the best way to do this? something where we can organically pass eg. float* by reference for a single param
        // float* d = test.a[0].data();

        std::cout << "size of TestStruct6 : " << sizeof(TestStruct6) << std::endl;

        const GLint testUniformCount = 2;

        GLuint clientOffsets[testUniformCount]
        {
            offsetof(TestStruct6, a),
            offsetof(TestStruct6, testInstances6_b)
        };

        const GLchar* names[testUniformCount] =
        {
            "testInstances6",
            "testInstances6_b",
        };

        GLuint rval[testUniformCount] = { 0u };
        GLint rval2[testUniformCount] = { 0u };

        glGetUniformIndices(program, testUniformCount, names, rval);
        glGetActiveUniformsiv(program, testUniformCount, rval, GL_UNIFORM_OFFSET, rval2);

        bool passed = true;
        for (int i = 0; i < testUniformCount; i++)
        {
            passed = (rval2[i] == clientOffsets[i]) && passed;
        }

        std::cout << "Test Result : " << (passed ? "PASSED" : "FAILED") << std::endl;

        if (!passed || verbose)
        {
            for (int i = 0; i < testUniformCount; i++)
            {
                std::cout << names[i] << " :: " << rval[i] << "\n\tGLSL offset : " << rval2[i] << "\n\tClient Offset : " << clientOffsets[i] << std::endl;
            }
        }
    }
};


#include "testshaders.h"
//#include <Virtuoso/GL/GLFWApplication.h>

int main(void)
{
    if (!glfwInit())
        return -1;
 
    glfw::Window::Hints hnts;
    glfw::Window wind(640, 480, hnts, "Simple");

    wind.MakeContextCurrent();
    
    /*while (wind)
    {
        wind.SwapBuffers();
        glfw::Events::Poll();
    }*/

   
    const GLubyte* vendorStr = glGetString(GL_RENDERER);

    std::cout << "OpenGL Renderer : " << vendorStr << std::endl;
    {
        gl::Program bunnyProg(
            Virtuoso::GL::Program(
                {
                    Virtuoso::GL::Shader(GL_VERTEX_SHADER, bunnyVert),
                    Virtuoso::GL::Shader(GL_FRAGMENT_SHADER, bunnyFrag)
                }
        ));

        std::cout << "sizeof(vec3) : " << sizeof(std140::vec3) << std::endl;
        std::cout << "Align of vec4 " << alignof(std140::vec4) << std::endl;
        std::cout << "Size of float[4] in glsl ubo std140 : " << sizeof(std140::Array<std140::ArrayAlignment<GLfloat>::ArrayAlignedType, 4>) << std::endl;
        std::cout << "Size of vec2[4] in glsl ubo std140 : " << sizeof(std140::Array<std140::ArrayAlignment<std140::vec2>::ArrayAlignedType, 4>) << std::endl;
        std::cout << "Alignment of array aligned float " << alignof(std140::ArrayAlignment<GLfloat>::ArrayAlignedType) << std::endl;

        int tn = 1;
        const int totalTests = 8;
        std::cout << "\n\nTEST " << tn++ << " of " << totalTests << std::endl;
        TestStruct::uboOffsetTest(bunnyProg.name());

        std::cout << "\n\nTEST " << tn++ << " of " << totalTests << std::endl;
        TestStruct2::uboOffsetTest(bunnyProg.name());

        std::cout << "\n\nTEST " << tn++ << " of " << totalTests << std::endl;
        TestStruct3::uboOffsetTest(bunnyProg.name());

        std::cout << "\n\nTEST " << tn++ << " of " << totalTests << std::endl;
        TestStruct3::uboOffsetTest2(bunnyProg.name());

        std::cout << "\n\nTEST " << tn++ << " of " << totalTests << std::endl;
        TestStruct4::uboOffsetTest(bunnyProg.name());

        std::cout << "\n\nTEST " << tn++ << " of " << totalTests << std::endl;
        TestStruct5::uboOffsetTest(bunnyProg.name());

        std::cout << std140::ArrayAlignment<GLfloat>::AlignmentValue << std::endl;



        std::cout << "\n\nTEST " << tn++ << " of " << totalTests << std::endl;
        TestStruct6::uboOffsetTest(bunnyProg.name());


        std::cout << "\n\nTEST " << tn++ << " of " << totalTests << std::endl;
        ActualAppTest(bunnyProg.name());

        std::cout << "\n\nTEST " << tn++ << " of " << totalTests << std::endl;
        TestMatrixStruct::uboOffsetTest(bunnyProg.name());


        std::cout << "\n\nTEST " << tn++ << " of " << totalTests << std::endl;
        TestDoubleStruct::uboOffsetTest(bunnyProg.name());


        std::cout << "\n\nTEST " << tn++ << " of " << totalTests << std::endl;
        TestDoubleStruct2::uboOffsetTest(bunnyProg.name());
    }
    glfwTerminate();

    return 0;
}
