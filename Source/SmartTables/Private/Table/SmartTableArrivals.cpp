// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "SmartTableArrivals.h"

#include "Math/UnrealMathUtility.h"

void FSmartTableArrivals::Apply( const FSmartTableRowSetDiff & Diff, double Now )
{
    if ( Diff.bReplaced )
    {
        Reset();
    }

    for ( const FName RowId : Diff.Left )
    {
        Unmark( RowId );
    }

    for ( const FSmartTableKeptRow & Row : Diff.Arrived )
    {
        Mark( Row.RowId, Now );
    }
}

void FSmartTableArrivals::Mark( FName RowId, double Now )
{
    FSmartTableRowArrival & Arrival = Rows.FindOrAdd( RowId );

    Arrival.ArrivedAt = Now;
    Arrival.bPlayed   = false;
}

void FSmartTableArrivals::Unmark( FName RowId )
{
    Rows.Remove( RowId );
}

bool FSmartTableArrivals::IsNew( FName RowId ) const
{
    return Rows.Contains( RowId );
}

bool FSmartTableArrivals::OwesEntrance( FName RowId ) const
{
    const FSmartTableRowArrival * Arrival = Rows.Find( RowId );

    return Arrival && !Arrival->bPlayed;
}

void FSmartTableArrivals::MarkPlayed( FName RowId, bool bPlayed )
{
    if ( FSmartTableRowArrival * Arrival = Rows.Find( RowId ) )
    {
        Arrival->bPlayed = bPlayed;
    }
}

void FSmartTableArrivals::Expire( double Now, float WindowSeconds )
{
    for ( auto It = Rows.CreateIterator(); It; ++It )
    {
        if ( Now - It.Value().ArrivedAt > WindowSeconds )
        {
            It.RemoveCurrent();
        }
    }
}

bool FSmartTableArrivals::IsRevealed( float RowTop, float RowHeight, float ListTop, float ListBottom, float Fraction )
{
    if ( RowHeight <= 0.0f )
    {
        return false;
    }

    const float Shown = FMath::Max( 0.0f, FMath::Min( RowTop + RowHeight, ListBottom ) - FMath::Max( RowTop, ListTop ) );

    return Shown / RowHeight >= Fraction;
}
