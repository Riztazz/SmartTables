// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "SmartTableRowSetDiff.h"

void FSmartTableRowSetDiff::Add( const FSmartTableRowSetDiff & Later )
{
    if ( Later.bReplaced )
    {
        *this = Later;
        return;
    }

    Arrived.RemoveAll( [ &Later ]( const FSmartTableKeptRow & Row )
    {
        return Later.Left.Contains( Row.RowId );
    } );
    Left.Append( Later.Left );

    for ( const FSmartTableKeptRow & Row : Later.Arrived )
    {
        Left.Remove( Row.RowId );
    }
    Arrived.Append( Later.Arrived );
}

FSmartTableRowSetDiff FSmartTableRowSetDiff::Between( TConstArrayView< FName > Before, TConstArrayView< FName > After )
{
    const TSet< FName > BeforeIds( Before );

    FSmartTableRowSetDiff Diff;

    for ( int32 Row = 0; Row < After.Num(); ++Row )
    {
        if ( !BeforeIds.Contains( After[ Row ] ) )
        {
            Diff.Arrived.Add( { After[ Row ], Row } );
        }
    }

    if ( Diff.Arrived.Num() == After.Num() )
    {
        return Replaced();
    }

    const TSet< FName > AfterIds( After );

    for ( const FName RowId : Before )
    {
        if ( !AfterIds.Contains( RowId ) )
        {
            Diff.Left.Add( RowId );
        }
    }

    return Diff;
}
