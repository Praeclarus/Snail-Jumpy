
//~ Miscellaneous
inline b8
os_input::WasWindowResized(){
    b8 Result = ((LastWindowSize.Width  != WindowSize.Width) ||
                 (LastWindowSize.Height != WindowSize.Height));
    return Result;
}

//~ Modifiers

inline b8
os_input::TestModifier(os_key_flags Flags){
    b8 Result;
    if(Flags & KeyFlag_Any){
        Flags &= 0b0111;
        Result = ((KeyFlags & Flags) == Flags);
    }else{
        Result = KeyFlags == Flags;
    }
    return(Result);
}

//~ Mouse 
inline b8 
os_input::MouseUp(os_mouse_button Button, os_key_flags Flags){
    key_state ButtonState = MouseState[Button];
    b8 Result = !((ButtonState & KeyState_IsDown) && TestModifier(Flags));
    
    return(Result);
}

inline b8 
os_input::MouseDown(os_mouse_button Button, os_key_flags Flags){
    key_state ButtonState = MouseState[Button];
    b8 Result = ((ButtonState & KeyState_IsDown) && TestModifier(Flags));
    
    return(Result);
}

inline b8 
os_input::MouseJustUp(os_mouse_button Button, os_key_flags Flags){
    key_state ButtonState = MouseState[Button];
    b8 Result = ((ButtonState & KeyState_JustUp) && TestModifier(Flags));
    
    return(Result);
}

inline b8 
os_input::MouseJustDown(os_mouse_button Button, os_key_flags Flags){
    key_state ButtonState = MouseState[Button];
    b8 Result = ((ButtonState & KeyState_JustDown) && TestModifier(Flags));
    
    return(Result);
}

//~ Keyboard
inline b8 
os_input::KeyUp(u32 Key, os_key_flags Flags){
    key_state KeyState = KeyboardState[Key];
    b8 Result = !((KeyState & KeyState_IsDown) && TestModifier(Flags));
    
    return(Result);
}

inline b8 
os_input::KeyJustUp(u32 Key, os_key_flags Flags){
    key_state KeyState = KeyboardState[Key];
    b8 Result = ((KeyState & KeyState_JustUp) || !TestModifier(Flags));
    
    return(Result);
}

inline b8 
os_input::KeyJustDown(u32 Key, os_key_flags Flags){
    key_state KeyState = KeyboardState[Key];
    b8 Result = ((KeyState & KeyState_JustDown) && TestModifier(Flags));
    
    return(Result);
}

inline b8 
os_input::KeyRepeat(u32 Key, os_key_flags Flags){
    key_state KeyState = KeyboardState[Key];
    b8 Result = ((KeyState & KeyState_RepeatDown) && TestModifier(Flags));
    
    return(Result);
}

inline b8 
os_input::KeyDown(u32 Key, os_key_flags Flags){
    key_state KeyState = KeyboardState[Key];
    b8 Result = ((KeyState & KeyState_IsDown) && TestModifier(Flags));
    
    return(Result);
}

//~ Text input

inline range_s32
text_input_context::GetSelectionRange(){
    range_s32 Result = MakeRangeS32(CursorPosition, CursorPosition);
    if(SelectionMark >= 0){
        Result = MakeRangeS32(CursorPosition, SelectionMark);
    }
    return Result;
}

inline b8
text_input_context::TryDeleteSelection(){
    if(SelectionMark >= 0){
        HistoryChangeEvent(TextInputEvent_RemoveRange);
        BufferDeleteRange(GetSelectionRange());
        SelectionMark = -1;
        return true;
    }
    
    return false;
}

inline void
text_input_context::MaybeSetSelection(){
    if(!Input->TestModifier(KeyFlag_Shift|KeyFlag_Any)){
        SelectionMark = -1;
    }else if(SelectionMark < 0){
        SelectionMark = CursorPosition;
    }
}

//- Buffer stuff
inline void
text_input_context::BufferDeleteRange(range_s32 Range){
    Range = RangeCrop(MakeRangeS32(0, BufferLength), Range);
    MoveMemory(&Buffer[Range.Start],&Buffer[Range.End], BufferLength-(Range.End-1));
    s32 Size = RangeSize(Range);
    ZeroMemory(&Buffer[BufferLength-Size], Size);
    CursorPosition = Range.Start;
    
    Flags |= TextInputFlag_IsDirty;
}

