// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#pragma once

#include "Containers/Array.h"
#include "Containers/ArrayView.h"
#include "Containers/Set.h"
#include "HAL/Platform.h"
#include "SmartTableKeptRow.h"
#include "UObject/NameTypes.h"

struct SMARTTABLES_API FSmartTableRowSetDiff
{

    TArray< FSmartTableKeptRow > Arrived;

    TSet< FName > Left;

    bool bReplaced = false;

    static FSmartTableRowSetDiff Replaced()
    {
        FSmartTableRowSetDiff Diff;
        Diff.bReplaced = true;

        return Diff;
    }

    bool NamesRows() const
    {
        return !bReplaced && ( !Arrived.IsEmpty() || !Left.IsEmpty() );
    }

    void Add( const FSmartTableRowSetDiff & Later );

    static FSmartTableRowSetDiff Between( TConstArrayView< FName > Before, TConstArrayView< FName > After );
};
