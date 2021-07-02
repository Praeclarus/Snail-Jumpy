
//~ Types
global_constant u32 FONT_GLYPH_COUNT = 95;

struct font {
 stbtt_bakedchar CharData[FONT_GLYPH_COUNT];
 render_texture Texture;
 u32 TextureWidth, TextureHeight;
 f32 Size, Ascent, Descent;
};

struct font_system {
 // TODO(Tyler): I am unsure about using strings for font names
 hash_table<string, font> Fonts;
 
 void Initialize(memory_arena *Arena);
 
 void  LoadFont(string Name, const char *FontPath, f32 Size);
 void  ResizeFont(string Name, const char *FontPath, f32 Size);
 font *FindFont(string Name);
};

void
font_system::Initialize(memory_arena *Arena){
 Fonts = PushHashTable<string, font>(Arena, 128);
}

//~ Normal fonts

void
font_system::LoadFont(string Name, const char *FontPath, f32 Size){
 font *Font = FindOrCreateInHashTablePtr(&Fonts, Name);
 // TODO(Tyler): This is a totally random way to try and calculate a texture to fit all letters
 const u32 Height = ((u32)Size+1)*30;
 const u32 Width = Height;
 os_file *File = OpenFile(FontPath, OpenFile_Read);
 u64 FileSize = GetFileSize(File);
 u8 *FileData = PushArray(&TransientStorageArena, u8, FileSize);
 ReadFile(File, 0, FileData, FileSize);
 CloseFile(File);
 
 u8 *Bitmap;
 BEGIN_TIMED_BLOCK(PushMemory);
 Bitmap = PushSpecialArray(&TransientStorageArena, u8, Width*Height, NoneAndAlign(4));
 END_TIMED_BLOCK();
 
 f32 Ascent, Descent, LineGap;
 stbtt_GetScaledFontVMetrics(FileData, 0, Size, &Ascent, &Descent, &LineGap);
 BEGIN_TIMED_BLOCK(FontLoading);
 stbtt_BakeFontBitmap(FileData, 0, Size, Bitmap, Width, Height, 32, FONT_GLYPH_COUNT, Font->CharData);
 END_TIMED_BLOCK();
 
 Font->Texture = MakeTexture(TextureFlag_Blend);
 TextureUpload(Font->Texture, Bitmap, Width, Height, 1);
 Font->TextureWidth = Width;
 Font->TextureHeight = Height;
 Font->Size = Size;
 Font->Ascent = Ascent;
 Font->Descent = Descent;
}

void
font_system::ResizeFont(string Name, const char *FontPath, f32 Size){
 font *Font = FindInHashTablePtr(&Fonts, Name);
 Assert(Font);
 
 // TODO(Tyler): This is a totally random way to try and calculate a texture to fit all letters
 const u32 Width = ((u32)Size+1)*FONT_GLYPH_COUNT;
 const u32 Height = (u32)(Size+1);
 os_file *File = OpenFile(FontPath, OpenFile_Read);
 u64 FileSize = GetFileSize(File);
 u8 *FileData = PushArray(&TransientStorageArena, u8, FileSize);
 ReadFile(File, 0, FileData, FileSize);
 CloseFile(File);
 
 u8 *Bitmap;
 BEGIN_TIMED_BLOCK(PushMemory);
 Bitmap = PushSpecialArray(&TransientStorageArena, u8, Width*Height, NoneAndAlign(4));
 END_TIMED_BLOCK();
 
 f32 Ascent, Descent, LineGap;
 BEGIN_TIMED_BLOCK(FontLoading);
 stbtt_GetScaledFontVMetrics(FileData, 0, Size, &Ascent, &Descent, &LineGap);
 stbtt_BakeFontBitmap(FileData, 0, Size, Bitmap, Width, Height, 32, FONT_GLYPH_COUNT, Font->CharData);
 END_TIMED_BLOCK();
 
#if 0
 BEGIN_TIMED_BLOCK(BitmapToPixels);
 u32 TextureSize = Width*Height;
 for(u32 I=0; I<TextureSize; I+=16){
  u64 Value0 = *(u64 *)(Bitmap+I);
  Pixels[I+7] = (u32)(Value0>>32) | 0x00FFFFFF;
  Pixels[I+6] = (u32)(Value0>>24) | 0x00FFFFFF;
  Pixels[I+5] = (u32)(Value0>>16) | 0x00FFFFFF;
  Pixels[I+4] = (u32)(Value0>> 8) | 0x00FFFFFF;
  Pixels[I+3] = (u32)(Value0)     | 0x00FFFFFF;
  Pixels[I+2] = (u32)(Value0<< 8) | 0x00FFFFFF;
  Pixels[I+1] = (u32)(Value0<<16) | 0x00FFFFFF;
  Pixels[I  ] = (u32)(Value0<<24) | 0x00FFFFFF;
  u64 Value1 = *(u64 *)(Bitmap+8+I);
  Pixels[I+15] = (u32)(Value1>>32) | 0x00FFFFFF;
  Pixels[I+14] = (u32)(Value1>>24) | 0x00FFFFFF;
  Pixels[I+13] = (u32)(Value1>>16) | 0x00FFFFFF;
  Pixels[I+12] = (u32)(Value1>> 8) | 0x00FFFFFF;
  Pixels[I+11] = (u32)(Value1)     | 0x00FFFFFF;
  Pixels[I+10] = (u32)(Value1<< 8) | 0x00FFFFFF;
  Pixels[I+ 9] = (u32)(Value1<<16) | 0x00FFFFFF;
  Pixels[I+ 8] = (u32)(Value1<<24) | 0x00FFFFFF;
  
 }
 
 for(u32 I=TextureSize/16; I<TextureSize%16; I++){
  Pixels[I] = (Bitmap[I]<<24)+0x00FFFFFF;
 }
 END_TIMED_BLOCK();
#endif
 
 TextureUpload(Font->Texture, Bitmap, Width, Height, 1);
 Font->TextureWidth = Width;
 Font->TextureHeight = Height;
 Font->Size = Size;
 Font->Ascent = Ascent;
 Font->Descent = Descent;
}

font *
font_system::FindFont(string Name){
 font *Result = FindInHashTablePtr(&Fonts, Name);
 return(Result);
}
