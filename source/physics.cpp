physics_debugger PhysicsDebugger;

//~ Debug stuff
#if defined(SNAIL_JUMPY_DEBUG_BUILD)
inline void
physics_debugger::Begin(){
    Current = {};
    Layout = CreateLayout(1500, 700, 30, DebugFont.Size, 100, -10.2f);
    Origin = V2(4.0f, 4.0f);
    DrawPoint(Origin, V20, WHITE);
}

inline void
physics_debugger::End(){
    Paused = {};
    StartOfPhysicsFrame = true;
}

inline b8
physics_debugger::DefineStep(){
    if(Current >= Paused &&
       (Flags & PhysicsDebuggerFlags_StepPhysics)){
        return(true);
    }
    
    Current++;
    
    return(false);
}

inline void
physics_debugger::BreakWhen(b8 Value){
    if(!(Flags & PhysicsDebuggerFlags_StepPhysics) && 
       Value){
        Flags |= PhysicsDebuggerFlags_StepPhysics;
        Paused = Current;
        Paused++;
    }
}

inline b8 
physics_debugger::IsCurrent(){
    b8 Result = (PhysicsDebugger.Current == PhysicsDebugger.Paused);
    return(Result);
}

// TODO(Tyler): Using GameCamera.P here in order to combat the point being moved
// by the camera is not good.
inline void
physics_debugger::DrawPoint(v2 Offset, v2 Point, color Color){
    if(!(Flags & PhysicsDebuggerFlags_StepPhysics)) return;
    RenderRect(CenterRect(Offset-GameCamera.P + Scale*Point, V2(0.05f)),
               -10.3f, Color, &GameCamera);
}

inline void
physics_debugger::DrawLineFrom(v2 Offset, v2 A, v2 Delta, color Color){
    if(!(Flags & PhysicsDebuggerFlags_StepPhysics)) return;
    RenderLineFrom(Offset-GameCamera.P+Scale*A, Scale*Delta, -10, 0.02f, Color, &GameCamera);
}

inline void
physics_debugger::DrawNormal(v2 Offset, v2 A, v2 Delta, color Color){
    if(!(Flags & PhysicsDebuggerFlags_StepPhysics)) return;
    Delta = Normalize(Delta);
    RenderLineFrom(Offset-GameCamera.P+Scale*A, 0.2f*Delta, -10, 0.015f, Color, &GameCamera);
}

inline void
physics_debugger::DrawLine(v2 Offset, v2 A, v2 B, color Color){
    if(!(Flags & PhysicsDebuggerFlags_StepPhysics)) return;
    RenderLine(Offset-GameCamera.P+Scale*A, Offset+Scale*B, -10, 0.02f, Color, &GameCamera);
}

inline void
physics_debugger::DrawString(const char *Format, ...){
    if(!(Flags & PhysicsDebuggerFlags_StepPhysics)) return;
    va_list VarArgs;
    va_start(VarArgs, Format);
    VLayoutString(&PhysicsDebugger.Layout, &DebugFont, BLACK, Format, VarArgs);
    va_end(VarArgs);
}

inline void
physics_debugger::DrawStringAtP(v2 P, const char *Format, ...){
    if(!(Flags & PhysicsDebuggerFlags_StepPhysics)) return;
    va_list VarArgs;
    va_start(VarArgs, Format);
    
    P.Y += 0.1f;
    v2 StringP = GameCamera.WorldPToScreenP(P);
    VRenderFormatString(&DebugFont, BLACK, StringP, -10.0f, Format, VarArgs);
    
    va_end(VarArgs);
}

inline void
physics_debugger::DrawPolygon(v2 *Points, u32 PointCount){
    if(!(Flags & PhysicsDebuggerFlags_StepPhysics)) return;
    for(u32 I=0; I < PointCount; I++){
        u32 J = (I+1)%PointCount;
        DrawLine(Origin, Points[I], Points[J], PURPLE);
        DrawPoint(Origin, Points[I], GREEN);
        
        v2 P = Origin + (Scale * Points[I]);
        P.Y += 0.1f;
        v2 NameP = GameCamera.WorldPToScreenP(P);
        RenderFormatString(&DebugFont, BLACK, NameP, -10.0f, "%u", I);
    }
}

inline void
physics_debugger::DrawBaseGJK(v2 AP, v2 BP, v2 Delta, v2 *Points, u32 PointCount){
    if(!(Flags & PhysicsDebuggerFlags_StepPhysics)) return;
    PhysicsDebugger.DrawPoint(AP, V20, WHITE);
    PhysicsDebugger.DrawPoint(BP, V20, DARK_GREEN);
    PhysicsDebugger.DrawLineFrom(PhysicsDebugger.Origin, V20, Delta, ORANGE);
    PhysicsDebugger.DrawPolygon(Points, PointCount);
    PhysicsDebugger.DrawString("Delta: (%f, %f)", Delta.X, Delta.Y);
} 
#else
inline void physics_debugger::Begin(){}
inline void physics_debugger::End(){}
inline b8   physics_debugger::DefineStep(){ return(false); }
inline void physics_debugger::BreakWhen(b8 Value){}
inline b8   physics_debugger::IsCurrent(){ return(false); }
inline void physics_debugger::DrawPoint(v2 Offset, v2 Point, color Color){}
inline void physics_debugger::DrawLineFrom(v2 Offset, v2 A, v2 Delta, color Color){}
inline void physics_debugger::DrawNormal(v2 Offset, v2 A, v2 Delta, color Color){}
inline void physics_debugger::DrawLine(v2 Offset, v2 A, v2 B, color Color){}
inline void physics_debugger::DrawString(const char *Format, ...){}
inline void physics_debugger::DrawStringAtP(v2 P, const char *Format, ...){}
inline void physics_debugger::DrawPolygon(v2 *Points, u32 PointCount){}
inline void physics_debugger::DrawBaseGJK(v2 AP, v2 BP, v2 Delta, v2 *Points, u32 PointCount){} 
#endif


//~ Boundary stuff7
internal inline b8
IsPointInBoundary(v2 Point, collision_boundary *Boundary, v2 Base=V2(0,0)){
    b8 Result = IsPointInRect(Point, OffsetRect(Boundary->Bounds, Base+Boundary->Offset));
    return(Result);
}

internal inline void
RenderBoundary(camera *Camera, collision_boundary *Boundary, f32 Z, v2 Offset){
    Offset += Boundary->Offset;
    color Color_ = Color(0.0f, 0.8f, 0.8f, 0.3f);
    switch(Boundary->Type){
        case BoundaryType_None: break;
        case BoundaryType_Rect: {
            RenderRect(OffsetRect(Boundary->Bounds, Offset), Z, 
                       Color_, Camera);
        }break;
        case BoundaryType_FreeForm: {
            u32 Count = Boundary->FreeFormPointCount;
            for(u32 I=0; I < Count; I++){
                v2 PointA = Boundary->FreeFormPoints[I] + Offset;
                v2 PointB = Boundary->FreeFormPoints[(I+1)%Count] + Offset;
                RenderLine(PointA, PointB, Z-0.15f, 0.02f, BLUE, Camera);
            }
            
        }break;
        default: INVALID_CODE_PATH;
    }
    RenderRectOutline(OffsetRect(Boundary->Bounds, Offset), Z-0.1f,
                      RED, Camera, 0.015f);
}

