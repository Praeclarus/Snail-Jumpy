#ifndef SNAIL_JUMPY_MATH_H
#define SNAIL_JUMPY_MATH_H

// TODO(Tyler): Remove this dependency!
#include <math.h>

//~ Helper functions

internal inline s32
RoundF32ToS32(f32 A)
{
    s32 Result;
    if(A < 0)
    {
        Result = (s32)(A - 0.5f);
    }
    else
    {
        Result = (s32)(A + 0.5f);
    }
    return(Result);
}

internal inline s32
TruncateF32ToS32(f32 A)
{
    return((s32)A);
}

internal inline u32
TruncateF32ToU32(f32 A)
{
    return((u32)A);
}

internal inline f32
FloorF32(f32 A){
    return(floorf(A));
}

internal inline u32
CeilF32ToS32(f32 A)
{
    u32 Result = (u32)ceilf(A);
    return(Result);
}

#define Minimum(A, B) ((A) > (B) ? (B) : (A))
#define Maximum(A, B) ((A) > (B) ? (A) : (B))

internal inline f32
Square(f32 A)
{
    f32 Result = A*A;
    return(Result);
}

internal inline f32
SquareRoot(f32 A)
{
    f32 Result = sqrtf(A);
    return(Result);
}

internal inline f32
Sine(f32 A)
{
    return(sinf(A));
}

internal inline f32
Cosine(f32 A)
{
    return(cosf(A));
}

internal inline f32
Tangent(f32 A)
{
    return(tanf(A));
}

internal inline f32
ModF32(f32 A, f32 B)
{
    f32 Result = (f32)fmod(A, B);
    return(Result);
}

internal inline f32
AbsoluteValue(f32 A)
{
    f32 Result = (A < 0) ? -A : A;
    return(Result);
}

internal inline f32
ToPowerOf(f32 Base, f32 Exponent){
    f32 Result = powf(Base, Exponent);
    return(Result);
}

//~ Vectors

typedef union v2 v2;
union v2
{
    struct
    {
        f32 X, Y;
    };
    struct
    {
        f32 Width, Height;
    };
};

// TODO(Tyler): Possibly implement operations for this?
typedef union v2s v2s;
union v2s
{
    struct
    {
        s32 X, Y;
    };
    struct
    {
        s32 Width, Height;
    };
};

internal inline v2
operator+(v2 A, v2 B)
{
    v2 Result;
    Result.X = A.X + B.X;
    Result.Y = A.Y + B.Y;
    return(Result);
}

internal inline v2
operator-(v2 A, v2 B)
{
    v2 Result;
    Result.X = A.X - B.X;
    Result.Y = A.Y - B.Y;
    return(Result);
}

internal inline v2
V2Invert(v2 A)
{
    v2 Result;
    Result.X = -A.X;
    Result.Y = -A.Y;
    return(Result);
}

internal inline v2
operator*(v2 A, float B)
{
    v2 Result;
    Result.X = A.X * B;
    Result.Y = A.Y * B;
    return(Result);
}

internal inline v2
operator*(float B, v2 A)
{
    v2 Result;
    Result.X = A.X * B;
    Result.Y = A.Y * B;
    return(Result);
}

internal inline v2
operator/(v2 A, float B)
{
    v2 Result;
    Result.X = A.X / B;
    Result.Y = A.Y / B;
    return(Result);
}

internal inline v2
operator+=(v2 &A, v2 B)
{
    A = A + B;
    return(A);
}

internal inline v2
operator-=(v2 &A, v2 B)
{
    A = A - B;
    return(A);
}

internal inline v2
operator*=(v2 &A, float B)
{
    A = B * A;
    return(A);
}

internal inline v2
operator/=(v2 &A, float B)
{
    A = A / B;
    return(A);
}

internal inline f32
Inner(v2 A, v2 B)
{
    float Result = (A.X*B.X)+(A.Y*B.Y);
    return(Result);
}

internal inline f32
LengthSquared(v2 V)
{
    f32 Result = Inner(V, V);
    return(Result);
}

internal inline v2
Normalize(v2 V){
    v2 Result = V/SquareRoot(LengthSquared(V));
    return(Result);
}

internal inline f32
SafeRatioN(f32 Numerator, f32 Denominator, f32 N){
    f32 Result = N;
    
    if(Denominator != 0.0f){
        Result = Numerator / Denominator;
    }
    
    return(Result);
}

internal inline f32
SafeRatio0(f32 Numerator, f32 Denominator){
    f32 Result = SafeRatioN(Numerator, Denominator, 0.0f);
    return(Result);
}

internal inline f32
SafeRatio1(f32 Numerator, f32 Denominator){
    f32 Result = SafeRatioN(Numerator, Denominator, 1.0f);
    return(Result);
}

internal inline u64
SafeRatioN(u64 Numerator, u64 Denominator, u64 N){
    u64 Result = N;
    
    if(Denominator != 0.0f){
        Result = Numerator / Denominator;
    }
    
    return(Result);
}

internal inline u64
SafeRatio0(u64 Numerator, u64 Denominator){
    u64 Result = SafeRatioN(Numerator, Denominator, 0);
    return(Result);
}

union v4 {
    struct {
        f32 X, Y, Z, W;
    };
    struct {
        f32 R, G, B, A;
    };
};

typedef v4 color;


#endif // SNAIL_JUMPY_MATH_H
