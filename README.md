# GL_UBO
Header only library for declaring UBO structures in client C++ code with correct alignment layouts

## Unit Test
Run CMakeLists.txt to make a test application that builds a bunch of sample shaders and compares the offsets of each member with the offsets in the client C++ Code.

## Std140.h
This header defines types you can use to define UBO's in the std140 memory layout in the client code.
This is desirable so you can operate on UBO's client side, and easily update the GPU buffer memory either with persistent mapped buffers, or memcpy into a mapped buffer, or BufferSubData, etc.

Otherwise we have to query buffer member offsets for the program and update each member

To facilitate this we define in namespace std140 aligned data types

### Primitive Types
We have primitive types that just map to the corresponding OpenGL typedefs
```c++
    double64_t; 
    bool32_t;
    int32_t;
    uint32_t;
    float32_t
```    
    
    
### Vector Types
There are the vector types you'd expect from GLSL as well
```c++
     vec2,vec3,vec4    (float)
     uvec2,uvec3,uvec4 (uint)
     dvec2,dvec3,dvec4 (double)
     ivec2,ivec3,ivec4 (int)
     bvec2,bvec3,bvec4 (bool)
```

We use std::array as the base type for our glsl vectors client side

### Matrix Types
There are also the GLSL Matrix types
```c++
 mat2,mat3,mat4, dmat2,dmat3,dmat4
 mat2x3,mat2x4,mat3x2,mat3x4,mat4x2,mat4x3
 dmat2x3,dmat2x4,dmat3x2,dmat3x4,dmat4x2,dmat4x3
```

### Arrays
std140 has interesting alignment rules for arrays, so we define Array<type,length>
Array<> uses std::array as the base type
Always use Array<> when defining arrays for your UBO client side, whether they're arrays of structs or of primitive types

### Structs
 When you define structs within your ubo, also inherit from UBOStruct<>
 Even if you aren't using an array, there are alignment requirements
 std140 specifies that the base alignment of a struct is the larger of the alignment of the largest base type IN the struct and the alignment of a vec4
 
 The UBOStruct<> template argument lets you specify the largest type in the struct (since c++ doesn't have any kind of type introspection that would let us do this automatically). But you can safely ignore this and it'll default to vec4, except in the case you have dvec3, dvec4, or double matrices in your struct.
 
 In fact, when making test cases, I couldn't think of one where this actually mattered even when using double types, so if you forget it's probably fine.
 
 
 ## Examples
 Here's an example use case:
 
```c++
struct DirectionalLight : public UBOStruct<>
{
    std140::vec3 direction;
    std140::vec3 color;
};

const std::size_t MAX_DIRECTIONAL_LIGHTS = 25u;

struct DirectionalLightUBO : public UBOStruct<>
{
    std140::int32_t nDirectionalLights = 0;
    std140::Array<DirectionalLight, MAX_DIRECTIONAL_LIGHTS> directionalLights;
};

```

So in our application code we can just memcpy into a mapped buffer all of these structures without having to worry about individual member offsets
Included alongside this header is main.cpp which has a series of unit tests verifying that the offsets match between the client and GLSL program for a variety of structure layouts.
