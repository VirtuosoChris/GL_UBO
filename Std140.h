#pragma once
#include <array>

/// Intro and Usage
/// This header defines types you can use to define UBO's in the std140 memory layout in the client code
/// This is desirable so you can operate on UBO's client side, and easily update the GPU buffer memory
/// Either with persistent mapped buffers, or memcpy into a mapped buffer, or BufferSubData, etc.
/// Otherwise we have to query buffer member offsets for the program and update each member
///
/// To facilitate this we define in namespace std140 aligned data types
/// 
/// We have primitive types that just map to the corresponding OpenGL typedefs
///     double64_t; 
///     bool32_t;
///     int32_t;
///     uint32_t;
///     float32_t
///
/// There are the vector types you'd expect from GLSL as well
///     vec2,vec3,vec4    (float)
///     uvec2,uvec3,uvec4 (uint)
///     dvec2,dvec3,dvec4 (double)
///     ivec2,ivec3,ivec4 (int)
///     bvec2,bvec3,bvec4 (bool)
/// 
/// We use std::array as the base type for our glsl vectors client side
/// 
/// There are also the GLSL Matrix types
/// mat2,mat3,mat4, dmat2,dmat3,dmat4
/// mat2x3,mat2x4,mat3x2,mat3x4,mat4x2,mat4x3
/// dmat2x3,dmat2x4,dmat3x2,dmat3x4,dmat4x2,dmat4x3
///
/// std140 has interesting alignment rules for arrays, so we define Array<type,length>
/// Array<> uses std::array as the base type
/// Always use Array<> when defining arrays for your UBO client side, whether they're arrays of structs or of primitive types
///
/// When you define structs for your ubo, also inherit from UBOStruct<>
/// Even if you aren't using an array, there are alignment requirements
/// std140 specifies that the base alignment of a struct is the larger of the alignment of the largest base type IN the struct and the alignment of a vec4
/// The UBOStruct<> template argument lets you specify the largest type in the struct (since c++ doesn't have any kind of type introspection that would let us do this automatically)
/// But you can safely ignore this and it'll default to vec4, except in the case you have dvec3, dvec4, or double matrices in your struct.
/// In fact, when making test cases, I couldn't think of one where this actually mattered even when using double types, so if you forget it's probably fine.
/// Here's an example use case:

/**
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
**/

/// So in our application code we can just memcpy into a mapped buffer all of these structures without having to worry about individual member offsets
/// Included alongside this header is main.cpp which has a series of unit tests verifying that the offsets match between the client and GLSL program for a variety of structure layouts

namespace std140
{
    // https://www.khronos.org/registry/OpenGL/specs/gl/glspec45.core.pdf#page=159

    // have these macros because the (portable) alignas can't be used on typedefs.

#if defined(__GNUC__) || defined(__clang__)
	#define ALIGN(x) __attribute__ ((aligned(x)))
#elif defined(_MSC_VER)
	#define ALIGN(x) __declspec(align(x))
#else
#  error "Unknown compiler; can't define ALIGNOF"
#endif

    template <typename P, int SZ>
    constexpr std::size_t vectorAlignment()
    {
        switch (SZ)
        {
        case 2:
            return sizeof(P) * 2u;
        case 3:
        case 4:
            return sizeof(P) * 4u;
        default:
            return 0;
        }
    }


    template <typename P, int SZ>
    struct VectorAlignment
    {
        static constexpr std::size_t vectorAlignment()
        {
            switch (SZ)
            {
            case 2:
                return sizeof(P) * 2u;
            case 3:
            case 4:
                return sizeof(P) * 4u;
            default:
                return 0;
            }
        }

        const static std::size_t AlignmentValue = vectorAlignment();
    };


    template <typename TYPE, std::size_t ALIGN>
    struct ALIGN(ALIGN) ArrayAlignedStruct : public TYPE
    {
        using TYPE::operator=;
        using TYPE::TYPE;
    };


    template<typename T, std::size_t align>
    struct AlignedPrimitiveType
    {
        alignas(align) T value;

        operator T& () { return value; }

        AlignedPrimitiveType(const T& in) : value(in) {}
        T& operator=(const T& rval) { value = rval; return value; }

        AlignedPrimitiveType() : value(0) {}

        /// Get pointer to value type.  Eg. so you can pass an array element by reference to a c function.
        /// WARNING -- DO NOT USE THIS POINTER AS A C-STYLE ARRAY!
        T* data() { return &value; }
    };

    typedef ALIGN(4) GLfloat float32_t;
    typedef ALIGN(8) GLdouble double64_t;
    typedef ALIGN(4) GLboolean bool32_t;
    typedef ALIGN(4) GLint int32_t;
    typedef ALIGN(4) GLuint uint32_t;

    // We just use std::array as a base type for our client side vectors
    template <typename P, int SZ>
    struct Vector : public std::array<P, SZ>
    {
        constexpr static std::size_t length() { return SZ; }
    };

    ///\todo why do i need the extra parens in this stupid macro ?
    typedef ALIGN((VectorAlignment<GLfloat, 2>::AlignmentValue)) Vector<GLfloat, 2> vec2;
    typedef ALIGN((VectorAlignment<GLfloat, 3>::AlignmentValue)) Vector<GLfloat, 3> vec3;
    typedef ALIGN((VectorAlignment<GLfloat, 4>::AlignmentValue)) Vector<GLfloat, 4> vec4;
    
