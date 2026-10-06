// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#pragma once

#include "SmartTableLazyNode.h"
#include "Templates/SharedPointer.h"
#include "UObject/NameTypes.h"

class FDetailWidgetRow;
class FProperty;
class IDetailChildrenBuilder;
class IDetailLayoutBuilder;
class IDetailsView;
class IPropertyHandle;
class IPropertyTypeCustomization;

class FSmartTableStyleNode : public FSmartTableLazyNode
{
public:
    FSmartTableStyleNode( TSharedRef< IPropertyHandle > InHandle, IDetailLayoutBuilder & InLayout );

    static bool ShouldDefer( const FProperty & Property );

    static void AddChild( IDetailChildrenBuilder & ChildrenBuilder, TSharedRef< IPropertyHandle > ChildHandle );

    static void AddChildren( IDetailChildrenBuilder & ChildrenBuilder, const TSharedRef< IPropertyHandle > & Parent );

    static FText FilterText( const TSharedRef< IPropertyHandle > & Handle );

    virtual void GenerateHeaderRowContent( FDetailWidgetRow & NodeRow ) override;
    virtual FName GetName() const override;
    virtual TSharedPtr< IPropertyHandle > GetPropertyHandle() const override;

protected:
    virtual void GenerateRealChildren( IDetailChildrenBuilder & ChildrenBuilder ) override;

private:
    TSharedRef< IPropertyHandle > Handle;

    TWeakPtr< IDetailsView > DetailsView;

    TSharedPtr< IPropertyTypeCustomization > BrushCustomization;
};
