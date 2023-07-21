#ifndef TYLER_BASICS_H
#define TYLER_BASICS_H

#ifndef TYLER_BASICS_NO_UNITY_BUILD
#define TYLER_BASICS_IMPLEMENTATION
#define tyler_function internal
#define tyler_global   global
#else // TYLER_BASICS_UNITY_BUILD
#error This file is not arranged yet to work without a unity build!
#endif // TYLER_BASICS_UNITY_BUILD

#include <stdint.h>
#include <float.h>
// Remove this dependency
#include <math.h>

#ifndef TYLER_SNPRINTF
#define TYLER_SNPRINTF snprintf
#endif

#ifndef TYLER_VSNPRINTF
#define TYLER_VSNPRINTF vsnprintf
#endif

#if defined(_MSC_VER) && !defined(__clang__)
#define BREAK  __debugbreak();
#endif

//~ Primitive types
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef s8  b8;
typedef s16 b16;
typedef s32 b32;
typedef s64 b64;

typedef size_t umw;

typedef float  f32;
typedef double f64;

#define internal        static
#define global          static
#define global_constant static const
#define local_persist   static
#define local_constant  static const

#define Swap(A, B) do { decltype(A) Temp = (A); (A) = (B); (B) = Temp; } while (0)
#define Minimum(A, B) ((A) > (B) ? (B) : (A))
#define Maximum(A, B) ((A) > (B) ? (A) : (B))
#define Clamp(Value, Min, Max) Maximum(Minimum(Value, Max), Min);

#define ArrayCount(Arr) (sizeof(Arr)/sizeof(*Arr))

#if defined(SNAIL_JUMPY_DEBUG_BUILD)
#define Assert(Expr) {if (!(Expr)) __debugbreak();}
#else
#define Assert(Expr) {(Expr);}
#endif

#define INVALID_CODE_PATH   BREAK
#define NOT_IMPLEMENTED_YET BREAK
#define DONT_COMPILE _ThIs_sHaLl_nOt_CoMpIle

#define U8_MAX  0xff
#define U16_MAX 0xffff
#define U32_MAX 0xffffffff
#define U64_MAX 0xffffffffffffffff

#define S8_MAX  0x7f
#define S16_MAX 0x7fff
#define S32_MAX 0x7fffffff
#define S64_MAX 0x7fffffffffffffff

#define S8_MIN  (-S8_MAX-1)
#define S16_MIN (-S16_MAX-1)
#define S32_MIN (-S32_MAX-1)
#define S64_MIN (-S64_MAX-1)

#define F32_NAN               NAN
#define F32_POSITIVE_INFINITY INFINITY 
#define F32_NEGATIVE_INFINITY -INFINITY

#define F64_NAN               0x7FF7FFFFFFFFFFFF 
#define F64_POSITIVE_INFINITY 0x7FF0000000000000 
#define F64_NEGATIVE_INFINITY 0xFFF0000000000000 

#define flag8(Enum)  u8
#define flag16(Enum) u16
#define flag32(Enum) u32
#define flag64(Enum) u64

#define GLUE_(a,b) a##b
#define GLUE(a,b) GLUE_(a,b)

#define STRINGIFY(X) #X

// TODO(Tyler): I'm not sure how I feel about these
#define FOR_RANGE_(Type, Iterator, Start, End) for(Type Iterator=(Start); Iterator<(End); Iterator++)
#define FOR_RANGE(Iterator, Start, End) FOR_RANGE_(u32, Iterator, Start, End)

tyler_function void *CantAlloc_(){
    INVALID_CODE_PATH;
    return 0;
}

#define CANT_ALLOC(Type) (Type *)CantAlloc_()

//~ Unit functions
#define Kilobytes(Size) (1024*(Size))
#define Megabytes(Size) (1024*Kilobytes(Size))
#define Gigabytes(Size) (1024L*(u64)Megabytes(Size))

#define KB(Size) Kilobytes(Size)
#define MB(Size) Megabytes(Size)
#define GB(Size) Gigabytes(Size)

#define USecToMSec(USeconds)  ((f32)(USeconds)/1000.0f)
#define USecToMSecF(USeconds) ((f32)(USeconds)/1000.0f)

//~ Miscellaneous constants
global_constant u32 DEFAULT_BUFFER_SIZE = 512;

//~ Linked list
#define DLIST_ADD(Sentinel, Node) do{ \
(Node)->Prev = (Sentinel); \
(Node)->Next = (Sentinel)->Next; \
(Node)->Prev->Next = (Node); \
(Node)->Next->Prev = (Node); \
}while(0)

#define DLIST_ADD_LAST(Sentinel, Node) do{ \
(Node)->Prev = (Sentinel)->Prev; \
(Node)->Next = (Sentinel); \
(Node)->Prev->Next = (Node); \
(Node)->Next->Prev = (Node); \
}while(0)

#define DLIST_REMOVE(Node) do{ \
(Node)->Next->Prev = (Node)->Prev; \
(Node)->Prev->Next = (Node)->Next; \
(Node)->Next = (Node)->Prev = 0; \
}while(0); 

#define DLIST_INIT(Sentinel) do{ \
(Sentinel)->Next = (Sentinel); \
(Sentinel)->Prev = (Sentinel); \
}while(0);

template <typename T> internal inline T *
FreelistAlloc_(T **List){
    T *Result = *List;
    *List = (*List)->NextFree;
    return Result;
}

#define FREELIST_ALLOC(List, AllocCode) ((List) ? FreelistAlloc_(&(List)) : (AllocCode))

#define FREELIST_FREE(List, Node) do{ \
(Node)->NextFree = (List); \
(List) = (Node); \
}while(0);

//~ Required declarations for platforms
// NOTE(Tyler): The declarations are here so that one knows what tyler_functions need to be implemented for
// different platforms.
// TODO(Tyler): Have this have a default option

#ifdef TYLER_BASICS_USE_MALLOC
#include <stdlib.h>
tyler_function void *
OSDefaultAlloc(umw Size){ return calloc(Size, 1); } // calloc does zero initialization
tyler_function void *
OSDefaultRealloc(void *Memory, umw Size) { 
    void *Result = realloc(Memory, Size);
    ZeroMemory(Result, Size);
    return Result;
}
tyler_function void *
OSDefaultFree(void *Pointer){ return free(Pointerm); }
#else // TYLER_BASICS_USE_MALLOC
tyler_function void *OSDefaultAlloc(umw Size);
tyler_function void *OSDefaultRealloc(void *Memory, umw Size);
tyler_function void  OSDefaultFree(void *Pointer);
#endif // TYLER_BASICS_USE_MALLOC

#ifdef TYLER_BASICS_IMPLEMENTATION

//~ Basic Math

global_constant f32 PI = 3.141592653589f;
global_constant f32 TAU = 2.0f*PI;

tyler_function inline s32
RoundToS32(f32 A){
    s32 Result;
    if(A < 0) Result = (s32)(A - 0.5f);
    else Result = (s32)(A + 0.5f);
    return(Result);
}

tyler_function inline s32
Truncate(f32 A){
    return((s32)A);
}

tyler_function inline f32
Floor(f32 A){
    return(floorf(A));
}

internal inline f32
FloorTo(f32 A, f32 CellSize){
    A /= CellSize;
    f32 Result = Floor(A);
    Result *= CellSize;
    
    return(Result);
}

tyler_function inline f32
Ceil(f32 A){
    return(ceilf(A));
}

internal inline f32
CeilTo(f32 A, f32 CellSize){
    A /= CellSize;
    f32 Result = Ceil(A);
    Result *= CellSize;
    
    return(Result);
}

tyler_function inline f32
Round(f32 A)
{
    f32 Result;
    if(A < 0) Result = Floor((A - 0.5f));
    else Result = Floor((A + 0.5f));
    return(Result);
}

internal inline f32
RoundTo(f32 A, f32 CellSize){
    A /= CellSize;
    f32 Result = Round(A);
    Result *= CellSize;
    
    return(Result);
}

tyler_function inline u32
CeilToS32(f32 A)
{
    u32 Result = (u32)ceilf(A);
    return(Result);
}

tyler_function inline f32
Square(f32 A)
{
    f32 Result = A*A;
    return(Result);
}

tyler_function inline f32
SquareRoot(f32 A)
{
    f32 Result = sqrtf(A);
    return(Result);
}

tyler_function inline f32
Sin(f32 A)
{
    return(sinf(A));
}

tyler_function inline f32
Cos(f32 A)
{
    return(cosf(A));
}

tyler_function inline f32
Tan(f32 A)
{
    return(tanf(A));
}

tyler_function inline f32
ModF32(f32 A, f32 B)
{
    f32 Result = (f32)fmod(A, B);
    return(Result);
}

tyler_function inline f32
AbsoluteValue(f32 A)
{
    f32 Result = (A < 0) ? -A : A;
    return(Result);
}

tyler_function inline s32
AbsoluteValue(s32 A)
{
    s32 Result = (A < 0) ? -A : A;
    return(Result);
}

tyler_function inline f32
SignOf(f32 A){
    f32 Result = (A < 0) ? -1.0f : 1.0f;
    return(Result);
}

tyler_function inline f32
ToPowerOf(f32 Base, f32 Exponent){
    f32 Result = powf(Base, Exponent);
    return(Result);
}

tyler_function inline f32
SafeRatioN(f32 Numerator, f32 Denominator, f32 N){
    f32 Result = N;
    
    if(Denominator != 0.0f){
        Result = Numerator / Denominator;
    }
    
    return(Result);
}

tyler_function inline f32
SafeRatio0(f32 Numerator, f32 Denominator){
    f32 Result = SafeRatioN(Numerator, Denominator, 0.0f);
    return(Result);
}

tyler_function inline f32
SafeRatio1(f32 Numerator, f32 Denominator){
    f32 Result = SafeRatioN(Numerator, Denominator, 1.0f);
    return(Result);
}

tyler_function inline u64
SafeRatioN(u64 Numerator, u64 Denominator, u64 N){
    u64 Result = N;
    
    if(Denominator != 0.0f){
        Result = Numerator / Denominator;
    }
    
    return(Result);
}

tyler_function inline u64
SafeRatio0(u64 Numerator, u64 Denominator){
    u64 Result = SafeRatioN(Numerator, Denominator, 0);
    return(Result);
}

tyler_function inline f32
Lerp(f32 A, f32 B, f32 T){
    T = Clamp(T, 0.0f, 1.0f);
    f32 Result = A + T*(B-A);
    return(Result);
}

tyler_function inline b8
IsEven(s32 A){
    b8 Result = (A % 2) == 0;
    return(Result);
}

tyler_function inline b8
IsOdd(s32 A){
    b8 Result = (A % 2) == 1;
    return(Result);
}

tyler_function inline s32
NormalizeDegrees(s32 D){
    s32 Result;
    if(D < 0){
        Result = D;
        while(Result < 0) Result += 360; 
    }else{
        Result = D % 360;
    }
    return(Result);
}

//~ V2

union v2 {
    struct {
        f32 X;
        f32 Y;
    };
    struct {
        f32 Width;
        f32 Height;
    };
    f32 E[2];
};

