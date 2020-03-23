#if !defined(SNAIL_JUMPY_INTRINSICS_H)
#define SNAIL_JUMPY_INTRINSICS_H

typedef struct _bit_scan_result bit_scan_result;
struct _bit_scan_result
{
    b32 Found;
    u32 Index;
};

#if defined(_MSC_VER) && !defined(__clang__)
#include <intrin.h>
#pragma intrinsic(_BitScanForward)

internal inline bit_scan_result
ScanForLeastSignificantSetBit(u32 Mask)
{
    bit_scan_result Result;
    Result.Found = _BitScanForward64(&(DWORD)Result.Index, Mask);
    return(Result);
}

#elif defined(__clang__)
#include <intrin.h>

internal inline bit_scan_result
ScanForLeastSignificantSetBit(u32 Mask)
{
    bit_scan_result Result;
    Result.Found = _BitScanForward64((unsigned long *)&Result.Index, Mask);
    return(Result);
}

#else
#error Please implement intrinsics for this compiler!
#endif

#endif // SNAIL_JUMPY_INTRINSICS_H
