#ifndef SNAIL_JUMPY_UI_H
#define SNAIL_JUMPY_UI_H

struct font {
    stbtt_bakedchar CharData[93];
    render_texture_handle Texture;
    u32 TextureWidth, TextureHeight;
    f32 Size, Ascent, Descent;
};

struct text_box_data {
    char Buffer[512];
    u32 BufferIndex;
};

enum ui_primitive_type {
    PrimitiveType_None,
    PrimitiveType_Rectangle,
    PrimitiveType_String,
};

// TODO(Tyler): I don't think the cache performance on this would be too bad, as each
// item is almost contiguous, if it proves a problem, it might be best to allocate a bunch 
// of these from a block of temporary memory
struct ui_primitive {
    ui_primitive *Next;
    ui_primitive_type Type;
    
    color Color;
    f32 Z;
    union {
        // Rectangle
        struct {
            v2 Min;
            v2 Max;
        };
        
        // String
        struct {
            font *Font;
            v2 P;
            // TODO(Tyler): This is probably not good to make this fixed size
            char String[512];
        };
    };
};

struct ui_manager {
    ui_primitive *FirstPrimitive;
    
    u32 SelectedWidgetId;
    
    // TODO(Tyler): Perhaps this should be part of the os_input structure?
    b8 HandledInput;
};

#endif //SNAIL_JUMPY_UI_H
