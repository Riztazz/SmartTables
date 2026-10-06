// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "SmartTableCellDragDropOp.h"

#include "Components/PanelWidget.h"
#include "SmartTable.h"
#include "SmartTableCell.h"

void USmartTableCellDragDropOp::Drop_Implementation( const FPointerEvent & PointerEvent )
{
    EndDrag();

    Super::Drop_Implementation( PointerEvent );
}

void USmartTableCellDragDropOp::DragCancelled_Implementation( const FPointerEvent & PointerEvent )
{
    EndDrag();

    Super::DragCancelled_Implementation( PointerEvent );
}

void USmartTableCellDragDropOp::EndDrag()
{
    if ( USmartTable * Table = SourceTable.Get() )
    {
        Table->EndCellDrag();
    }

    ReleaseCellsIn( DefaultDragVisual );
}

void USmartTableCellDragDropOp::ReleaseCellsIn( UWidget * Widget )
{
    if ( !Widget )
    {
        return;
    }

    if ( USmartTableCell * Cell = Cast< USmartTableCell >( Widget ) )
    {
        Cell->ReleaseCell();
        return;
    }

    if ( UPanelWidget * Panel = Cast< UPanelWidget >( Widget ) )
    {
        for ( int32 Index = 0; Index < Panel->GetChildrenCount(); ++Index )
        {
            ReleaseCellsIn( Panel->GetChildAt( Index ) );
        }
    }
}