inline u32
text_input_context::BufferInsertChars(u32 Position, char *Chars, u32 CharCount){
    if(BufferLength+CharCount < DEFAULT_BUFFER_SIZE){
        MoveMemory(&Buffer[Position+CharCount],
                   &Buffer[Position],
                   BufferLength-Position);
        CopyMemory(&Buffer[Position], Chars, CharCount);
        BufferLength += CharCount;
        Flags |= TextInputFlag_IsDirty;
        return CharCount;
    }
    return 0;
}

//- Undo/redo
inline text_input_history_node *
text_input_context::HistoryAddNode(){
    text_input_history_node *Node = CurrentHistoryNode->Next;
    while(Node != &HistorySentinel){
        text_input_history_node *Next = Node->Next;
        DLIST_REMOVE(Node);
        FREELIST_FREE(Input->TextInputFreeHistoryNode, Node);
        Node = Next;
    }
    
    text_input_history_node *Result = FREELIST_ALLOC(Input->TextInputFreeHistoryNode, 
                                                     ArenaPushType(HistoryMemory, text_input_history_node));
    
    Result->Buffer = ArenaPushArray(HistoryMemory, char, BufferLength);
    CopyMemory(Result->Buffer, Buffer, BufferLength);
    Result->BufferLength = BufferLength;
    Result->CursorPosition = CursorPosition;
    
    DLIST_ADD_LAST(&HistorySentinel, Result);
    CurrentHistoryNode = Result;
    
    Flags &= ~TextInputFlag_IsDirty;
    
    return Result;
}

inline void
text_input_context::HistoryUndo(){
    HistoryChangeEvent(TextInputEvent_Other);
    if(CurrentHistoryNode == &HistorySentinel) return; 
    
    text_input_history_node *Node = CurrentHistoryNode->Prev;
    
    CopyMemory(Buffer, Node->Buffer, Node->BufferLength);
    if(BufferLength > Node->BufferLength) ZeroMemory(&Buffer[Node->BufferLength], BufferLength-Node->BufferLength);
    
    BufferLength = Node->BufferLength;
    CursorPosition = Node->CursorPosition;
    
    CurrentHistoryNode = Node;
}

inline void
text_input_context::HistoryRedo(){
    HistoryChangeEvent(TextInputEvent_Other);
    if(CurrentHistoryNode->Next == &HistorySentinel) return; 
    
    text_input_history_node *Node = CurrentHistoryNode->Next;
    
    CopyMemory(Buffer, Node->Buffer, Node->BufferLength);
    if(BufferLength > Node->BufferLength) ZeroMemory(&Buffer[Node->BufferLength], BufferLength-Node->BufferLength);
    
    BufferLength = Node->BufferLength;
    CursorPosition = Node->CursorPosition;
    
    CurrentHistoryNode = Node;
}

inline void
text_input_context::HistoryChangeEvent(text_input_event_type Type){
    if((LastEvent != Type) && 
       (Flags & TextInputFlag_IsDirty)){
        HistoryAddNode();
    }
    LastEvent = Type;
}

//- Core

inline void
text_input_context::Initialize(memory_arena *Memory){
    HistoryMemory = Memory;
    Reset();
}

inline void
text_input_context::Reset(){
    ZeroMemory(Buffer, DEFAULT_BUFFER_SIZE);
    BufferLength = 0;
    SelectionMark = -1;
    CursorPosition = 0;
    
    DLIST_INIT(&HistorySentinel);
    
    CurrentHistoryNode = &HistorySentinel;
}