tyler_function inline v2
V2(f32 X, f32 Y){ 
    v2 Result = v2{X, Y}; 
    return(Result);
}

tyler_function inline v2
V2(f32 XY){ 
    v2 Result = V2(XY, XY); 
    return(Result);
}

// TODO(Tyler): Possibly implement operations for this?
union v2s {
    struct {
        s32 X;
        s32 Y;
    };
    struct {
        s32 Width;
        s32 Height;
    };
};

tyler_function inline v2s
V2S(s32 X, s32 Y){ 
    v2s Result = v2s{X, Y}; 
    return(Result);
}

tyler_function inline v2s
V2S(s32 XY){ 
    v2s Result = v2s{XY, XY}; 
    return(Result);
}

tyler_function inline v2s
V2S(v2 A){ 
    v2s Result; 
    Result.X = (s32)A.X;
    Result.Y = (s32)A.Y;
    return(Result);
}

tyler_function inline v2
V2(v2s A){ 
    v2 Result = v2{(f32)A.X, (f32)A.Y}; 
    return(Result);
}

tyler_function inline b8
operator==(v2 A, v2 B)
{
    b8 Result = (A.X==B.X) && (A.Y==B.Y);
    return(Result);
}

tyler_function inline b8
operator!=(v2 A, v2 B)
{
    b8 Result = (A.X!=B.X) || (A.Y!=B.Y);
    return(Result);
}

tyler_function inline v2
operator+(v2 A, v2 B)
{
    v2 Result;
    Result.X = A.X + B.X;
    Result.Y = A.Y + B.Y;
    return(Result);
}

tyler_function inline v2
operator-(v2 A, v2 B)
{
    v2 Result;
    Result.X = A.X - B.X;
    Result.Y = A.Y - B.Y;
    return(Result);
}

tyler_function inline v2
operator-(v2 A)
{
    v2 Result;
    Result.X = -A.X;
    Result.Y = -A.Y;
    return(Result);
}

tyler_function inline v2
V2Invert(v2 A)
{
    v2 Result;
    Result.X = -A.X;
    Result.Y = -A.Y;
    return(Result);
}

tyler_function inline v2
operator*(v2 A, f32 B)
{
    v2 Result;
    Result.X = A.X * B;
    Result.Y = A.Y * B;
    return(Result);
}

tyler_function inline v2
operator*(f32 B, v2 A)
{
    v2 Result;
    Result.X = A.X * B;
    Result.Y = A.Y * B;
    return(Result);
}

tyler_function inline v2
operator/(v2 A, f32 B)
{
    v2 Result;
    Result.X = A.X / B;
    Result.Y = A.Y / B;
    return(Result);
}

tyler_function inline v2
operator+=(v2 &A, v2 B)
{
    A = A + B;
    return(A);
}

tyler_function inline v2
operator-=(v2 &A, v2 B)
{
    A = A - B;
    return(A);
}

tyler_function inline v2
operator*=(v2 &A, f32 B)
{
    A = B * A;
    return(A);
}

tyler_function inline v2
operator/=(v2 &A, f32 B)
{
    A = A / B;
    return(A);
}

tyler_function inline f32
V2Dot(v2 A, v2 B) {
    f32 Result = (A.X*B.X)+(A.Y*B.Y);
    return(Result);
}

tyler_function inline v2
V2Hadamard(v2 A, v2 B){
    v2 Result;
    Result.X = A.X*B.X;
    Result.Y = A.Y*B.Y;
    return(Result);
}

tyler_function inline v2
V2Clockwise90(v2 A, v2 Origin=V2(0,0)){
    A -= Origin;
    v2 Result = V2(A.Y, -A.X);
    Result += Origin;
    return(Result);
}

tyler_function inline v2
V2CounterClockwise90(v2 A, v2 Origin=V2(0,0)){
    A -= Origin;
    v2 Result = V2(-A.Y, A.X);
    Result += Origin;
    return(Result);
}

tyler_function inline f32
V2LengthSquared(v2 V){
    f32 Result = V2Dot(V, V);
    return(Result);
}

tyler_function inline f32
V2Length(v2 V){
    f32 Result = SquareRoot(V2LengthSquared(V));
    return(Result);
}

tyler_function inline v2
V2Normalize(v2 V){
    f32 Length = SquareRoot(V2LengthSquared(V));
    v2 Result = {};
    if(Length > 0.0f){
        Result = V/Length;
    }
    return(Result);
}

// Perpendicular to A in the direction of B
tyler_function inline v2 
V2TripleProduct(v2 A, v2 B){
    // A cross B cross A = (A cross B) cross A
    f32 Z = (A.X*B.Y)-(A.Y*B.X);
    v2 Result = V2(-Z*A.Y, Z*A.X);
    return(Result);
}

tyler_function inline v2
V2Minimum(v2 A, v2 B){
    v2 Result;
    Result.X = Minimum(A.X, B.X);
    Result.Y = Minimum(A.Y, B.Y);
    
    return(Result);
}

tyler_function inline v2
V2Maximum(v2 A, v2 B){
    v2 Result;
    Result.X = Maximum(A.X, B.X);
    Result.Y = Maximum(A.Y, B.Y);
    
    return(Result);
}

tyler_function inline v2s
V2SMinimum(v2s A, v2s B){
    v2s Result;
    Result.X = Minimum(A.X, B.X);
    Result.Y = Minimum(A.Y, B.Y);
    
    return(Result);
}

tyler_function inline v2s
V2SMaximum(v2s A, v2s B){
    v2s Result;
    Result.X = Maximum(A.X, B.X);
    Result.Y = Maximum(A.Y, B.Y);
    
    return(Result);
}

tyler_function inline v2
V2Square(v2 V){
    v2 Result;
    Result.X = V.X*V.X;
    Result.Y = V.Y*V.Y;
    return(Result);
}

tyler_function inline v2
V2Floor(v2 V){
    v2 Result;
    Result.X = Floor(V.X);
    Result.Y = Floor(V.Y);
    return(Result);
}

tyler_function inline v2
V2Round(v2 V){
    v2 Result;
    Result.X = Round(V.X);
    Result.Y = Round(V.Y);
    return(Result);
}

tyler_function inline v2
V2Ceil(v2 V){
    v2 Result;
    Result.X = Ceil(V.X);
    Result.Y = Ceil(V.Y);
    return(Result);
}

tyler_function inline v2
V2Lerp(v2 A, v2 B, f32 T){
    T = Clamp(T, 0.0f, 1.0f);
    v2 Result = T*A + (1.0f-T)*B;
    return(Result);
}

tyler_function inline v2
V2AbsoluteValue(v2 A){
    A.X = AbsoluteValue(A.X);
    A.Y = AbsoluteValue(A.Y);
    return A;
}

internal inline v2
V2RoundTo(v2 P, v2 CellSize){
    P.X /= CellSize.X;
    P.Y /= CellSize.Y;
    v2 Result = V2(Round(P.X), Round(P.Y));
    Result.X *= CellSize.X;
    Result.Y *= CellSize.Y;
    
    return(Result);
}

internal inline v2
V2CeilTo(v2 P, v2 CellSize){
    P.X /= CellSize.X;
    P.Y /= CellSize.Y;
    v2 Result = V2(Ceil(P.X), Ceil(P.Y));
    Result.X *= CellSize.X;
    Result.Y *= CellSize.Y;
    
    return(Result);
}

internal inline v2
V2FloorTo(v2 P, v2 CellSize){
    P.X /= CellSize.X;
    P.Y /= CellSize.Y;
    v2 Result = V2(Floor(P.X), Floor(P.Y));
    Result.X *= CellSize.X;
    Result.Y *= CellSize.Y;
    
    return(Result);
}

internal inline v2 
V2NewCoords(v2 Point, v2 NewX, v2 NewY){
    v2 Result;
    Result.X = V2Dot(NewX, Point);
    Result.Y = V2Dot(NewY, Point);
    return Result;
}

//~ V2S
tyler_function inline v2s
operator+(v2s A, v2s B)
{
    v2s Result;
    Result.X = A.X + B.X;
    Result.Y = A.Y + B.Y;
    return(Result);
}

tyler_function inline v2s
operator-(v2s A, v2s B)
{
    v2s Result;
    Result.X = A.X - B.X;
    Result.Y = A.Y - B.Y;
    return(Result);
}

tyler_function inline v2s
operator-(v2s A)
{
    v2s Result;
    Result.X = -A.X;
    Result.Y = -A.Y;
    return(Result);
}

tyler_function inline v2s
V2Invert(v2s A)
{
    v2s Result;
    Result.X = -A.X;
    Result.Y = -A.Y;
    return(Result);
}

tyler_function inline v2s
operator*(v2s A, s32 B)
{
    v2s Result;
    Result.X = A.X * B;
    Result.Y = A.Y * B;
    return(Result);
}

tyler_function inline v2s
operator*(s32 B, v2s A)
{
    v2s Result;
    Result.X = A.X * B;
    Result.Y = A.Y * B;
    return(Result);
}

tyler_function inline v2s
operator/(v2s A, s32 B)
{
    v2s Result;
    Result.X = A.X / B;
    Result.Y = A.Y / B;
    return(Result);
}

tyler_function inline v2s
operator+=(v2s &A, v2s B)
{
    A = A + B;
    return(A);
}

tyler_function inline v2s
operator-=(v2s &A, v2s B)
{
    A = A - B;
    return(A);
}

tyler_function inline v2s
operator*=(v2s &A, s32 B)
{
    A = B * A;
    return(A);
}

tyler_function inline v2s
operator/=(v2s &A, s32 B)
{
    A = A / B;
    return(A);
}

//~
union v3s {
    struct {
        s32 X;
        s32 Y;
        s32 Z;
    };
    struct {
        v2s XY;
        s32 Z;
    };
    struct {
        s32 Width;
        s32 Height;
        s32 Depth;
    };
};

tyler_function inline v3s 
V3S(s32 X, s32 Y, s32 Z){
    v3s Result = {X, Y, Z};
    return Result;
}

tyler_function inline v3s 
V3S(s32 XYZ){
    v3s Result = {XYZ, XYZ, XYZ};
    return Result;
}

tyler_function inline v3s 
V3S(v2s XY, s32 Z){
    v3s Result = {XY.X, XY.Y, Z};
    return Result;
}

//~
union v4s {
    struct {
        s32 X;
        s32 Y;
        s32 Z;
        s32 W;
    };
    struct {
        v2s XY;
        v2s ZW;
    };
    struct {
        v3s XYZ;
        s32 W;
    };
};

tyler_function inline v4s 
V4S(s32 X, s32 Y, s32 Z, s32 W){
    v4s Result = {X, Y, Z, W};
    return Result;
}

tyler_function inline v4s 
V4S(s32 XYZW){
    v4s Result = {XYZW, XYZW, XYZW, XYZW};
    return Result;
}

tyler_function inline v4s 
V4S(v2s XY, v2s ZW){
    v4s Result = {XY.X, XY.Y, ZW.X, ZW.Y};
    return Result;
}

