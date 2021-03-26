physics_debugger PhysicsDebugger;

//~ Debug stuff
inline void
physics_debugger::Begin(){
    Current = {};
    Layout = CreateLayout(200, 300, 30, DebugFont.Size, 100, -10.2f);
    Origin = V2(4.0f, 4.0f);
}

inline b8
physics_debugger::DefineStep(){
    if(DoDebug){
        if(Current.Position >= Paused.Position &&
           (Flags & PhysicsDebuggerFlags_StepPhysics)){
            return(true);
        }
        
        Current.Position++;
        
        if(++Current.Object > Paused.Object){
            Paused.Object = Current.Object;
        }
    }
    
    return(false);
}

inline void
physics_debugger::BreakWhen(b8 Value){
    if(!(Flags & PhysicsDebuggerFlags_StepPhysics) && 
       Value){
        Flags |= PhysicsDebuggerFlags_StepPhysics;
        Paused = Current;
        Paused.Position++;
    }
}

inline b8 
physics_debugger::IsCurrent(){
    b8 Result = (DoDebug &&
                 (PhysicsDebugger.Current.Object == PhysicsDebugger.Paused.Object));
    return(Result);
}

// TODO(Tyler): Using GameCamera.P here in order to combat the point being moved
// by the camera is not good.
inline void
physics_debugger::DrawPoint(v2 Offset, v2 Point, color Color){
    if(!(Flags & PhysicsDebuggerFlags_StepPhysics)) return;
    RenderCenteredRectangle(Offset-GameCamera.P + Scale*Point, V2(0.05f, 0.05f),
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
    Delta = Normalize(Delta); // A bit of a waste for most cases
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

//~ Boundary management
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
            RenderRectangle(Offset+Boundary->Bounds.Min, Offset+Boundary->Bounds.Max, Z, 
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
    RenderRectangleOutlineMinMax(Offset+Boundary->Bounds.Min, Offset+Boundary->Bounds.Max, Z-0.1f,
                                 RED, Camera, 0.015f);
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

//~ Helpers
internal inline physics_collision
MakeDummyCollision(){
    physics_collision Result = {};
    Result.TimeOfImpact = F32_POSITIVE_INFINITY;
    return(Result);
}

//~ Physics

void
physics_system::Initialize(memory_arena *Arena){
    BucketArrayInitialize(&Objects, Arena);
    BucketArrayInitialize(&StaticObjects, Arena);
    BoundaryMemory          = PushNewArena(Arena, 3*128*sizeof(collision_boundary));
    PermanentBoundaryMemory = PushNewArena(Arena, 128*sizeof(collision_boundary));
}

void
physics_system::Reload(u32 Width, u32 Height){
    BucketArrayRemoveAll(&Objects);
    BucketArrayRemoveAll(&StaticObjects);
    ClearArena(&BoundaryMemory);
    
    PhysicsDebugger.Paused = {};
    PhysicsDebugger.StartOfPhysicsFrame = true;
}

dynamic_physics_object *
physics_system::AddObject(collision_boundary *Boundaries, u8 Count){
    dynamic_physics_object *Result = BucketArrayAlloc(&Objects);
    Result->Boundaries = Boundaries;
    Result->BoundaryCount = Count;
    Result->Mass = 1.0f; // Default mass
    
    return(Result);
}

static_physics_object *
physics_system::AddStaticObject(collision_boundary *Boundaries, u8 Count){
    static_physics_object *Result = BucketArrayAlloc(&StaticObjects);
    Result->Boundaries = Boundaries;
    Result->BoundaryCount = Count;
    
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

internal inline v2
GJKSupport(collision_boundary *Boundary, 
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
    // TODO(Tyler): Currently this only supports single collision boundaries
    v2 Result = GJKSupport(BoundaryA, AP, Delta, Direction) - GJKSupport(BoundaryB, BP, V20, -Direction);
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
                    *Direction = TripleProduct(Simplex[0]-Simplex[1], V2(0.01f, 0.0f)-Simplex[1]);
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
            // TODO(Tyler): This is a place that could be optimized
            
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
DoGJK(v2 Simplex[3], collision_boundary *BoundaryA, v2 AP, collision_boundary *BoundaryB, v2 BP, v2 Delta){
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
        if(PhysicsDebugger.DefineStep()){ return(false); }
        if(PhysicsDebugger.IsCurrent()){
            PhysicsDebugger.DrawPoint(AP, V20, WHITE);
            PhysicsDebugger.DrawPoint(BP, V20, BLUE);
            PhysicsDebugger.DrawLineFrom(PhysicsDebugger.Origin, V20, Delta, ORANGE);
            
            v2 Normal = Normalize(Direction);
            PhysicsDebugger.DrawNormal(PhysicsDebugger.Origin, PhysicsDebugger.Base, Normal, PINK);
            
            PhysicsDebugger.DrawString("SimplexCount: %u", SimplexCount);
            for(u32 I=0; I < SimplexCount; I++){
                u32 J = (I+1)%SimplexCount;
                PhysicsDebugger.DrawLine(PhysicsDebugger.Origin, Simplex[I], Simplex[J], PURPLE);
                PhysicsDebugger.DrawPoint(PhysicsDebugger.Origin, Simplex[I], GREEN);
            }
        }
        
        
        v2 NewPoint = CalculateSupport(BoundaryA, AP, BoundaryB, BP, Delta, Direction);
        if(Dot(NewPoint, Direction) < 0.0f){ 
            // The new point is in the wrong direction because there is no point going the right direction
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

// Expanding polytope algorithm that expands the poltope in the direction of the difference of the velocities,
// This requires a GJK with a sweeping support function
internal epa_result
DoVelocityEPA(collision_boundary *BoundaryA, v2 AP, collision_boundary *BoundaryB, v2 BP, v2 Delta, v2 Simplex[3]){
    TIMED_FUNCTION();
    
    const f32 Epsilon = 0.0001f;
    
    dynamic_array<v2> Polytope; 
    DynamicArrayInitialize(&Polytope, 20, &TransientStorageArena);
    DynamicArrayPushBack(&Polytope, Simplex[0]);
    DynamicArrayPushBack(&Polytope, Simplex[1]);
    DynamicArrayPushBack(&Polytope, Simplex[2]);
    
    v2 DeltaNormal = Normalize(Clockwise90(Delta));
    v2 DeltaDirection = Normalize(Delta);
    
    epa_result Result = {};
    while(true){
        b8 FoundEdge = false;
        b8 IsColinear = false;
        u32 EdgeIndex = 0;
        f32 EdgeDistance = 0;
        v2 InverseNormal = {};
        v2 IntersectionPoint = {};
        
        for(u32 I=0; I < Polytope.Count; I++){
            v2 A = Polytope[I];
            v2 B = Polytope[(I+1)%Polytope.Count];
            
            f32 AAlongDeltaNormal = Dot(DeltaNormal, A);
            f32 BAlongDeltaNormal = Dot(DeltaNormal, B);
            
            if(((-Epsilon <= AAlongDeltaNormal) && (AAlongDeltaNormal <= Epsilon)) && // Collinear
               ((-Epsilon <= BAlongDeltaNormal) && (BAlongDeltaNormal <= Epsilon))){
                FoundEdge = true;
                IsColinear = true;
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
                        FoundEdge = true;
                        EdgeIndex = I;
                        InverseNormal = Normalize(TripleProduct(B-A, -A));
                        EdgeDistance = Dot(InverseNormal, -A);
                        IntersectionPoint = Point;
                        break;
                    }else if(!IsColinear){
                        FoundEdge = true;
                        EdgeIndex = I;
                        InverseNormal = Normalize(TripleProduct(B-A, -A));
                        EdgeDistance = Dot(InverseNormal, -A);
                        IntersectionPoint = Point;
                    }
                }
            }
        }
        PhysicsDebugger.BreakWhen(!FoundEdge);
        
        f32 Distance;
        v2 NewPoint = V20;
        if((InverseNormal.X == 0.0f) && (InverseNormal.Y == 0.0f)){
            Distance = EdgeDistance;
        }else{
            NewPoint = CalculateSupport(BoundaryA, AP, BoundaryB, BP, Delta, -InverseNormal);
            Distance = Dot(NewPoint, -InverseNormal);
        }
        
        // DEBUG
        if(PhysicsDebugger.DefineStep()){  return(Result); }
        if(PhysicsDebugger.IsCurrent()){
            PhysicsDebugger.DrawPoint(AP, V20, WHITE);
            PhysicsDebugger.DrawPoint(BP, V20, BLUE);
            PhysicsDebugger.DrawLineFrom(PhysicsDebugger.Origin, V20, Delta, ORANGE);
            
            for(u32 I=0; I < Polytope.Count; I++){
                u32 J = (I+1)%Polytope.Count;
                PhysicsDebugger.DrawLine(PhysicsDebugger.Origin, Polytope[I], Polytope[J], PURPLE);
                PhysicsDebugger.DrawPoint(PhysicsDebugger.Origin, Polytope[I], GREEN);
            }
            
            f32 Percent = Dot(DeltaDirection, IntersectionPoint)/Dot(DeltaDirection, Delta);
            if((Delta.X == 0.0f) && (Delta.Y == 0.0f)){
                Percent = 0.0f;
            }
            
            v2 Base = 0.5f*(Polytope[EdgeIndex] + Polytope[(EdgeIndex+1)%Polytope.Count]);
            
            PhysicsDebugger.DrawNormal(PhysicsDebugger.Origin, Base, -InverseNormal, PINK);
            PhysicsDebugger.DrawNormal(PhysicsDebugger.Origin, Base,  InverseNormal, YELLOW);
            PhysicsDebugger.DrawPoint(PhysicsDebugger.Origin, IntersectionPoint, ORANGE);
            
            PhysicsDebugger.DrawString("Polytope.Count: %u", Polytope.Count);
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
                v2 Difference = IntersectionPoint - Delta;
                Result.Correction = InverseNormal * Dot(-InverseNormal, Difference);
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
    
    if(PhysicsDebugger.DefineStep()){ return(Result); 
    }
    if(PhysicsDebugger.IsCurrent()){
        PhysicsDebugger.DrawPoint(BP, V20, BLUE);
        for(u32 I=0; I < Polytope.Count; I++){
            u32 J = (I+1)%Polytope.Count;
            PhysicsDebugger.DrawLine(PhysicsDebugger.Origin, Polytope[I], Polytope[J], PURPLE);
            PhysicsDebugger.DrawPoint(PhysicsDebugger.Origin, Polytope[I], GREEN);
            
            if(PhysicsDebugger.Flags & PhysicsDebuggerFlags_StepPhysics){
                v2 P = PhysicsDebugger.Origin + (PhysicsDebugger.Scale * Polytope[I]);
                P.Y += 0.1f;
                v2 NameP = GameCamera.WorldPToScreenP(P);
                RenderFormatString(&DebugFont, BLACK, NameP.X, NameP.Y, -10.0f, "%u", I);
            }
        }
        
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
    local_constant f32 Epsilon = 0.0001f;
    
    physics_collision Result = {};
    Result.AlongDelta        = Dot(Normalize(Delta), BP-AP);
    Result.TimeOfImpact      = F32_POSITIVE_INFINITY;
    
    if((-Epsilon <= Delta.X) && (Delta.X <= Epsilon) &&
       (-Epsilon <= Delta.Y) && (Delta.Y <= Epsilon)){
        return(Result);
    }
    
    v2 Simplex[3];
    if(DoGJK(Simplex, BoundaryA, AP, BoundaryB, BP, Delta)){
        epa_result EPAResult = DoVelocityEPA(BoundaryA, AP, BoundaryB, BP, Delta, Simplex);
        Result.Normal            = EPAResult.Normal;
        Result.Correction        = EPAResult.Correction;
        Result.TimeOfImpact      = EPAResult.TimeOfImpact;
    }
    
    return(Result);
}

void 
physics_system::DoStaticCollisions(physics_collision *OutCollision, collision_boundary *Boundary, v2 P, v2 Delta){
    local_constant f32 Epsilon = 0.0001f;
    
    FOR_BUCKET_ARRAY(ItB, &StaticObjects){
        static_physics_object *ObjectB = ItB.Item;
        
        v2 Simplex[3];
        physics_collision Collision = DoCollision(Boundary, P, ObjectB->Boundaries, ObjectB->P, Delta);
        Collision.ObjectB = ObjectB;
        if(Collision.TimeOfImpact < OutCollision->TimeOfImpact){
            *OutCollision = Collision;
        }else if((Collision.TimeOfImpact > OutCollision->TimeOfImpact-Epsilon) && 
                 (Collision.TimeOfImpact < OutCollision->TimeOfImpact+Epsilon)){
            if(Collision.AlongDelta < OutCollision->AlongDelta){
                *OutCollision = Collision;
            }
        }
    }
}

void
physics_system::DoCollisionsRelative(physics_collision *OutCollision, collision_boundary *Boundary, v2 P, v2 Delta, bucket_location StartLocation={}){
    local_constant f32 Epsilon = 0.0001f;
    
    StartLocation.ItemIndex++;
    FOR_BUCKET_ARRAY_FROM(ItB, &Objects, StartLocation){
        //PhysicsDebugger.DoDebug = ObjectA->DebugInfo.DebugThisOne || ItB.Item->DebugInfo.DebugThisOne;
        dynamic_physics_object *ObjectB = ItB.Item;
        
        v2 RelativeDelta = Delta-ObjectB->Delta;
        
        v2 Simplex[3];
        physics_collision Collision = DoCollision(Boundary, P, ObjectB->Boundaries, ObjectB->P, RelativeDelta);
        Collision.ObjectB = ObjectB;
        Collision.IsDynamic = true;
        
        if(Collision.TimeOfImpact < OutCollision->TimeOfImpact){
            *OutCollision = Collision;
        }else if((Collision.TimeOfImpact > OutCollision->TimeOfImpact-Epsilon) && 
                 (Collision.TimeOfImpact < OutCollision->TimeOfImpact+Epsilon)){
            if(Collision.AlongDelta < OutCollision->AlongDelta){
                *OutCollision = Collision;
            }
        }
        
        //PhysicsDebugger.DoDebug = ObjectA->DebugInfo.DebugThisOne;
    }
}

void
physics_system::DoCollisionsNotRelative(physics_collision *OutCollision, collision_boundary *Boundary, v2 P, v2 Delta, physics_object *SkipObject=0){
    local_constant f32 Epsilon = 0.0001f;
    
    FOR_BUCKET_ARRAY(ItB, &Objects){
        //PhysicsDebugger.DoDebug = ObjectA->DebugInfo.DebugThisOne || ItB.Item->DebugInfo.DebugThisOne;
        dynamic_physics_object *ObjectB = ItB.Item;
        
        if(ObjectB == SkipObject) continue;
        
        v2 Simplex[3];
        physics_collision Collision = DoCollision(Boundary, P, ObjectB->Boundaries, ObjectB->P, Delta);
        Collision.ObjectB = ObjectB;
        Collision.IsDynamic = true;
        
        if(Collision.TimeOfImpact < OutCollision->TimeOfImpact){
            *OutCollision = Collision;
        }else if((Collision.TimeOfImpact > OutCollision->TimeOfImpact-Epsilon) && 
                 (Collision.TimeOfImpact < OutCollision->TimeOfImpact+Epsilon)){
            if(Collision.AlongDelta < OutCollision->AlongDelta){
                *OutCollision = Collision;
            }
        }
        
        //PhysicsDebugger.DoDebug = ObjectA->DebugInfo.DebugThisOne;
    }
}

void
physics_system::DoFloorRaycast(dynamic_physics_object *Object, f32 Depth=0.2f){
    PhysicsDebugger.DoDebug = true;
    
    if(PhysicsDebugger.DefineStep()){ return; }
    if(PhysicsDebugger.IsCurrent()){
        PhysicsDebugger.DrawString("No collision");
    }
    
    v2 Raycast = V2(0, -Depth);
    physics_collision Collision = MakeDummyCollision();
    DoStaticCollisions(&Collision, Object->Boundaries, Object->P, Raycast);
    DoCollisionsNotRelative(&Collision, Object->Boundaries, Object->P, Raycast, Object);
    
    if(PhysicsDebugger.DefineStep()){ return; }
    if(PhysicsDebugger.IsCurrent()){
        PhysicsDebugger.DrawString("No collision");
    }
    
    if(Collision.TimeOfImpact < 1.0f){
        if(!(Object->State & PhysicsObjectState_Falling)){
            Object->P += Raycast*Collision.TimeOfImpact;
            Object->P += Collision.Correction;
            Object->dP -= Collision.Normal*Dot(Object->dP, Collision.Normal);
            Object->Delta -= Collision.Normal*Dot(Object->Delta, Collision.Normal);
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
                PhysicsDebugger.DrawPoint(ObjectB->P, V20, BLUE);
                PhysicsDebugger.DrawNormal(ObjectB->P, V20, Collision.Normal, PINK);
                PhysicsDebugger.DrawString("TimeOfImpact: %f", Collision.TimeOfImpact);
                PhysicsDebugger.DrawString("Correction: (%f, %f)", Collision.Correction.X, Collision.Correction.Y);
                PhysicsDebugger.DrawString("MassA: %f, MassB: %f", Object->Mass, ObjectB->Mass);
            }
        }
    }else{
        Object->State |= PhysicsObjectState_Falling;
    }
    
}

void
physics_system::DoPhysics(){
    TIMED_FUNCTION();
    local_constant f32 Epsilon = 0.0001f;
    
    // DEBUG prepare for new physics frame
    PhysicsDebugger.Begin();
    
    f32 dTime = OSInput.dTime;
    
    dynamic_physics_object **DynamicPhysicsObjectBs = PushArray(&TransientStorageArena, dynamic_physics_object *, Objects.Count);
    physics_collision *Collisions = PushArray(&TransientStorageArena, physics_collision, Objects.Count);
    b8 *DidCollides               = PushArray(&TransientStorageArena, b8, Objects.Count);
    f32 *Masses                   = PushArray(&TransientStorageArena, f32, Objects.Count);
    
    FOR_BUCKET_ARRAY(It, &Objects){
        dynamic_physics_object *Object = It.Item;
        
        // NOTE(Tyler): This may not be the best way to integrate dP, Delta, but it works
        f32 DragCoefficient = 1.0f;
        Object->ddP.X += -DragCoefficient*Object->dP.X*AbsoluteValue(Object->dP.X);
        Object->ddP.Y += -DragCoefficient*Object->dP.Y*AbsoluteValue(Object->dP.Y);
        v2 ReferencedP = V20;
        if(Object->ReferenceFrame){ 
            ReferencedP = dTime*Object->ReferenceFrame->dP;
        }
        Object->Delta = (dTime*Object->dP + 
                         ReferencedP +
                         0.5f*Square(dTime)*Object->ddP);
        Object->dP += dTime*Object->ddP;
        Object->ddP = {};
        
        // DEBUG
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
            
            if(IsPointInBoundary(GameCamera.ScreenPToWorldP(OSInput.MouseP), Object->Boundaries, Object->P)){
                PhysicsDebugger.DrawPoint(Object->P, V20, RED);
                PhysicsDebugger.DrawPoint(V2(Object->P.X, Object->P.Y+0.4f), V20, RED);
                PhysicsDebugger.DrawLineFrom(Object->P, V20,  Object->Delta, GREEN);
                v2 P = Object->P;
                P.Y += 0.2f;
                v2 StringP = GameCamera.WorldPToScreenP(P);;
                RenderFormatString(&DebugFont, YELLOW, StringP.X, StringP.Y, -12.0f, "(%f, %f)", Object->Delta.X, Object->Delta.Y);
            }
        }
        
#if defined(SNAIL_JUMPY_DEBUG_BUILD)
        if(DebugConfig.Overlay & DebugOverlay_Boundaries){
            RenderBoundary(&GameCamera, Object->Boundaries, -10.0f, Object->P);
        }
#endif
    }
    
    // TODO(Tyler): Move this into physics_debugger
    if(PhysicsDebugger.StartOfPhysicsFrame){
        PhysicsDebugger.StartOfPhysicsFrame = false;
    }
    
#if defined(SNAIL_JUMPY_DEBUG_BUILD)
    if(DebugConfig.Overlay & DebugOverlay_Boundaries){
        FOR_BUCKET_ARRAY(It, &StaticObjects){
            static_physics_object *Object = It.Item;
            RenderBoundary(&GameCamera, Object->Boundaries, -10.0f, Object->P);
        }
    }
#endif
    
    u32 IterationsToDo = Objects.Count*PHYSICS_ITERATIONS_PER_OBJECT;
    f32 FrameTimeRemaining = 1.0f;
    u32 Iteration = 0;
    while((FrameTimeRemaining > 0) &&
          (Iteration < IterationsToDo)){
        Iteration++;
        
        f32 CurrentTimeOfImpact = 1.0f;
        u32 AIndex = 0;
        
        FOR_BUCKET_ARRAY(ItA, &Objects){
            dynamic_physics_object *ObjectA = ItA.Item;
            //ObjectA->State |= PhysicsObjectState_Falling;
            
            DidCollides[AIndex] = false;
            Collisions[AIndex] = {};
            Masses[AIndex] = ObjectA->Mass;
            
            // DEBUG
            // TODO(Tyler): Move this into physics_debugger
            PhysicsDebugger.DoDebug = ObjectA->DebugInfo.DebugThisOne;
            PhysicsDebugger.DrawPoint(PhysicsDebugger.Origin, V20, WHITE);
            
            physics_collision Collision = MakeDummyCollision();
            DoStaticCollisions(&Collision, ObjectA->Boundaries, ObjectA->P, ObjectA->Delta);
            DoCollisionsRelative(&Collision, ObjectA->Boundaries, ObjectA->P, ObjectA->Delta, ItA.Location);
            
            if(Collision.IsDynamic){
                DynamicPhysicsObjectBs[AIndex] = (dynamic_physics_object *)Collision.ObjectB;
            }
            if(Collision.TimeOfImpact < CurrentTimeOfImpact){
                CurrentTimeOfImpact = Collision.TimeOfImpact;
                Collisions[AIndex]  = Collision;
                DidCollides[AIndex] = true;
                for(u32 I=0; I<AIndex; I++){
                    Collisions[I] = {};
                    DidCollides[I] = false;
                }
            }else if((Collision.TimeOfImpact > CurrentTimeOfImpact-Epsilon) && 
                     (Collision.TimeOfImpact < CurrentTimeOfImpact+Epsilon)){
                if(DidCollides[AIndex]){
                    if(Collision.AlongDelta < Collisions[AIndex].AlongDelta){
                        DidCollides[AIndex] = true;
                        Collisions[AIndex]  = Collision;
                    }
                }else{
                    DidCollides[AIndex] = true;
                    Collisions[AIndex]  = Collision;
                }
            }
            
            AIndex++;
        }
        
        // TODO(Tyler): Move into physics_debugger
        PhysicsDebugger.DoDebug = true;
        f32 COR = 1.0f;
        FrameTimeRemaining -= FrameTimeRemaining*CurrentTimeOfImpact;
        {
            u32 Index = 0;
            FOR_BUCKET_ARRAY(It, &Objects){
                physics_collision *Collision = &Collisions[Index];
                
                dynamic_physics_object *ObjectA = It.Item;
                
                if(DidCollides[Index]){
                    f32 Steepness = 0.1f;
                    if(Collision->Normal.Y > Steepness){
                        ObjectA->State &= ~PhysicsObjectState_Falling;
                    }
                    
                    physics_collision *Collision = &Collisions[Index];
                    physics_object *ObjectB = Collision->ObjectB;
                    
                    ObjectA->P += CurrentTimeOfImpact*ObjectA->Delta;
                    ObjectA->Delta -= ObjectA->Delta*CurrentTimeOfImpact;
                    
                    
                    f32 CorrectionPercent = ObjectA->Mass / (1/ObjectA->Mass + 1/ObjectB->Mass);
                    if(ObjectA->Mass == F32_POSITIVE_INFINITY) { CorrectionPercent = 0.0f; }
                    ObjectA->Delta += CorrectionPercent*Collision->Correction;
                    
                    
                    ObjectA->dP    -= COR*Collision->Normal*Dot(ObjectA->dP, Collision->Normal);
                    ObjectA->Delta -= COR*Collision->Normal*Dot(ObjectA->Delta, Collision->Normal);
                    
                    if(PhysicsDebugger.DefineStep()){ return; }
                    if(PhysicsDebugger.IsCurrent()){
                        PhysicsDebugger.DrawString("Yes collision");
                        PhysicsDebugger.DrawPoint(ObjectA->P, V20, WHITE);
                        if(ObjectB) { 
                            PhysicsDebugger.DrawPoint(ObjectB->P, V20, BLUE);
                            PhysicsDebugger.DrawNormal(ObjectB->P, V20, Collision->Normal, PINK);
                            PhysicsDebugger.DrawString("MassA: %f, MassB: %f", ObjectA->Mass, ObjectB->Mass);
                        }
                        PhysicsDebugger.DrawString("CurrentTimeOfImpact: %f", CurrentTimeOfImpact);
                        PhysicsDebugger.DrawString("TimeOfImpact: %f", Collision->TimeOfImpact);
                        PhysicsDebugger.DrawString("Correction: (%f, %f)", Collision->Correction.X, Collision->Correction.Y);
                    }
                    
                }else{
                    ObjectA->P  += CurrentTimeOfImpact*ObjectA->Delta;
                    ObjectA->Delta -= CurrentTimeOfImpact*ObjectA->Delta;
                }
                
                Index++;
            }
        }
        
        // So that we don't update the position of ObjectB until after its actually collided
        for(u32 I=0; I < Objects.Count; I++){
            physics_collision *Collision = &Collisions[I];
            dynamic_physics_object *ObjectB = DynamicPhysicsObjectBs[I];
            
            if(DidCollides[I] && ObjectB){
                //physics_object *ObjectA = Collision->ObjectA;
                
                f32 CorrectionPercent = ObjectB->Mass / ( Masses[I] + ObjectB->Mass);
                if(ObjectB->Mass == F32_POSITIVE_INFINITY) { CorrectionPercent = 0.0f; }
                ObjectB->P -= CorrectionPercent*Collision->Correction;
                
                if(Dot(ObjectB->Delta, Collision->Normal) > 0){
                    ObjectB->dP    -= COR*Collision->Normal*Dot(ObjectB->dP, Collision->Normal);
                    ObjectB->Delta -= COR*Collision->Normal*Dot(ObjectB->Delta, Collision->Normal);
                }
            }
        }
        
        if(PhysicsDebugger.DefineStep()){
            return;
        }
    }
    
    FOR_BUCKET_ARRAY(It, &Objects){
        dynamic_physics_object *Object = It.Item;
        DoFloorRaycast(Object);
        
        if(PhysicsDebugger.DefineStep()){
            return;
        }
    }
    
    // DEBUG
    PhysicsDebugger.Paused = {};
    PhysicsDebugger.StartOfPhysicsFrame = true;
}

inline void
entity_manager::DoPhysics(){
    PhysicsSystem.DoPhysics();
}