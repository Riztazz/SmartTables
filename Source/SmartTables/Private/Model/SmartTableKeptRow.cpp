// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "SmartTableKeptRow.h"

#include "Table/SmartTableRowSpace.h"

int32 FSmartTableKeptRow::FindNow( int32 NumRows, TFunctionRef< FName( int32 ) > RowIdOf ) const
{
    return SmartTable::RowSpace::FindIdNearInEveryRow( NumRows, RowId, NaturalRow, RowIdOf );
}