//~ Colors
union color {
    struct { 
        f32 R, G, B, A;
    };
    f32 E[4];
};

tyler_function inline color
MakeColor(f32 R, f32 G, f32 B, f32 A=1.0f){
    color Result = color{R, G, B, A};
    return(Result);
}

tyler_function inline color
ColorMix(color A, color B, f32 Value){
    Value = Clamp(Value, 0.0f, 1.0f);
    color Result;
    Result.R = Value*A.R + (1.0f-Value)*B.R;
    Result.G = Value*A.G + (1.0f-Value)*B.G;
    Result.B = Value*A.B + (1.0f-Value)*B.B;
    Result.A = Value*A.A + (1.0f-Value)*B.A;
    return(Result);
}

tyler_function inline color
ColorAlphiphy(color Color, f32 Alpha){
    Alpha = Clamp(Alpha, 0.0f, 1.0f);
    color Result = Color;
    Result.A *= Alpha;
    return(Result);
}

tyler_function inline color
operator*(f32 X, color Color)
{
    color Result;
    Result.R = X*Color.R;
    Result.G = X*Color.G;
    Result.B = X*Color.B;
    Result.A = X*Color.A;
    return Result;
}

tyler_function inline color
operator*(color Color, f32 X)
{
    color Result = X*Color;
    return Result;
}

tyler_function inline color
operator*=(color &Color, f32 X)
{
    Color = X*Color;
    return Color;
}

tyler_function inline color
operator/(color Color, f32 X)
{
    color Result;
    Result.R = Color.R/X;
    Result.G = Color.G/X;
    Result.B = Color.B/X;
    Result.A = Color.A/X;
    return Result;
}

tyler_function inline color
operator/=(color &Color, f32 X)
{
    Color = Color/X;
    return Color;
}

typedef u32 color_u32;
tyler_function inline color_u32
MakeColorU32(color C){
    C*=255.0f;
    
    color_u32 Result = ((((u32)C.R) << 24) |
                        (((u32)C.G) << 16) |
                        (((u32)C.B) << 8) |
                        (((u32)C.A) << 0));
    return Result;
}

tyler_function inline color
MakeColor(color_u32 C){
    color Result = {};
    Result.R = (f32)((C >> 24) & 0xff);
    Result.G = (f32)((C >> 16) & 0xff);
    Result.B = (f32)((C >> 8)  & 0xff);
    Result.A = (f32)((C >> 0)  & 0xff);
    Result /= 255.0f;
    return Result;
}

//~ Basic colors

global_constant color BLACK      = MakeColor(0.0f,  0.0f,  0.0f, 1.0f);
global_constant color WHITE      = MakeColor(1.0f,  1.0f,  1.0f, 1.0f);
global_constant color RED        = MakeColor(1.0f,  0.0f,  0.0f, 1.0f);
global_constant color YELLOW     = MakeColor(1.0f,  1.0f,  0.0f, 1.0f);
global_constant color BLUE       = MakeColor(0.0f,  0.0f,  1.0f, 1.0f);
global_constant color GREEN      = MakeColor(0.0f,  1.0f,  0.0f, 1.0f);
global_constant color DARK_GREEN = MakeColor(0.0f,  0.5f,  0.0f, 1.0f);
global_constant color BROWN      = MakeColor(0.41f, 0.20f, 0.0f, 1.0f);
global_constant color PINK       = MakeColor(1.0f,  0.0f,  1.0f, 1.0f);
global_constant color PURPLE     = MakeColor(0.42f, 0.05f, 0.68f,1.0f);
global_constant color ORANGE     = MakeColor(1.0f,  0.5f,  0.0f, 1.0f);

//~ HSB color
struct hsb_color {
    f32 Hue, Saturation, Brightness;
};

tyler_function hsb_color 
HSBColor(f32 Hue, f32 Saturation, f32 Brightness){
    hsb_color Result = {Hue, Saturation, Brightness};
    return(Result);
}

tyler_function color
HSBToRGB(hsb_color HSBColor){
    f32 Hue = Clamp(HSBColor.Hue, 0.0f, 360.0f);
    Hue /= 60.0f;
    f32 Saturation = Clamp(HSBColor.Saturation, 0.0f, 1.0f);
    f32 Brightness = Clamp(HSBColor.Brightness, 0.0f, 1.0f);
    
    
    f32 Chroma = Brightness*Saturation;
    f32 X      = Chroma * (1.0f - AbsoluteValue(ModF32(Hue, 2.0f) - 1.0f)); 
    
    color Result = {};
    u32 HueU32 = (u32)Hue;
    switch(HueU32){
        case 0: { Result = MakeColor(Chroma,      X,   0.0f); }break;
        case 1: { Result = MakeColor(     X, Chroma,   0.0f); }break;
        case 2: { Result = MakeColor(  0.0f, Chroma,      X); }break;
        case 3: { Result = MakeColor(  0.0f,      X, Chroma); }break;
        case 4: { Result = MakeColor(     X,   0.0f, Chroma); }break;
        case 5: 
        case 6: { Result = MakeColor(Chroma,   0.0f,      X); }break;
        default: { INVALID_CODE_PATH; }break;
    }
    
    f32 M = Brightness-Chroma;
    Result.R += M;
    Result.G += M;
    Result.B += M;
    
    return(Result);
}

//~ Range S32
union range_s32 {
    struct {
        s32 Min;
        s32 Max;
    };
    struct {
        s32 Start;
        s32 End;
    };
};

tyler_function inline range_s32
MakeRangeS32(s32 Start, s32 End){
    range_s32 Result;
    Result.Start = Minimum(Start, End);
    Result.End   = Maximum(Start, End);
    return Result;
}

tyler_function inline range_s32
SizeRangeS32(s32 Start, s32 Size){
    range_s32 Result = MakeRangeS32(Start, Start+Size);
    return Result;
}

tyler_function inline s32
RangeSize(range_s32 Range){
    return Range.End-Range.Start;
}

tyler_function inline range_s32
RangeRectify(range_s32 Range){
    range_s32 Result;
    Result.Start = Minimum(Range.Start, Range.End);
    Result.End   = Maximum(Range.Start, Range.End);
    return Result;
}

tyler_function inline range_s32
RangeShift(range_s32 Range, s32 Shift){
    range_s32 Result = Range;
    Result.Start += Shift;
    Result.End   += Shift;
    return Result;
}

tyler_function inline b8
RangeContains(range_s32 Range, s32 Value){
    b8 Result = (Range.Min < Value) && (Value < Range.Max);
    return Result;
}

tyler_function inline b8
RangeContainsInclusive(range_s32 Range, s32 Value){
    b8 Result = (Range.Min <= Value) && (Value <= Range.Max);
    return Result;
}

tyler_function inline b8
RangeContains(range_s32 A, range_s32 B){
    b8 Result = (A.Min < B.Min) && (B.Max < A.Max);
    return Result;
}

tyler_function inline b8
RangeContainsInclusive(range_s32 A, range_s32 B){
    b8 Result = (A.Min <= B.Min) && (B.Max <= A.Max);
    return Result;
}

tyler_function inline range_s32
RangeCrop(range_s32 Super, range_s32 Sub){
    range_s32 Result;
    Result.Min = Maximum(Super.Min, Sub.Min);
    Result.Max = Minimum(Super.Max, Sub.Max);
    return Result;
}

tyler_function inline s32
RangeClamp(range_s32 R, s32 S){
    S = Clamp(S, R.Min, R.Max);
    return S;
}

tyler_function inline b8
RangeOverlaps(range_s32 A, range_s32 B){
    b8 Result = (RangeContains(A, B.Min) || RangeContains(A, B.Max));
    return Result;
}

tyler_function inline b8
RangeOverlapsInclusive(range_s32 A, range_s32 B){
    b8 Result = (RangeContainsInclusive(A, B.Min) || RangeContainsInclusive(A, B.Max));
    return Result;
}

//~ Range F32
union range_f32 {
    struct {
        f32 Min;
        f32 Max;
    };
    struct {
        f32 Start;
        f32 End;
    };
};

#define INFINITE_RANGE MakeRangeF32(F32_NEGATIVE_INFINITY, F32_POSITIVE_INFINITY)

tyler_function inline range_f32
MakeRangeF32(f32 Start, f32 End){
    range_f32 Result;
    Result.Start = Minimum(Start, End);
    Result.End   = Maximum(Start, End);
    return Result;
}

tyler_function inline range_f32
SizeRangeF32(f32 Start, f32 Size){
    range_f32 Result = MakeRangeF32(Start, Start+Size);
    return Result;
}

tyler_function inline range_f32
CenterRangeF32(f32 Center, f32 Size){
    Center -= 0.5f*Size;
    range_f32 Result = MakeRangeF32(Center, Center+Size);
    return Result;
}

tyler_function inline f32
RangeSize(range_f32 Range){
    return Range.End-Range.Start;
}

tyler_function inline range_f32
RangeRectify(range_f32 Range){
    range_f32 Result;
    Result.Start = Minimum(Range.Start, Range.End);
    Result.End   = Maximum(Range.Start, Range.End);
    return Result;
}

tyler_function inline range_f32
RangeShift(range_f32 Range, f32 Shift){
    range_f32 Result = Range;
    Result.Start += Shift;
    Result.End   += Shift;
    return Result;
}

tyler_function inline b8
RangeContains(range_f32 Range, f32 Value){
    b8 Result = (Range.Min < Value) && (Value < Range.Max);
    return Result;
}

tyler_function inline b8
RangeContainsInclusive(range_f32 Range, f32 Value){
    b8 Result = (Range.Min <= Value) && (Value <= Range.Max);
    return Result;
}

tyler_function inline b8
RangeContains(range_f32 A, range_f32 B){
    b8 Result = (A.Min < B.Min) && (B.Max < A.Max);
    return Result;
}

tyler_function inline b8
RangeContainsInclusive(range_f32 A, range_f32 B){
    b8 Result = (A.Min <= B.Min) && (B.Max <= A.Max);
    return Result;
}

tyler_function inline range_f32
RangeCrop(range_f32 Super, range_f32 Sub){
    range_f32 Result;
    Result.Min = Maximum(Super.Min, Sub.Min);
    Result.Max = Minimum(Super.Max, Sub.Max);
    return Result;
}

tyler_function inline f32
RangeClamp(range_f32 R, f32 S){
    S = Clamp(S, R.Min, R.Max);
    return S;
}

tyler_function inline b8
RangeOverlaps(range_f32 A, range_f32 B){
    b8 Result = (RangeContains(A, B.Min) || RangeContains(A, B.Max));
    return Result;
}

tyler_function inline b8
RangeOverlapsInclusive(range_f32 A, range_f32 B){
    b8 Result = (RangeContainsInclusive(A, B.Min) || RangeContainsInclusive(A, B.Max));
    return Result;
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
    struct {
        f32 X0, Y0;
        f32 X1, Y1;
    };
    struct {
        f32 Left,  Bottom;
        f32 Right, Top;
    };
};

struct rect_s32 {
    v2s Min;
    v2s Max;
};

