
collision_boundary *WallBoundary;
collision_boundary *CoinBoundary;
collision_boundary *ProjectileBoundary;

collision_boundary *DEBUGPlayerBoundary;
collision_boundary *DEBUGWedgeBoundary;
collision_boundary *DEBUGCircleBoundary;

physics_debugger PhysicsDebugger;

//~ Debug stuff
inline void
physics_debugger::NewFrame(){
    PhysicsDebugger.Current = {};
    Layout = CreateLayout(200, 300, 30, DebugFont.Size, 100, -10.2f);
}

inline void
physics_debugger::AdvanceCurrentObject(){
    if(++Current.Object > Paused.Object){
        Paused.Object = Current.Object;
    }
}

inline b8 
physics_debugger::AdvanceCurrentPosition(){
    b8 Result = ((Flags & PhysicsDebuggerFlags_StepPhysics) &&
                 (Current.Position >= Paused.Position));
    Current.Position++;
    return(Result);
}

inline b8 
physics_debugger::TestPosition(){
    b8 Result = ((Flags & PhysicsDebuggerFlags_StepPhysics) &&
                 (Current.Position >= Paused.Position));
    return(Result);
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

//~ Boundary management
internal inline b8
IsPointInBoundary(v2 Point, collision_boundary *Boundary, v2 Base=V2(0,0)){
    b8 Result = false;
    switch(Boundary->Type){
        case BoundaryType_None: break;
        case BoundaryType_Rect: {
            Result = IsV2InRectangle(Point, OffsetRect(Boundary->Bounds, Base+Boundary->Offset));
        }break;
        case BoundaryType_Circle: {
            f32 Distance = SquareRoot(LengthSquared(Point-(Base+Boundary->Offset)));
            Result = (Distance <= Boundary->Radius);
        }break;
        case BoundaryType_Wedge: {
            NOT_IMPLEMENTED_YET
        }break;
        default: INVALID_CODE_PATH;
    }
    return(Result);
}

internal inline void
RenderBoundary(camera *Camera, collision_boundary *Boundary, f32 Z, v2 Offset){
    switch(Boundary->Type){
        case BoundaryType_None: break;
        case BoundaryType_Rect: {
            v2 P = Offset + Boundary->Offset;
            RenderRectangle(P+Boundary->Bounds.Min, P+Boundary->Bounds.Max, Z, 
                            Color(1.0f, 0.0f, 0.0f, 0.5f), Camera);
        }break;
        case BoundaryType_Circle: {
            RenderCircle(Offset+Boundary->Offset, Boundary->Radius, Z,
                         Color(1.0f, 0.0f, 0.0f, 0.5f), Camera);
        }break;
        default: INVALID_CODE_PATH;
    }
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
MakeCollisionCircle(v2 Offset, f32 Radius){
    collision_boundary Result = {};
    Result.Type = BoundaryType_Circle;
    Result.Offset = Offset;
    Result.Radius = Radius;
    Result.Bounds = CenterRect(V20, 2*V2(Radius, Radius));
    return(Result);
}

//~ Spacial hashing
internal void
CollisionSystemNewFrame(){
    TIMED_FUNCTION();
    
#if 0    
    
    CollisionTable.Items = PushArray(&TransientStorageArena, collision_table_item *, 
                                     CollisionTable.Width*CollisionTable.Height);
    ZeroMemory(CollisionTable.Items, 
               CollisionTable.Width*CollisionTable.Height*sizeof(collision_table_item));
    
    FOR_BUCKET_ARRAY(&EntityManager.Walls){
        wall_entity *Entity = BucketArrayGetItemPtr(&EntityManager.Walls, Location);
        collision_boundary *Boundary = &Entity->Boundary;
        
        rect_s32 Bounds = RectS32(Boundary->Bounds);
        
        for(s32 Y = Bounds.Min.Y; Y <= Bounds.Max.Y; Y++){
            for(s32 X = Bounds.Min.X; X <= Bounds.Max.X; X++){
                collision_table_item *NewItem = PushStruct(&TransientStorageArena,
                                                           collision_table_item);
                NewItem->Next = CollisionTable.Items[Y*CollisionTable.Width + X];
                CollisionTable.Items[Y*CollisionTable.Width + X] = NewItem;
                NewItem->EntityType = EntityType_Wall;
                NewItem->EntityPtr = Entity;
            }
        }
    }
    
    FOR_BUCKET_ARRAY(&EntityManager.Coins){
        coin_entity *Entity = BucketArrayGetItemPtr(&EntityManager.Coins, Location);
        collision_boundary *Boundary = &Entity->Boundary;
        
        rect_s32 Bounds = RectS32(Boundary->Bounds);
        
        for(s32 Y = Bounds.Min.Y; Y <= Bounds.Max.Y; Y++){
            for(s32 X = Bounds.Min.X; X <= Bounds.Max.X; X++){
                collision_table_item *NewItem = PushStruct(&TransientStorageArena,
                                                           collision_table_item);
                NewItem->Next = CollisionTable.Items[Y*CollisionTable.Width + X];
                CollisionTable.Items[Y*CollisionTable.Width + X] = NewItem;
                NewItem->EntityType = EntityType_Coin;
                NewItem->EntityPtr = Entity;
            }
        }
    }
    
    FOR_BUCKET_ARRAY(&EntityManager.Doors){
        door_entity *Entity = BucketArrayGetItemPtr(&EntityManager.Doors, Location);
        collision_boundary *Boundary = &Entity->Boundary;
        
        rect_s32 Bounds = RectS32(Boundary->Bounds);
        
        for(s32 Y = Bounds.Min.Y; Y <= Bounds.Max.Y; Y++){
            for(s32 X = Bounds.Min.X; X <= Bounds.Max.X; X++){
                collision_table_item *NewItem = PushStruct(&TransientStorageArena,
                                                           collision_table_item);
                NewItem->Next = CollisionTable.Items[Y*CollisionTable.Width + X];
                CollisionTable.Items[Y*CollisionTable.Width + X] = NewItem;
                NewItem->EntityType = EntityType_Door;
                NewItem->EntityPtr = Entity;
            }
        }
    }
    
    FOR_BUCKET_ARRAY(&EntityManager.Enemies){
        enemy_entity *Entity = BucketArrayGetItemPtr(&EntityManager.Enemies, Location);
        
        for(u32 I = 0; I < Entity->BoundaryCount; I++){
            collision_boundary *Boundary = &Entity->Boundaries[I];
            rect_s32 Bounds = RectS32(Boundary->Bounds);
            
            for(s32 Y = Bounds.Min.Y; Y <= Bounds.Max.Y; Y++){
                for(s32 X = Bounds.Min.X; X <= Bounds.Max.X; X++){
                    collision_table_item *NewItem = PushStruct(&TransientStorageArena,
                                                               collision_table_item);
                    NewItem->Next = CollisionTable.Items[Y*CollisionTable.Width + X];
                    CollisionTable.Items[Y*CollisionTable.Width + X] = NewItem;
                    NewItem->EntityType = Entity->Type;
                    NewItem->EntityPtr = Entity;
                }
            }
        }
    }
    
    
    FOR_BUCKET_ARRAY(&EntityManager.Projectiles){
        projectile_entity *Entity = BucketArrayGetItemPtr(&EntityManager.Projectiles, Location);
        
        for(u32 I = 0; I < Entity->BoundaryCount; I++){
            collision_boundary *Boundary = &Entity->Boundaries[I];
            rect_s32 Bounds = RectS32(Boundary->Bounds);
            
            for(s32 Y = Bounds.Min.Y; Y <= Bounds.Max.Y; Y++){
                for(s32 X = Bounds.Min.X; X <= Bounds.Max.X; X++){
                    collision_table_item *NewItem = PushStruct(&TransientStorageArena,
                                                               collision_table_item);
                    NewItem->Next = CollisionTable.Items[Y*CollisionTable.Width + X];
                    CollisionTable.Items[Y*CollisionTable.Width + X] = NewItem;
                    NewItem->EntityType = EntityType_Projectile;
                    NewItem->EntityPtr = Entity;
                }
            }
        }
    }
    
    // Player
    {
        player_entity *Entity = EntityManager.Player;
        for(u32 I = 0; I < Entity->BoundaryCount; I++){
            collision_boundary *Boundary = &Entity->Boundaries[I];
            rect_s32 Bounds = RectS32(Boundary->Bounds);
            
            for(s32 Y = Bounds.Min.Y; Y <= Bounds.Max.Y; Y++){
                for(s32 X = Bounds.Min.X; X <= Bounds.Max.X; X++){
                    collision_table_item *NewItem = PushStruct(&TransientStorageArena,
                                                               collision_table_item);
                    NewItem->Next = CollisionTable.Items[Y*CollisionTable.Width + X];
                    CollisionTable.Items[Y*CollisionTable.Width + X] = NewItem;
                    NewItem->EntityType = Entity->Type;
                    NewItem->EntityPtr = Entity;
                }
            }
        }
    }
#endif
    
}

//~ Physics

void
physics_system::Initialize(memory_arena *Arena){
    BucketArrayInitialize(&Objects, Arena);
    BucketArrayInitialize(&StaticObjects, Arena);
    BoundaryMemory = PushNewArena(Arena, 512*sizeof(collision_boundary));
    
    WallBoundary = PhysicsSystem.AllocBoundaries(1);
    *WallBoundary = MakeCollisionRect(V20, TILE_SIZE);
    //*WallBoundary = MakeCollisionCircle(V20, 0.75f*TILE_SIDE);
    
    CoinBoundary = PhysicsSystem.AllocBoundaries(1);
    *CoinBoundary = MakeCollisionRect(V20, V2(0.3f, 0.3f));
    
    ProjectileBoundary = PhysicsSystem.AllocBoundaries(1);
    *ProjectileBoundary = MakeCollisionRect(V20, V2(0.1f, 0.1f));
    
    DEBUGPlayerBoundary = PhysicsSystem.AllocBoundaries(1);
    *DEBUGPlayerBoundary = MakeCollisionRect(V20, TILE_SIZE);
    
    DEBUGWedgeBoundary = PhysicsSystem.AllocBoundaries(1);
    DEBUGWedgeBoundary->Type = BoundaryType_Wedge;
    DEBUGWedgeBoundary->Bounds.Min = V2(-0.5f, 0.0f);
    DEBUGWedgeBoundary->Bounds.Max = V2( 0.0f, 0.5f);
    DEBUGWedgeBoundary->Points[0] = V2(-0.5f,  0.0f);
    DEBUGWedgeBoundary->Points[1] = V2( 0.0f,  0.0f);
    DEBUGWedgeBoundary->Points[2] = V2( 0.0f,  0.5f);
    
    DEBUGCircleBoundary = PhysicsSystem.AllocBoundaries(1);
    DEBUGCircleBoundary->Type = BoundaryType_Circle;
    DEBUGCircleBoundary->Bounds.Min = V2(-0.5f, 0.0f);
    DEBUGCircleBoundary->Bounds.Max = V2( 0.0f, 0.5f);
    DEBUGCircleBoundary->Radius = 0.5f;
}

void
physics_system::Reload(u32 Width, u32 Height){
    SpaceTable.Width = Truncate((f32)Width / TILE_SIDE);
    SpaceTable.Height = Truncate((f32)Height / TILE_SIDE);
    BucketArrayRemoveAll(&Objects);
    BucketArrayRemoveAll(&StaticObjects);
    
    PhysicsDebugger.Paused.Position = 0;
}

physics_object *
physics_system::AddObject(collision_boundary *Boundaries, u8 Count){
    physics_object *Result = BucketArrayAlloc(&Objects);
    Result->Boundaries = Boundaries;
    Result->BoundaryCount = Count;
    
    return(Result);
}

physics_object *
physics_system::AddStaticObject(collision_boundary *Boundaries, u8 Count){
    physics_object *Result = BucketArrayAlloc(&StaticObjects);
    Result->Boundaries = Boundaries;
    Result->BoundaryCount = Count;
    
    return(Result);
}

collision_boundary *
physics_system::AllocBoundaries(u32 Count){
    collision_boundary *Result = PushArray(&BoundaryMemory, collision_boundary, Count);
    return(Result);
}

//~ GJK stuff

internal inline v2
GJKSupport(collision_boundary *Boundary, 
           physics_object *Object, v2 Delta,
           v2 Direction){
    v2 Result = {};
    
    switch(Boundary->Type){
        case BoundaryType_Rect: {
            v2 Min = Boundary->Bounds.Min;
            v2 Max = Boundary->Bounds.Max;
            v2 Points[4] = {
                Min,
                V2(Min.X, Max.Y),
                Max,
                V2(Max.X, Min.Y),
            };
            for(u32 I=0; I < 4; I++){
                if(Dot(Points[I], Direction) > Dot(Result, Direction)){
                    Result = Points[I];
                }
            }
        }break;
        case BoundaryType_Circle: {
            // TODO(Tyler): There might be a better way to do this
            if((Direction.X ==  0.0f) && (Direction.Y ==  0.0f)){ // Avoid NANs
                Direction = V2(1.0f, 0.0f);
            }
            
            f32 DirectionLength = Length(Direction);
            v2 UnitDirection = Direction/DirectionLength;
            Result = UnitDirection * Boundary->Radius;
        }break;
        case BoundaryType_Wedge: {
            for(u32 I=0; I < 3; I++){
                v2 Point = Boundary->Points[I];
                if(Dot(Point, Direction) > Dot(Result, Direction)){
                    Result = Point;
                }
            }
        }break;
        default: INVALID_CODE_PATH;
    }
    
    Result += Object->P;
    if(Dot(Delta, Direction) > 0.0f){
        Result += Delta;
    }
    
    return(Result);
}

internal inline v2 
CalculateSupport(physics_object *ObjectA, physics_object *ObjectB, v2 Delta, v2 Direction){
    // TODO(Tyler): Currently this only supports single collision boundaries
    v2 Result = GJKSupport(ObjectA->Boundaries, ObjectA, Delta, Direction) - GJKSupport(ObjectB->Boundaries, ObjectB, V20, -Direction);
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
DoGJK(v2 Simplex[3], physics_object *ObjectA, physics_object *ObjectB, v2 Delta){
    
    f32 ObjectBPAlongDelta = Dot(Delta, ObjectB->P-ObjectA->P);
    u32 SimplexCount = 1; // Account for the first support point
    v2 Direction = ObjectA->P - ObjectB->P;
    if((Direction.X == 0.0f) && (Direction.Y == 0.0f)){
        Direction = V2(1, 0);
    }
    Simplex[0] = CalculateSupport(ObjectA, ObjectB, Delta, Direction);
    Direction = -Simplex[0];
    
    b8 DoesCollide = false;
    PhysicsDebugger.Base = Simplex[0];
    while(true){
        if(PhysicsDebugger.AdvanceCurrentPosition()){
            break;
        }
        
        v2 NewPoint = CalculateSupport(ObjectA, ObjectB, Delta, Direction);
        if(Dot(NewPoint, Direction) < 0){ 
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
    
    // DEBUG
    if(PhysicsDebugger.Current.Object == PhysicsDebugger.Paused.Object){
        v2 Normal = Normalize(Direction);
        f32 Length = 0.4f;
        RenderLineFrom(PhysicsDebugger.Base+ObjectA->DebugInfo->Offset, Length*Normal, -10, 0.015f, GJK_SIMPLEX2_COLOR, &GameCamera);
        
        LayoutString(&PhysicsDebugger.Layout, &DebugFont, BLACK, "SimplexCount: %u", SimplexCount);
        v2 Offset = ObjectA->DebugInfo->Offset;
        for(u32 I=0; I < SimplexCount; I++){
            u32 J = (I+1)%SimplexCount;
            RenderLine(Offset+Simplex[I], Offset+Simplex[J], -10, 0.015f, BLUE, &GameCamera);
            RenderCenteredRectangle(Offset+Simplex[I], V2(0.025f, 0.025f), -10.1f, GREEN, &GameCamera);
        }
        
        RenderCenteredRectangle(ObjectB->P, V2(0.05f, 0.05f), -10.1f, BLUE, &GameCamera);
    }
    
    return(DoesCollide);
}

struct epa_result {
    f32 ActualTimeOfImpact;
    f32 WorkingTimeOfImpact;
    v2 Normal;
};

// Expanding polytope algorithm that expands the poltope in the direction of the difference of the velocities,
// This requires a GJK with a sweeping support function
internal epa_result
DoVelocityEPA(physics_object *ObjectA, physics_object *ObjectB, v2 Delta, v2 Simplex[3]){
    TIMED_FUNCTION();
    
    const f32 Epsilon = 0.0001f;
    
    dynamic_array<v2> Polytope; 
    DynamicArrayInitialize(&Polytope, 20, &TransientStorageArena);
    DynamicArrayPushBack(&Polytope, Simplex[0]);
    DynamicArrayPushBack(&Polytope, Simplex[1]);
    DynamicArrayPushBack(&Polytope, Simplex[2]);
    
    v2 DeltaNormal = Normalize(Clockwise90(Delta));
    
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
                InverseNormal = Normalize(TripleProduct(B-A, -A));
                EdgeDistance = Dot(InverseNormal, -A);
                IntersectionPoint = V20;
            }else if(((AAlongDeltaNormal >= 0) && (BAlongDeltaNormal <= 0)) || 
                     ((AAlongDeltaNormal <= 0) && (BAlongDeltaNormal >= 0))){ // The delta line  intersects
                AAlongDeltaNormal = AbsoluteValue(AAlongDeltaNormal);
                BAlongDeltaNormal = AbsoluteValue(BAlongDeltaNormal);
                f32 ABPercent = AAlongDeltaNormal/(AAlongDeltaNormal + BAlongDeltaNormal);
                v2 Point = A + ABPercent*(B-A);
                
                v2 DeltaDirection = Normalize(Delta);
                f32 PointAlongDeltaDirection = Dot(DeltaDirection, Point);
                f32 DeltaLength = Dot(DeltaDirection, Delta);
                
                if((-Epsilon <= PointAlongDeltaDirection) && 
                   (PointAlongDeltaDirection <= DeltaLength + Epsilon)){ // The delta intersects
                    FoundEdge = true;
                    EdgeIndex = I;
                    InverseNormal = Normalize(TripleProduct(B-A, -A));
                    EdgeDistance = Dot(InverseNormal, -A);
                    IntersectionPoint = Point;
                    
                    break;
                }else if(-Epsilon <= PointAlongDeltaDirection){ // The  delta ray intersects
                    if(!IsColinear){
                        FoundEdge = true;
                        EdgeIndex = I;
                        InverseNormal = Normalize(TripleProduct(B-A, -A));
                        EdgeDistance = Dot(InverseNormal, -A);
                        IntersectionPoint = Point;
                    }
                }
            }
        }
        Assert(FoundEdge);
        
        f32 Distance;
        v2 NewPoint = V20;
        if((InverseNormal.X == 0.0f) && (InverseNormal.Y == 0.0f)){
            Distance = EdgeDistance;
        }else{
            v2 Normal = -InverseNormal;
            NewPoint = CalculateSupport(ObjectA, ObjectB, Delta, Normal);
            Distance = Dot(NewPoint, Normal);
        }
        
        // DEBUG
        {
            if(PhysicsDebugger.AdvanceCurrentPosition()){
                return(Result);
            }
            PhysicsDebugger.AdvanceCurrentObject();
            if(PhysicsDebugger.Current.Object == PhysicsDebugger.Paused.Object){
                v2 Offset = ObjectA->DebugInfo->Offset;
                for(u32 I=0; I < Polytope.Count; I++){
                    u32 J = (I+1)%Polytope.Count;
                    RenderLine(Offset+Polytope[I], Offset+Polytope[J], -10, 0.015f, BLUE, &GameCamera);
                    RenderCenteredRectangle(Offset+Polytope[I], V2(0.025f, 0.025f), -10.1f, GREEN, &GameCamera);
                }
                
                v2 Base = 0.5f*(Polytope[EdgeIndex] + Polytope[(EdgeIndex+1)%Polytope.Count]);
                
                f32 Percent = IntersectionPoint.X/Delta.X;
                if((Delta.X == 0.0f) && (Delta.Y == 0.0f)){
                    Percent = 0.0f;
                }else if(Delta.X == 0.0f){
                    Percent = IntersectionPoint.Y/Delta.Y;
                }
                
                f32 Length = 0.2f;
                RenderCenteredRectangle(ObjectB->P, V2(0.05f, 0.05f), -10.1f, BLUE, &GameCamera);
                
                RenderLineFrom(Base+ObjectA->DebugInfo->Offset, Length*-InverseNormal, -10, 0.015f, GJK_SIMPLEX1_COLOR, &GameCamera);
                RenderLineFrom(Base+ObjectA->DebugInfo->Offset, Length*InverseNormal, -10, 0.015f, GJK_SIMPLEX3_COLOR, &GameCamera);
                RenderCenteredRectangle(Offset+IntersectionPoint, V2(0.03f, 0.03f), -10.3f, ORANGE, &GameCamera);
                
                LayoutString(&PhysicsDebugger.Layout, &DebugFont, BLACK, "Polytope.Count: %u", Polytope.Count);
                LayoutString(&PhysicsDebugger.Layout, &DebugFont, BLACK, "TOI: %f", 1.0f-Percent);
            }
        }
        
        if((-Epsilon <= (Distance-EdgeDistance)) && ((Distance-EdgeDistance) <= Epsilon)){
            f32 Percent = IntersectionPoint.X/Delta.X;
            if((Delta.X == 0.0f) && (Delta.Y == 0.0f)){
                Percent = 0.0f;
            }else if(Delta.X == 0.0f){
                Percent = IntersectionPoint.Y/Delta.Y;
            }
            
            Result.WorkingTimeOfImpact = 1.0f - Percent;
            if(Result.WorkingTimeOfImpact > 1.0f){
                Result.WorkingTimeOfImpact = 1.0f;
            }else if(Result.WorkingTimeOfImpact < 0.0f){
                Result.WorkingTimeOfImpact = 0.0f;
            }
            Result.ActualTimeOfImpact = 1.0f - Percent;
            Result.Normal = InverseNormal;
            
            break;
        }else{
            DynamicArrayInsertNewArrayItem(&Polytope, EdgeIndex+1, NewPoint);
        }
    }
    
    return(Result);
}

void
physics_system::DoPhysics(){
    TIMED_FUNCTION();
    CollisionSystemNewFrame();
    
    // DEBUG prepare for new physics frame
    PhysicsDebugger.NewFrame();
    
    FOR_BUCKET_ARRAY(ObjectA, &Objects){
        RenderBoundary(&GameCamera, ObjectA->Boundaries, -10.0f, ObjectA->P);
        
        
        // NOTE(Tyler): This may not be the best way to integrate dP, Delta, but it works
        f32 DragCoefficient = 0.7f;
        ObjectA->ddP.X += -DragCoefficient*ObjectA->dP.X*AbsoluteValue(ObjectA->dP.X);
        ObjectA->ddP.Y += -DragCoefficient*ObjectA->dP.Y*AbsoluteValue(ObjectA->dP.Y);
        v2 Delta = (OSInput.dTime*ObjectA->dP + 
                    0.5f*Square(OSInput.dTime)*ObjectA->ddP);
        ObjectA->dP += OSInput.dTime*ObjectA->ddP;
        ObjectA->ddP = {};
        
        // DEBUG
        ObjectA->DebugInfo->Offset= ObjectA->P;
        RenderCenteredRectangle(ObjectA->DebugInfo->Offset, V2(0.025f, 0.025f), -10.2f, WHITE, &GameCamera);
        if(PhysicsDebugger.Flags & PhysicsDebuggerFlags_StepPhysics){
            if(!ObjectA->DebugInfo->DidInitialdP){
                ObjectA->DebugInfo->dP = ObjectA->dP;
                ObjectA->DebugInfo->ddP = ObjectA->ddP;
                ObjectA->DebugInfo->DidInitialdP = true;
            }
            Assert(ObjectA->DebugInfo);
            ObjectA->dP = ObjectA->DebugInfo->dP;
            ObjectA->ddP = ObjectA->DebugInfo->ddP;
            RenderLineFrom(ObjectA->DebugInfo->Offset, Delta, -10.0f, 0.02f, GREEN, &GameCamera);
        }
        
        f32 FrameTimeRemaining = 1.0f;
        u32 Iteration = 0;
        while((FrameTimeRemaining > 0) &&
              (Iteration < MAX_PHYSICS_ITERATIONS)){
            Iteration++;
            
            const f32 Epsilon = 0.0001f;
            if((Delta.X > -Epsilon) && (Delta.X < Epsilon) &&
               (Delta.Y > -Epsilon) && (Delta.Y < Epsilon)) break;
            
            dynamic_array<epa_result> CollidingEPAResults;
            DynamicArrayInitialize(&CollidingEPAResults, 8, &TransientStorageArena);
            dynamic_array<physics_object *> CollidingObjects;
            DynamicArrayInitialize(&CollidingObjects, 8, &TransientStorageArena);
            
            b8 DoesCollideAny = false;
            
            FOR_BUCKET_ARRAY(ObjectB, &StaticObjects){
                v2 Simplex[3];
                // CurrentPosition is incremented in DoGJK
                if(DoGJK(Simplex, ObjectA, ObjectB, Delta)){
                    
                    epa_result EPAResult = DoVelocityEPA(ObjectA, ObjectB, Delta, Simplex);
                    DynamicArrayPushBack(&CollidingEPAResults, EPAResult);
                    DynamicArrayPushBack(&CollidingObjects, &ObjectB);
                    
                    DoesCollideAny = true;
                }
                
                if(PhysicsDebugger.AdvanceCurrentPosition()){
                    goto end_for_objecta_;
                }
                
                PhysicsDebugger.AdvanceCurrentObject();
            }
            
            if(DoesCollideAny){
                epa_result Colliding = {};
                Colliding.ActualTimeOfImpact = F32_POSITIVE_INFINITY;
                f32 CollidingObjectDistanceAlongDelta = F32_POSITIVE_INFINITY;
                
                u32 DEBUGCollidingIndex = 0;
                physics_object *DEBUGCollidingObject = 0;
                
                // This loop could be removed by something else that does it more directly
                for(u32 I = 0; I < CollidingEPAResults.Count; I++){
                    epa_result EPAResult = CollidingEPAResults[I];
                    physics_object *ObjectB = CollidingObjects[I];
                    if(EPAResult.ActualTimeOfImpact < Colliding.ActualTimeOfImpact){
                        Colliding = EPAResult;
                        CollidingObjectDistanceAlongDelta = Dot(Delta, ObjectB->P-ObjectA->P);
                        DEBUGCollidingIndex = I;
                        DEBUGCollidingObject = ObjectB;
                    }else if(EPAResult.ActualTimeOfImpact == Colliding.ActualTimeOfImpact){
                        f32 ObjectBDistanceAlongDelta = Dot(Delta, ObjectB->P-ObjectA->P);
                        if(ObjectBDistanceAlongDelta < CollidingObjectDistanceAlongDelta){
                            Colliding = EPAResult;
                            CollidingObjectDistanceAlongDelta = ObjectBDistanceAlongDelta;
                            DEBUGCollidingIndex = I;
                            DEBUGCollidingObject = ObjectB;
                        }
                    }
                }
                
                // DEBUG
                if(PhysicsDebugger.Current.Object == PhysicsDebugger.Paused.Object){
                    LayoutString(&PhysicsDebugger.Layout, &DebugFont, BLACK, "Actual TOI: %f", Colliding.ActualTimeOfImpact);
                    LayoutString(&PhysicsDebugger.Layout, &DebugFont, BLACK, "Working TOI: %f", Colliding.WorkingTimeOfImpact);
                    
                    f32 Length = 0.2f;
                    RenderLineFrom(DEBUGCollidingObject->P, Length*Colliding.Normal, -10, 0.015f, GJK_SIMPLEX1_COLOR, &GameCamera);
                    RenderCenteredRectangle(DEBUGCollidingObject->P, V2(0.05f, 0.05f), -10.1f, BLUE, &GameCamera);
                }
                if(PhysicsDebugger.AdvanceCurrentPosition()){
                    goto end_for_objecta_;
                }
                
                
                f32 COR = 1.0f;
                
                FrameTimeRemaining -= FrameTimeRemaining*Colliding.WorkingTimeOfImpact;
                ObjectA->P += Colliding.ActualTimeOfImpact*Delta;
                v2 OlddP = ObjectA->dP;
                ObjectA->dP = ObjectA->dP - COR*Dot(Colliding.Normal, ObjectA->dP)*Colliding.Normal;
                Delta = Delta - COR*Dot(Colliding.Normal, Delta)*Colliding.Normal;
            }else{
                FrameTimeRemaining -= FrameTimeRemaining;
                ObjectA->P += Delta;
            }
        }
        
        { // DEBUG
            PhysicsDebugger.Paused = {};
            ObjectA->DebugInfo->DidInitialdP = false;
        }
    }end_for_objecta_:;
}

inline void
entity_manager::DoPhysics(){
    PhysicsSystem.DoPhysics();
}