
internal inline render_texture
MakeAndUploadTexture(u8 *Pixels, u32 Width, u32 Height, u32 Channels){
    render_texture Result = MakeTexture();
    TextureUpload(Result, Pixels, Width, Height, Channels);
    return Result;
}

template <typename T> internal inline void 
AssetProcessorAssign(T **Attribute, u8 *Data){
    *Attribute = (T *)Data;
}

internal inline asset_font_glyph
MakeAssetFontGlyph(v2s Offset, s32 Width){
    asset_font_glyph Result = {};
    Result.Offset = Offset;
    Result.Width = Width;
    return Result;
}