tyler_function inline rect
operator+(rect A, v2 B){
    rect Result;
    Result.Min = A.Min + B;
    Result.Max = A.Max + B;
    return(Result);
}

tyler_function inline rect
operator+(v2 B, rect A){
    rect Result;
    Result.Min = A.Min + B;
    Result.Max = A.Max + B;
    return(Result);
}

tyler_function inline rect
operator-(rect A, v2 B){
    rect Result;
    Result.Min = A.Min - B;
    Result.Max = A.Max - B;
    return(Result);
}

tyler_function inline rect
operator*(rect A, f32 B){
    rect Result;
    Result.Min = A.Min * B;
    Result.Max = A.Max * B;
    return(Result);
}

tyler_function inline rect
operator*(f32 B, rect A){
    rect Result;
    Result.Min = A.Min * B;
    Result.Max = A.Max * B;
    return(Result);
}

tyler_function inline rect
operator/(rect A, f32 B){
    rect Result;
    Result.Min = A.Min / B;
    Result.Max = A.Max / B;
    return(Result);
}

tyler_function inline rect
operator+=(rect &A, v2 B){
    A.Min += B;
    A.Max += B;
    return(A);
}

tyler_function inline rect
operator-=(rect &A, v2 B){
    A.Min -= B;
    A.Max -= B;
    return(A);
}

tyler_function inline rect
operator*=(rect &A, f32 B){
    A.Min *= B;
    A.Max *= B;
    return(A);
}

tyler_function inline rect
operator/=(rect &A, f32 B){
    A.Min /= B;
    A.Max /= B;
    return(A);
}

tyler_function inline rect
MakeRect(v2 Min, v2 Max){
    rect Result;
    Result.Min = Min;
    Result.Max = Max;
    return(Result);
}

tyler_function inline rect
MakeRect(f32 MinX, f32 MinY, f32 MaxX, f32 MaxY){
    rect Result = MakeRect(V2(MinX, MinY), V2(MaxX, MaxY));
    return(Result);
}

tyler_function inline rect_s32
RectS32(v2s Min, v2s Max){
    rect_s32 Result;
    Result.Min = Min;
    Result.Max = Max;
    return(Result);
}

tyler_function inline rect_s32 
RectS32(rect Rect){
    rect_s32 Result;
    Result.Min.X = Truncate(Rect.Min.X);
    Result.Min.Y = Truncate(Rect.Min.Y);
    Result.Max.X = (s32)Ceil(Rect.Max.X);
    Result.Max.Y = (s32)Ceil(Rect.Max.Y);
    return(Result);
}

tyler_function inline rect
CenterRect(v2 P, v2 Size){
    rect Result;
    Result.Min = P - 0.5f*Size;
    Result.Max = P + 0.5f*Size;
    return(Result);
}

tyler_function inline rect
RectRectify(rect Rect){
    rect Result = {};
    Result.Min.X = Minimum(Rect.Min.X, Rect.Max.X);
    Result.Min.Y = Minimum(Rect.Min.Y, Rect.Max.Y);
    Result.Max.X = Maximum(Rect.Min.X, Rect.Max.X);
    Result.Max.Y = Maximum(Rect.Min.Y, Rect.Max.Y);
    
    return(Result);
}

tyler_function inline rect
TopLeftRect(v2 TopLeft, v2 Size){
    rect Result;
    Result.Min = V2(TopLeft.X,        TopLeft.Y-Size.Y);
    Result.Max = V2(TopLeft.X+Size.X, TopLeft.Y);
    return(Result);
}

tyler_function inline rect
SizeRect(v2 Min, v2 Size){
    rect Result;
    Result.Min = Min;
    Result.Max = Min+Size;
    return(Result);
}

tyler_function inline v2
RectSize(rect Rect){
    v2 Result = Rect.Max - Rect.Min;
    return(Result);
}

tyler_function inline range_f32
RectXRange(rect Rect){
    return MakeRangeF32(Rect.X0, Rect.X1);
}

tyler_function inline range_f32
RectYRange(rect Rect){
    return MakeRangeF32(Rect.Y0, Rect.Y1);
}

tyler_function inline f32
RectArea(rect Rect){
    v2 Size = RectSize(Rect);
    return Size.X*Size.Y;
}

tyler_function inline f32
RectWidth(rect Rect){
    f32 Result = Rect.Right - Rect.Left;
    return Result;
}

tyler_function inline b8
IsPointInRect(v2 Point, rect Rect){
    b8 Result = ((Rect.Min.X < Point.X) && (Point.X < Rect.Max.X) &&
                 (Rect.Min.Y < Point.Y) && (Point.Y < Rect.Max.Y));
    return(Result);
}

tyler_function inline b8
DoRectsOverlap(rect A, rect B){
    b8 Result = ((A.Min.X <= B.Max.X) &&
                 (B.Min.X <= A.Max.X) &&
                 (A.Min.Y <= B.Max.Y) &&
                 (B.Min.Y <= A.Max.Y));
    return(Result);
}

tyler_function inline rect
RectGrow(rect Rect, f32 G){
    rect Result = Rect;
    Result.Min -= V2(G, G);
    Result.Max += V2(G, G);
    return(Result);
}

tyler_function inline rect
RectGrow(rect Rect, v2 G){
    rect Result = Rect;
    Result.Min -= G;
    Result.Max += G;
    return(Result);
}

tyler_function inline v2
RectCenter(rect Rect){
    v2 Result = {};
    v2 Size = RectSize(Rect);
    Result = Rect.Min + 0.5f*Size;
    
    return(Result);
}

tyler_function inline rect
RectSweep(rect RectA, v2 Delta){
    rect RectB = RectA + Delta;
    rect Result;
    Result.Min = V2Minimum(RectA.Min, RectB.Min);
    Result.Max = V2Maximum(RectA.Max, RectB.Max);
    
    return(Result);
}

tyler_function inline rect
RectLerp(rect A, rect B, f32 T){
    rect Result;
    Result.Min = V2Lerp(A.Min, B.Min, T);
    Result.Max = V2Lerp(A.Max, B.Max, T);
    return(Result);
}

tyler_function inline rect
RectPercent(rect R, f32 X0Percent, f32 Y0Percent, f32 X1Percent, f32 Y1Percent){
    rect Result;
    v2 Size = RectSize(R);
    Result.X0 = R.X0 + Size.X*X0Percent;
    Result.Y0 = R.Y0 + Size.Y*Y0Percent;
    Result.X1 = R.X0 + Size.X*X1Percent;
    Result.Y1 = R.Y0 + Size.Y*Y1Percent;
    return Result;
}

tyler_function inline rect
RectMoveUp(rect R, f32 Amount){
    R.Y0 += Amount;
    R.Y1 += Amount;
    return R;
}

tyler_function inline rect
RectMoveRight(rect R, f32 Amount){
    R.X0 += Amount;
    R.X1 += Amount;
    return R;
}

tyler_function inline rect
RectRound(rect R){
    R.Min = V2Round(R.Min);
    R.Max = V2Round(R.Max);
    return R;
}

//~ Intrinsics
struct bit_scan_result
{
    u32 Index;
    b8 Found;
};

#if defined(_MSC_VER) && !defined(__clang__)
#include <intrin.h>
#pragma intrinsic(_BitScanForward)

tyler_function inline bit_scan_result
ScanForLeastSignificantSetBit(u64 Mask){
    bit_scan_result Result;
    Result.Found = _BitScanForward64(&(unsigned long)Result.Index, Mask);
    return(Result);
}

tyler_function inline bit_scan_result
ScanForMostSignificantSetBit(u64 Mask){
    bit_scan_result Result;
    Result.Found = _BitScanReverse64(&(unsigned long)Result.Index, Mask);
    return(Result);
}

tyler_function inline u32
CountLeadingZeroes(u32 Value){
    u32 Result = (u32)__lzcnt(Value);
    return(Result);
}

tyler_function inline u32
CountLeadingOnes(u32 Value){
    u32 Result = (u32)__lzcnt(~Value);
    return(Result);
}

tyler_function void
CopyMemory(void *To, const void *From, umw Size) {
#if 0
    for (umw I = 0; I < Size; I++)
    {
        *((u8*)To+I) = *((u8*)From+I);
    }
#else
    __movsb((u8 *)To, (u8 *)From, Size);
#endif
}

tyler_function void
ZeroMemory(void *Memory, umw Size) {
#if 0
    for (umw I = 0; I < Size; I++){
        *((u8*)Memory+I) = 0;
    }
#else
    __stosb((u8 *)Memory, 0, Size);
#endif
}

tyler_function u32 
PopcountU32(u32 A){
    u32 Result = _mm_popcnt_u32(A);
    return(Result);
}

tyler_function inline u64 
AtomicAddU64(u64 volatile *Value, u64 Addend)
{
    u64 Result = _InterlockedExchangeAdd64((__int64 volatile *)Value, Addend);
    return Result;
}

#else 
#error Please implement intrinsics for this compiler!
#endif

//~ Threading

struct ticket_mutex {
    u64 TicketCounter;
    u64 Serving;
};

tyler_function inline void 
TicketMutexBegin(ticket_mutex *Mutex){
    u64 Ticket = AtomicAddU64(&Mutex->TicketCounter, 1);
    while(Mutex->Serving != Ticket) { _mm_pause(); }
}

tyler_function inline void 
TicketMutexEnd(ticket_mutex *Mutex){
    AtomicAddU64(&Mutex->Serving, 1);
}

//~ General helpers

// NOTE(Tyler): Length excluding the null terminator
tyler_function u32
CStringLength(const char *String){
    u32 Result = 0;
    for(char C = *String; C; C = *(++String)){
        Result++;
    }
    return(Result);
}

tyler_function void
CopyCString(char *To, const char *From, u32 MaxSize){
    u32 I = 0;
    while(From[I] && (I < MaxSize-1)){
        To[I] = From[I];
        I++;
    }
    To[I] = 0;
}

tyler_function void
CopyCString(char *To, const char *From){
    CopyCString(To, From, CStringLength(From)+1);
}

//~
// NOTE(Tyler): This exists because windows has crlf line endings!!!
tyler_function inline b8
IsNewLine(char C){
    b8 Result = ((C == '\n') ||
                 (C == '\r'));
    return Result;
}

tyler_function inline b8
IsWhiteSpace(char C){
    b8 Result = ((C == ' ') ||
                 (C == '\t') ||
                 (C == '\n') ||
                 (C == '\r'));
    return(Result);
}

tyler_function inline b8
IsALetter(char C){
    b8 Result = ((('a' <= C) && (C <= 'z')) ||
                 (('A' <= C) && (C <= 'Z')));
    return(Result);
}

tyler_function inline b8
IsANumber(char C){
    b8 Result = (('0' <= C) && (C <= '9'));
    return(Result);
}

tyler_function inline char 
CharToLower(char C){
    if((('A' <= C) && (C <= 'Z'))) C -= 'A'-'a';
    return C;
}

tyler_function inline void 
CStringMakeLower(char *S){
    for(u32 I=0; S[I]; I++){
        if((('A' <= S[I]) && (S[I] <= 'Z'))) S[I] -= 'A'-'a';
    }
}

