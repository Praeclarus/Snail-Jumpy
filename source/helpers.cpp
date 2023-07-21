global f32 Counter;

internal direction
InverseDirection(direction Direction){
    local_constant direction Table[Direction_TOTAL] = {
        Direction_None,
        Direction_South,
        Direction_Southwest,
        Direction_West,
        Direction_Northwest,
        Direction_North,
        Direction_Northeast,
        Direction_East,
        Direction_Southeast,
    };
    direction Result = Table[Direction];
    return(Result);
}

internal inline u32
GetRandomNumberJustSeed(u32 Seed){
    u32 RandomNumber = RANDOM_NUMBER_TABLE[(Seed * 3124 + 3809) % ArrayCount(RANDOM_NUMBER_TABLE)];
    return(RandomNumber);
}

internal inline u32
GetRandomNumber(u32 Seed){
    u32 RandomNumber = RANDOM_NUMBER_TABLE[(u32)(Counter*4132.0f + Seed) % ArrayCount(RANDOM_NUMBER_TABLE)];
    return(RandomNumber);
}

internal inline f32
GetRandomFloat(u32 Seed, u32 Spread=5, f32 Power=0.2f){
    s32 Random = ((s32)GetRandomNumber(Seed)) % Spread;
    f32 Result = Power * (f32)Random;
    return(Result);
}

internal inline b8 
StopSeeking(char C){
    b8 Result = (!IsALetter(C) &&
                 !IsANumber(C));
    return Result;
}

internal inline range_s32
SeekForward(const char *Buffer, u32 BufferLength, u32 Start){
    range_s32 Result = MakeRangeS32(Start, BufferLength);
    b8 HitAlphabetic = false;
    for(u32 I=Start; I<=BufferLength; I++){
        char C = Buffer[I];
        Result.End = I;
        if(StopSeeking(C)){
            if(HitAlphabetic) break;
        }else HitAlphabetic = true;
    }
    
    return Result;
}

internal inline range_s32
SeekBackward(const char *Buffer, s32 End){
    range_s32 Result = MakeRangeS32(0, End);
    if(End == 0) return Result;
    b8 HitAlphabetic = false;
    for(s32 I=Result.End-1; I>=0; I--){
        char C = Buffer[I];
        if(StopSeeking(C)){
            if(HitAlphabetic) break;
        }else HitAlphabetic = true;
        Result.Start = I;
    }
    
    return Result;
}

internal inline u32
CountWordMatchCount(const char *A, const char *B){
    u32 Result = 0;
    while(*A && *B){
        if(*A == ' ') { A++; continue; }
        if(*B == ' ') { B++; continue; }
        if(*A != *B){
            if(CharToLower(*A) != CharToLower(*B)) return Result;
        }
        A++;
        B++;
        Result++;
    }
    
    return Result;
}