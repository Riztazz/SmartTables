// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "View/SmartTableRoot.h"

#include "Input/Events.h"
#include "Input/Reply.h"
#include "Layout/Geometry.h"
#include "SmartTable.h"

void SSmartTableRoot::Construct( const FArguments & InArgs, USmartTable * InTable, TSharedPtr< SWidget > InFocusTarget )
{
    Table       = InTable;
    FocusTarget = InFocusTarget;

    ChildSlot[ InArgs._Content.Widget ];
}

bool SSmartTableRoot::SupportsKeyboardFocus() const
{
    return true;
}

FReply SSmartTableRoot::OnMouseButtonDown( const FGeometry &, const FPointerEvent & )
{
    const USmartTable * Owner = Table.Get();
    if ( !Owner || !Owner->IsFocusOnPointerInteractionEnabled() )
    {
        return FReply::Unhandled();
    }

    return FReply::Handled();
}

FReply SSmartTableRoot::OnFocusReceived( const FGeometry &, const FFocusEvent & )
{
    const TSharedPtr< SWidget > Target = FocusTarget.Pin();

    return Target.IsValid() ? FReply::Handled().SetUserFocus( Target.ToSharedRef(), EFocusCause::SetDirectly ) : FReply::Unhandled();
}
