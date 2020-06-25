#include <stdlib.h>
#include <stdio.h>

#define STB_IMAGE_IMPLEMENTATION
#include "../third_party/stb_image.h"
#define STB_RECT_PACK_IMPLEMENTATION
#include "../third_party/stb_rect_pack.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "../third_party/stb_truetype.h"
#define STB_SPRINTF_IMPLEMENTATION
#include "../third_party/stb_sprintf.h"

#include "../snail_jumpy.h"

#include "asset_processor.h"

#include "../snail_jumpy_stream.cpp"

global memory_arena Arena;
global hash_table<const char *, asset_command> CommandTable;
global hash_table<const char *, asset_spec> AssetSpecTable;
global hash_table<const char *, asset> AssetTable;
global asset *CurrentAsset;

struct entire_file {
    void *Data;
    u64 Size;
};
internal entire_file
ReadEntireFile(memory_arena *Arena_, char *Path) {
    FILE *File = fopen(Path, "rb");
    Assert(File);
    entire_file Result = {0};
    
    Assert(fseek(File, 0, SEEK_END) == 0);
    Result.Size = ftell(File);
    Assert(fseek(File, 0, SEEK_SET) == 0);
    
    Result.Data = PushMemory(Arena_, Result.Size);
    Assert(Result.Data);
    fread(Result.Data, 1, Result.Size, File);
    
    fclose(File);
    return(Result);
}

int 
main(int ArgCount, char **Args){
    InitializeAssetLoader();
    
    Assert(ArgCount > 1);
    LoadAssetFile(Args[1]);
    
    for(u32 ProfileIndex = 0;
        ProfileIndex < ProfileData.CurrentBlockIndex+1;
        ProfileIndex++){
        profiled_block *Block = &ProfileData.Blocks[ProfileIndex];
        if(Block->Name == 0){
            continue;
        }
        
        for(u32 I = 0; I < Block->Level; I++){
            printf("    ");
        }
        printf("%s: %llu\n", Block->Name, Block->CycleCount);
    }
}