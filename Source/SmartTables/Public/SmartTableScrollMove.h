// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#pragma once

#include "HAL/Platform.h"
#include "Misc/CoreMiscDefines.h"
#include "Misc/Optional.h"
#include "SmartTableKeptRow.h"
#include "SmartTableRowSetDiff.h"
#include "Templates/Function.h"
#include "UObject/NameTypes.h"

enum class ESmartTableRowSetScroll : uint8
{

    Stay,

    ToRow,

    Hold
};

struct FSmartTableRowSetScroll
{
    ESmartTableRowSetScroll Kind = ESmartTableRowSetScroll::Stay;

    int32 PresentedRow = INDEX_NONE;
};

struct FSmartTableRowSetFacts
{
    bool bScrollToAddedRow = true;

    bool bStickToEnd = true;

    bool bViewAtEnd = true;

    bool bMoving = false;

    int32 PresentedBefore = 0;

    int32 PresentedAfter = 0;

    float OffsetBefore = 0.0f;
};

struct FSmartTableHeldRow
{

    FName RowId;

    int32 PresentedRow = INDEX_NONE;

    float Offset = 0.0f;

    float OffsetAt( int32 PresentedNow ) const
    {
        return PresentedNow + ( Offset - PresentedRow );
    }
};

struct FSmartTableScrollTarget
{

    float Offset = 0.0f;

    FSmartTableKeptRow Row;

    float ItemsPerSecond = 0.0f;
};

struct SMARTTABLES_API FSmartTableScrollMove
{

    TOptional< FSmartTableScrollTarget > Move;

    bool bViewAtEnd = true;

    bool bOpenedAtEnd = false;

    FSmartTableHeldRow LeftAtTop;

    bool Aim( int32 PresentedRow, int32 PresentedCount, const FSmartTableKeptRow & Row, float RowsOnScreen, float From, float Seconds );

    int32 Step( float Current, float DeltaTime, float & OutOffset );

    void Stop()
    {
        Move.Reset();
    }

    void Reset()
    {
        *this = FSmartTableScrollMove();
    }

    static constexpr float EndSlackRows = 0.25f;

    bool IsHeadingTo( int32 NaturalRow ) const
    {
        return Move.IsSet() && Move->Row.NaturalRow == NaturalRow;
    }

    static bool IsViewAtEnd( float RemainingFraction, int32 NumPresented );

    static FSmartTableRowSetScroll ScrollAfterRowSet( const FSmartTableRowSetFacts & Facts, const FSmartTableRowSetDiff & Diff, TFunctionRef< int32() > LastDrawnArrival );
};
