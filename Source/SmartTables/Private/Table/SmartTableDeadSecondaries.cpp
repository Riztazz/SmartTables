// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "SmartTableDeadSecondaries.h"

#include "SmartTableSorting.h"

void FSmartTableDeadSecondaries::Recompute( const FSmartTableSortSpec & Spec, int32 NumPresentedRows, TArrayView< const FName > Candidates, TFunctionRef< FSmartTableSortKey( int32 PresentedRow, FName ColumnId ) > ReadKey )
{
    Clear();

    if ( Spec.IsEmpty() )
    {
        return;
    }

    Columns = SmartTable::Sorting::FindColumnsThatCannotBreakTies( NumPresentedRows, Spec.Columns[ 0 ].ColumnId, Candidates, ReadKey );
}

void FSmartTableDeadSecondaries::Clear()
{
    bStale = false;
    Columns.Reset();
}
