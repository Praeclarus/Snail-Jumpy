#ifndef SNAIL_JUMPY_SIMD_H
#define SNAIL_JUMPY_SIMD_H

//~ Basics

#define M(a, i) ((float *)&(a))[i]


//~ f32_x4
struct f32_x4 {
 __m128 V;
};

internal inline f32_x4
ZeroF32X4(){
 f32_x4 Result;
 Result.V = _mm_setzero_ps();
 return(Result);
}

internal inline f32_x4
F32X4(f32 E0, f32 E1, f32 E2, f32 E3){
 f32_x4 Result;
 Result.V = _mm_setr_ps(E0, E1, E2, E3);
 return(Result);
}

internal inline f32_x4
F32X4(f32 EAll){
 f32_x4 Result;
 Result.V = _mm_set1_ps(EAll);
 return(Result);
}

internal inline f32 &
GetOneF32(f32_x4 V, u32 I){
 // TODO(Tyler): I don't know the "correct" way to do this
 f32 &Result = ((f32 *)&V)[I];
 return(Result);
}

internal inline f32_x4
operator+(f32_x4 A, f32_x4 B){
 f32_x4 Result;
 Result.V = _mm_add_ps(A.V, B.V);
 return(Result);
}

internal inline f32_x4
operator-(f32_x4 A, f32_x4 B){
 f32_x4 Result;
 Result.V = _mm_sub_ps(A.V, B.V);
 return(Result);
}

internal inline f32_x4
operator-(f32_x4 A){
 f32_x4 Result = ZeroF32X4() - A;
 return(Result);
}

internal inline f32_x4
operator*(f32_x4 A, f32_x4 B){
 f32_x4 Result;
 Result.V = _mm_mul_ps(A.V, B.V);
 return(Result);
}

internal inline f32_x4
operator/(f32_x4 A, f32_x4 B){
 f32_x4 Result;
 Result.V = _mm_div_ps(A.V, B.V);
 return(Result);
}

internal inline f32_x4 &
operator+=(f32_x4 &A, f32_x4 B){
 A = A + B;
 return(A);
}

internal inline f32_x4 &
operator-=(f32_x4 &A, f32_x4 B){
 A = A - B;
 return(A);
}

internal inline f32_x4 &
operator*=(f32_x4 &A, f32_x4 B){
 A = A * B;
 return(A);
}

internal inline f32_x4 &
operator/=(f32_x4 &A, f32_x4 B){
 A = A / B;
 return(A);
}

internal inline f32_x4
Square(f32_x4 A)
{
 f32_x4 Result = A*A;
 return(Result);
}

internal inline f32_x4
AbsoluteValue(f32_x4 A)
{
 u32 MaskU32 = (u32)(1 << 31);
 __m128 Mask = _mm_set1_ps(*(f32 *)&MaskU32);
 
 f32_x4 Result;
 Result.V = _mm_andnot_ps(Mask, A.V);
 return(Result);
}

//~ v2_x4
struct v2_x4 {
 f32_x4 X;
 f32_x4 Y;
};

internal inline v2_x4
V2X4(f32_x4 X, f32_x4 Y){
 v2_x4 Result;
 Result.X = X;
 Result.Y = Y;
 return(Result);
}

internal inline v2_x4
V2X4(f32_x4 XY){
 v2_x4 Result;
 Result.X = XY;
 Result.Y = XY;
 return(Result);
}

internal inline v2_x4
V2X4(v2 V){
 v2_x4 Result;
 Result.X = F32X4(V.X);
 Result.Y = F32X4(V.Y);
 return(Result);
}

internal inline v2_x4
operator+(v2_x4 A, v2_x4 B)
{
 v2_x4 Result;
 Result.X = A.X + B.X;
 Result.Y = A.Y + B.Y;
 return(Result);
}

internal inline v2_x4
operator-(v2_x4 A, v2_x4 B)
{
 v2_x4 Result;
 Result.X = A.X - B.X;
 Result.Y = A.Y - B.Y;
 return(Result);
}

internal inline v2_x4
operator-(v2_x4 A)
{
 v2_x4 Result;
 Result.X = -A.X;
 Result.Y = -A.Y;
 return(Result);
}

internal inline v2_x4
V2Invert(v2_x4 A)
{
 v2_x4 Result;
 Result.X = -A.X;
 Result.Y = -A.Y;
 return(Result);
}

internal inline v2_x4
operator*(v2_x4 A, f32_x4 B)
{
 v2_x4 Result;
 Result.X = A.X * B;
 Result.Y = A.Y * B;
 return(Result);
}

internal inline v2_x4
operator*(f32_x4 B, v2_x4 A)
{
 v2_x4 Result;
 Result.X = A.X * B;
 Result.Y = A.Y * B;
 return(Result);
}

internal inline v2_x4
operator/(v2_x4 A, f32_x4 B)
{
 v2_x4 Result;
 Result.X = A.X / B;
 Result.Y = A.Y / B;
 return(Result);
}

internal inline v2_x4
operator+=(v2_x4 &A, v2_x4 B)
{
 A = A + B;
 return(A);
}

internal inline v2_x4
operator-=(v2_x4 &A, v2_x4 B)
{
 A = A - B;
 return(A);
}

internal inline v2_x4
operator*=(v2_x4 &A, f32_x4 B)
{
 A = B * A;
 return(A);
}

internal inline v2_x4
operator/=(v2_x4 &A, f32_x4 B)
{
 A = A / B;
 return(A);
}

internal inline f32_x4
Dot(v2_x4 A, v2_x4 B) {
 f32_x4 Result = (A.X*B.X)+(A.Y*B.Y);
 return(Result);
}


#endif