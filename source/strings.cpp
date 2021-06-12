
struct string_manager {
    memory_arena Memory;
    hash_table<const char *, const char *> Table;
    
    void Initialize(memory_arena *Arena);
    const char *GetString(const char *String);
    char *MakeBuffer(u32 Size=DEFAULT_BUFFER_SIZE);
    
    template<typename T> T * 
        GetInHashTablePtr(hash_table<const char *, T> *Table, const char *Key);
};

void
string_manager::Initialize(memory_arena *Arena){
    Memory = PushNewArena(Arena, Kilobytes(32));
    Table = PushHashTable<const char *, const char *>(Arena, 512);
}

const char *
string_manager::GetString(const char *String){
    const char *Result = FindInHashTable(&Table, String);
    if(!Result){
        Result = PushCString(&Memory, String);
        InsertIntoHashTable(&Table, String, Result);
    }
    return(Result);
}

char *
string_manager::MakeBuffer(u32 Size){
    char *Result = PushArray(&Memory, char, Size);
    return(Result);
}

template<typename T> 
T *
string_manager::GetInHashTablePtr(hash_table<const char *, T> *Table, const char *Key){
    T *Result = FindInHashTablePtr(Table, Key);
    if(!Result){
        const char *String = GetString(Key);
        Result = CreateInHashTablePtr(Table, String);
    }
    return(Result);
}