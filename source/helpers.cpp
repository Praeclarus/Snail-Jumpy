//~ Helpers
internal void
CopyMemory(const void *To, const void *From, umw Size) {
    for (umw I = 0; I < Size; I++)
    {
        *((u8*)To+I) = *((u8*)From+I);
    }
}

internal void
MoveMemory(const void *To, const void *From, umw Size) {
    memmove((void *)To, (void *)From, Size);
}

internal void
ZeroMemory(void *Memory, umw Size) {
    for (umw I = 0; I < Size; I++){
        *((u8*)Memory+I) = 0;
    }
}

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