inline void
text_input_context::ProcessKey(os_key_code Key){
    if(Key == KeyCode_NULL){
    }else if(Key < U8_MAX){
        char Char = (char)Key;
        
        if((Char == 'C') && Input->TestModifier(KeyFlag_Control|KeyFlag_Any)){ //- Copy
            range_s32 Range = GetSelectionRange();
            OSCopyChars(&Buffer[Range.Start], RangeSize(Range));
        }else if((Char == 'V') && Input->TestModifier(KeyFlag_Control|KeyFlag_Any)){ //- Paste
            HistoryChangeEvent(TextInputEvent_AddRange);
            TryDeleteSelection();
            char *ToPaste = OSPasteChars(&GlobalTransientMemory);
            CursorPosition += BufferInsertChars(CursorPosition, ToPaste, CStringLength(ToPaste));
            HistoryAddNode();
        }else if((Char == 'X') && Input->TestModifier(KeyFlag_Control|KeyFlag_Any)){ //- Cut
            HistoryChangeEvent(TextInputEvent_RemoveRange);
            range_s32 Range = GetSelectionRange();
            OSCopyChars(&Buffer[Range.Start], RangeSize(Range));
            TryDeleteSelection();
        }else if((Char == 'A') && Input->TestModifier(KeyFlag_Control|KeyFlag_Any)){ //- Select all
            HistoryChangeEvent(TextInputEvent_MoveCursor);
            CursorPosition = BufferLength;
            SelectionMark = 0;
        }else if((Char == 'Z') && Input->TestModifier(KeyFlag_Control|KeyFlag_Any)){ HistoryUndo();
        }else if((Char == 'Y') && Input->TestModifier(KeyFlag_Control|KeyFlag_Any)){ HistoryRedo();
        }else{ //- Normal editing
            HistoryChangeEvent(TextInputEvent_AddChar);
            TryDeleteSelection();
            
            Char = CharToLower(Char);
            if(Input->TestModifier(KeyFlag_Shift|KeyFlag_Any)) Char = KEYBOARD_SHIFT_TABLE[Char]; 
            Assert(Char);
            CursorPosition += BufferInsertChars(CursorPosition, &Char, 1);
            
            if(Char == ' ') HistoryAddNode();
        }
        
    }else if(Key == KeyCode_BackSpace){
        if(!TryDeleteSelection()){
            HistoryChangeEvent(TextInputEvent_BackSpace);
            if(Input->TestModifier(KeyFlag_Control|KeyFlag_Any)){
                BufferDeleteRange(SeekBackward(Buffer, CursorPosition));
                HistoryAddNode();
            }else{
                BufferDeleteRange(SizeRangeS32(CursorPosition, -1));
            }
        }
    }else if(Key == KeyCode_Delete){
        if(!TryDeleteSelection()){
            HistoryChangeEvent(TextInputEvent_Delete);
            if(Input->TestModifier(KeyFlag_Control|KeyFlag_Any)){
                BufferDeleteRange(SeekForward(Buffer, BufferLength, CursorPosition));
                HistoryAddNode();
            }else{
                BufferDeleteRange(SizeRangeS32(CursorPosition, 1));
            }
        }
    }else if(Key == KeyCode_Left){
        MaybeSetSelection();
        if(Input->TestModifier(KeyFlag_Control|KeyFlag_Any)){
            CursorPosition = SeekBackward(Buffer, CursorPosition).Start;
        }
        while(0 < CursorPosition) if(!IsNewLine(Buffer[--CursorPosition])) break;
        HistoryChangeEvent(TextInputEvent_MoveCursor);
        
    }else if(Key == KeyCode_Right){
        MaybeSetSelection();
        if(Input->TestModifier(KeyFlag_Control|KeyFlag_Any)){
            CursorPosition = SeekForward(Buffer, BufferLength, CursorPosition).End;
        }
        while(CursorPosition < (s32)BufferLength) if(!IsNewLine(Buffer[++CursorPosition])) break;
        HistoryChangeEvent(TextInputEvent_MoveCursor);
        
    }else if(Key == KeyCode_Home){
        MaybeSetSelection();
        while(0 < CursorPosition) if((--CursorPosition > 0) && IsNewLine(Buffer[CursorPosition-1])) break;
        HistoryChangeEvent(TextInputEvent_MoveCursor);
        
    }else if(Key == KeyCode_End){
        MaybeSetSelection();
        while(CursorPosition < (s32)BufferLength) if(IsNewLine(Buffer[++CursorPosition])) break;
        HistoryChangeEvent(TextInputEvent_MoveCursor);
        
    }else if(Key == KeyCode_Return){
        Flags |= TextInputFlag_DoEnd;
    }else if(Key == KeyCode_Escape){
        SelectionMark = -1;
    }
    
    BufferLength = CStringLength(Buffer);
    CursorPosition = Minimum((u32)CursorPosition, BufferLength);
    Assert(CursorPosition >= 0);
}

inline void 
text_input_context::LoadFromBuffer(const char *From, u32 Length){
    CopyCString(Buffer, From, TEXT_INPUT_BUFFER_SIZE);
    BufferLength = Length;
    CursorPosition = BufferLength;
}

inline void 
text_input_context::LoadFromBuffer(const char *From){
    LoadFromBuffer(From, CStringLength(From));
}

//- os_input stuff
inline void
os_input::BeginTextInput(text_input_context *Context){
    TextInput = Context;
    TextInput->Input = this;
    TextInput->Flags &= ~TextInputFlag_DoEnd;
    FOR_RANGE(I, 0, KeyCode_TOTAL){
        KeyboardState[I] = 0;
    }
}

inline b8
os_input::MaybeEndTextInput(){
    if(!TextInput) return true;
    b8 Result = (TextInput->Flags & TextInputFlag_DoEnd);
    return Result;
}

inline void
os_input::EndTextInput(){
    TextInput = 0;
}
