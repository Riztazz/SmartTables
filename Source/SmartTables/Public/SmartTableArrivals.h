// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#pragma once

#include "Containers/Map.h"
#include "HAL/Platform.h"
#include "SmartTableRowSetDiff.h"
#include "UObject/NameTypes.h"

struct FSmartTableRowArrival
{

    double ArrivedAt = 0.0;

    bool bPlayed = false;
};

struct SMARTTABLES_API FSmartTableArrivals
{

    void Apply( const FSmartTableRowSetDiff & Diff, double Now );

    void Mark( FName RowId, double Now );

    void Unmark( FName RowId );

    bool IsEmpty() const
    {
        return Rows.IsEmpty();
    }

    bool IsNew( FName RowId ) const;

    bool OwesEntrance( FName RowId ) const;

    void MarkPlayed( FName RowId, bool bPlayed );

    void Expire( double Now, float WindowSeconds );

    void Reset()
    {
        *this = FSmartTableArrivals();
    }

    static bool IsRevealed( float RowTop, float RowHeight, float ListTop, float ListBottom, float Fraction );

private:
    TMap< FName, FSmartTableRowArrival > Rows;
};
