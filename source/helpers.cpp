global f32 Counter;

internal u32
CStringLength(const char *String){
    u32 Result = 0;
    for(char C = *String; C; C = *(++String)){
        Result++;
    }
    return(Result);
}

internal void
CopyCString(char *To, const char *From, u32 MaxSize){
    u32 I = 0;
    while(From[I] && (I < MaxSize-1)){
        To[I] = From[I];
        I++;
    }
    To[I] = '\0';
}

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
