// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#pragma once

#include "Widgets/Views/SHeaderRow.h"

class USmartTable;

class SSmartTableHeaderRow : public SHeaderRow
{
public:

    virtual FReply OnMouseButtonUp( const FGeometry & Geometry, const FPointerEvent & MouseEvent ) override;

    virtual void OnMouseCaptureLost( const FCaptureLostEvent & CaptureLostEvent ) override;

    TWeakObjectPtr< USmartTable > Table;
};
