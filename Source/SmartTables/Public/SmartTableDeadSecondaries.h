// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#pragma once

#include "Containers/ArrayView.h"
#include "Containers/Set.h"
#include "SmartTableSortKey.h"
#include "SmartTableTypes.h"
#include "Templates/Function.h"
#include "UObject/NameTypes.h"

struct SMARTTABLES_API FSmartTableDeadSecondaries
{

    void MarkStale()
    {
        bStale = true;
    }

    bool IsStale() const
    {
        return bStale;
    }

    bool Contains( FName ColumnId ) const
    {
        return Columns.Contains( ColumnId );
    }

    void Recompute( const FSmartTableSortSpec & Spec, int32 NumPresentedRows, TArrayView< const FName > Candidates, TFunctionRef< FSmartTableSortKey( int32 PresentedRow, FName ColumnId ) > ReadKey );

    void Clear();

private:
    TSet< FName > Columns;
    bool bStale = true;
};
