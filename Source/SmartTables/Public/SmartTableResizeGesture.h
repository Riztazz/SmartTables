// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#pragma once

#include "Containers/Map.h"
#include "Misc/TVariant.h"
#include "UObject/NameTypes.h"

struct FSmartTableNoResize
{
};

struct FSmartTablePointerResize
{

    FName ColumnId;

    float PressLocalX = 0.0f;

    float StartWidth = 0.0f;

    int32 UserIndex = INDEX_NONE;

    uint32 PointerIndex = 0;
};

struct FSmartTableIntentResize
{
    FName ColumnId;
};

struct SMARTTABLES_API FSmartTableResizeGesture
{

    TVariant< FSmartTableNoResize, FSmartTablePointerResize, FSmartTableIntentResize > Kind;

    TMap< FName, float > StartWidths;

    bool IsActive() const
    {
        return !Kind.IsType< FSmartTableNoResize >();
    }

    bool IsFromIntent() const
    {
        return Kind.IsType< FSmartTableIntentResize >();
    }

    const FSmartTablePointerResize * AsPointer() const
    {
        return Kind.TryGet< FSmartTablePointerResize >();
    }

    FName GetColumnId() const
    {
        if ( const FSmartTablePointerResize * Pointer = Kind.TryGet< FSmartTablePointerResize >() )
        {
            return Pointer->ColumnId;
        }

        if ( const FSmartTableIntentResize * Intent = Kind.TryGet< FSmartTableIntentResize >() )
        {
            return Intent->ColumnId;
        }

        return NAME_None;
    }

    bool IsDrivenBy( int32 ByUser, uint32 ByPointer ) const
    {
        const FSmartTablePointerResize * Pointer = Kind.TryGet< FSmartTablePointerResize >();

        return Pointer && Pointer->UserIndex == ByUser && Pointer->PointerIndex == ByPointer;
    }

    bool IsAtRest() const
    {
        return !IsActive() && StartWidths.IsEmpty();
    }

    void Reset()
    {
        *this = FSmartTableResizeGesture();
    }
};