internal inline collision_boundary
MakeCollisionPoint(){
    collision_boundary Result = {};
    Result.Type = BoundaryType_Point;
    return(Result);
}

internal inline collision_boundary
MakeCollisionRect(v2 Offset, v2 Size){
    collision_boundary Result = {};
    Result.Type = BoundaryType_Rect;
    Result.Offset = Offset;
    Result.Bounds = CenterRect(V20, Size);
    return(Result);
}

internal inline collision_boundary
MakeCollisionWedge(v2 Offset, f32 X, f32 Y, memory_arena *Arena=0){
    if(!Arena) Arena = &PhysicsSystem.BoundaryMemory;
    
    collision_boundary Result = {};
    Result.Type = BoundaryType_FreeForm;
    Result.Offset = Offset;
    f32 MinX = Minimum(X, 0.0f);
    f32 MinY = Minimum(Y, 0.0f);
    Result.Bounds.Min = V2(MinX, MinY);
    
    f32 MaxX = Maximum(X, 0.0f);
    f32 MaxY = Maximum(Y, 0.0f);
    Result.Bounds.Max = V2(MaxX, MaxY);
    
    Result.FreeFormPointCount = 3;
    Result.FreeFormPoints = PushArray(Arena, v2, 3);
    Result.FreeFormPoints[0] = V20;
    Result.FreeFormPoints[1] = V2(X, 0.0f);
    Result.FreeFormPoints[2] = V2(0.0f, Y);
    
    return(Result);
}

internal inline collision_boundary
MakeCollisionCircle(v2 Offset, f32 Radius, u32 Segments, memory_arena *Arena=0){
    if(!Arena) Arena = &PhysicsSystem.BoundaryMemory;
    
    collision_boundary Result = {};
    Result.Type = BoundaryType_FreeForm;
    Result.Offset = Offset;
    Result.Bounds = CenterRect(V20, 2*V2(Radius));
    
    // TODO(Tyler): There is a likely a better way to do this that doesn't require
    // calculation beforehand
    Result.FreeFormPointCount = Segments;
    Result.FreeFormPoints = PushArray(Arena, v2, Segments);
    f32 T = 0.0f;
    f32 Step = 1.0f/(f32)Segments;
    for(u32 I = 0; I <= Segments; I++){
        Result.FreeFormPoints[I] = V2(Radius*Cos(T*TAU), Radius*Sin(T*TAU));
        T += Step;
    }
    
    return(Result);
}

internal inline collision_boundary
MakeCollisionPill(v2 Offset, f32 Radius, f32 Height, u32 HalfSegments, memory_arena *Arena=0){
    // TODO(Tyler): Formalize this?
    if(!Arena) Arena = &PhysicsSystem.BoundaryMemory;
    
    collision_boundary Result = {};
    Result.Type = BoundaryType_FreeForm;
    Result.Offset = Offset;
    Result.Bounds = Rect(V2(-Radius, -Radius), V2(Radius, Height+Radius));
    
    u32 Segments = 2*HalfSegments;
    u32 ActualSegments = Segments + 2;
    Result.FreeFormPointCount = ActualSegments;
    Result.FreeFormPoints = PushArray(Arena, v2, ActualSegments);
    
    f32 HeightOffset = Height;
    f32 T = 0.0f;
    f32 Step = 1.0f/((f32)Segments);
    u32 Index = 0;
    
    for(u32 I=0; I <= HalfSegments; I++){
        Result.FreeFormPoints[Index] = V2(Radius*Cos(T*TAU), Radius*Sin(T*TAU));
        Result.FreeFormPoints[Index].Y += HeightOffset;
        T += Step;
        Index++;
    }
    T -= Step;
    
    HeightOffset = 0;
    for(u32 I=0; I <= HalfSegments; I++){
        Result.FreeFormPoints[Index] = V2(Radius*Cos(T*TAU), Radius*Sin(T*TAU));
        Result.FreeFormPoints[Index].Y += HeightOffset;
        T += Step;
        Index++;
    }
    
    
    return(Result);
}

internal rect
GetBoundsOfBoundaries(collision_boundary *Boundaries, u32 BoundaryCount){
    v2 Min = V2(F32_POSITIVE_INFINITY);
    v2 Max = V2(F32_NEGATIVE_INFINITY);
    for(u32 I=0; I < BoundaryCount; I++){
        Min = MinimumV2(Boundaries[I].Bounds.Min+Boundaries[I].Offset, Min);
        Max = MaximumV2(Boundaries[I].Bounds.Max+Boundaries[I].Offset, Max);
    }
    rect Result = Rect(Min, Max);
    return(Result);
}

internal inline void
SetObjectBoundaries(physics_object *Object, collision_boundary *Boundaries, u8 Count){
    Object->Boundaries = Boundaries;
    Object->BoundaryCount = Count;
    Object->Bounds = GetBoundsOfBoundaries(Boundaries, Count);
}

internal inline void
RenderObject(physics_object *Object){
    if(Object->State & PhysicsObjectState_Inactive) return;
    
    RenderRectOutline(OffsetRect(Object->Bounds, Object->P), -10.0f, GREEN, &GameCamera, 0.02f);
    for(collision_boundary *Boundary = Object->Boundaries;
        Boundary < Object->Boundaries+Object->BoundaryCount;
        Boundary++){
        RenderBoundary(&GameCamera, Boundary, -10.1f, Object->P);
    }
}

//~ Collision stuff
internal inline physics_collision
MakeCollision(){
    physics_collision Result = {};
    Result.TimeOfImpact = F32_POSITIVE_INFINITY;
    return(Result);
}

internal inline physics_collision
MakeBCollision(physics_collision Collision, dynamic_physics_object *ObjectA){
    physics_collision Result = Collision;
    Result.Normal = -Result.Normal;
    Result.ObjectB = ObjectA;
    return(Result);
}

internal b8
CollisionResponseStub(entity *Data, physics_collision *Collision){
    b8 Result = false;
    return(Result);
}

//~ Physics

void
physics_system::Initialize(memory_arena *Arena){
    BucketArrayInitialize(&ParticleSystems, Arena);
    BucketArrayInitialize(&Objects, Arena);
    BucketArrayInitialize(&StaticObjects, Arena);
    BucketArrayInitialize(&TriggerObjects, Arena);
    BucketArrayInitialize(&Tilemaps, Arena);
    ParticleMemory          = PushNewArena(Arena, 64*128*sizeof(physics_particle_x4));
    BoundaryMemory          = PushNewArena(Arena, 3*128*sizeof(collision_boundary));
    PermanentBoundaryMemory = PushNewArena(Arena, 128*sizeof(collision_boundary));
}

void
physics_system::Reload(u32 Width, u32 Height){
    BucketArrayRemoveAll(&ParticleSystems);
    BucketArrayRemoveAll(&Objects);
    BucketArrayRemoveAll(&StaticObjects);
    BucketArrayRemoveAll(&TriggerObjects);
    BucketArrayRemoveAll(&Tilemaps);
    ClearArena(&ParticleMemory);
    ClearArena(&BoundaryMemory);
    
    PhysicsDebugger.Paused = {};
    PhysicsDebugger.StartOfPhysicsFrame = true;
}

