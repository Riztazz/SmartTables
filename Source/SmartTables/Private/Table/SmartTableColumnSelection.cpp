// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "SmartTableColumnSelection.h"

#include "Math/UnrealMathUtility.h"

bool FSmartTableColumnSelection::Select( FName ColumnId, TArrayView< const FName > ShownIds )
{
    const int32 At = ColumnId.IsNone() ? INDEX_NONE : ShownIds.Find( ColumnId );
    if ( !ColumnId.IsNone() && At == INDEX_NONE )
    {
        return false;
    }

    if ( Selected == ColumnId )
    {

        LastIndex = At;

        return false;
    }

    Selected  = ColumnId;
    LastIndex = At;

    return true;
}

bool FSmartTableColumnSelection::Move( int32 Delta, TArrayView< const FName > ShownIds )
{
    if ( ShownIds.IsEmpty() )
    {
        return Select( NAME_None, ShownIds );
    }

    const int32 At = ShownIds.Find( Selected );
    if ( At == INDEX_NONE )
    {
        return Select( Delta < 0 ? ShownIds.Last() : ShownIds[ 0 ], ShownIds );
    }

    const int32 Count = ShownIds.Num();

    return Select( ShownIds[ ( ( At + Delta ) % Count + Count ) % Count ], ShownIds );
}

FName FSmartTableColumnSelection::Resolve( FName ColumnId, TArrayView< const FName > ShownIds )
{
    if ( !ColumnId.IsNone() )
    {
        Select( ColumnId, ShownIds );

        return ColumnId;
    }

    Revalidate( ShownIds );

    if ( Selected.IsNone() && !ShownIds.IsEmpty() )
    {

        Select( ShownIds[ 0 ], ShownIds );
    }

    return Selected;
}

bool FSmartTableColumnSelection::Revalidate( TArrayView< const FName > ShownIds )
{
    if ( Selected.IsNone() || ShownIds.Contains( Selected ) )
    {
        return false;
    }

    if ( ShownIds.IsEmpty() )
    {
        return Select( NAME_None, ShownIds );
    }

    return Select( ShownIds[ FMath::Clamp( LastIndex, 0, ShownIds.Num() - 1 ) ], ShownIds );
}