tyler_function constexpr b8
CompareCStrings(const char *A, const char *B){
    while(*A && *B){
        if(*A++ != *B++){
            return false;
        }
    }
    if(*A != *B) return false;
    
    return true;
}

enum alphabetic_order {
    AlphabeticOrder_Equal,
    AlphabeticOrder_AFirst,
    AlphabeticOrder_BFirst,
};
tyler_function constexpr alphabetic_order
CStringsAlphabeticOrder(const char *A, const char *B){
    while(*A && *B){
        if(*A < *B)          return AlphabeticOrder_AFirst;
        else if(*A++ > *B++) return AlphabeticOrder_BFirst;
    }
    if(*A < *B)      return AlphabeticOrder_AFirst;
    else if(*A > *B) return AlphabeticOrder_BFirst;
    
    return AlphabeticOrder_Equal;
}

tyler_function b8
IsStringASubset(const char *A, const char *B){
    while(*A && *B){
        if(*A++ != *B++){
            return false;
        }
    }
    
    return true;
}

tyler_function constexpr u64
HashString(const char *String){
    u64 Result = 71984823;
    while(char Char = *String++) {
        Result += (Char << 5) + Char;
    }
    return(Result);
}

tyler_function inline b8
IsFirstStringFirst(const char *A, const char *B){
    while(*A && *B){
        if(*A < *B) return true;
        else if(*A > *B) return false;
        
        *A++;
        *B++;
    }
    
    return (*A == 0);
}

//~ Memory arena

tyler_function inline umw
AlignValue(umw Value, umw Alignment){
    umw Result = ((Value+(Alignment-1)) & ~(Alignment-1));
    return(Result);
}

typedef u32 memory_flags;
enum memory_flags_ {
    MemoryFlag_None       = (0 << 0),
    MemoryFlag_ZeroMemory = (1 << 1),
};

struct memory_info {
    memory_flags Flags;
    umw Alignment;
};

tyler_function inline memory_info
ZeroAndAlign(umw Alignment=4){
    memory_info Result;
    Result.Flags = MemoryFlag_ZeroMemory;
    Result.Alignment = Alignment;
    return(Result);
}

tyler_function inline memory_info
NoneAndAlign(umw Alignment=4){
    memory_info Result;
    Result.Flags = MemoryFlag_None;
    Result.Alignment = Alignment;
    return(Result);
}

struct memory_arena {
    u8 *Memory;
    umw Used;
    umw Size;
};

tyler_function void
InitializeArena(memory_arena *Arena, void *Memory, umw Size){
    *Arena = {};
    Arena->Memory = (u8 *)Memory;
    Arena->Size = Size;
}

tyler_function memory_arena
MakeArena(memory_arena *Arena, umw Size){
    Assert((Arena->Used+Size) < Arena->Size);
    memory_arena Result;
    Result.Memory = Arena->Memory+Arena->Used;
    Arena->Used += Size;
    Result.Size = Size;
    Result.Used = 0;
    ZeroMemory(Result.Memory, Result.Size);
    return(Result);
}

#define ArenaPushType(Arena, Type) (Type *)ArenaPush(Arena, sizeof(Type))
#define ArenaPushArray(Arena, Type, Count) (Type *)ArenaPush(Arena, sizeof(Type)*(Count))

#define ArenaPushTypeSpecial(Arena, Type, Info) (Type *)ArenaPush(Arena, sizeof(Type), Info)
#define ArenaPushArraySpecial(Arena, Type, Count, Info) (Type *)ArenaPush(Arena, sizeof(Type)*(Count), Info)

tyler_function void *
ArenaPush(memory_arena *Arena, umw Size, memory_info Info=ZeroAndAlign(4)){
    Size = AlignValue(Size, Info.Alignment);
    umw UnAligned = (umw)(Arena->Memory+Arena->Used);
    u8 *Result = (u8 *)AlignValue(UnAligned, Info.Alignment);
    umw Difference = (umw)Result - UnAligned;
    Arena->Used += Size+Difference;
    Assert(Arena->Used <= Arena->Size);
    
    if(Info.Flags & MemoryFlag_ZeroMemory){
        ZeroMemory(Result, Size);
    }
    
    return(Result);
}

tyler_function void *
ArenaResizeMemory(memory_arena *Arena, void *OldMemory, umw OldSize, umw NewSize, 
                  memory_info Info=ZeroAndAlign(4)){
    // We just forget about the old allocation, this shouldn't probably shouldn't be
    // used in arenas that are never cleared
    void *Result = ArenaPush(Arena, NewSize, Info);
    CopyMemory(Result, OldMemory, OldSize);
    
    return(Result);
}

tyler_function inline char *
ArenaPushCString(memory_arena *Arena, const char *String){
    u32 Size = CStringLength(String)+1;
    char *Result = ArenaPushArray(Arena, char, Size);
    CopyCString(Result, String, Size);
    return(Result);
}

// TODO(Tyler): Part of me doesn't like having to calls to vsnprintf.
tyler_function char *
VArenaPushFormatCString(memory_arena *Arena, const char *Format, va_list VarArgs){
    s32 Length = TYLER_VSNPRINTF(0, 0, Format, VarArgs);
    char *Buffer = ArenaPushArray(Arena, char, Length+1);
    TYLER_VSNPRINTF(Buffer, Length+1, Format, VarArgs);
    return Buffer;
}

tyler_function char *
ArenaPushFormatCString(memory_arena *Arena, const char *Format, ...){
    va_list VarArgs;
    va_start(VarArgs, Format);
    char *Result = VArenaPushFormatCString(Arena, Format, VarArgs);
    va_end(VarArgs);
    return Result;
}

tyler_function char *
VArenaCStringConcatenateN(memory_arena *Arena, u32 N, va_list VarArgs){
    u32 TotalLength = 0;
    va_list LengthArgs;
    va_copy(LengthArgs, VarArgs);
    FOR_RANGE(I, 0, N){
        const char *String = va_arg(LengthArgs, const char *);
        TotalLength += CStringLength(String);
    }
    va_end(LengthArgs);
    
    char *Result = ArenaPushArray(Arena, char, TotalLength+1);
    u32 LengthCounter = 0;
    FOR_RANGE(I, 0, N){
        const char *String = va_arg(VarArgs, const char *);
        u32 Length = CStringLength(String);
        CopyMemory(Result+LengthCounter, String, Length);
        LengthCounter += Length;
    }
    
    return Result;
}

tyler_function char *
ArenaCStringConcatenateN(memory_arena *Arena, u32 N, ...){
    va_list VarArgs;
    va_start(VarArgs, N);
    char *Result = VArenaCStringConcatenateN(Arena, N, VarArgs);
    va_end(VarArgs);
    return Result;
}

tyler_function char *
ArenaCStringConcatenate(memory_arena *Arena, const char *A, const char *B){
    return ArenaCStringConcatenateN(Arena, 2, A, B);
}

tyler_function inline char *
ArenaPushLowerCString(memory_arena *Arena, const char *S){
    u32 L = CStringLength(S);
    char *Result = ArenaPushArray(Arena, char, L+1);
    for(u32 I=0; S[I]; I++){
        Result[I] = CharToLower(S[I]);
    }
    Result[L] = 0;
    return Result;
}

tyler_function void
ArenaClear(memory_arena *Arena){
    Arena->Used = 0;
}

//~ Memory arena markers
struct memory_arena_marker {
    umw Used;
};

tyler_function inline memory_arena_marker 
ArenaBeginMarker(memory_arena *Arena){
    memory_arena_marker Result = {};
    Result.Used = Arena->Used;
    return(Result);
}

tyler_function inline void
ArenaEndMarker(memory_arena *Arena, memory_arena_marker *Marker){
    Assert(Arena->Used >= Marker->Used);
    Arena->Used = Marker->Used;
}

tyler_function inline void 
ArenaMarkerAdd(memory_arena_marker *Marker, u32 Used){
    Marker->Used += Used;
}

struct scoped_memory_arena_marker {
    memory_arena *Arena;
    memory_arena_marker Marker;
    inline scoped_memory_arena_marker(memory_arena *Arena_){
        Arena = Arena_;
        Marker = ArenaBeginMarker(Arena_);
    }
    
    inline ~scoped_memory_arena_marker(){
        ArenaEndMarker(Arena, &Marker);
    }
};

#define ARENA_SCOPED_MARKER_(ID, Arena) scoped_memory_arena_marker ScopedMemoryArenaMarker_##ID(Arena)
#define ARENA_SCOPED_MARKER(Arena) ARENA_SCOPED_MARKER_(__LINE__, Arena)
#define ARENA_FUNCTION_MARKER(Arena) ARENA_SCOPED_MARKER_(__FUNCTION__, Arena)

//~ Variable definitions

global memory_arena GlobalPermanentMemory;
global memory_arena GlobalTransientMemory;

//~

tyler_function void
MoveMemory(void *To, const void *From, umw Size){
    if(Size == 0) return;
    u8 *Temp = ArenaPushArray(&GlobalTransientMemory, u8, Size);
    CopyMemory(Temp, From, Size);
    CopyMemory(To, Temp, Size);
}

//~ Array

template<typename T>
struct array {
    T *Items;
    u32 Count;
    u32 MaxCount;
    
    inline T &operator[](s64 Index){
        Assert(Index < Count);
        return(Items[Index]);
    }
    
    inline operator b8(){  return (Items != 0) || (Count > 0); }
};

template<typename T> tyler_function inline array<T>
MakeArray(memory_arena *Arena, u32 MaxCount){
    array<T> Result = {};
    Result.Items = ArenaPushArray(Arena, T, MaxCount);
    Result.MaxCount = MaxCount;
    return(Result);
}

template<typename T> tyler_function inline array<T>
MakeFullArray(memory_arena *Arena, u32 Count, umw Alignment=4){
    array<T> Result = {};
    Result.Items = ArenaPushArraySpecial(Arena, T, Count, ZeroAndAlign(Alignment));
    Result.Count = Count;
    Result.MaxCount = Count;
    return(Result);
}

template<typename T> tyler_function inline array<T>
MakeFullArray(T *Items, u32 Count){
    array<T> Result = {};
    Result.Items = Items;
    Result.Count = Count;
    Result.MaxCount = Count;
    return(Result);
}

template<typename T> tyler_function inline array<T>
MakeFullArrayFromArgs(memory_arena *Arena, u32 Count, ...){
    va_list VarArgs;
    va_start(VarArgs, Count);
    array<T> Result = MakeArray(Arena, Count);
    FOR_RANGE(I, 0, Count){
        T Arg = va_arg(VarArgs, T);
        ArrayAdd(&Result, Arg);
    }
    va_end(VarArgs);
    return Result;
}

template<typename T> tyler_function inline array<T>
MakeFullArrayFromArgs_(memory_arena *Arena, u32 Count, ...){
    va_list VarArgs;
    va_start(VarArgs, Count);
    array<T> Result = MakeArray<T>(Arena, Count);
    FOR_RANGE(I, 0, Count){
        T Arg = va_arg(VarArgs, T);
        ArrayAdd(&Result, Arg);
    }
    va_end(VarArgs);
    return Result;
}