physics_particle_system *
physics_system::AddParticleSystem(v2 P, collision_boundary *Boundary, u32 Count,
                                  f32 COR=1.0f){
    physics_particle_system *Result = BucketArrayAlloc(&ParticleSystems);
    Result->Particles = CreateFullArray<physics_particle_x4>(&ParticleMemory, Count, 16);
    Result->Boundary = Boundary;
    Result->P = P;
    Result->COR = COR;
    return(Result);
}

dynamic_physics_object *
physics_system::AddObject(collision_boundary *Boundaries, u8 Count){
    dynamic_physics_object *Result = BucketArrayAlloc(&Objects);
    SetObjectBoundaries(Result, Boundaries, Count);
    
    Result->Mass = 1.0f; // Default mass
    Result->Response = CollisionResponseStub;
    return(Result);
}

static_physics_object *
physics_system::AddStaticObject(collision_boundary *Boundaries, u8 Count){
    static_physics_object *Result = BucketArrayAlloc(&StaticObjects);
    SetObjectBoundaries(Result, Boundaries, Count);
    Result->Mass = F32_POSITIVE_INFINITY;
    Result->Response = CollisionResponseStub;
    return(Result);
}

trigger_physics_object *
physics_system::AddTriggerObject(collision_boundary *Boundaries, u8 Count){
    trigger_physics_object *Result = BucketArrayAlloc(&TriggerObjects);
    Result->Boundaries = Boundaries;
    Result->BoundaryCount = Count;
    Result->Mass = F32_POSITIVE_INFINITY;
    Result->Response = CollisionResponseStub;
    Result->State &= ~PhysicsObjectState_Inactive;
    return(Result);
}

physics_tilemap *
physics_system::AddTilemap(u8 *Tilemap, u8 Value, u32 Width, u32 Height, v2 TileSize){
    physics_tilemap *Result = BucketArrayAlloc(&Tilemaps);
    Result->Map       = Tilemap;
    Result->Value     = Value;
    Result->MapWidth  = Width;
    Result->MapHeight = Height;
    Result->TileSize  = TileSize;
    Result->Boundary  = AllocBoundaries(1);
    *Result->Boundary = MakeCollisionRect(V20, TileSize);
    
    return(Result);
}

collision_boundary *
physics_system::AllocPermanentBoundaries(u32 Count){
    collision_boundary *Result = PushArray(&PermanentBoundaryMemory, collision_boundary, Count);
    return(Result);
}

collision_boundary *
physics_system::AllocBoundaries(u32 Count){
    collision_boundary *Result = PushArray(&BoundaryMemory, collision_boundary, Count);
    return(Result);
}

//~ Collision detection

internal inline b8
DoAABBTest(rect BoundsA, v2 Offset, v2 AP, 
           rect BoundsB, v2 BP, v2 Delta){
    BoundsA = OffsetRect(BoundsA, Offset);
    rect RectA1 = BoundsA;
    rect RectA2 = OffsetRect(RectA1, Delta);
    rect RectA;
    RectA.Min = MinimumV2(RectA1.Min, RectA2.Min);
    RectA.Max = MaximumV2(RectA1.Max, RectA2.Max);
    RectA = OffsetRect(RectA, AP);
    rect RectB = BoundsB;
    RectB = OffsetRect(RectB, BP);
    b8 Result = DoRectsOverlap(RectA, RectB);
    return(Result);
}

internal inline v2
DoSupport(collision_boundary *Boundary, 
          v2 P, v2 Delta,
          v2 Direction){
    v2 Result = {};
    
    switch(Boundary->Type){
        case BoundaryType_Rect: {
            v2 Min = Boundary->Bounds.Min;
            v2 Max = Boundary->Bounds.Max;
            Result = Min;
            if(Dot(V2(1, 0), Direction) > 0.0f) { Result.X = Max.X; }
            if(Dot(V2(0, 1), Direction) > 0.0f) { Result.Y = Max.Y; }
        }break;
        case BoundaryType_FreeForm: {
            for(u32 I = 0; I < Boundary->FreeFormPointCount; I++){
                v2 Point = Boundary->FreeFormPoints[I];
                if(Dot(Point, Direction) > Dot(Result, Direction)){
                    Result = Point;
                }
            }
        }break;
        case BoundaryType_None: break;
        case BoundaryType_Point: break;
        default: INVALID_CODE_PATH;
    }
    
    Result += Boundary->Offset;
    Result +=  P;
    if(Dot(Delta, Direction) > 0.0f){
        Result += Delta;
    }
    
    return(Result);
}

internal inline v2 
CalculateSupport(collision_boundary *BoundaryA, v2 AP, collision_boundary *BoundaryB, v2 BP, v2 Delta, v2 Direction){
    v2 Result = DoSupport(BoundaryA, AP, Delta, Direction) - DoSupport(BoundaryB, BP, V20, -Direction);
    return(Result);
}

internal b8
UpdateSimplex(v2 Simplex[3], u32 *SimplexCount, v2 *Direction){
    b8 Result = false;
    
    switch(*SimplexCount){
        case 2: {
            if(Dot(Simplex[0]-Simplex[1], -Simplex[1]) > 0.0f){
                *Direction = TripleProduct(Simplex[0]-Simplex[1], -Simplex[1]);
                if((Direction->X == 0.0f) && (Direction->Y == 0.0f)){
                    // TODO(Tyler): I have no idea if there is a better way to do this
                    *Direction = TripleProduct(Simplex[0]-Simplex[1], V2(0.1f, 0.1f)-Simplex[1]);
                }
                
                PhysicsDebugger.Base = 0.5f*(Simplex[0]+Simplex[1]);
            }else{
                *Direction = -Simplex[1];
                Simplex[0] = Simplex[1];
                *SimplexCount = 1;
                
                PhysicsDebugger.Base = Simplex[0];
            }
        }break;
        case 3: {
            // TODO(Tyler): This is a place that could be significantly improved
            
            v2 P2P0 = Simplex[0]-Simplex[2];
            v2 P2P1 = Simplex[1]-Simplex[2];
            f32 Z = P2P1.X*P2P0.Y - P2P1.Y*P2P0.X;
            
            v2 P2P0Direction = V2(-Z*P2P0.Y, Z*P2P0.X);
            b8 OutsideP2P0 = Dot(P2P0Direction, -Simplex[2]) >= 0.0f;
            v2 P2P1Direction = V2(Z*P2P1.Y, -Z*P2P1.X);
            b8 OutsideP2P1 = Dot(P2P1Direction, -Simplex[2]) >= 0.0f;
            b8 AlongP2P0 = Dot(P2P0 , -Simplex[2]) >= 0.0f;
            b8 AlongP2P1 = Dot(P2P1, -Simplex[2]) >= 0.0f;
            
            if(!OutsideP2P0 && OutsideP2P1 && AlongP2P1){
                // Area 4
                Simplex[0] = Simplex[1];
                Simplex[1] = Simplex[2];
                *SimplexCount = 2;
                *Direction = P2P1Direction;
                PhysicsDebugger.Base = 0.5f*(Simplex[0]+Simplex[1]);
            }else if(OutsideP2P0 && !OutsideP2P1 && AlongP2P0){
                // Area 6
                Simplex[1] = Simplex[2];
                *SimplexCount = 2;
                *Direction = P2P0Direction;
                PhysicsDebugger.Base = 0.5f*(Simplex[0]+Simplex[1]);
            }else if(OutsideP2P0 && OutsideP2P1 && !AlongP2P0 && !AlongP2P0){
                // Area 5
                Simplex[0] = Simplex[2];
                *SimplexCount = 1;
                *Direction = -Simplex[0];
                PhysicsDebugger.Base = Simplex[0];
            }else{
                // Area 7, we have enclosed the origin and have a collision
                Result = true;
            }
        }break;
        default: {
            INVALID_CODE_PATH;
        }break;
    }
    
    return(Result);
}

