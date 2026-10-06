// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#pragma once

#include "Templates/SharedPointer.h"
#include "UObject/WeakObjectPtrTemplates.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"

class USmartTable;

class SSmartTableRoot : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS( SSmartTableRoot )
    {
    }
    SLATE_DEFAULT_SLOT( FArguments, Content )
    SLATE_END_ARGS()

    void Construct( const FArguments & InArgs, USmartTable * InTable, TSharedPtr< SWidget > InFocusTarget );

    virtual bool SupportsKeyboardFocus() const override;

    virtual FReply OnMouseButtonDown( const FGeometry &, const FPointerEvent & ) override;

    virtual FReply OnFocusReceived( const FGeometry &, const FFocusEvent & ) override;

private:
    TWeakObjectPtr< USmartTable > Table;

    TWeakPtr< SWidget > FocusTarget;
};
