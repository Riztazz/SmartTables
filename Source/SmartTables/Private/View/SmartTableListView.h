// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#pragma once

#include "View/SmartTableRowIndex.h"
#include "Widgets/Views/SListView.h"

class USmartTable;

class SSmartTableListView : public SListView< int32 >
{
public:
    virtual FReply OnMouseButtonDown( const FGeometry & Geometry, const FPointerEvent & MouseEvent ) override;
    virtual FReply OnMouseMove( const FGeometry & Geometry, const FPointerEvent & MouseEvent ) override;

    bool IsRightDragScrolling() const;

    TWeakObjectPtr< USmartTable > Table;

private:

    float RightDragDistance = 0.0f;
};