internal inline b8
DoGJK(v2 Simplex[3], 
      collision_boundary *BoundaryA, v2 AP, 
      collision_boundary *BoundaryB, v2 BP, v2 Delta){
    f32 ObjectBPAlongDelta = Dot(Delta, BP-AP);
    u32 SimplexCount = 1; // Account for the first support point
    v2 Direction = AP - BP;
    if((Direction.X == 0.0f) && (Direction.Y == 0.0f)){
        Direction = V2(1, 0);
    }
    Simplex[0] = CalculateSupport(BoundaryA, AP, BoundaryB, BP, Delta, Direction);
    Direction = -Simplex[0];
    
    b8 DoesCollide = false;
    PhysicsDebugger.Base = Simplex[0];
    while(true){
        if(PhysicsDebugger.DefineStep()) return(false);
        if(PhysicsDebugger.IsCurrent()){
            PhysicsDebugger.DrawBaseGJK(AP, BP, Delta, Simplex, SimplexCount);
            
            PhysicsDebugger.DrawNormal(PhysicsDebugger.Origin, PhysicsDebugger.Base, Direction, PINK);
            
            PhysicsDebugger.DrawString("Simplex Count: %u", SimplexCount);
        }
        
        
        v2 NewPoint = CalculateSupport(BoundaryA, AP, BoundaryB, BP, Delta, Direction);
        if(Dot(NewPoint, Direction) < 0.0f){ 
            // The new point is in the wrong direction, hence
            // there is no point going the right direction, hence
            // the simplex doesn't enclose the origin, hence
            // no collision
            break;
        }
        Simplex[SimplexCount] = NewPoint;
        SimplexCount++;
        if(UpdateSimplex(Simplex, &SimplexCount, &Direction)){
            DoesCollide = true;
            break;
        }
    }
    
    return(DoesCollide);
}

struct epa_result {
    v2 Correction;
    f32 TimeOfImpact;
    v2 Normal;
};

// Variation on the expanding polytope algorithm that takes the object's delta into account
internal epa_result
DoDeltaEPA(collision_boundary *BoundaryA, v2 AP, collision_boundary *BoundaryB, v2 BP, v2 Delta, v2 Simplex[3]){
    const f32 Epsilon = 0.0001f;
    
    dynamic_array<v2> Polytope; 
    DynamicArrayInitialize(&Polytope, 20, &TransientStorageArena);
    DynamicArrayPushBack(&Polytope, Simplex[0]);
    DynamicArrayPushBack(&Polytope, Simplex[1]);
    DynamicArrayPushBack(&Polytope, Simplex[2]);
    
    v2 DeltaNormal = Normalize(Clockwise90(Delta));
    v2 DeltaDirection = Normalize(Delta);
    
    enum found_edge_type {
        FoundEdge_None,
        FoundEdge_Colinear,
        FoundEdge_Beyond,
        FoundEdge_Ordinary,
    };
    
    epa_result Result = {};
    while(true){
        u32 EdgeIndex = 0;
        f32 EdgeDistance = 0;
        v2 InverseNormal = {};
        v2 IntersectionPoint = {};
        found_edge_type FoundEdge = FoundEdge_None;
        
        for(u32 I=0; I < Polytope.Count; I++){
            v2 A = Polytope[I];
            v2 B = Polytope[(I+1)%Polytope.Count];
            
            f32 AAlongDeltaNormal = Dot(DeltaNormal, A);
            f32 BAlongDeltaNormal = Dot(DeltaNormal, B);
            
            if(((-Epsilon <= AAlongDeltaNormal) && (AAlongDeltaNormal <= Epsilon)) && // Collinear
               ((-Epsilon <= BAlongDeltaNormal) && (BAlongDeltaNormal <= Epsilon))){
                FoundEdge = FoundEdge_Colinear;
                EdgeIndex = I;
                InverseNormal = V20;
                EdgeDistance = Dot(InverseNormal, -A);
                IntersectionPoint = V20;
            }else if(((AAlongDeltaNormal >= 0) && (BAlongDeltaNormal <= 0)) || 
                     ((AAlongDeltaNormal <= 0) && (BAlongDeltaNormal >= 0))){ // The delta line  intersects
                AAlongDeltaNormal = AbsoluteValue(AAlongDeltaNormal);
                BAlongDeltaNormal = AbsoluteValue(BAlongDeltaNormal);
                f32 ABPercent = AAlongDeltaNormal/(AAlongDeltaNormal + BAlongDeltaNormal);
                v2 Point = A + ABPercent*(B-A);
                
                f32 PointAlongDeltaDirection = Dot(DeltaDirection, Point);
                f32 DeltaLength = Dot(DeltaDirection, Delta);
                
                if(-Epsilon <= PointAlongDeltaDirection){
                    if(PointAlongDeltaDirection <= DeltaLength+Epsilon){ // The delta intersects
                        FoundEdge = FoundEdge_Ordinary;
                        EdgeIndex = I;
                        InverseNormal = Normalize(TripleProduct(B-A, -A));
                        EdgeDistance = Dot(InverseNormal, -A);
                        PointAlongDeltaDirection = Clamp(PointAlongDeltaDirection, 0.0f, DeltaLength);
                        IntersectionPoint = PointAlongDeltaDirection*DeltaDirection;
                        break;
                    }else if(FoundEdge != FoundEdge_Colinear){ // Intersect along the delta but beyond it
                        FoundEdge = FoundEdge_Beyond;
                        EdgeIndex = I;
                        InverseNormal = Normalize(TripleProduct(B-A, -A));
                        EdgeDistance = Dot(InverseNormal, -A);
                        IntersectionPoint = Point;
                    }
                }
            }
        }
        PhysicsDebugger.BreakWhen(FoundEdge == FoundEdge_None);
        
        f32 Distance;
        v2 NewPoint = V20;
        if((InverseNormal.X == 0.0f) && (InverseNormal.Y == 0.0f)){
            Distance = EdgeDistance;
        }else{
            NewPoint = CalculateSupport(BoundaryA, AP, BoundaryB, BP, Delta, -InverseNormal);
            Distance = Dot(NewPoint, -InverseNormal);
        }
        
        //~ DEBUG
        if(PhysicsDebugger.DefineStep()) return(Result);
        if(PhysicsDebugger.IsCurrent()){
            PhysicsDebugger.DrawBaseGJK(AP, BP, Delta, Polytope.Items, Polytope.Count);
            
            f32 Percent = Dot(DeltaDirection, IntersectionPoint)/Dot(DeltaDirection, Delta);
            if((Delta.X == 0.0f) && (Delta.Y == 0.0f)){
                Percent = 0.0f;
            }
            
            v2 Base = 0.5f*(Polytope[EdgeIndex] + Polytope[(EdgeIndex+1)%Polytope.Count]);
            
            PhysicsDebugger.DrawNormal(PhysicsDebugger.Origin, Base, -InverseNormal, PINK);
            PhysicsDebugger.DrawNormal(PhysicsDebugger.Origin, Base,  InverseNormal, YELLOW);
            PhysicsDebugger.DrawPoint(PhysicsDebugger.Origin, IntersectionPoint, ORANGE);
            
            PhysicsDebugger.DrawString("Polytope Count: %u", Polytope.Count);
            PhysicsDebugger.DrawString("Time of impact: %f", 1.0f-Percent);
        }
        
        f32 DistanceEpsilon = 0.00001f;
        if((-DistanceEpsilon <= (Distance-EdgeDistance)) && ((Distance-EdgeDistance) <= DistanceEpsilon)){
            f32 Percent = Dot(DeltaDirection, IntersectionPoint)/Dot(DeltaDirection, Delta);
            if((Delta.X == 0.0f) && (Delta.Y == 0.0f)){
                Percent = 0.0f;
            }
            
            f32 TimeEpsilon = 0.0001f;
            Result.TimeOfImpact = (1.0f - Percent) - TimeEpsilon;
            if(Result.TimeOfImpact > 1.0f){
                Result.TimeOfImpact = 1.0f;
            }else if(Result.TimeOfImpact < -TimeEpsilon){
                //v2 Difference = Delta - IntersectionPoint;
                v2 Difference = Delta - (Percent+Epsilon)*Delta;
                Result.Correction = InverseNormal * Dot(InverseNormal, Difference);
                //Result.Correction = Difference;
                Result.TimeOfImpact = 0.0f;
            }else if(Result.TimeOfImpact < 0.0f){
                Result.TimeOfImpact = 0.0f;
            }
            Result.Normal = InverseNormal;
            
            break;
        }else{
            DynamicArrayInsertNewArrayItem(&Polytope, EdgeIndex+1, NewPoint);
        }
    }
    
    //~ DEBUG
    if(PhysicsDebugger.DefineStep()) return(Result); 
    if(PhysicsDebugger.IsCurrent()){
        PhysicsDebugger.DrawBaseGJK(AP, BP, Delta, Polytope.Items, Polytope.Count);
        
        PhysicsDebugger.DrawNormal(PhysicsDebugger.Origin, V20, Result.Normal, PINK);
        
        PhysicsDebugger.DrawString("Polytope.Count: %u", Polytope.Count);
        PhysicsDebugger.DrawString("Time of impact: %f", Result.TimeOfImpact);
        PhysicsDebugger.DrawString("Correction: (%f, %f)", Result.Correction.X, Result.Correction.Y);
    }
    
    
    return(Result);
}

