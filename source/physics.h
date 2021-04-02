#ifndef SNAIL_JUMPY_PHYSICS_H
#define SNAIL_JUMPY_PHYSICS_H

//~ Debug stuff
struct debug_physics_info {
    v2 P, dP, ddP, Delta;
};

typedef u32 physics_debugger_flags;
enum physics_debugger_flags_ {
    PhysicsDebuggerFlags_None = 0,
    PhysicsDebuggerFlags_StepPhysics = (1 << 0),
};

// The debugger currently only supports single moving objects
struct physics_debugger {
    camera Camera;
    physics_debugger_flags Flags;
    // Arbitrary numbers to keep track of position, do not work between physics frames
    u32 Current;
    u32 Paused;
    layout Layout;
    f32 Scale = 3.0f;
    v2 Origin;
    b8 StartOfPhysicsFrame = true;
    
    // These are so that functions don't need extra return values or arguments
    union {
        v2 Base; // Used by UpdateSimplex to know where to draw the direction from
    };
    
    inline void Begin();
    inline void End();
    inline b8   DefineStep();
    inline void BreakWhen(b8 Value); // Assert is a macro, so it can't be the name here
    inline b8   IsCurrent();
    
    inline void DrawPolygon(v2 *Points, u32 PointCount);
    inline void DrawLine(v2 Offset, v2 A, v2 B, color Color);
    inline void DrawLineFrom(v2 Offset, v2 A, v2 Delta, color Color);
    inline void DrawNormal(v2 Offset, v2 Point, v2 Delta, color Color);
    inline void DrawPoint(v2 Offset, v2 Point, color Color);
    inline void DrawString(const  char *String, ...);
    inline void DrawStringAtP(v2 P, const char *Format, ...);
    
    inline void DrawBaseGJK(v2 AP, v2 BP, v2 Delta, v2 *Points, u32 PointCount);
};

//~ Collision boundary
enum collision_boundary_type {
    BoundaryType_None,
    BoundaryType_Rect,
    BoundaryType_FreeForm,
};

typedef v2 gjk_simplex[3];

struct collision_boundary {
    collision_boundary_type Type;
    v2 Offset;
    rect Bounds;
    
    union {
        // Rects just use 'rect Bounds'
        
        // FreeForm
        struct {
            v2 *FreeFormPoints;
            u32 FreeFormPointCount;
        };
        
        // Wedges
        struct {
            // but a direction is needed
            v2 WedgePoints[3];
        };
    };
};

global_constant u32 MAX_BOUNDARY_CHILDREN = 3;

//~ Physics

struct entity;
struct physics_collision;
typedef b8   collision_response_function(entity *Data, physics_collision *Collision);
typedef void trigger_response_function(entity *Data, entity *EntityB);

enum physics_object_state_flags_ {
    PhysicsObjectState_None    = 0,
    PhysicsObjectState_Falling = (1 << 0),
    PhysicsObjectState_Floats  = (1 << 1),
    PhysicsObjectState_IsInactive = (1 << 2),
};
typedef u32 physics_object_state_flags;

struct physics_object {
    v2 P;
    f32 Mass;
    collision_boundary *Boundaries;
    physics_object_state_flags State;
    u8 BoundaryCount;
    
    union{
        collision_response_function *Response;
        trigger_response_function   *TriggerResponse;
    };
    entity *Entity;
};

struct static_physics_object : public physics_object {
};

struct trigger_physics_object : public physics_object {
};

struct dynamic_physics_object : public physics_object {
    v2 dP, TargetdP, ddP;
    f32 AccelerationFactor = 0.7f;
    v2 Delta;
    debug_physics_info DebugInfo;
    v2 FloorNormal;
    v2 FloorTangent;
    dynamic_physics_object *ReferenceFrame;
};

struct physics_collision {
    physics_object *ObjectB;
    b8 IsDynamic;
    
    u32 BIndexOffset;
    v2 Normal;
    v2 Correction;
    f32 TimeOfImpact;
    f32 AlongDelta;
};

struct physics_trigger {
    b8 IsValid;
    trigger_physics_object *Trigger;
};

struct physics_system {
    bucket_array<dynamic_physics_object, 64> Objects;
    bucket_array<static_physics_object, 64> StaticObjects;
    bucket_array<trigger_physics_object, 64> TriggerObjects;
    memory_arena PermanentBoundaryMemory;
    memory_arena BoundaryMemory;
    
    void Initialize(memory_arena *Arena);
    void Reload(u32 Width, u32 Height);
    void DoPhysics();
    void DoFloorRaycast(dynamic_physics_object *Object, f32 Depth);
    
    void DoStaticCollisions(physics_collision *OutCollision, collision_boundary *Boundary, v2 P, v2 Delta);
    void DoTriggerCollisions(physics_trigger *OutTrigger, collision_boundary *Boundary, v2 P, v2 Delta);
    void DoCollisionsRelative(physics_collision *OutCollision, collision_boundary *Boundary, v2 P, v2 Delta, bucket_location StartLocation);
    void DoCollisionsNotRelative(physics_collision *OutCollision, collision_boundary *Boundary, v2 P, v2 Delta, physics_object *SkipObject);
    dynamic_physics_object *AddObject(collision_boundary *Boundaries, u8 BoundaryCount);
    static_physics_object *AddStaticObject(collision_boundary *Boundaries, u8 BoundaryCount);
    trigger_physics_object *AddTriggerObject(collision_boundary *Boundaries, u8 BoundaryCount);
    collision_boundary *AllocPermanentBoundaries(u32 Count);
    collision_boundary *AllocBoundaries(u32 Count);
};

#endif //SNAIL_JUMPY_PHYSICS_H