template<typename T> tyler_function inline T
ArrayGet(array<T> *Array, s64 Index){
    Assert(Index < Array->Count);
    return Array->Items[Index];
}

template<typename T> tyler_function inline void
ArrayClear(array<T> *Array){
    Array->Count = 0;
}

template<typename T> tyler_function inline T *
ArrayMaybeAdd(array<T> *Array, T Item){
    T *Result = 0;
    if(Array->Count+1 <= Array->MaxCount){
        Result = &Array->Items[Array->Count];
        Array->Items[Array->Count++] = Item;
    }
    return Result;
}

template<typename T> tyler_function inline void
ArrayAdd(array<T> *Array, T Item){
    Assert(ArrayMaybeAdd(Array, Item));
}

template<typename T> tyler_function inline T *
ArrayAlloc(array<T> *Array, u32 N=1){
    T *Result = 0;
    if(Array->Count+N <= Array->MaxCount){
        Result = &Array->Items[Array->Count];
        Array->Count += N;
    }else{
        Assert(0);
    }
    ZeroMemory(Result, sizeof(T));
    return(Result);
}

// A better insert might be better,
// following the same logic as ordered and unordered remove 
template<typename T> void
ArrayInsert(array<T> *Array, u32 Index, T Item){
    Assert(Index <= Array->Count);
    MoveMemory(&Array->Items[Index+1], 
               &Array->Items[Index], 
               (Array->Count-Index)*sizeof(T));
    Array->Items[Index] = Item;
    Array->Count++;
}

template<typename T> tyler_function inline T *
ArrayInsertAlloc(array<T> *Array, u32 Index){
    Assert(Index <= Array->Count);
    MoveMemory(&Array->Items[Index+1], 
               &Array->Items[Index], 
               (Array->Count-Index)*sizeof(T));
    T *NewItem = &Array->Items[Index];
    Array->Count++;
    return(NewItem);
}

template<typename T> tyler_function inline void
ArrayOrderedRemove(array<T> *Array, u32 Index){
    Assert(Index < Array->Count);
    MoveMemory(&Array->Items[Index], 
               &Array->Items[Index+1], 
               (Array->Count-Index)*sizeof(T));
    Array->Count--;
}

template<typename T> tyler_function inline void
ArrayUnorderedRemove(array<T> *Array, u32 Index){
    Array->Items[Index] = Array->Items[Array->Count-1];
    Array->Count--;
}

template<typename T> tyler_function inline b8
ArrayRemoveByValue(array<T> *Array, T Value){
    for(u32 I=0; I<Array->Count; I++){
        if(ArrayGet(Array, I) == Value){
            ArrayOrderedRemove(Array, I);
            return true;
        }
    }
    return false;
}

template<typename T> tyler_function inline b8
ArrayHasItem(array<T> *Array, T Item){
    for(u32 I=0; I<Array->Count; I++){
        if(ArrayGet(Array, I) == Item) return true;
    }
    return false;
}

template<typename T> tyler_function inline array<T>
ArrayFinalize(memory_arena *Arena, array<T> *Array){
    array<T> Result = MakeArray<T>(Arena, Array->Count);
    for(u32 I=0; I<Array->Count; I++){
        *ArrayAlloc(&Result) = ArrayGet(Array, I);
    }
    return Result;
}

//~ Stack
template<typename T> 
using stack = array<T>;

template<typename T> tyler_function inline stack<T>
MakeStack(memory_arena *Arena, u32 MaxCount){
    stack<T> Result = {};
    Result = MakeArray<T>(Arena, MaxCount);
    return(Result);
}

template<typename T> tyler_function inline void
StackPush(stack<T> *Stack, T Item){
    ArrayAdd(Stack, Item);
}

template<typename T> tyler_function inline void
StackPushSafe(stack<T> *Stack, T Item){
    if(Stack->Count == Stack->MaxCount){
        ArrayOrderedRemove(Stack, 0);
    }
    ArrayAdd(Stack, Item);
}

template<typename T> tyler_function inline T *
StackPushAlloc(stack<T> *Stack, u32 N=1){
    T *Result = ArrayAlloc(Stack, N);
    return(Result);
}

template<typename T> tyler_function inline T
StackPop(stack<T> *Stack){
    T Result = ArrayGet(Stack, Stack->Count-1);
    ArrayOrderedRemove(Stack, Stack->Count-1);
    return(Result);
}

template<typename T> tyler_function inline T
StackPeek(stack<T> *Stack, u32 N=0){
    N++;
    Assert(N <= Stack->Count);
    T Result = Stack->Items[Stack->Count-N];
    return(Result);
}

template<typename T> tyler_function inline void
StackClear(stack<T> *Stack){
    ArrayClear(Stack);
}

//~ Dynamic array
template <typename T>
struct dynamic_array {
    memory_arena *Arena;
    
    T *Items;
    u32 Count;
    u32 Capacity;
    
    inline T &operator[](s64 Index){
        Assert(Index < Count);
        return(Items[Index]);
    }
    
    inline operator b8(){  return(Items != 0); }
    inline operator b16(){ return(Items != 0); }
    inline operator b32(){ return(Items != 0); }
    inline operator b64(){ return(Items != 0); }
};

template <typename T> tyler_function void 
InitializeArray(dynamic_array<T> *Array, int InitialCapacity, memory_arena *Arena=0){
    *Array = {};
    if(Arena) Array->Items = ArenaPushArray(Arena, T, InitialCapacity);
    else Array->Items = (T *)OSDefaultAlloc(InitialCapacity*sizeof(T));
    Array->Arena = Arena;
    Array->Capacity = InitialCapacity;
}

template <typename T> tyler_function inline dynamic_array<T>
MakeDynamicArray(s32 InitialCapacity, memory_arena *Arena=0){
    dynamic_array<T> Result = {};
    if(Arena) Result.Items = ArenaPushArray(Arena, T, InitialCapacity);
    else Result.Items = (T *)OSDefaultAlloc(InitialCapacity*sizeof(T));
    Result.Arena = Arena;
    Result.Capacity = InitialCapacity;
    
    return Result;
}

template <typename T> tyler_function inline dynamic_array<T>
MakeDynamicArray(memory_arena *Arena, s32 InitialCapacity){
    return MakeDynamicArray<T>(InitialCapacity, Arena);
}

template <typename T> tyler_function inline void
DynamicArrayFree(dynamic_array<T> *Array){
    if(!Array->Arena) OSDefaultFree(Array->Items); 
}

template<typename T> tyler_function inline T
ArrayGet(dynamic_array<T> *Array, s64 Index){
    Assert(Index < Array->Count);
    return Array->Items[Index];
}

template <typename T> tyler_function inline array<T>
MakeArray(dynamic_array<T> *Array){
    array<T> Result = MakeFullArray(Array->Items, Array->Count);
    return Result;
}

template <typename T> void 
DeleteArray(dynamic_array<T> *Array){
    if(!Array->Arena) DefaultFree(Array->Items);
}

template <typename T> tyler_function void 
ArrayClear(dynamic_array<T> *Array){
    Array->Count = 0;
}

template <typename T> tyler_function void 
ArrayAdd(dynamic_array<T> *Array, T *item){
    if(Array->Count >= Array->Capacity){
        umw OldSize = Array->Capacity*sizeof(T);
        umw NewSize = 2*Array->Capacity*sizeof(T);
        Array->Capacity *= 2;
        if(Array->Arena) Array->Items = (T *)ArenaResizeMemory(Array->Arena, Array->Items, OldSize, NewSize);
        else Array->Items = (T *)OSDefaultRealloc(Array->Items, NewSize);
    }
    Array->Items[Array->Count++] = *item;
}

template <typename T> void 
ArrayAdd(dynamic_array<T> *Array, T item){
    ArrayAdd(Array, &item);
}

template<typename T> tyler_function inline T *
ArrayAlloc(dynamic_array<T> *Array, u32 N=1){
    T *Result = 0;
    if(Array->Count+N >= Array->Capacity){
        umw OldSize = Array->Capacity*sizeof(T);
        umw NewSize = 2*Array->Capacity*sizeof(T);
        Array->Capacity *= 2;
        if(Array->Arena) Array->Items = (T *)ArenaResizeMemory(Array->Arena, Array->Items, OldSize, NewSize);
        else Array->Items = (T *)OSDefaultRealloc(Array->Items, NewSize);
    }
    Result = &Array->Items[Array->Count];
    Array->Count += N;
    *Result = {};
    return(Result);
}

template<typename T> tyler_function void
ArrayInsert(dynamic_array<T> *Array, u32 Index, T Item){
    if(Array->Count+1 >= Array->Capacity){
        umw OldSize = Array->Capacity*sizeof(T);
        umw NewSize = 2*Array->Capacity*sizeof(T);
        Array->Capacity *= 2;
        if(Array->Arena) Array->Items = (T *)ArenaResizeMemory(Array->Arena, Array->Items, OldSize, NewSize);
        else Array->Items = (T *)OSDefaultRealloc(Array->Items, NewSize);
    }
    MoveMemory(&Array->Items[Index+1], 
               &Array->Items[Index], 
               (Array->Count-Index)*sizeof(T));
    Array->Items[Index] = Item;
    Array->Count++;
}

template<typename T> tyler_function T *
ArrayInsertAlloc(dynamic_array<T> *Array, u32 Index){
    if(Array->Count+1 >= Array->Capacity){
        umw OldSize = Array->Capacity*sizeof(T);
        umw NewSize = 2*Array->Capacity*sizeof(T);
        Array->Capacity *= 2;
        if(Array->Arena) Array->Items = (T *)ArenaResizeMemory(Array->Arena, Array->Items, OldSize, NewSize);
        else Array->Items = (T *)OSDefaultRealloc(Array->Items, NewSize);
    }
    MoveMemory(&Array->Items[Index+1], 
               &Array->Items[Index], 
               (Array->Count-Index)*sizeof(T));
    T *Result = &Array->Items[Index];
    Array->Count++;
    return Result;
}

template<typename T> tyler_function inline void
ArrayOrderedRemove(dynamic_array<T> *Array, u32 Index){
    MoveMemory(&Array->Items[Index], 
               &Array->Items[Index+1], 
               (Array->Count-Index)*sizeof(T));
    Array->Count--;
}

template<typename T> tyler_function inline void
ArrayUnorderedRemove(dynamic_array<T> *Array, u32 Index){
    Array->Items[Index] = Array->Items[Array->Count-1];
    Array->Count--;
}

template<typename T> tyler_function inline b8
ArrayHasItem(dynamic_array<T> *Array, T Item){
    for(u32 I=0; I<Array->Count; I++){
        if(ArrayGet(Array, I) == Item) return true;
    }
    return false;
}
template<typename T> tyler_function inline array<T>
ArrayFinalize(memory_arena *Arena, dynamic_array<T> *Array){
    array<T> Result = MakeArray<T>(Arena, Array->Count);
    for(u32 I=0; I<Array->Count; I++){
        *ArrayAlloc(&Result) = ArrayGet<T>(Array, I);
    }
    return Result;
}

