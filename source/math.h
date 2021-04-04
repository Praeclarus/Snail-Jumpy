#ifndef SNAIL_JUMPY_MATH_H
#define SNAIL_JUMPY_MATH_H

// TODO(Tyler): Remove this dependency!
#include <math.h>

//~ Mathy stuff

global_constant f32 PI = 3.141592653589f;
global_constant f32 TAU = 2.0f*PI;

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
Truncate(f32 A)
{
    return((s32)A);
}

internal inline f32
Floor(f32 A){
    return(floorf(A));
}

internal inline f32
Clamp(f32 Value, f32 Min, f32 Max){
    f32 Result = Value;
    if(Result < Min){
        Result = Min;
    }else if(Result > Max){
        Result = Max;
    }
    return(Result);
}

internal inline f32
Ceil(f32 A){
    return(ceilf(A));
}

internal inline f32
Round(f32 A)
{
    f32 Result;
    if(A < 0)
    {
        Result = Floor((A - 0.5f));
    }
    else
    {
        Result = Floor((A + 0.5f));
    }
    return(Result);
}

internal inline u32
CeilToS32(f32 A)
{
    u32 Result = (u32)ceilf(A);
    return(Result);
}

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
Sin(f32 A)
{
    return(sinf(A));
}

internal inline f32
Cos(f32 A)
{
    return(cosf(A));
}

internal inline f32
Tan(f32 A)
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
SignOf(f32 A){
    f32 Result = (A < 0) ? -1.0f : 1.0f;
    return(Result);
}

internal inline f32
ToPowerOf(f32 Base, f32 Exponent){
    f32 Result = powf(Base, Exponent);
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

//~ V2s

#define V20 V2(0, 0)
union v2
{
    struct
    {
        f32 X;
        f32 Y;
    };
    struct
    {
        f32 Width;
        f32 Height;
    };
};

internal inline v2
V2(f32 X, f32 Y){ 
    v2 Result = v2{X, Y}; 
    return(Result);
}

internal inline v2
V2(f32 XY){ 
    v2 Result = V2(XY, XY); 
    return(Result);
}

// TODO(Tyler): Possibly implement operations for this?
typedef union v2s v2s;
union v2s
{
    struct
    {
        s32 X;
        s32 Y;
    };
    struct
    {
        s32 Width;
        s32 Height;
    };
};

internal inline v2s
V2S(s32 X, s32 Y){ 
    v2s Result = v2s{X, Y}; 
    return(Result);
}

internal inline v2s
V2S(v2 A){ 
    v2s Result; 
    Result.X = (s32)A.X;
    Result.Y = (s32)A.Y;
    return(Result);
}

internal inline v2
V2(v2s A){ 
    v2 Result = v2{(f32)A.X, (f32)A.Y}; 
    return(Result);
}

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
operator-(v2 A)
{
    v2 Result;
    Result.X = -A.X;
    Result.Y = -A.Y;
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
Dot(v2 A, v2 B) {
    float Result = (A.X*B.X)+(A.Y*B.Y);
    return(Result);
}

internal inline v2
Clockwise90(v2 A, v2 Origin=V2(0,0)){
    A -= Origin;
    v2 Result = V2(A.Y, -A.X);
    Result += Origin;
    return(Result);
}

internal inline v2
CounterClockwise90(v2 A, v2 Origin=V2(0,0)){
    A -= Origin;
    v2 Result = V2(-A.Y, A.X);
    Result += Origin;
    return(Result);
}

internal inline v2
Invert(v2 A, v2 Origin=V2(0,0)){
    A -= Origin;
    v2 Result = -A;
    Result += Origin;
    return(Result);
}

internal inline f32
LengthSquared(v2 V){
    f32 Result = Dot(V, V);
    return(Result);
}

internal inline f32
Length(v2 V){
    f32 Result = SquareRoot(LengthSquared(V));
    return(Result);
}

internal inline v2
Normalize(v2 V){
    f32 Length = SquareRoot(LengthSquared(V));
    v2 Result = {0};
    if(Length > 0.0f){
        Result = V/Length;
    }
    return(Result);
}

// Perpendicular to A in the direction of B
internal inline v2 
TripleProduct(v2 A, v2 B){
    // A cross B cross A = (A cross B) cross A
    f32 Z = (A.X*B.Y)-(A.Y*B.X);
    v2 Result = V2(-Z*A.Y, Z*A.X);
    return(Result);
}

//~ Colors
union v4 {
    struct {
        f32 X, Y, Z, W;
    };
    struct {
        f32 R, G, B, A;
    };
};

typedef v4 color;
internal inline color
Color(f32 R, f32 G, f32 B, f32 A){
    color Result = color{R, G, B, A};
    return(Result);
}

//~ Rectangles
union rect {
    struct {
        v2 Min;
        v2 Max;
    };
    struct {
        v2 E[2];
    };
};

struct rect_s32 {
    v2s Min;
    v2s Max;
};

internal inline rect
Rect(v2 Min, v2 Max){
    rect Result;
    Result.Min = Min;
    Result.Max = Max;
    return(Result);
}

internal inline rect_s32
RectS32(v2s Min, v2s Max){
    rect_s32 Result;
    Result.Min = Min;
    Result.Max = Max;
    return(Result);
}

internal inline rect_s32 
RectS32(rect Rect){
    rect_s32 Result;
    Result.Min.X = Truncate(Rect.Min.X);
    Result.Min.Y = Truncate(Rect.Min.Y);
    Result.Max.X = (s32)Ceil(Rect.Max.X);
    Result.Max.Y = (s32)Ceil(Rect.Max.Y);
    return(Result);
}

internal inline rect
CenterRect(v2 P, v2 Size){
    rect Result;
    Result.Min = P - 0.5f*Size;
    Result.Max = P + 0.5f*Size;
    return(Result);
}

internal inline rect
OffsetRect(rect Rect, v2 Offset){
    rect Result = Rect;
    Result.Min += Offset;
    Result.Max += Offset;
    return(Result);
}

internal inline v2
RectSize(rect Rect){
    v2 Result = Rect.Max - Rect.Min;
    return(Result);
}

internal inline b8
IsPointInRect(v2 Point, rect Rect){
    b8 Result = ((Rect.Min.X < Point.X) && (Point.X < Rect.Max.X) &&
                 (Rect.Min.Y < Point.Y) && (Point.Y < Rect.Max.Y));
    return(Result);
}

internal inline b8
DoRectsOverlap(rect A, rect B){
    b8 Result = ((A.Min.X <= B.Max.X) &&
                 (B.Min.X <= A.Max.X) &&
                 (A.Min.Y <= B.Max.Y) &&
                 (B.Min.Y <= A.Max.Y));
    return(Result);
}

internal inline rect
GrowRect(rect Rect, f32 G){
    rect Result = Rect;
    Result.Min -= V2(G, G);
    Result.Max += V2(G, G);
    return(Result);
}

internal inline v2
GetRectCenter(rect Rect){
    v2 Result = {};
    v2 Size = RectSize(Rect);
    Result = Rect.Min + 0.5f*Size;
    
    return(Result);
}

internal void
LogMessage(char *Format, ...);

internal inline rect
FixRect(rect Rect){
    rect Result = {};
    Result.Min.X = Minimum(Rect.Min.X, Rect.Max.X);
    Result.Min.Y = Minimum(Rect.Min.Y, Rect.Max.Y);
    Result.Max.X = Maximum(Rect.Min.X, Rect.Max.X);
    Result.Max.Y = Maximum(Rect.Min.Y, Rect.Max.Y);
    
    return(Result);
}

#endif // SNAIL_JUMPY_MATH_H
