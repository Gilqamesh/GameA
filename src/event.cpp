internal inline i32
_Heap_Parent(i32 Index)
{
    return ((Index - 1) / 2);
}

internal inline i32
_Heap_Child_Left(i32 Index)
{
    return (2 * Index + 1);
}

internal inline i32
_Heap_Child_Right(i32 Index)
{
    return (2 * Index + 2);
}

template <typename event_type>
internal void
PushEvent(table<event_type> *Table, event_type Event)
{
    ASSERT(Table->CurrentSize < Table->AllocatedSize);

    i32 Index = Table->CurrentSize++;
    Table->Data[Index] = Event;

    // Swim operation on heap to restore heap property
    i32 ParentIndex = _Heap_Parent(Index);
    while (Index && Table->Data[Index].Target < Table->Data[ParentIndex].Target)
    {
        event_type Tmp = Table->Data[Index];
        Table->Data[Index] = Table->Data[ParentIndex];
        Table->Data[ParentIndex] = Tmp;
        Index = ParentIndex;
        ParentIndex = _Heap_Parent(ParentIndex);
    }
}

template <typename event_type>
internal void
PopEvent(table<event_type> *Table)
{
    ASSERT(Table->CurrentSize);

    Table->Data[0] = Table->Data[--Table->CurrentSize];

    // Sink operation on heap to restore heap property
    if (Table->CurrentSize)
    {
        b32 Swapped;
        i32 SwapIndex;
        i32 Index = 0;
        do
        {
            Swapped = false;
            SwapIndex = 0;
            i32 LeftChildIndex = _Heap_Child_Left(Index);
            i32 RightChildIndex = _Heap_Child_Right(Index);
            if (LeftChildIndex < Table->CurrentSize)
            {
                if (RightChildIndex < Table->CurrentSize)
                {
                    if (Table->Data[LeftChildIndex].Target < Table->Data[RightChildIndex].Target)
                    {
                        SwapIndex = LeftChildIndex;
                    }
                    else
                    {
                        SwapIndex = RightChildIndex;
                    }
                }
                else
                {
                    SwapIndex = LeftChildIndex;
                }
            }
            if (SwapIndex && Table->Data[SwapIndex].Target < Table->Data[Index].Target)
            {
                Swapped = true;
                event_type Tmp = Table->Data[Index];
                Table->Data[Index] = Table->Data[SwapIndex];
                Table->Data[SwapIndex] = Tmp;
                Index = SwapIndex;
            }
        } while (Swapped);
    }
}
