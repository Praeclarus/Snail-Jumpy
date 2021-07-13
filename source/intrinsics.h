#if !defined(SNAIL_JUMPY_INTRINSICS_H)
#define SNAIL_JUMPY_INTRINSICS_H
struct bit_scan_result
{
 u32 Index;
 b8 Found;
};

#if defined(_MSC_VER) && !defined(__clang__)
#include <intrin.h>
#pragma intrinsic(_BitScanForward)

internal inline bit_scan_result
ScanForLeastSignificantSetBit(u64 Mask){
 bit_scan_result Result;
 Result.Found = _BitScanForward64(&(unsigned long)Result.Index, Mask);
 return(Result);
}

internal inline bit_scan_result
ScanForMostSignificantSetBit(u64 Mask){
 bit_scan_result Result;
 Result.Found = _BitScanReverse64(&(unsigned long)Result.Index, Mask);
 return(Result);
}

internal inline u32
CountLeadingZeroes(u32 Value){
 u32 Result = (u32)__lzcnt(Value);
 return(Result);
}

internal inline u32
CountLeadingOnes(u32 Value){
 u32 Result = (u32)__lzcnt(~Value);
 return(Result);
}

internal void
CopyMemory(const void *To, const void *From, umw Size) {
#if 0
 for (umw I = 0; I < Size; I++)
 {
  *((u8*)To+I) = *((u8*)From+I);
 }
#else
 __movsb((u8 *)To, (u8 *)From, Size);
#endif
}

internal void
ZeroMemory(void *Memory, umw Size) {
#if 0
 for (umw I = 0; I < Size; I++){
  *((u8*)Memory+I) = 0;
 }
#else
 __stosb((u8 *)Memory, 0, Size);
#endif
}

internal u32 
PopcountU32(u32 A){
 u32 Result = _mm_popcnt_u32(A);
 return(Result);
}

#else 
#error Please implement intrinsics for this compiler!
#endif

#endif // SNAIL_JUMPY_INTRINSICS_H
