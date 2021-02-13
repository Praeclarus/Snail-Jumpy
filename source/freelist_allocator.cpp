// TODO(Tyler): Maybe this should be more similar to a pool??

struct freelist_node {
    freelist_node *Previous;
    freelist_node *Next;
    u32 FreeUnits;
};

struct freelist_allocator {
    freelist_node *Head;
    memory_arena *Arena;
    u32 UnitSize;
};

internal void
FreelistInitialize(freelist_allocator *Allocator, memory_arena *Arena, u32 InitialUnits, u32 UnitSize){
    *Allocator = {};
    Allocator->Arena = Arena;
    Allocator->UnitSize = Maximum(UnitSize, sizeof(freelist_node));
    Allocator->Head = (freelist_node *)PushMemory(Arena, InitialUnits*Allocator->UnitSize);
    Allocator->Head->Previous = Allocator->Head;
    Allocator->Head->Next = 0;
    Allocator->Head->FreeUnits = InitialUnits;
}

internal void *
FreelistAlloc(freelist_allocator *Allocator, u32 Units){
    void *Result = 0;
    freelist_node *Node = Allocator->Head;
    while(Node){
        if(Node->FreeUnits >= Units){
            Result = (void *)Node;
            
            break;
        }
        Node = Node->Next;
    }
    return(Result);
}

internal void
FreelistFree(freelist_allocator *Allocator, void *Data, u32 Units){
    freelist_node *NewNode = (freelist_node *)Data;
    NewNode->Previous = NewNode;
    Allocator->Head->Previous = NewNode;
    NewNode->Next = Allocator->Head;
    NewNode->FreeUnits = Units;
    Allocator->Head = NewNode;
}