#if !defined(SNAIL_JUMPY_H)
#define SNAIL_JUMPY_H

#include "snail_jumpy_primitive_types.h"
#include "snail_jumpy_allocators.cpp"
#include "snail_jumpy_debug.h"
#include "snail_jumpy_types.cpp"
#include "snail_jumpy_math.h"
#include "snail_jumpy_render.h"
#include "snail_jumpy_os.h"
#include "snail_jumpy_intrinsics.h"
#include "snail_jumpy_asset.h"
#include "snail_jumpy_entity.h"
#include "snail_jumpy_random.h"
#include "snail_jumpy_ui.h"
#include "snail_jumpy_level.h"
#include "snail_jumpy_editor.h"

//~ Big game things
enum game_mode {
    GameMode_None,
    GameMode_Menu,
    GameMode_MainGame,
    GameMode_LevelEditor,
    GameMode_Overworld,
    GameMode_OverworldEditor,
};
struct state_change_data {
    b8 DidChange;
    game_mode NewMode;
    const char *NewLevel;
};

#pragma pack(push, 1)
struct overworld_file_header {
    char Header[3]; // 'S', 'J', 'O'
    u32 Version;
    u32 WidthInTiles;
    u32 HeightInTiles;
    u32 TeleporterCount;
    u32 DoorCount;
};
#pragma pack(pop)

#endif