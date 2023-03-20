
void 
audio_mixer::Initialize(memory_arena *Arena){
    SoundMemory = MakeArena(Arena, Kilobytes(16));
    MusicMasterVolume = SoundEffectMasterVolume = V2(1);
    DLIST_INIT(&FirstSound);
}

sound_handle
audio_mixer::PlaySound(asset_sound_effect *Asset, mixer_sound_flags Flags, f32 PlaybackSpeed, f32 Volume0, f32 Volume1){
    sound_handle Handle = {};
    if(!Asset) return Handle;
    
    TicketMutexBegin(&FreeSoundMutex);
    
#if 0
    if(!FirstFreeSound){
        FirstFreeSound = ;
    }
    mixer_sound *Sound = FirstFreeSound;
    FirstFreeSound = FirstFreeSound->Next;
#endif
    mixer_sound *Sound = FREELIST_ALLOC(FirstFreeSound, ArenaPushType(&SoundMemory, mixer_sound));
    
    sound_data *Data = &Asset->Sound;
    
    *Sound = {};
    
    Sound->Speed = PlaybackSpeed;
    Sound->Flags = Flags;
    Sound->Volume0 = Volume0*Asset->VolumeMultiplier;
    Sound->Volume1 = Volume1*Asset->VolumeMultiplier;
    
    DLIST_ADD(&FirstSound, Sound);
    
    Sound->Data = Data;
    
    Sound->ID = ++CurrentID;
    
    Handle.Sound = Sound;
    Handle.ID = Sound->ID;
    
    TicketMutexEnd(&FreeSoundMutex);
    
    return Handle;
}

void
audio_mixer::StopSound(sound_handle Handle){
    if(Handle.Sound->ID != Handle.ID) return;
    TicketMutexBegin(&SoundMutex);
    
    DLIST_REMOVE(Handle.Sound);
    FREELIST_FREE(FirstFreeSound, Handle.Sound);
    
    TicketMutexEnd(&SoundMutex);
}

