// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#pragma once

#include "SmartTableTypes.h"
#include "UObject/NameTypes.h"

struct SMARTTABLES_API FSmartTableCellDrag
{

    FName SourceRowId;
    FName SourceColumnId;

    FName TargetRowId;
    FName TargetColumnId;

    bool IsActive() const
    {
        return !SourceRowId.IsNone() || !TargetRowId.IsNone();
    }

    ESmartTableCellDragRole RoleOf( FName RowId, FName ColumnId ) const;

    void Begin( FName RowId, FName ColumnId );

    bool SetTarget( FName RowId, FName ColumnId );

    bool ClearTarget( FName RowId, FName ColumnId );

    bool End();

    void Reset()
    {
        *this = FSmartTableCellDrag();
    }
};
