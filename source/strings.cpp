
//~ Strings
struct string {
 u64 ID;
};

internal inline constexpr b8
operator==(string A, string B){
 b8 Result = (A.ID == B.ID);
 return(Result);
}

internal inline string
String(u64 ID){
 string Result = {ID};
 return(Result);
}

internal constexpr u64
HashKey(string Value) {
 u64 Result = Value.ID;
 return(Result);
}

internal constexpr b32
CompareKeys(string A, string B){
 b32 Result = (A == B);
 return(Result);
}

// String manager
struct string_manager {
 memory_arena Memory;
 hash_table<const char *, const char *> Table;
 
 void Initialize(memory_arena *Arena);
 
 string GetString(const char *String);
 const char *GetString(string String);
 char *MakeBuffer(u32 Size=DEFAULT_BUFFER_SIZE);
 template<typename T> T *GetInHashTablePtr(hash_table<string, T> *Table, const char *Key);
 template<typename T> T *FindInHashTablePtr(hash_table<string, T> *Table, const char *Key);
};

void
string_manager::Initialize(memory_arena *Arena){
 Memory = MakeArena(Arena, Kilobytes(32));
 Table = PushHashTable<const char *, const char *>(Arena, 512);
}

string
string_manager::GetString(const char *String){
 const char *ResultString = FindInHashTable(&Table, String);
 if(!ResultString){
  ResultString = ArenaPushCString(&Memory, String);
  InsertIntoHashTable(&Table, String, ResultString);
 }
 string Result;
 Result.ID = (u64)ResultString;
 return(Result);
}

const char *
string_manager::GetString(string String){
 const char *Result = (const char *)String.ID;
 
 return(Result);
}

char *
string_manager::MakeBuffer(u32 Size){
 char *Result = PushArray(&Memory, char, Size);
 return(Result);
}

template<typename T> T *
string_manager::GetInHashTablePtr(hash_table<string, T> *Table, const char *Key){
 string String = GetString(Key);
 T *Result = ::FindInHashTablePtr(Table, String);
 if(!Result){
  Result = CreateInHashTablePtr(Table, String);
 }
 return(Result);
}

template<typename T> T *
string_manager::FindInHashTablePtr(hash_table<string, T> *Table, const char *Key){
 string String = GetString(Key);
 T *Result = ::FindInHashTablePtr(Table, String);
 return(Result);
}
