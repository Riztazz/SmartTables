// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "View/SmartTableHeaderRow.h"

#include "Input/Events.h"
#include "SmartTable.h"

FReply SSmartTableHeaderRow::OnMouseButtonUp( const FGeometry & Geometry, const FPointerEvent & MouseEvent )
{
    USmartTable * OwningTable = Table.Get();

    if ( OwningTable && OwningTable->EndPointerResize( MouseEvent.GetUserIndex(), MouseEvent.GetPointerIndex() ) )
    {

        return FReply::Handled().ReleaseMouseCapture();
    }

    return SHeaderRow::OnMouseButtonUp( Geometry, MouseEvent );
}

void SSmartTableHeaderRow::OnMouseCaptureLost( const FCaptureLostEvent & CaptureLostEvent )
{
    if ( USmartTable * OwningTable = Table.Get() )
    {
        OwningTable->EndPointerResize( CaptureLostEvent.UserIndex, CaptureLostEvent.PointerIndex );
    }

    SHeaderRow::OnMouseCaptureLost( CaptureLostEvent );
}
