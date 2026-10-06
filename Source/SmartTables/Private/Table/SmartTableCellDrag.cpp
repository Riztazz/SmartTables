// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "SmartTableCellDrag.h"

ESmartTableCellDragRole FSmartTableCellDrag::RoleOf( FName RowId, FName ColumnId ) const
{
    if ( RowId.IsNone() )
    {
        return ESmartTableCellDragRole::None;
    }

    if ( RowId == SourceRowId && ColumnId == SourceColumnId )
    {
        return ESmartTableCellDragRole::Source;
    }

    if ( RowId == TargetRowId && ColumnId == TargetColumnId )
    {
        return ESmartTableCellDragRole::Target;
    }

    return ESmartTableCellDragRole::None;
}

void FSmartTableCellDrag::Begin( FName RowId, FName ColumnId )
{
    Reset();

    SourceRowId    = RowId;
    SourceColumnId = ColumnId;
}

bool FSmartTableCellDrag::SetTarget( FName RowId, FName ColumnId )
{
    if ( SourceRowId.IsNone() || ( TargetRowId == RowId && TargetColumnId == ColumnId ) )
    {
        return false;
    }

    TargetRowId    = RowId;
    TargetColumnId = ColumnId;

    return true;
}

bool FSmartTableCellDrag::ClearTarget( FName RowId, FName ColumnId )
{
    if ( TargetRowId != RowId || TargetColumnId != ColumnId )
    {
        return false;
    }

    TargetRowId    = NAME_None;
    TargetColumnId = NAME_None;

    return true;
}

bool FSmartTableCellDrag::End()
{
    if ( !IsActive() )
    {
        return false;
    }

    Reset();

    return true;
}