    typedef ALIGN((VectorAlignment<GLboolean, 2>::AlignmentValue)) Vector<GLboolean, 2> bvec2;
    typedef ALIGN((VectorAlignment<GLfloat, 3>::AlignmentValue)) Vector<GLboolean, 3> bvec3;
    typedef ALIGN((VectorAlignment<GLfloat, 4>::AlignmentValue)) Vector<GLboolean, 4> bvec4;

    typedef ALIGN((VectorAlignment<GLdouble, 2>::AlignmentValue)) Vector<GLdouble, 2> dvec2;
    typedef ALIGN((VectorAlignment<GLdouble, 3>::AlignmentValue)) Vector<GLdouble, 3> dvec3;
    typedef ALIGN((VectorAlignment<GLdouble, 4>::AlignmentValue)) Vector<GLdouble, 4> dvec4;

    typedef ALIGN((VectorAlignment<GLint, 2>::AlignmentValue)) Vector<GLint, 2> ivec2;
    typedef ALIGN((VectorAlignment<GLint, 3>::AlignmentValue)) Vector<GLint, 3> ivec3;
    typedef ALIGN((VectorAlignment<GLint, 4>::AlignmentValue)) Vector<GLint, 4> ivec4;

    typedef ALIGN((VectorAlignment<GLuint, 2>::AlignmentValue)) Vector<GLuint, 2> uvec2;
    typedef ALIGN((VectorAlignment<GLuint, 3>::AlignmentValue)) Vector<GLuint, 3> uvec3;
    typedef ALIGN((VectorAlignment<GLuint, 4>::AlignmentValue)) Vector<GLuint, 4> uvec4;

    template <typename T>
    struct ArrayAlignment
    {
        const static std::size_t AlignmentValue = std::max<std::size_t>(alignof(vec4), alignof(T));
        typedef ArrayAlignedStruct<T, AlignmentValue> ArrayAlignedType;
    };

    template<>
    struct ArrayAlignment<std140::float32_t>
    {
        static constexpr std::size_t AlignmentValue = std::max<std::size_t>(alignof(vec4), alignof(GLfloat));
        typedef AlignedPrimitiveType<GLfloat, AlignmentValue> ArrayAlignedType;
    };

    template<>
    struct ArrayAlignment<std140::double64_t>
    {
        static constexpr std::size_t AlignmentValue = std::max<std::size_t>(alignof(vec4), alignof(GLdouble));
        typedef AlignedPrimitiveType<GLdouble, AlignmentValue> ArrayAlignedType;
    };

    template<>
    struct ArrayAlignment<std140::bool32_t>
    {
        static constexpr std::size_t AlignmentValue = std::max<std::size_t>(alignof(vec4), alignof(GLboolean));
        typedef AlignedPrimitiveType<GLboolean, AlignmentValue> ArrayAlignedType;
    };


    template<>
    struct ArrayAlignment<std140::int32_t>
    {
        static constexpr std::size_t AlignmentValue = std::max<std::size_t>(alignof(vec4), alignof(GLint));
        typedef AlignedPrimitiveType<GLint, AlignmentValue> ArrayAlignedType;
    };

    template<>
    struct ArrayAlignment<std140::uint32_t>
    {
        static constexpr std::size_t AlignmentValue = std::max<std::size_t>(alignof(vec4), alignof(GLuint));
        typedef AlignedPrimitiveType<GLuint, AlignmentValue> ArrayAlignedType;
    };

    template <typename P, int SZ>
    struct Array : public std::array<typename ArrayAlignment<P>::ArrayAlignedType, SZ> {};

    template <typename P, int COLS, int ROWS, bool columnMajor = true>
    struct Matrix : public Array< Vector<P, columnMajor ? R : C>, columnMajor ? C : R >
    {
    };

    typedef Matrix<float, 2, 2> mat2;
    typedef Matrix<float, 3, 3> mat3;
    typedef Matrix<float, 4, 4> mat4;

    typedef Matrix<float, 2, 3> mat2x3;
    typedef Matrix<float, 2, 4> mat2x4;

    typedef Matrix<float, 3, 2> mat3x2;
    typedef Matrix<float, 3, 4> mat3x4;

    typedef Matrix<float, 4, 2> mat4x2;
    typedef Matrix<float, 4, 3> mat4x3;

    typedef Matrix<double, 2, 2> dmat2;
    typedef Matrix<double, 3, 3> dmat3;
    typedef Matrix<double, 4, 4> dmat4;

    typedef Matrix<double, 2, 3> dmat2x3;
    typedef Matrix<double, 2, 4> dmat2x4;

    typedef Matrix<double, 3, 2> dmat3x2;
    typedef Matrix<double, 3, 4> dmat3x4;

    typedef Matrix<double, 4, 2> dmat4x2;
    typedef Matrix<double, 4, 3> dmat4x3;

    template <typename T>
    constexpr std::size_t AlignOrVec4Align()
    {
        return alignof(T) > alignof(vec4) ? alignof(T) : alignof(vec4);
    }

    template <typename T = vec4>
    struct ALIGN(AlignOrVec4Align<T>())  UBOStruct
    {
    };
}