//~ Physics system

internal physics_collision
DoCollision(collision_boundary *BoundaryA, v2 AP, collision_boundary *BoundaryB, v2 BP, v2 Delta){
    physics_collision Result = MakeCollision();
    Result.AlongDelta        = Dot(Normalize(Delta), BP-AP);
    
    v2 Simplex[3];
    if(DoGJK(Simplex, BoundaryA, AP, BoundaryB, BP, Delta)){
        epa_result EPAResult = DoDeltaEPA(BoundaryA, AP, BoundaryB, BP, Delta, Simplex);
        Result.Normal            = EPAResult.Normal;
        Result.Correction        = EPAResult.Correction;
        Result.TimeOfImpact      = EPAResult.TimeOfImpact;
    }
    
    return(Result);
}

internal b8
ChooseCollision(f32 TimeOfImpact, f32 AlongDelta, physics_collision *Collision){
    local_constant f32 Epsilon = 0.0001f;
    b8 Result = false;
    
    if(Collision->TimeOfImpact < TimeOfImpact){
        Result = true;
    }else if((Collision->TimeOfImpact > TimeOfImpact-Epsilon) && 
             (Collision->TimeOfImpact < TimeOfImpact+Epsilon)){
        if(Collision->AlongDelta < AlongDelta){
            Result = true;
        }
    }
    
    return(Result);
}

void 
physics_system::DoStaticCollisions(physics_collision *OutCollision, collision_boundary *Boundary, v2 P, v2 Delta){
    if((Delta.X == 0.0f) && (Delta.Y == 0.0f)) { return; }
    
    FOR_BUCKET_ARRAY(ItB, &StaticObjects){
        static_physics_object *ObjectB = ItB.Item;
        if(ObjectB->State & PhysicsObjectState_Inactive) continue;
        if(!DoAABBTest(Boundary->Bounds, Boundary->Offset, P, ObjectB->Bounds, ObjectB->P, Delta)) continue;
        
        for(collision_boundary *BoundaryB = ObjectB->Boundaries;
            BoundaryB < ObjectB->Boundaries+ObjectB->BoundaryCount;
            BoundaryB++){
            physics_collision Collision = DoCollision(Boundary, P, BoundaryB, ObjectB->P, Delta);
            Collision.ObjectB = ObjectB;
            if(ChooseCollision(OutCollision->TimeOfImpact, OutCollision->AlongDelta, &Collision)){
                *OutCollision = Collision;
            }
        }
    }
    
    FOR_BUCKET_ARRAY(ItB, &Tilemaps){
        physics_tilemap *Tilemap = ItB.Item;
        rect Bounds = OffsetRect(Boundary->Bounds, Boundary->Offset+P);
        Bounds = SweepRect(Bounds, Delta);
        Bounds.Min.X /= Tilemap->TileSize.X;
        Bounds.Min.Y /= Tilemap->TileSize.Y;
        Bounds.Max.X /= Tilemap->TileSize.X;
        Bounds.Max.Y /= Tilemap->TileSize.Y;
        
        rect_s32 BoundsS32 = RectS32(Bounds);
        v2s TilemapMax = V2S(Tilemap->MapWidth, Tilemap->MapHeight);
        BoundsS32.Min = MaximumV2S(V2S(0), BoundsS32.Min);
        BoundsS32.Max = MaximumV2S(V2S(0), BoundsS32.Max);
        BoundsS32.Min = MinimumV2S(TilemapMax, BoundsS32.Min);
        BoundsS32.Max = MinimumV2S(TilemapMax, BoundsS32.Max);
        
        for(s32 Y = BoundsS32.Min.Y; Y < BoundsS32.Max.Y; Y++){
            for(s32 X = BoundsS32.Min.X; X < BoundsS32.Max.X; X++){
                u8 TileId = Tilemap->Map[(Y*Tilemap->MapWidth)+X];
                v2 TileP = V2((f32)X, (f32)Y);
                TileP += V2(0.5f);
                TileP.X *= Tilemap->TileSize.X;
                TileP.Y *= Tilemap->TileSize.Y;
                rect TileBounds = CenterRect(V20, Tilemap->TileSize);
                
                if(TileId == Tilemap->Value){
                    physics_collision Collision = DoCollision(Boundary, P, Tilemap->Boundary, TileP, Delta);
                    Collision.ObjectB = PushStruct(&TransientStorageArena, physics_object);
                    Collision.ObjectB->P          = TileP;
                    Collision.ObjectB->Bounds     = TileBounds;
                    Collision.ObjectB->Mass       = F32_POSITIVE_INFINITY;
                    Collision.ObjectB->Boundaries = Tilemap->Boundary;
                    Collision.ObjectB->BoundaryCount = 1;
                    if(ChooseCollision(OutCollision->TimeOfImpact, OutCollision->AlongDelta, &Collision)){
                        *OutCollision = Collision;
                    }
                }
            }
        }
    }
}

