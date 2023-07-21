
#if defined(SNAIL_JUMPY_DEBUG_BUILD)

#if defined(SNAIL_JUMPY_USE_PROCESSED_ASSETS)
#define SJA_DEBUG(Statement) 
#define SJA_DEBUG_EXPR(Statement) false
#else // defined(SNAIL_JUMPY_USE_PROCESSED_ASSETS)
#define SJA_DEBUG(Statement) Statement
#define SJA_DEBUG_EXPR(Expression) Expression
#endif // defined(SNAIL_JUMPY_USE_PROCESSED_ASSETS)

//#define DEBUG_DISPLAY_TEXT_INPUT_UNDO

#define DEBUG_DATA_INITIALIZE(State) debug_info_initializer DEBUG_INFO_INITIALIZER(State);
#define DO_DEBUG_INFO() debug_info_display DebugDisplay##__FUNCTION__
#define DEBUG_MESSAGE(...) DebugInfo.SubmitMessage(__VA_ARGS__)
#define DEBUG_STATEMENT(Statement) Statement

local_constant f32 DEBUG_FADEOUT_RANGE = 0.5f;

//~ 
struct debug_scope_time_elapsed {
    u64 Microseconds;
    u64 Cycles;
};

struct debug_scope_timer {
    u64 StartCycle;
    u64 StartTime;
    debug_scope_timer(){
        StartCycle = __rdtsc();
        StartTime = OSGetMicroseconds();
    }
    
    inline debug_scope_time_elapsed GetElapsed() {
        debug_scope_time_elapsed Result = {};
        Result.Cycles = __rdtsc()-StartCycle;
        Result.Microseconds = OSGetMicroseconds()-StartTime;
        return Result;
    }
};

//~ 
typedef u32 debug_display_flags;
enum debug_display_flags_ {
    DebugDisplay_None          = (0 << 0),
    DebugDisplay_Basic         = (1 << 0),
    DebugDisplay_TextInputUndo = (1 << 2),
    DebugDisplay_AssetLoading  = (1 << 4),
    DebugDisplay_Messages      = (1 << 5),
};

enum debug_message_type {
    DebugMessage_PerFrame,
    DebugMessage_Fadeout,
    DebugMessage_Asset,
    
    DebugMessage_EndPerFrame,
};

union debug_string_node {
    char Buffer[512];
    debug_string_node *NextFree;
};

struct debug_message {
    debug_message_type Type;
    u64 Hash;
    union{
        const char *Message;
        debug_string_node *StringNode;
    };
    u32 AssetLoadCounter;
    f32 Timeout;
};

struct debug_info {
    main_state *State;
    u64 InitTime;
    debug_display_flags DisplayFlags = (DebugDisplay_Basic|DebugDisplay_AssetLoading|DebugDisplay_Messages);
    asset_loading_status AssetLoadingStatus;
    
    dynamic_array<debug_message> Messages;
    debug_string_node StringNodes[512];
    debug_string_node *FirstFreeStringNode;
    
    inline void SubmitStringMessage(debug_message_type Type, const char *Message, f32 Timeout=1.0f);
    inline void VSubmitMessage(debug_message_type Type, f32 Timeout, const char *Format, va_list VarArgs);
    inline void SubmitMessage(debug_message_type Type, const char *Format, ...);
    inline void SubmitMessage(debug_message_type Type, f32 Timeout, const char *Format, ...);
    inline void EndFrame(debug_scope_time_elapsed Elapsed);
    inline void DoDebugHotkeys();
};

global debug_info DebugInfo;

struct debug_info_initializer {
    u64 StartTime;
    debug_info_initializer(main_state *State_){
        DebugInfo.State = State_;
        StartTime = OSGetMicroseconds();
        FOR_RANGE(I, 0, ArrayCount(DebugInfo.StringNodes)){
            FREELIST_FREE(DebugInfo.FirstFreeStringNode, &DebugInfo.StringNodes[I]);
        }
    }
    
    ~debug_info_initializer(){
        DebugInfo.InitTime = OSGetMicroseconds()-StartTime;
        DEBUG_MESSAGE(DebugMessage_Fadeout, "InitTime: %.2fms", USecToMSec(DebugInfo.InitTime));
    }
};

inline void
debug_info::DoDebugHotkeys(){
    os_input *Input = &State->Input;
    if(Input->KeyJustDown(KeyCode_F1)) DisplayFlags ^= DebugDisplay_Basic;
    if(Input->KeyJustDown(KeyCode_F2)) DisplayFlags ^= DebugDisplay_TextInputUndo;
    if(Input->KeyJustDown(KeyCode_F3)) DisplayFlags ^= DebugDisplay_AssetLoading;
    if(Input->KeyJustDown(KeyCode_F4)) DisplayFlags ^= DebugDisplay_Messages;
}

inline void
debug_info::SubmitStringMessage(debug_message_type Type, const char *String, f32 Timeout){
    if(!Messages.Items){
        DebugInfo.Messages = MakeDynamicArray<debug_message>(8);
    }
    debug_message Message = {};
    Message.Type = Type;
    Message.Hash = HashString(String);
    
    Message.Message = String;
    
    b8 SendToConsole = true;
    b8 KeepMessage = true;
    if(Type == DebugMessage_PerFrame){
        FOR_EACH(OtherMessage, &Messages){
            if((OtherMessage.Type == DebugMessage_EndPerFrame) &&
               (OtherMessage.Hash == Message.Hash)){
                SendToConsole = false;
            }
        }
    }else if(Type == DebugMessage_Fadeout){
        FOR_EACH(OtherMessage, &Messages){
            if((OtherMessage.Type == DebugMessage_Fadeout) && 
               (OtherMessage.Hash == Message.Hash)){
                KeepMessage = false;
                if(OtherMessage.Timeout > DEBUG_FADEOUT_RANGE) SendToConsole = false;
                OtherMessage.Timeout = Timeout;
            }
        }
    }
    
    SJA_DEBUG(Message.AssetLoadCounter = State->AssetLoader.LoadCounter);
    Message.Timeout = Timeout;
    
    if(SendToConsole) LogMessage(String); 
    if(KeepMessage){
        if(Type == DebugMessage_Fadeout){
            Message.StringNode = FREELIST_ALLOC(FirstFreeStringNode, CANT_ALLOC(debug_string_node));
            CopyCString(Message.StringNode->Buffer, String);
        }
        
        ArrayAdd(&Messages, Message);
    }
}