//~ Static arrays
template<typename T, u32 U>
struct static_array {
    T Items[U];
    u32 Count;
    
    inline T &operator[](s64 Index){
        Assert(Index < Count);
        return(Items[Index]);
    }
    
    inline operator b8(){  return (Items != 0) || (Count > 0); }
};

template<typename T, u32 U> tyler_function inline T
ArrayGet(static_array<T, U> *Array, s64 Index){
    Assert(Index < Array->Count);
    return Array->Items[Index];
}

template<typename T, u32 U> tyler_function inline void
ArrayClear(static_array<T, U> *Array){
    Array->Count = 0;
}

template<typename T, u32 U> tyler_function inline T *
ArrayMaybeAdd(static_array<T, U> *Array, T Item){
    T *Result = 0;
    if(Array->Count+1 <= U){
        Result = &Array->Items[Array->Count];
        Array->Items[Array->Count++] = Item;
    }
    return Result;
}

template<typename T, u32 U> tyler_function inline void
ArrayAdd(static_array<T, U> *Array, T Item){
    Assert(ArrayMaybeAdd(Array, Item));
}

template<typename T, u32 U> tyler_function inline T *
ArrayAlloc(static_array<T, U> *Array, u32 N=1){
    T *Result = 0;
    if(Array->Count+N <= U){
        Result = &Array->Items[Array->Count];
        Array->Count += N;
    }else{
        Assert(0);
    }
    ZeroMemory(Result, sizeof(T));
    return(Result);
}

// A better insert might be better,
// following the same logic as ordered and unordered remove 
template<typename T, u32 U> void
ArrayInsert(static_array<T, U> *Array, u32 Index, T Item){
    Assert(Index <= Array->Count);
    MoveMemory(&Array->Items[Index+1], 
               &Array->Items[Index], 
               (Array->Count-Index)*sizeof(T));
    Array->Items[Index] = Item;
    Array->Count++;
}

template<typename T, u32 U> tyler_function inline T *
ArrayInsertAlloc(static_array<T, U> *Array, u32 Index){
    Assert(Index <= Array->Count);
    MoveMemory(&Array->Items[Index+1], 
               &Array->Items[Index], 
               (Array->Count-Index)*sizeof(T));
    T *NewItem = &Array->Items[Index];
    Array->Count++;
    return(NewItem);
}

template<typename T, u32 U> tyler_function inline void
ArrayOrderedRemove(static_array<T, U> *Array, u32 Index){
    Assert(Index < Array->Count);
    MoveMemory(&Array->Items[Index], 
               &Array->Items[Index+1], 
               (Array->Count-Index)*sizeof(T));
    Array->Count--;
}

template<typename T, u32 U> tyler_function inline void
ArrayUnorderedRemove(static_array<T, U> *Array, u32 Index){
    Array->Items[Index] = Array->Items[Array->Count-1];
    Array->Count--;
}

template<typename T, u32 U> tyler_function inline b8
ArrayRemoveByValue(static_array<T, U> *Array, T Value){
    for(u32 I=0; I<Array->Count; I++){
        if(ArrayGet(Array, I) == Value){
            ArrayOrderedRemove(Array, I);
            return true;
        }
    }
    return false;
}

template<typename T, u32 U> tyler_function inline b8
ArrayHasItem(static_array<T, U> *Array, T Item){
    for(u32 I=0; I<Array->Count; I++){
        if(ArrayGet(Array, I) == Item) return true;
    }
    return false;
}

template<typename T, u32 U> tyler_function inline static_array<T, U>
ArrayFinalize(memory_arena *Arena, static_array<T, U> *Array){
    static_array<T, U> Result = MakeArray<T>(Arena, Array->Count);
    for(u32 I=0; I<Array->Count; I++){
        *ArrayAlloc(&Result) = ArrayGet(Array, I);
    }
    return Result;
}


//- Iteration
// TODO(Tyler): This does not work well for break statements.
template<typename T> tyler_function inline T *
ArrayForEachGet_(array<T> *Array, u32 I, int *Item){
    return &Array->Items[I];
}

template<typename T, u32 U> tyler_function inline T *
ArrayForEachGet_(static_array<T, U> *Array, u32 I, int *Item){
    return &Array->Items[I];
}

template<typename T> tyler_function inline T *
ArrayForEachGet_(dynamic_array<T> *Array, u32 I, int *Item){
    return &Array->Items[I];
}

#define FOR_EACH_PTR_(Item, Index, Array) \
for(u32 Index = 0, Keep_=true; Index < (Array)->Count; Index++, Keep_=true) \
for(auto *Item = &(Array)->Items[Index]; Keep_; Keep_=false)
#define FOR_EACH_(Item, Index, Array) \
for(u32 Index = 0, Keep_=true; Index < (Array)->Count; Index++, Keep_=true) \
for(auto &Item = (Array)->Items[Index]; Keep_; Keep_=false)

#define FOR_EACH_PTR(Item, Array) FOR_EACH_PTR_(Item, I_, Array)
#define FOR_EACH(Item, Array) FOR_EACH_(Item, I_, Array)

#define ARRAY_REMOVE_IN_LOOP(Array, Index) { ArrayUnorderedRemove(Array, Index); Index--; continue; }
#define ARRAY_REMOVE_IN_LOOP_ORDERED(Array, Index) { ArrayOrderedRemove(Array, Index); Index--; continue; }

//~ Array sorting 
// TODO(Tyler): All of this is very work in progress

// TODO(Tyler): This function uses a crude and basic sorting algorithm. 
// The sorting algorithm could be improved.
tyler_function inline void
ArraySortAlphabetical(memory_arena *Arena, array<const char *> *Array){
    array<const char *> Temp = MakeFullArray<const char *>(Arena, Array->Count);
    FOR_EACH_(S, I, Array){
        Temp[I] = S;
    }
    
    ArrayClear(Array);
    FOR_EACH(A, &Temp){
        u32 I = 0;
        FOR_EACH(B, Array){
            alphabetic_order Order = CStringsAlphabeticOrder(A, B);
            if(Order == AlphabeticOrder_AFirst){
                break;
            }
            I++;
        }
        ArrayInsert(Array, I, A);
    }
}


//~ Bucket array
// TODO(Tyler): This seems over complicated to me. 
// A simpler solution might be like a freelist for empty things? 
// Perhaps a system like an linked list of arrays?
global_constant u32 MAX_BUCKET_ITEMS = 64;
template<typename T, u32 U>
struct bucket_array_bucket {
    static_assert(U <= MAX_BUCKET_ITEMS);
    
    u32 Index;
    u32 Count;
    u64 Occupancy;// TODO(Tyler): This won't work for 32 bit systems, as there isn't
    // a _BitScanForward64 in those systems;
    T Items[U];
};

template<typename T, u32 U>
struct bucket_array {
    memory_arena *Arena;
    u32 Count;
    dynamic_array<bucket_array_bucket<T, U> *> Buckets;
    dynamic_array<bucket_array_bucket<T, U> *> UnfullBuckets;
};

struct bucket_index {
    u32 Bucket;
    u32 Item;
};

template<typename T>
struct bucket_array_iterator {
    T *Item;
    bucket_index Index;
    u32 I;
};

tyler_function inline bucket_index
BucketIndex(u32 Bucket, u32 Item){
    bucket_index Result;
    Result.Bucket = Bucket;
    Result.Item = Item;
    return Result;
}


template<typename T, u32 U>
tyler_function bucket_array_bucket<T, U> *
BucketArrayAllocBucket(bucket_array<T, U> *Array){
    typedef bucket_array_bucket<T, U> this_bucket; 
    // To avoid a comma inside the macro because it doesn't like that bucket<T, U> has a comma
    bucket_array_bucket<T,U> *Result = ArenaPushType(Array->Arena, this_bucket);
    *Result = {};
    Result->Index = Array->Buckets.Count;
    ArrayAdd(&Array->Buckets, Result);
    ArrayAdd(&Array->UnfullBuckets, Result);
    
    return(Result);
}

template<typename T, u32 U>
tyler_function void
InitializeBucketArray(bucket_array<T, U> *Array, memory_arena *Arena, u32 InitialBuckets=4){
    Assert(Arena);
    
    *Array = {};
    Array->Arena = Arena;
    InitializeArray(&Array->Buckets, InitialBuckets, Array->Arena);
    InitializeArray(&Array->UnfullBuckets, InitialBuckets, Array->Arena);
    bucket_index Index;
    bucket_array_bucket<T, U> *Bucket = BucketArrayAllocBucket(Array);
}

template<typename T, u32 U>
tyler_function T *
BucketArrayAlloc(bucket_array<T, U> *Array){
    T *Result = 0;
    if(Array->UnfullBuckets.Count == 0){
        BucketArrayAllocBucket(Array);
    }
    
    bucket_array_bucket<T, U> *Bucket = Array->UnfullBuckets[0];
    Assert(Bucket->Count < U);
    bit_scan_result BitScan = ScanForLeastSignificantSetBit(~Bucket->Occupancy);
    Assert(BitScan.Found);
    u32 Item = BitScan.Index;
    Result = &Bucket->Items[Item];
    *Result = {};
    Bucket->Occupancy |= (1ULL << Item);
    Bucket->Count++;
    Array->Count++;
    
    if(Bucket->Count >= U){ 
        ArrayUnorderedRemove(&Array->UnfullBuckets, 0);
    }
    
    return(Result);
}

template<typename T, u32 U>
tyler_function void
BucketArrayRemove(bucket_array<T, U> *Array, bucket_index Index){
    bucket_array_bucket<T, U> *Bucket = Array->Buckets[Index.Bucket];
    Assert(Bucket->Occupancy & (1ULL << Index.Item));
    
    Array->Count--;
    b8 WasFull = (Bucket->Count == U);
    Bucket->Count--;
    Bucket->Occupancy &= ~(1ULL << Index.Item);
    
    if(WasFull) ArrayAdd(&Array->UnfullBuckets, Bucket);
}

template<typename T, u32 U>
tyler_function void
BucketArrayRemoveAll(bucket_array<T, U> *Array){
    for(u32 I = 0; I < Array->Buckets.Count; I++){
        bucket_array_bucket<T, U> *Bucket = Array->Buckets[I];
        
        b8 WasFull = (Bucket->Count == U);
        Bucket->Count = 0;
        Bucket->Occupancy = 0;
        
        if(WasFull) ArrayAdd(&Array->UnfullBuckets, Bucket);
    }
    Array->Count = 0;
}

template<typename T, u32 U>
tyler_function inline bucket_array_iterator<T>
BucketArrayIteratorFromIndex(bucket_array<T, U> *Array, bucket_index Index){
    bucket_array_iterator<T> Result = {0, Index};
    return(Result);
}

template<typename T, u32 U>
tyler_function inline s32 
BucketArrayFindNextValidItem(bucket_array_bucket<T, U> *Bucket, s32 Index){
    u64 Occupancy = Bucket->Occupancy;
    Occupancy &= ~((1 << (Index+1))-1);
    
    while(true){
        bit_scan_result BitScan = ScanForLeastSignificantSetBit(Occupancy);
        if(!BitScan.Found) return -1;
        return BitScan.Index;
    }
}