void 
physics_system::DoTriggerCollisions(physics_trigger *OutTrigger, collision_boundary *Boundary, v2 P, v2 Delta){
    FOR_BUCKET_ARRAY(ItB, &TriggerObjects){
        trigger_physics_object *ObjectB = ItB.Item;
        if(ObjectB->State & PhysicsObjectState_Inactive) continue;
        if(!DoAABBTest(Boundary->Bounds, Boundary->Offset, P, ObjectB->Bounds, ObjectB->P, Delta)) continue;
        
        for(collision_boundary *BoundaryB = ObjectB->Boundaries;
            BoundaryB < ObjectB->Boundaries+ObjectB->BoundaryCount;
            BoundaryB++){
            v2 Simplex[3];
            
            if(DoGJK(Simplex, Boundary, P, BoundaryB, ObjectB->P, Delta)){
                OutTrigger->IsValid = true;
                OutTrigger->Trigger = ObjectB;
            }
        }
    }
}

void
physics_system::DoCollisionsRelative(physics_collision *OutCollision, collision_boundary *Boundary, v2 P, v2 Delta, b8 StartAtLocation=false, bucket_location StartLocation={}){
    if(!StartAtLocation){ 
        StartLocation = {}; 
    }else{
        StartLocation.ItemIndex++; 
    }
    FOR_BUCKET_ARRAY_FROM(ItB, &Objects, StartLocation){
        dynamic_physics_object *ObjectB = ItB.Item;
        if(ObjectB->State & PhysicsObjectState_Inactive) continue;
        
        v2 RelativeDelta = Delta-ObjectB->Delta;
        
        if(!DoAABBTest(Boundary->Bounds, Boundary->Offset, P, ObjectB->Bounds, ObjectB->P, RelativeDelta)){
            continue;
        }
        for(collision_boundary *BoundaryB = ObjectB->Boundaries;
            BoundaryB < ObjectB->Boundaries+ObjectB->BoundaryCount;
            BoundaryB++){
            physics_collision Collision = DoCollision(Boundary, P, BoundaryB, ObjectB->P, RelativeDelta);
            Collision.ObjectB = ObjectB;
            Collision.IsDynamic = true;
            Collision.BIndexOffset = ItB.I+1;
            if(ChooseCollision(OutCollision->TimeOfImpact, OutCollision->AlongDelta, &Collision)){
                *OutCollision = Collision;
            }
        }
    }
}

// NOTE(Tyler): No BIndexOffset
void
physics_system::DoCollisionsNotRelative(physics_collision *OutCollision, collision_boundary *Boundary, v2 P, v2 Delta, physics_object *SkipObject=0){
    FOR_BUCKET_ARRAY(ItB, &Objects){
        dynamic_physics_object *ObjectB = ItB.Item;
        if(ObjectB == SkipObject) continue; 
        if(ObjectB->State & PhysicsObjectState_Inactive) continue;
        if(!DoAABBTest(Boundary->Bounds, Boundary->Offset, P, ObjectB->Bounds, ObjectB->P, Delta)) continue;
        
        for(collision_boundary *BoundaryB = ObjectB->Boundaries;
            BoundaryB < ObjectB->Boundaries+ObjectB->BoundaryCount;
            BoundaryB++){
            physics_collision Collision = DoCollision(Boundary, P, BoundaryB, ObjectB->P, Delta);
            Collision.ObjectB = ObjectB;
            Collision.IsDynamic = true;
            if(ChooseCollision(OutCollision->TimeOfImpact, OutCollision->AlongDelta, &Collision)){
                *OutCollision = Collision;
            }
        }
    }
}

//~ Do physics

void
physics_system::DoFloorRaycast(dynamic_physics_object *Object, f32 Depth=0.2f){
    if(PhysicsDebugger.DefineStep()) return;
    if(PhysicsDebugger.IsCurrent()){
        PhysicsDebugger.DrawString("Floor raycast");
    }
    
    v2 Raycast = V2(0, -Depth);
    physics_collision Collision = MakeCollision();
    for(collision_boundary *Boundary = Object->Boundaries;
        Boundary < Object->Boundaries+Object->BoundaryCount;
        Boundary++){
        DoStaticCollisions(&Collision, Boundary, Object->P, Raycast);
        DoCollisionsNotRelative(&Collision, Boundary, Object->P, Raycast, Object);
    }
    
    if(PhysicsDebugger.DefineStep()) return;
    
    if(Collision.TimeOfImpact >= 1.0f){ 
        if(PhysicsDebugger.IsCurrent()){ PhysicsDebugger.DrawString("No floor, too far"); }
        Object->State |= PhysicsObjectState_Falling;
        return;
    }
    if(Collision.Normal.Y < WALKABLE_STEEPNESS) { 
        if(PhysicsDebugger.IsCurrent()){ PhysicsDebugger.DrawString("No floor, too steaph"); }
        return; 
    }
    
    Object->P += Raycast*Collision.TimeOfImpact;
    Object->P += Collision.Correction;
    
    Object->dP       -= Collision.Normal*Dot(Object->dP, Collision.Normal);
    Object->TargetdP -= Collision.Normal*Dot(Object->TargetdP, Collision.Normal);
    Object->FloorNormal = Collision.Normal;
    
    if(Collision.IsDynamic){
        Object->ReferenceFrame = (dynamic_physics_object *)Collision.ObjectB;
    }else{
        Object->ReferenceFrame = 0;
    }
    
    if(PhysicsDebugger.IsCurrent()){
        physics_object *ObjectB = Collision.ObjectB;
        PhysicsDebugger.DrawString("Yes floor");
        PhysicsDebugger.DrawPoint(Object->P, V20, WHITE);
        PhysicsDebugger.DrawPoint(ObjectB->P, V20, DARK_GREEN);
        PhysicsDebugger.DrawNormal(ObjectB->P, V20, Collision.Normal, PINK);
        PhysicsDebugger.DrawString("TimeOfImpact: %f", Collision.TimeOfImpact);
        PhysicsDebugger.DrawString("Correction: (%f, %f)", Collision.Correction.X, Collision.Correction.Y);
        PhysicsDebugger.DrawString("MassA: %f, MassB: %f", Object->Mass, ObjectB->Mass);
    }
}

