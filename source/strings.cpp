
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
MakeString(u64 ID){
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

union string_buffer_node {
 string_buffer_node *Next;
 char Buffer[DEFAULT_BUFFER_SIZE];
};

// String manager
struct string_manager {
 memory_arena StringMemory;
 hash_table<const char *, const char *> Table;
 
 memory_arena BufferMemory;
 string_buffer_node *NextBuffer;
 
 void Initialize(memory_arena *Arena);
 
 string GetString(const char *String);
 const char *GetString(string String);
 char *MakeBuffer();
 void  RemoveBuffer(char *Buffer);
 template<typename T> T *GetInHashTablePtr(hash_table<string, T> *Table, const char *Key);
 template<typename T> T *FindInHashTablePtr(hash_table<string, T> *Table, const char *Key);
};

void
string_manager::Initialize(memory_arena *Arena){
 StringMemory = MakeArena(Arena, Kilobytes(32));
 Table = PushHashTable<const char *, const char *>(Arena, 512);
 
 u32 BufferCount = 128;
 //u32 BufferCount = 2;
 BufferMemory = MakeArena(Arena, sizeof(string_buffer_node)*BufferCount);
 
 NextBuffer = 0;
 for(u32 I=0; I<BufferCount; I++){
  string_buffer_node *Node = PushStruct(&BufferMemory, string_buffer_node);
  Node->Next = NextBuffer;
  NextBuffer = Node;
 }
}

string
string_manager::GetString(const char *String){
 const char *ResultString = FindInHashTable(&Table, String);
 if(!ResultString){
  ResultString = ArenaPushCString(&StringMemory, String);
  InsertIntoHashTable(&Table, ResultString, ResultString);
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
string_manager::MakeBuffer(){
 Assert(NextBuffer);
 char *Result = NextBuffer->Buffer;
 NextBuffer = NextBuffer->Next;
 ZeroMemory(Result, DEFAULT_BUFFER_SIZE);
 return(Result);
}

void
string_manager::RemoveBuffer(char *Buffer){
 u8 *MemoryMin = BufferMemory.Memory;
 u8 *MemoryMax = BufferMemory.Memory+BufferMemory.Used;
 Assert((MemoryMin <= (u8 *)Buffer) && ((u8 *)Buffer < MemoryMax));
 string_buffer_node *Node = (string_buffer_node *)Buffer;
 Node->Next = NextBuffer;
 NextBuffer = Node;
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