//~ NOTE(Tyler): THIS RUNS IN A SEPARATE THREAD!
void
audio_mixer::OutputSamples(memory_arena *WorkingMemory, os_sound_buffer *SoundBuffer){
    //TIMED_FUNCTION();
    
    const __m128 SignMask = _mm_castsi128_ps(_mm_set1_epi32(0x80000000));
    const __m128 One = _mm_set1_ps(1.0f);
    const __m128 Half = _mm_set1_ps(0.49999f); // With epsilon
    
    ZeroMemory(SoundBuffer->Samples, SoundBuffer->SamplesToWrite*2*sizeof(*SoundBuffer->Samples));
    u32 MaxChunksToWrite = SoundBuffer->SamplesToWrite / 4;
    Assert(!(SoundBuffer->SamplesToWrite % 4));
    
    memory_arena_marker Marker = ArenaBeginMarker(WorkingMemory);
    __m128 *OutputChannel0 = ArenaPushArraySpecial(WorkingMemory, __m128, MaxChunksToWrite, ZeroAndAlign(16));
    __m128 *OutputChannel1 = ArenaPushArraySpecial(WorkingMemory, __m128, MaxChunksToWrite, ZeroAndAlign(16));
    
    TicketMutexBegin(&SoundMutex);
    mixer_sound *Sound = FirstSound.Next;
    while(Sound != &FirstSound){
        sound_data *SoundData = Sound->Data;
        Assert(SoundData->ChannelCount == 2);
        
        u32 RemainingSamples = SoundData->SampleCount-RoundToS32(Sound->SamplesPlayed);
        u32 RemainingChunks = RemainingSamples / 4;
        if(Sound->Flags & MixerSoundFlag_Loop){
            RemainingChunks = MaxChunksToWrite;
        }
        
        u32 ChunksToWrite = Minimum(MaxChunksToWrite, RemainingChunks);
        __m128 Volume0 = _mm_set1_ps(Sound->Volume0);
        __m128 Volume1 = _mm_set1_ps(Sound->Volume1);
        
        v2 MasterVolume = (Sound->Flags & MixerSoundFlag_Music) ? MusicMasterVolume : SoundEffectMasterVolume;
        
        __m128 MasterVolume0 = _mm_set1_ps(MasterVolume.E[0]);
        __m128 MasterVolume1 = _mm_set1_ps(MasterVolume.E[1]);
        {
            
        }
        f32 dSample = Sound->Speed*SoundData->BaseSpeed;
        __m128 dSampleM128 = _mm_set1_ps(4*dSample);
        __m128 SampleP = _mm_setr_ps(Sound->SamplesPlayed + 0.0f*dSample, 
                                     Sound->SamplesPlayed + 1.0f*dSample, 
                                     Sound->SamplesPlayed + 2.0f*dSample, 
                                     Sound->SamplesPlayed + 3.0f*dSample);
        
        s16 *Samples = SoundData->Samples;
        u32 TotalSampleCount = SoundData->ChannelCount*SoundData->SampleCount;
        
        __m128 *Dest0 = OutputChannel0;
        __m128 *Dest1 = OutputChannel1;
        for(u32 I=0; I < ChunksToWrite; I++){
            
            __m128 D0 = _mm_load_ps((float*)Dest0);
            __m128 D1 = _mm_load_ps((float*)Dest1);
            
            __m128i SampleIndex = _mm_cvtps_epi32(_mm_sub_ps(SampleP, Half));
            __m128 Fraction = _mm_sub_ps(SampleP, _mm_cvtepi32_ps(SampleIndex));
            
            // TODO(Tyler): It would work to save this and use it for the next iteration of the loop
            // (NextSampleValueA & NextSampleValueB)
            __m128 SampleValue0 = _mm_setr_ps(Samples[(((u32 *)&SampleIndex)[0]*2)     % TotalSampleCount],
                                              Samples[(((u32 *)&SampleIndex)[1]*2)     % TotalSampleCount],
                                              Samples[(((u32 *)&SampleIndex)[2]*2)     % TotalSampleCount],
                                              Samples[(((u32 *)&SampleIndex)[3]*2)     % TotalSampleCount]);
            __m128 SampleValue1 = _mm_setr_ps(Samples[(((u32 *)&SampleIndex)[0]*2 + 1) % TotalSampleCount],
                                              Samples[(((u32 *)&SampleIndex)[1]*2 + 1) % TotalSampleCount],
                                              Samples[(((u32 *)&SampleIndex)[2]*2 + 1) % TotalSampleCount],
                                              Samples[(((u32 *)&SampleIndex)[3]*2 + 1) % TotalSampleCount]);
            
            __m128 NextSampleValue0 = _mm_setr_ps(Samples[(((u32 *)&SampleIndex)[0]*2 + 2) % TotalSampleCount],
                                                  Samples[(((u32 *)&SampleIndex)[1]*2 + 2) % TotalSampleCount],
                                                  Samples[(((u32 *)&SampleIndex)[2]*2 + 2) % TotalSampleCount],
                                                  Samples[(((u32 *)&SampleIndex)[3]*2 + 2) % TotalSampleCount]);
            __m128 NextSampleValue1 = _mm_setr_ps(Samples[(((u32 *)&SampleIndex)[0]*2 + 3) % TotalSampleCount],
                                                  Samples[(((u32 *)&SampleIndex)[1]*2 + 3) % TotalSampleCount],
                                                  Samples[(((u32 *)&SampleIndex)[2]*2 + 3) % TotalSampleCount],
                                                  Samples[(((u32 *)&SampleIndex)[3]*2 + 3) % TotalSampleCount]);
            
            __m128 FinalSampleValue0 = _mm_add_ps(_mm_mul_ps(_mm_sub_ps(One, Fraction), SampleValue0), 
                                                  _mm_mul_ps(Fraction, NextSampleValue0));
            __m128 FinalSampleValue1 = _mm_add_ps(_mm_mul_ps(_mm_sub_ps(One, Fraction), SampleValue1), 
                                                  _mm_mul_ps(Fraction, NextSampleValue1));
            
            D0 = _mm_add_ps(D0, _mm_mul_ps(MasterVolume0, _mm_mul_ps(Volume0, FinalSampleValue0)));
            D1 = _mm_add_ps(D1, _mm_mul_ps(MasterVolume1, _mm_mul_ps(Volume1, FinalSampleValue1)));
            
            _mm_store_ps((float*)Dest0, D0);
            _mm_store_ps((float*)Dest1, D1);
            
            Dest0++;
            Dest1++;
            
            SampleP = _mm_add_ps(SampleP, dSampleM128);
        }
        
        Sound->SamplesPlayed += dSample*(f32)SoundBuffer->SamplesPerFrame;
        if((Sound->SamplesPlayed > SoundData->SampleCount) &&
           !(Sound->Flags & MixerSoundFlag_Loop)){
            
            TicketMutexBegin(&FreeSoundMutex);
            
            mixer_sound *Next = Sound->Next;
            DLIST_REMOVE(Sound);
            FREELIST_FREE(FirstFreeSound, Sound);
            Sound = Next;
            
            TicketMutexEnd(&FreeSoundMutex);
        }else{
            if(Sound->SamplesPlayed > SoundData->SampleCount){
                Sound->SamplesPlayed = 0;
            }
            Sound = Sound->Next;
        }
    }
    TicketMutexEnd(&SoundMutex);
    
    __m128 *Source0 = OutputChannel0;
    __m128 *Source1 = OutputChannel1;
    
    __m128i *SampleOut = (__m128i *)SoundBuffer->Samples;
    for(u32 I=0; I < MaxChunksToWrite; I++){
        __m128 S0 = _mm_load_ps((float *)Source0++);
        __m128 S1 = _mm_load_ps((float *)Source1++);
        
        __m128i L = _mm_cvtps_epi32(S0);
        __m128i R = _mm_cvtps_epi32(S1);
        
        __m128i LR0 = _mm_unpacklo_epi32(L, R);
        __m128i LR1 = _mm_unpackhi_epi32(L, R);
        
        __m128i S01 = _mm_packs_epi32(LR0, LR1);
        
        *SampleOut++ = S01;
    }
    
    ArenaEndMarker(WorkingMemory, &Marker);
}