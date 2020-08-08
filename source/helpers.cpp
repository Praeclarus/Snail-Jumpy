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
