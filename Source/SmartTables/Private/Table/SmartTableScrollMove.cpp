// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "SmartTableScrollMove.h"

#include "Math/UnrealMathUtility.h"
#include "Misc/AssertionMacros.h"

bool FSmartTableScrollMove::Aim( int32 PresentedRow, int32 PresentedCount, const FSmartTableKeptRow & Row, float RowsOnScreen, float From, float Seconds )
{
    Move.Reset();

    if ( Seconds <= 0.0f )
    {
        return false;
    }

    const float Target = FMath::Max( 0.0f, PresentedRow + 1.0f - RowsOnScreen );

    if ( PresentedCount > 0 && PresentedRow == PresentedCount - 1 && Target <= From )
    {
        return false;
    }

    if ( FMath::IsNearlyEqual( From, Target, 0.01f ) )
    {
        return false;
    }

    FSmartTableScrollTarget Aimed;

    Aimed.Offset         = Target;
    Aimed.Row            = Row;
    Aimed.ItemsPerSecond = FMath::Abs( Target - From ) / Seconds;

    Move = Aimed;

    return true;
}

int32 FSmartTableScrollMove::Step( float Current, float DeltaTime, float & OutOffset )
{
    checkf( Move.IsSet(), TEXT( "A step was asked for with nothing aimed. Callers test Move first." ) );

    const float Target = Move->Offset;
    const float Reach  = Move->ItemsPerSecond * DeltaTime;

    if ( FMath::Abs( Target - Current ) <= Reach )
    {
        const int32 Arrived = Move->Row.NaturalRow;

        Stop();

        OutOffset = Target;

        return Arrived;
    }

    OutOffset = Current + FMath::Sign( Target - Current ) * Reach;

    return INDEX_NONE;
}

bool FSmartTableScrollMove::IsViewAtEnd( float RemainingFraction, int32 NumPresented )
{
    return NumPresented <= 0 || RemainingFraction * NumPresented <= EndSlackRows;
}

FSmartTableRowSetScroll FSmartTableScrollMove::ScrollAfterRowSet( const FSmartTableRowSetFacts & Facts, const FSmartTableRowSetDiff & Diff, TFunctionRef< int32() > LastDrawnArrival )
{
    FSmartTableRowSetScroll Answer;

    if ( Diff.bReplaced || Facts.PresentedBefore == 0 )
    {
        return Answer;
    }

    if ( Facts.bScrollToAddedRow && !Diff.Arrived.IsEmpty() )
    {
        if ( Facts.bStickToEnd && Facts.bViewAtEnd )
        {
            if ( Facts.PresentedAfter > Facts.PresentedBefore )
            {
                Answer.Kind         = ESmartTableRowSetScroll::ToRow;
                Answer.PresentedRow = Facts.PresentedAfter - 1;
            }

            return Answer;
        }

        if ( !Facts.bStickToEnd )
        {
            const int32 Arrival = LastDrawnArrival();
            if ( Arrival != INDEX_NONE )
            {
                Answer.Kind         = ESmartTableRowSetScroll::ToRow;
                Answer.PresentedRow = Arrival;

                return Answer;
            }
        }
    }

    if ( Diff.NamesRows() && !Facts.bMoving && !Facts.bViewAtEnd && Facts.OffsetBefore > 0.0f )
    {
        Answer.Kind = ESmartTableRowSetScroll::Hold;
    }

    return Answer;
}