void
physics_system::DoPhysics(){
    TIMED_FUNCTION();
    local_constant f32 Epsilon = 0.0001f;
    
    // DEBUG
    PhysicsDebugger.Begin();
    
    f32 dTime = OSInput.dTime;
    
    physics_collision *Collisions = PushArray(&TransientStorageArena, physics_collision, Objects.Count);
    for(u32 I=0; I<Objects.Count; I++){
        Collisions[I] = MakeCollision();
    }
    physics_trigger *Triggers = PushArray(&TransientStorageArena, physics_trigger, Objects.Count);
    
    //~  Calculate entity deltas
    FOR_BUCKET_ARRAY(It, &Objects){
        dynamic_physics_object *Object = It.Item;
        
        // NOTE(Tyler): This may not be the best way to integrate dP, Delta, but it works
        f32 DragCoefficient = 0.7f;
        Object->ddP.X += -DragCoefficient*Object->dP.X*AbsoluteValue(Object->dP.X);
        Object->ddP.Y += -DragCoefficient*Object->dP.Y*AbsoluteValue(Object->dP.Y);
        Object->Delta = (dTime*Object->dP + 
                         0.5f*Square(dTime)*Object->ddP);
        Object->dP += dTime*Object->ddP;
        Object->TargetdP += dTime*Object->ddP;
        Object->dP += Object->AccelerationFactor*(Object->TargetdP-Object->dP);
        Object->ddP = {};
        
        //~ DEBUG
        if(PhysicsDebugger.StartOfPhysicsFrame){
            Object->DebugInfo.P = Object->P;
            Object->DebugInfo.dP = Object->dP;
            Object->DebugInfo.ddP = Object->ddP;
            Object->DebugInfo.Delta = Object->Delta;
        }
        if(PhysicsDebugger.Flags & PhysicsDebuggerFlags_StepPhysics){
            Object->P = Object->DebugInfo.P;
            Object->dP = Object->DebugInfo.dP;
            Object->ddP = Object->DebugInfo.ddP;
            Object->Delta = Object->DebugInfo.Delta;
        }
        
        // TODO(Tyler): This needs to be calculated each iteration, so that delta changes
        // are taken into account
        if(Object->ReferenceFrame){ 
            dynamic_physics_object *Reference = Object->ReferenceFrame;
            u32 ReferenceCount = 0;
            while(Reference &&
                  (ReferenceCount < 5)){
                ReferenceCount++;
                Object->Delta += Reference->Delta;
                Reference = Reference->ReferenceFrame;
            }
        }
        
    }
    
    // TODO(Tyler): Move this into physics_debugger
    if(PhysicsDebugger.StartOfPhysicsFrame){
        PhysicsDebugger.StartOfPhysicsFrame = false;
    }
    
    
    //~ DEBUG
#if defined(SNAIL_JUMPY_DEBUG_BUILD)
    if(DebugConfig.Overlay & DebugOverlay_Boundaries){
        // NOTE(Tyler): This doesn't need to be done here, but it is nice to have 
        // as few #ifs as possible
        FOR_BUCKET_ARRAY(It, &Objects){
            RenderObject(It.Item);
        }
        
        FOR_BUCKET_ARRAY(It, &StaticObjects){
            RenderObject(It.Item);
        }
        
        FOR_BUCKET_ARRAY(It, &TriggerObjects){
            RenderObject(It.Item);
        }
    }
    
#endif
    
    //~ Do collisions
    u32 IterationsToDo = Objects.Count*PHYSICS_ITERATIONS_PER_OBJECT;
    f32 FrameTimeRemaining = 1.0f;
    u32 Iteration = 0;
    while((FrameTimeRemaining > 0) &&
          (Iteration < IterationsToDo)){
        Iteration++;
        
        f32 CurrentTimeOfImpact = 1.0f;
        
        //~ Detect collisions
        FOR_BUCKET_ARRAY(ItA, &Objects){
            dynamic_physics_object *ObjectA = ItA.Item;
            
            local_constant f32 Epsilon = 0.0001f;
            if((-Epsilon <= ObjectA->Delta.X) && (ObjectA->Delta.X <= Epsilon) &&
               (-Epsilon <= ObjectA->Delta.Y) && (ObjectA->Delta.Y <= Epsilon)){
                ObjectA->Delta = {};
            }
            
            if((-Epsilon <= ObjectA->dP.X) && (ObjectA->dP.X <= Epsilon) &&
               (-Epsilon <= ObjectA->dP.Y) && (ObjectA->dP.Y <= Epsilon)){
                ObjectA->dP = {};
            }
            
            //~ DEBUG
            
            physics_collision Collision = MakeCollision();
            physics_trigger Trigger = {};
            for(collision_boundary *Boundary = ObjectA->Boundaries;
                Boundary < ObjectA->Boundaries+ObjectA->BoundaryCount;
                Boundary++){
                DoStaticCollisions(&Collision, Boundary, ObjectA->P, ObjectA->Delta);
                DoCollisionsRelative(&Collision, Boundary, ObjectA->P, ObjectA->Delta, true, ItA.Location);
                DoTriggerCollisions(&Trigger, Boundary, ObjectA->P, ObjectA->Delta);
            }
            
            u32 BIndex = ItA.I + Collision.BIndexOffset;
            if(ChooseCollision(CurrentTimeOfImpact, F32_POSITIVE_INFINITY, &Collision)){
                Collisions[ItA.I] = Collision;
                Triggers[ItA.I]   = Trigger;
                if(Collision.TimeOfImpact < CurrentTimeOfImpact){
                    CurrentTimeOfImpact = Collision.TimeOfImpact;
                    for(u32 I=0; I<ItA.I; I++){
                        Collisions[I] = MakeCollision();
                        Triggers[I]   = {};
                    }
                }
                
                if(Collision.IsDynamic){
                    Collisions[BIndex]  = MakeBCollision(Collision, ObjectA);
                }
            }else if(Trigger.IsValid){
                Triggers[ItA.I] = Trigger;
            }
        }
        
        //~ Solve collisions
        
        f32 COR = 1.0f;
        FrameTimeRemaining -= FrameTimeRemaining*CurrentTimeOfImpact;
        FOR_BUCKET_ARRAY(It, &Objects){
            if(PhysicsDebugger.DefineStep()) return;
            
            physics_collision *Collision = &Collisions[It.I];
            
            dynamic_physics_object *ObjectA = It.Item;
            
            ObjectA->P += CurrentTimeOfImpact*ObjectA->Delta;
            ObjectA->Delta -= ObjectA->Delta*CurrentTimeOfImpact;
            if(PhysicsDebugger.IsCurrent()){
                PhysicsDebugger.DrawPoint(ObjectA->P, V20, YELLOW);
            }
            
            if(Triggers[It.I].IsValid){
                trigger_physics_object *Trigger = Triggers[It.I].Trigger;
                Trigger->TriggerResponse(Trigger->Entity, ObjectA->Entity);
                if(PhysicsDebugger.IsCurrent()){
                    PhysicsDebugger.DrawString("Entity hit trigger");
                }
            }
            
            if(Collisions[It.I].TimeOfImpact < 1.0f){
                
                if(ObjectA->Response(ObjectA->Entity, Collision)){
                    if(PhysicsDebugger.IsCurrent()){
                        PhysicsDebugger.DrawString("Entity handled collision");
                    }
                }else{
                    physics_object *ObjectB = Collision->ObjectB;
                    
                    if(Dot(ObjectA->Delta, Collision->Normal) < 0.0f){
                        ObjectA->dP       -= COR*Collision->Normal*Dot(ObjectA->dP, Collision->Normal);
                        ObjectA->TargetdP -= COR*Collision->Normal*Dot(ObjectA->TargetdP, Collision->Normal);
                        ObjectA->Delta    -= COR*Collision->Normal*Dot(ObjectA->Delta, Collision->Normal);
                    }
                    
                    f32 CorrectionPercent = ObjectA->Mass / (1/ObjectA->Mass + 1/ObjectB->Mass);
                    if(ObjectA->Mass == F32_POSITIVE_INFINITY) { CorrectionPercent = 0.0f; }
                    ObjectA->P += CorrectionPercent*Collision->Correction;
                    
                    if(Collision->Normal.Y > WALKABLE_STEEPNESS){
                        ObjectA->State &= ~PhysicsObjectState_Falling;
                    }
                    
                    //~ DEBUG
                    if(PhysicsDebugger.IsCurrent()){
                        PhysicsDebugger.DrawString("Yes collision");
                        PhysicsDebugger.DrawPoint(ObjectA->P-CorrectionPercent*Collision->Correction, V20, YELLOW);
                        PhysicsDebugger.DrawPoint(ObjectA->P, V20, WHITE);
                        if(ObjectB) { 
                            PhysicsDebugger.DrawPoint(ObjectB->P, V20, DARK_GREEN);
                            PhysicsDebugger.DrawNormal(ObjectB->P, V20, Collision->Normal, PINK);
                            PhysicsDebugger.DrawString("MassA: %f, MassB: %f", ObjectA->Mass, ObjectB->Mass);
                        }
                        PhysicsDebugger.DrawString("CurrentTimeOfImpact: %f", CurrentTimeOfImpact);
                        PhysicsDebugger.DrawString("TimeOfImpact: %f", Collision->TimeOfImpact);
                        PhysicsDebugger.DrawString("Correction: (%f, %f)", Collision->Correction.X, Collision->Correction.Y);
                        PhysicsDebugger.DrawString("Along delta: %f", Collision->AlongDelta);
                    }
                }
                
                Collisions[It.I] = MakeCollision();
            }
        }
        
        if(PhysicsDebugger.DefineStep()) return;
    }
    
    //~ Do floor raycasts
    FOR_BUCKET_ARRAY(It, &Objects){
        dynamic_physics_object *Object = It.Item;
        if(!(Object->State & PhysicsObjectState_DontFloorRaycast) &&
           !(Object->State & PhysicsObjectState_Falling)){
            DoFloorRaycast(Object);
        }
        
        if(PhysicsDebugger.DefineStep()) return;
    }
    
    //~ Do particles
#define RepeatExpr(V) F32X4(V, V, V, V)
    
    // TODO(Tyler): This isn't a very good particle system. We might want 
    // particle collisions though which might be hard with this system
    FOR_BUCKET_ARRAY(It, &ParticleSystems){
        physics_particle_system *System = It.Item;
        f32 COR = System->COR;
        
        for(u32 I=0; I < System->Particles.Count; I++){
            physics_particle_x4 *Particle = &System->Particles[I];
            
            f32 BaseLifetime = 0.5f;
            f32_x4 BaseLifetimeX4 = F32X4(BaseLifetime);
            
            f32_x4 dTimeX4 = F32X4(dTime);
            f32_x4 DragCoefficient = F32X4(0.7f);
            v2_x4 ddP = V2X4(F32X4(0.0f), F32X4(-11.0f));
            ddP.X += -DragCoefficient*Particle->dP.X*AbsoluteValue(Particle->dP.X);
            ddP.Y += -DragCoefficient*Particle->dP.Y*AbsoluteValue(Particle->dP.Y);
            v2_x4 Delta = (dTimeX4*Particle->dP + 
                           F32X4(0.5f)*Square(dTimeX4)*ddP);
            Particle->dP += dTimeX4*ddP;
            
            Particle->P += Delta;
            
            v2_x4 StartP = V2X4(System->P);
            v2_x4 StartdP = V2X4(System->StartdP);
            
            u32 Seed = (u32)(u64)Particle;
            f32_x4 RandomPX = StartP.X + RepeatExpr(GetRandomFloat(Seed+=(u32)__rdtsc()>>3, 12, 0.1f));
            f32_x4 RandomPY = StartP.Y + RepeatExpr(GetRandomFloat(Seed+=(u32)__rdtsc()>>3, 13, 0.1f));
            f32_x4 RandomdPX = StartdP.X + RepeatExpr(GetRandomFloat(Seed+=(u32)__rdtsc()>>3, 19, 0.2f));
            f32_x4 RandomdPY = StartdP.Y + RepeatExpr(GetRandomFloat(Seed+=(u32)__rdtsc()>>3, 17, 0.2f));
            f32_x4 RandomLifetimes = BaseLifetimeX4 + RepeatExpr(GetRandomFloat(Seed+=Seed, 5, 0.2f));
            
            __m128 M = _mm_cmple_ps(Particle->Lifetime.V, _mm_setzero_ps());
            Particle->Lifetime.V = _mm_or_ps(_mm_and_ps(M, RandomLifetimes.V), 
                                             _mm_andnot_ps(M, Particle->Lifetime.V));
            Particle->P.X.V = _mm_or_ps(_mm_and_ps(M, RandomPX.V), 
                                        _mm_andnot_ps(M, Particle->P.X.V));
            Particle->P.Y.V = _mm_or_ps(_mm_and_ps(M, RandomPY.V), 
                                        _mm_andnot_ps(M, Particle->P.Y.V));
            Particle->dP.X.V = _mm_or_ps(_mm_and_ps(M, RandomdPX.V), 
                                         _mm_andnot_ps(M, Particle->dP.X.V));
            Particle->dP.Y.V = _mm_or_ps(_mm_and_ps(M, RandomdPY.V), 
                                         _mm_andnot_ps(M, Particle->dP.Y.V));
            Particle->Lifetime -= dTimeX4;
            
            // TODO(Tyler): The rendering here is one of the slowest parts
            for(u32 I=0; I < 4; I++){
                f32 Lifetime = GetOneF32(Particle->Lifetime, I);
                if(Lifetime > 0.0f){
                    v2 P = V2(GetOneF32(Particle->P.X, I), GetOneF32(Particle->P.Y, I));
                    color C = Color(0.6f, 0.5f, 0.3f, 1.0f);
                    C = Alphiphy(C, Lifetime/BaseLifetime);
                    RenderRect(CenterRect(P, V2(0.07f)), -10.0f, C, &GameCamera);
                }
            }
            
        }
    }
#undef RepeatExpr
    
    // DEBUG
    PhysicsDebugger.End();
}

inline void
entity_manager::DoPhysics(){
    PhysicsSystem.DoPhysics();
}