template<typename T, u32 U>
tyler_function inline bucket_array_iterator<T>
BucketArrayBeginIteration(bucket_array<T, U> *Array){
    bucket_array_iterator<T> Result = {};
    
    while(true){
        bucket_array_bucket<T, U> *Bucket = Array->Buckets[Result.Index.Bucket];
        s32 Index = BucketArrayFindNextValidItem(Bucket, -1);
        if(Index > 0){
            Result.Index.Item = (u32)Index;
            break;
        }else{
            if(Result.Index.Bucket == Array->Buckets.Count-1) break;
            Result.Index.Bucket++;
        }
    }
    
    Result.Item = BucketArrayGet(Array, Result.Index);
    return(Result);
}

template<typename T, u32 U>
tyler_function inline b8
BucketArrayNextIteration(bucket_array<T, U> *Array, bucket_array_iterator<T> *Iterator){
    bucket_array_bucket<T, U> *Bucket = Array->Buckets[Iterator->Index.Bucket];
    s32 Index = BucketArrayFindNextValidItem(Bucket, Iterator->Index.Item);
    if(Index > 0){
        Iterator->Index.Item = Index;
        Iterator->I++;
        return true;
    }
    
    Iterator->Index.Bucket++;
    while(Iterator->Index.Bucket < Array->Buckets.Count){
        bucket_array_bucket<T, U> *Bucket = Array->Buckets[Iterator->Index.Bucket];
        s32 Index = BucketArrayFindNextValidItem(Bucket, Iterator->Index.Item);
        if(Index > 0){
            Iterator->Index.Item = Index;
            Iterator->I++;
            return true;
        }
        Iterator->Index.Bucket++;
    }
    
    return false;
}

template<typename T, u32 U>
tyler_function inline b8
BucketArrayContinueIteration(bucket_array<T, U> *Array, bucket_array_iterator<T> *Iterator){
    b8 Result = ((Iterator->I < Array->Count) &&
                 (Iterator->Index.Bucket < Array->Buckets.Count));
    if(Result){
        Iterator->Item = BucketArrayGet(Array, Iterator->Index);
        if(Iterator->Item->Flags & EntityFlag_Deleted) return false;
    }
    
    return(Result);
}

template<typename T, u32 U>
tyler_function inline T *
BucketArrayGet(bucket_array<T, U> *Array, bucket_index Index){
    bucket_array_bucket<T, U> *Bucket = Array->Buckets[Index.Bucket];
    T *Result = 0;
    if(Bucket->Occupancy & (1ULL << Index.Item))
        Result = &Bucket->Items[Index.Item];
    return(Result);
}

#define BUCKET_ARRAY_FOR_EACH(Iterator, Array) \
for(auto Iterator = BucketArrayBeginIteration(Array); \
BucketArrayContinueIteration(Array, &Iterator); \
BucketArrayNextIteration(Array, &Iterator))

#define BUCKET_ARRAY_FOR_EACH_(Iterator, Array, Initial) \
for(auto Iterator = BucketArrayIteratorFromIndex(Array, Initial); \
BucketArrayContinueIteration(Array, &Iterator); \
BucketArrayNextIteration(Array, &Iterator))

//~ Simple and very naive hash table
// // TODO(Tyler): This hash table implementation could be improved significantly


// NOTE(Tyler): For each key type that is defined for this hash table implementation,
// one must define two tyler_functions for that key type: 'HashKey' AND 'CompareKeys'. 

//- Predefined key type tyler_functions
// NOTE(Tyler): These are basic keys that would be commonly used in a hash table
tyler_function constexpr u64
HashKey(const char *String) {
    u64 Result = HashString(String);
    return(Result);
}

tyler_function constexpr u64
HashKey(u64 Value) {
    u64 Result = Value;
    return(Result);
}

tyler_function constexpr b32
CompareKeys(const char *A, const char *B){
    b32 Result = CompareCStrings(A, B);
    return(Result);
}

tyler_function constexpr b32
CompareKeys(u64 A, u64 B){
    b32 Result = (A == B);
    return(Result);
}

//- Implementation
template <typename KeyType, typename ValueType>
struct hash_table_bucket {
    u64 Hash;
    KeyType Key;
    ValueType Value;
    
    union{
        hash_table_bucket *Next;
        hash_table_bucket *NextFree;
    };
};

template <typename KeyType, typename ValueType>
struct hash_table {
    memory_arena *Arena;
    u32 UsedBucketCount;
    u32 MaxBucketCount;
    u32 Count;
    hash_table_bucket<KeyType, ValueType> *Buckets;
    
    hash_table_bucket<KeyType, ValueType> *NextFreeBucket;
};

template <typename KeyType, typename ValueType>
tyler_function constexpr void
HashTableInit(hash_table<KeyType, ValueType> *Table, memory_arena *Arena, u32 MaxBuckets){
    Table->Arena = Arena;
    //Result.Buckets = ArenaPushArray(Arena, decltype(*Result.Buckets), MaxBuckets);
    Table->Buckets = (decltype(Table->Buckets))ArenaPush(Arena, sizeof(*Table->Buckets)*MaxBuckets);
    Table->MaxBucketCount = MaxBuckets;
}

template <typename KeyType, typename ValueType>
tyler_function constexpr hash_table<KeyType, ValueType>
MakeHashTable(memory_arena *Arena, u32 MaxBuckets){
    hash_table<KeyType, ValueType> Result = {};
    HashTableInit(&Result, Arena, MaxBuckets);
    return(Result);
}

template <typename KeyType, typename ValueType>
tyler_function inline void 
HashTableCopy(hash_table<KeyType, ValueType> *OutTable, hash_table<KeyType, ValueType> *InTable){
    Assert(OutTable->MaxBuckets >= InTable->BucketsUsed);
    for(u32 I=0; I<InTable->MaxBuckets; I++){
        if(InTable->Hashes[I]){
            HashTableInsert(OutTable, InTable->Keys[I], InTable->Values[I]);
        }
    }
}

template <typename KeyType, typename ValueType>
tyler_function hash_table_bucket<KeyType, ValueType> *
HashTableGetBucket(hash_table<KeyType, ValueType> *Table, KeyType Key, b8 MakeNew=false){
    u64 Hash = HashKey(Key);
    
    u32 Index = Hash % Table->MaxBucketCount;
    auto *Bucket = &Table->Buckets[Index];
    if(Bucket->Hash == 0){
        if(!MakeNew) return 0;
        Table->UsedBucketCount++;
        Table->Count++;
        Bucket->Hash = Hash;
        Bucket->Key = Key;
    }else {
        while(Bucket){
            if((Bucket->Hash == Hash) && CompareKeys(Key, Bucket->Key)) break;
            
            if(MakeNew && !Bucket->Next){
                Bucket->Next = FREELIST_ALLOC(Table->NextFreeBucket, (decltype(Bucket))ArenaPush(Table->Arena, sizeof(*Bucket)));
                Table->Count++;
                Bucket = Bucket->Next;
                Bucket->Hash = Hash;
                Bucket->Key = Key;
                break;
            }
            Bucket = Bucket->Next;
        }
    }
    
    return Bucket;
}

template <typename KeyType, typename ValueType>
tyler_function constexpr void
HashTableRemove(hash_table<KeyType, ValueType> *Table, KeyType Key){
    u64 Hash = HashKey(Key);
    
    u32 Index = Hash % Table->MaxBucketCount;
    auto *Bucket = &Table->Buckets[Index];
    decltype(Bucket) PrevBucket = 0;
    if(Bucket->Hash == 0){
        INVALID_CODE_PATH;
    }else {
        while(Bucket){
            if((Bucket->Hash == Hash) && CompareKeys(Key, Bucket->Key)) break;
            PrevBucket = Bucket;
            Bucket = Bucket->Next;
        }
    }
    
    if(!PrevBucket){ // The bucket is the root bucket
        if(Bucket->Next) *Bucket = *Bucket->Next;
        else{
            *Bucket = {};
            Table->UsedBucketCount--;
        }
    }else{
        PrevBucket->Next = Bucket->Next;
        FREELIST_FREE(Table->NextFreeBucket, Bucket);
    }
    Table->Count--;
}

template <typename KeyType, typename ValueType>
tyler_function ValueType *
HashTableAlloc(hash_table<KeyType, ValueType> *Table, KeyType Key){
    auto *Bucket = HashTableGetBucket(Table, Key, true);
    Assert(Bucket);
    Assert(CompareKeys(Bucket->Key, Key));
    
    ValueType *Result = &Bucket->Value;
    return Result;
}

template <typename KeyType, typename ValueType>
tyler_function constexpr void
HashTableAdd(hash_table<KeyType, ValueType> *Table, KeyType Key, ValueType Value){
    ValueType *V = HashTableAlloc(Table, Key);
    *V = Value;
}

template <typename KeyType, typename ValueType>
tyler_function constexpr ValueType *
HashTableFindPtr(hash_table<KeyType, ValueType> *Table, KeyType Key){
    auto *Bucket = HashTableGetBucket(Table, Key, false);
    
    if(!Bucket) return 0;
    return &Bucket->Value;
}

template <typename KeyType, typename ValueType>
tyler_function constexpr ValueType
HashTableFind(hash_table<KeyType, ValueType> *Table, KeyType Key, b8 *Found=0){
    ValueType *ResultPtr = HashTableFindPtr(Table, Key);
    if(ResultPtr){
        if(Found) *Found = true;
        return *ResultPtr;
    }
    return {};
}

// TODO(Tyler): This could be way more efficient
template <typename KeyType, typename ValueType>
tyler_function constexpr ValueType *
HashTableGetPtr(hash_table<KeyType, ValueType> *Table, KeyType Key){
    auto *Bucket = HashTableGetBucket(Table, Key, true);
    Assert(Bucket);
    
    ValueType *Result = &Bucket->Value;
    return(Result);
}

template <typename KeyType, typename ValueType>
tyler_function constexpr ValueType
HashTableGet(hash_table<KeyType, ValueType> *Table, KeyType Key){
    ValueType Result = *HashTableGetPtr(Table, Key);
    return(Result);
}

#define HASH_TABLE_FOR_EACH_BUCKET_(Bucket, Index, Count, Table) \
for(u32 Index=0, Count=0; Index<(Table)->MaxBucketCount; Index++) \
for(auto *BucketPtr=&(Table)->Buckets[Index], *Keep_=BucketPtr; BucketPtr && BucketPtr->Hash; BucketPtr=BucketPtr->Next,Keep_=BucketPtr) \
for(auto &Bucket = *BucketPtr; Keep_; Count++, Keep_=0)
#define HASH_TABLE_FOR_EACH_BUCKET(Bucket, Table) HASH_TABLE_FOR_EACH_BUCKET_(Bucket, I_, Count_, Table)

#endif //TYLER_BASICS_IMPLEMENTATION

#endif //TYLER_BASICS_H
