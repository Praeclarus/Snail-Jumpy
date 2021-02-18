#ifndef SNAIL_JUMPY_PHYSICS_H
#define SNAIL_JUMPY_PHYSICS_H

//~ Debug stuff
global_constant color GJK_SIMPLEX1_COLOR = color{1.0f,  0.0f,  1.0f, 1.0f};
global_constant color GJK_SIMPLEX2_COLOR = color{0.7f,  0.0f,  0.7f, 1.0f};
global_constant color GJK_SIMPLEX3_COLOR = color{0.4f,  0.0f,  0.4f, 1.0f};

struct debug_physics_info {
    v2 Offset;
    v2 dP, ddP;
    b8 DidInitialdP;
};

typedef u32 physics_debugger_flags;
enum physics_debugger_flags_ {
    PhysicsDebuggerFlags_None = 0,
    PhysicsDebuggerFlags_StepPhysics = (1 << 0),
};

struct physics_debugger_position {
    // Arbitrary numbers to keep track of position, do not work between physics frames
    u32 Position;
    u32 Object;
};

// The debugger currently only supports single moving objects
struct physics_debugger {
    physics_debugger_flags Flags = PhysicsDebuggerFlags_StepPhysics;
    physics_debugger_position Current;
    physics_debugger_position Paused;
    layout Layout;
    f32 Scale = 0.5f;
    
    // These are so that functions don't need extra return values or arguments
    union {
        v2 Base; // Used by UpdateSimplex to know where to draw the direction from
    };
    
    inline void NewFrame();
    inline void AdvanceCurrentObject();
    inline b8   AdvanceCurrentPosition();
    inline b8   TestPosition();
    inline void BreakWhen(b8 Value); // Assert is a macro, so it can't be the name here
};

//~ Collision boundary
enum collision_boundary_type {
    BoundaryType_None,
    BoundaryType_Rect,
    BoundaryType_Circle,
    BoundaryType_Wedge,
};

typedef u32 collision_flags;
enum _collision_flags {
    CollisionFlag_None = 0,
    CollisionFlag_CanStandOn = (1 << 0),
};

typedef v2 gjk_simplex[3];

struct collision_boundary {
    collision_boundary_type Type;
    collision_flags Flags;
    v2 Offset;
    rect Bounds;
    
    union {
        // Rects just use 'rect Bounds'
        
        // Circle
        struct {
            f32 Radius;
        };
        
        // Wedges
        struct {
            // Currently the only wedges in the game are 1 tall by 1 wide
            // but a direction is needed
            v2 Points[3];
        };
    };
};

global_constant u32 MAX_BOUNDARY_CHILDREN = 3;

//~ Collision table
struct collision_table_item {
    collision_table_item *Next;
    
    entity_type EntityType;
    void *EntityPtr;
};

struct collision_table {
    u32 Width;
    u32 Height;
    collision_table_item **Items;
};

//~ Physics

struct physics_object {
    v2 P, dP, ddP;
    f32 Mass;
    collision_boundary *Boundaries;
    u8 BoundaryCount;
    
    debug_physics_info *DebugInfo;
};

struct physics_system {
    bucket_array<physics_object, 64> Objects;
    bucket_array<physics_object, 64> StaticObjects;
    //freelist_allocator BoundaryAllocator;
    memory_arena BoundaryMemory;
    collision_table SpaceTable;
    
    void Initialize(memory_arena *Arena);
    void Reload(u32 Width, u32 Height);
    void DoPhysics();
    physics_object *AddObject(collision_boundary *Boundaries, u8 BoundaryCount);
    physics_object *AddStaticObject(collision_boundary *Boundaries, u8 BoundaryCount);
    collision_boundary *AllocBoundaries(u32 Count);
};

#endif //SNAIL_JUMPY_PHYSICS_H
