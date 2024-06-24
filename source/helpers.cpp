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

internal inline v2 
DirectionToV2(direction Direction){
    local_constant f32 SQRT_2 = SquareRoot(2.0f);
    switch(Direction){
        case Direction_North:     return V2( 0,    1);
        case Direction_Northeast: return V2( 0.5,  0.5)*SQRT_2;
        case Direction_East:      return V2( 1,    0);
        case Direction_Southeast: return V2( 0.5, -0.5)*SQRT_2;
        case Direction_South:     return V2( 0,   -1);
        case Direction_Southwest: return V2(-0.5, -0.5)*SQRT_2;
        case Direction_West:      return V2(-1,    0);
        case Direction_Northwest: return V2(-0.5,  0.5)*SQRT_2;
        
    }
    INVALID_CODE_PATH;
    return V2(0);
}

internal inline direction
V2ToDirection(v2 V){
    f32 Greatest = F32_NEGATIVE_INFINITY;
    direction Result = Direction_None;
    
#define V2_TO_DIRECTION_(Direction) \
if(V2Dot(V, DirectionToV2(Direction)) > Greatest){ \
Greatest = V2Dot(V, DirectionToV2(Direction));     \
Result = Direction;                                \
}
    if(0){}
    V2_TO_DIRECTION_(Direction_North)
        V2_TO_DIRECTION_(Direction_Northeast)
        V2_TO_DIRECTION_(Direction_East)
        V2_TO_DIRECTION_(Direction_Southeast)
        V2_TO_DIRECTION_(Direction_South)
        V2_TO_DIRECTION_(Direction_Southwest)
        V2_TO_DIRECTION_(Direction_West)
        V2_TO_DIRECTION_(Direction_Northwest)
        
#undef V2_TO_DIRECTION_
    
    
    Assert(Greatest > F32_NEGATIVE_INFINITY);
    return Result;
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

#define RANDOM_SEED ((u32)(341234*Counter)+(u32)(HashKey(__FILE__) * __LINE__))
#define RANDOM_SEED_(Seed) ((u32)(341234*(Counter+Seed))+(u32)(HashKey(__FILE__) * __LINE__))

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