inline void
debug_info::VSubmitMessage(debug_message_type Type, f32 Timeout, const char *Format, va_list VarArgs){
    SubmitStringMessage(Type, VArenaPushFormatCString(&GlobalTransientMemory, Format, VarArgs), Timeout);
}

inline void
debug_info::SubmitMessage(debug_message_type Type, const char *Format, ...){
    va_list VarArgs;
    va_start(VarArgs, Format);
    VSubmitMessage(Type, 1.0f, Format, VarArgs);
    va_end(VarArgs);
}

inline void
debug_info::SubmitMessage(debug_message_type Type, f32 Timeout, const char *Format, ...){
    va_list VarArgs;
    va_start(VarArgs, Format);
    VSubmitMessage(Type, Timeout, Format, VarArgs);
    va_end(VarArgs);
}

inline void
debug_info::EndFrame(debug_scope_time_elapsed Elapsed) {
    asset_system *Assets = &State->Assets;
    SJA_DEBUG(asset_loader *Loader = &State->AssetLoader);
    game_renderer *Renderer = &State->Renderer;
    os_input *Input = &State->Input;
    
    render_group *Group = Renderer->GetRenderGroup(RenderGroupID_Font);
    font *Font = State->UI.Fonts.FindFont(String("normal_font"));
    
    v2 DebugP = V2(10, Renderer->OutputSize.Y-100);
    if(DisplayFlags & DebugDisplay_Basic){
#if 0
        DebugP.Y += FontRenderString(Renderer, Font, DebugP, WHITE, 
                                     "Counter: %.2f | Cycles: %08llu | Time: %04llu | FPS: %.2f | Scale: %.1f", 
                                     Counter, Elapsed.Cycles, Elapsed.Microseconds, 1.0/Input->dTime, Renderer->CameraScale);
#endif
    }
    
    if(DisplayFlags & DebugDisplay_AssetLoading){
        if(AssetLoadingStatus == AssetLoadingStatus_Warnings){
#if 0
            DebugP.Y += FontRenderString(Renderer, Font, DebugP, RED,
                                         "Asset loader warnings!", 
                                         Counter, Elapsed.Cycles, Elapsed.Microseconds, 1.0/Input->dTime, Renderer->CameraScale);
#endif
        }
    }
    
    if(DisplayFlags & DebugDisplay_TextInputUndo){
        text_input_context *Context = Input->TextInput;
        if(!Context) return;
        
        u32 I = 0;
        text_input_history_node *Node = &Context->HistorySentinel;
        do{
#if 0
            DebugP.Y -= FontRenderString(Renderer, Font, DebugP, WHITE, "%s [%p]: '%.*s'", 
                                         (Node == Context->CurrentHistoryNode) ? ">" : " ",
                                         Node, Node->BufferLength, Node->Buffer);
#endif
            
            Node = Node->Next;
            I++;
        }while((Node != &Context->HistorySentinel) || (I > 30));
    }
    
    if(DisplayFlags & DebugDisplay_Messages){
        v2 DebugP = V2(250, 190);
        FOR_EACH_(Message, Index, &Messages){
            if((Message.Type == DebugMessage_Asset) && 
               SJA_DEBUG_EXPR(Message.AssetLoadCounter < Loader->LoadCounter)){
                ARRAY_REMOVE_IN_LOOP_ORDERED(&Messages, Index);
            }
            
            if(Message.Type == DebugMessage_EndPerFrame){
                ARRAY_REMOVE_IN_LOOP_ORDERED(&Messages, Index);
            }
            
            local_constant f32 EPSILON       = 0.05f;
            color Color = GREEN;
            if(Message.Type == DebugMessage_Fadeout){
                if(Message.Timeout <= DEBUG_FADEOUT_RANGE){
                    f32 T = Message.Timeout/DEBUG_FADEOUT_RANGE;
                    Color = ColorAlphiphy(Color, Square(T));
                }
                if(Message.Timeout <= EPSILON){
                    FREELIST_FREE(FirstFreeStringNode, Message.StringNode);
                    ARRAY_REMOVE_IN_LOOP_ORDERED(&Messages, Index);
                }
            }
            
            DebugP.Y -= Font->Size;
            RenderFormatString(Group, Font, Color, DebugP, ZLayer(ZLayer_DebugUI), "%s", Message.Message);
            
            Message.Timeout -= Input->dTime;
            
            if(Message.Type == DebugMessage_PerFrame){
                Message.Type = DebugMessage_EndPerFrame;
            }
        }
    }
}

//~ 
struct debug_info_display {
    debug_scope_timer Timer;
    debug_info_display() : Timer() {
        DebugInfo.DoDebugHotkeys();
    }
    
    ~debug_info_display(){ 
        DebugInfo.EndFrame(Timer.GetElapsed()); 
    }
};

#else
#define DO_DEBUG_INFO(...)
#define DEBUG_DATA_INITIALIZE(...)
#define DEBUG_MESSAGE(...) 
#define DEBUG_STATEMENT(Statement) 
#endif