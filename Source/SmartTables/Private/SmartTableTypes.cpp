// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "SmartTableTypes.h"

ESmartTableSortMode FSmartTableSortSpec::GetModeFor( FName ColumnId ) const
{
    const FSmartTableSortColumn * Found = Columns.FindByPredicate( [ ColumnId ]( const FSmartTableSortColumn & Column )
    {
        return Column.ColumnId == ColumnId;
    } );

    return Found ? Found->Mode : ESmartTableSortMode::None;
}

int32 FSmartTableSortSpec::GetPriorityFor( FName ColumnId ) const
{
    return Columns.IndexOfByPredicate( [ ColumnId ]( const FSmartTableSortColumn & Column )
    {
        return Column.ColumnId == ColumnId;
    } );
}

FName FSmartTableColumn::GetValueBinding() const
{
    return BindingName.IsNone() ? ColumnId : BindingName;
}

FText FSmartTableColumn::GetLabel() const
{
    return Header.IsEmpty() ? FText::FromName( ColumnId ) : Header;
}

FName FSmartTableColumn::GetSortBinding() const
{
    return SortBindingName.IsNone() ? GetValueBinding() : SortBindingName;
}
