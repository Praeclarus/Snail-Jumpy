
//~ Camera

inline void
camera::DirectMove(v2 dP, world_data *World){
    ActualP += dP;
    v2 MapSize = TILE_SIDE*v2{(f32)World->Width, (f32)World->Height};
    if((ActualP.X+32.0f*TILE_SIDE) > MapSize.X){
        ActualP.X = MapSize.X - 32.0f*TILE_SIDE;
    }else if(ActualP.X < 0.0f){
        ActualP.X = 0.0f;
    }
    if((ActualP.Y+18.0f*TILE_SIDE) > MapSize.Y){
        ActualP.Y = MapSize.Y - 18.0f*TILE_SIDE;
    }else if(ActualP.Y < 0.0f){
        ActualP.Y = 0.0f;
    }
    TargetP = ActualP;
}

inline void
camera::Move(v2 dP, world_data *World){
    TargetP += dP;
    v2 MapSize = TILE_SIDE*v2{(f32)World->Width, (f32)World->Height};
    if((TargetP.X+32.0f*TILE_SIDE) > MapSize.X){
        TargetP.X = MapSize.X - 32.0f*TILE_SIDE;
    }else if(TargetP.X < 0.0f){
        TargetP.X = 0.0f;
    }
    if((TargetP.Y+18.0f*TILE_SIDE) > MapSize.Y){
        TargetP.Y = MapSize.Y - 18.0f*TILE_SIDE;
    }else if(TargetP.Y < 0.0f){
        TargetP.Y = 0.0f;
    }
}

inline void
camera::SetCenter(v2 Center, world_data *World){
    v2 MapSize = TILE_SIDE*v2{(f32)World->Width, (f32)World->Height};
    TargetP = Center - 0.5f * TILE_SIDE*v2{32.0f, 18.0f};
    if((TargetP.X+32.0f*TILE_SIDE) > MapSize.X){
        TargetP.X = MapSize.X - 32.0f*TILE_SIDE;
    }else if(TargetP.X < 0.0f){
        TargetP.X = 0.0f;
    }
    if((TargetP.Y+18.0f*TILE_SIDE) > MapSize.Y){
        TargetP.Y = MapSize.Y - 18.0f*TILE_SIDE;
    }else if(TargetP.Y < 0.0f){
        TargetP.Y = 0.0f;
    }
}

inline v2
camera::ScreenPToWorldP(v2 ScreenP){
    v2 Result = ScreenP / MetersToPixels + P;
    return(Result);
}

inline v2
camera::WorldPToScreenP(v2 WorldP){
    v2 Result = (WorldP - P) * MetersToPixels;
    return(Result);
}

inline void
camera::Update(){
    MetersToPixels = Minimum((OSInput.WindowSize.Width/32.0f), (OSInput.WindowSize.Height/18.0f)) / 0.5f;
    
    ActualP += MoveFactor*(TargetP-ActualP);
    if(ShakeTimeRemaining > 0.0f){
        P = ActualP;
        P.X += ShakeStrength*0.5f*Cos(ShakeTimeRemaining*ShakeFrequency*1.5f);
        P.Y += ShakeStrength*Cos(ShakeTimeRemaining*ShakeFrequency);
        
        ShakeTimeRemaining -= OSInput.dTime;
    }else{
        P = ActualP;
    }
}

inline void
camera::Shake(f32 Time, f32 Strength, f32 Frequency){
    ShakeTimeRemaining += Time;
    ShakeFrequency = Frequency;
    ShakeStrength = Strength;
}