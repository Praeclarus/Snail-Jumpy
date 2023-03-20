#ifndef GAME_H
#define GAME_H

enum game_mode {
    GameMode_None,
    GameMode_Debug,
    GameMode_MainGame,
    GameMode_Menu,
    GameMode_WorldEditor,
};
struct state_change_data {
    b8 DidChange;
    game_mode NewMode;
    const char *NewLevel;
};

struct settings_state {
    sound_handle MusicHandle;
    
    os_key_code PlayerJump  = KeyCode_Space;
    os_key_code PlayerShoot = (os_key_code)'X';
    os_key_code PlayerLeft  = KeyCode_Left;
    os_key_code PlayerRight = KeyCode_Right;
};

#endif //GAME_H
