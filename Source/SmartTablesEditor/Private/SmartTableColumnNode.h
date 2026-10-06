// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#pragma once

#include "SmartTableLazyNode.h"
#include "Templates/SharedPointer.h"

class FSmartTableColumnsBuilder;
class IPropertyHandle;

class FSmartTableColumnNode : public FSmartTableLazyNode
{
public:
    FSmartTableColumnNode( TSharedRef< IPropertyHandle > InElementHandle, int32 InIndex, TWeakPtr< FSmartTableColumnsBuilder > InOwner );

    virtual void GenerateHeaderRowContent( FDetailWidgetRow & NodeRow ) override;
    virtual FName GetName() const override;
    virtual TSharedPtr< IPropertyHandle > GetPropertyHandle() const override;

protected:
    virtual void GenerateRealChildren( IDetailChildrenBuilder & ChildrenBuilder ) override;

private:
    FText Title() const;

    TSharedRef< IPropertyHandle > ElementHandle;
    int32 Index = 0;

    TWeakPtr< FSmartTableColumnsBuilder > Owner;
};
