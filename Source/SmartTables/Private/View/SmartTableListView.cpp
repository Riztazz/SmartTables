// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "View/SmartTableListView.h"

#include "Framework/Application/SlateApplication.h"
#include "SmartTable.h"

FReply SSmartTableListView::OnMouseButtonDown( const FGeometry & Geometry, const FPointerEvent & MouseEvent )
{

    if ( MouseEvent.GetEffectingButton() == EKeys::RightMouseButton )
    {
        RightDragDistance = 0.0f;
    }

    return SListView< int32 >::OnMouseButtonDown( Geometry, MouseEvent );
}

FReply SSmartTableListView::OnMouseMove( const FGeometry & Geometry, const FPointerEvent & MouseEvent )
{

    const FReply Reply = SListView< int32 >::OnMouseMove( Geometry, MouseEvent );

    if ( !MouseEvent.IsMouseButtonDown( EKeys::RightMouseButton ) )
    {
        RightDragDistance = 0.0f;

        return Reply;
    }

    if ( !bEnableRightClickScrolling )
    {
        return Reply;
    }

    USmartTable * OwningTable = Table.Get();

    if ( !OwningTable || !OwningTable->AllowsHorizontalScroll() )
    {
        return Reply;
    }

    RightDragDistance += MouseEvent.GetCursorDelta().Size();

    if ( RightDragDistance < FSlateApplication::Get().GetDragTriggerDistance() )
    {
        return Reply;
    }

    OwningTable->ScrollColumnsBy( -MouseEvent.GetCursorDelta().X / FMath::Max( Geometry.Scale, UE_KINDA_SMALL_NUMBER ) );

    return Reply;
}

bool SSmartTableListView::IsRightDragScrolling() const
{
    return IsRightClickScrolling() || RightDragDistance >= FSlateApplication::Get().GetDragTriggerDistance();
}
