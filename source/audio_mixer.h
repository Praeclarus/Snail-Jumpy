#ifndef AUDIO_MIXER_H
#define AUDIO_MIXER_H

typedef u8 mixer_sound_flags;
enum mixer_sound_flags_ {
    MixerSoundFlag_None  = (0 << 0),
    MixerSoundFlag_Loop  = (1 << 0),
    MixerSoundFlag_Music = (1 << 1),
};

typedef u64 sound_id;
struct mixer_sound {
    mixer_sound_flags Flags;
    sound_data *Data;
    f32 SamplesPlayed;
    f32 Speed;
    
    f32 Volume0;
    f32 Volume1;
    
    union {
        mixer_sound *Next;
        mixer_sound *NextFree;
    };
    mixer_sound *Prev;
    sound_id ID;
};

struct sound_handle {
    mixer_sound *Sound;
    sound_id ID;
};

struct audio_mixer {
    sound_id CurrentID;
    
    memory_arena SoundMemory;
    
    mixer_sound FirstSound;
    mixer_sound *FirstFreeSound;
    
    ticket_mutex FreeSoundMutex;
    ticket_mutex SoundMutex;
    
    v2 MusicMasterVolume;
    v2 SoundEffectMasterVolume;
    
    sound_handle PlaySound(asset_sound_effect *Asset, mixer_sound_flags Flags=MixerSoundFlag_None, 
                           f32 PlaybackSpeed=1.0f, f32 Volume1=1.0f, f32 Volume2=1.0f);
    void StopSound(sound_handle Handle);
    
    //~
    void Initialize(memory_arena *Arena);
    void OutputSamples(memory_arena *WorkingMemory, os_sound_buffer *SoundBuffer);
};

#endif //AUDIO_MIXER_H
