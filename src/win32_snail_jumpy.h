#if !defined(WIN32_SNAIL_JUMPY_H)
#define WIN32_SNAIL_JUMPY_H

typedef struct _win32_backbuffer win32_backbuffer;
struct _win32_backbuffer {
    BITMAPINFO Info;
    void *Memory;
    s32 Width, Height;
    s32 Pitch;
    s32 BytesPerPixel;
};

#endif
