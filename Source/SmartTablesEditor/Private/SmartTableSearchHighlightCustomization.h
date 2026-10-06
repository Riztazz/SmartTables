// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#pragma once

#include "IPropertyTypeCustomization.h"
#include "Templates/SharedPointer.h"

class FDetailWidgetRow;
class IDetailChildrenBuilder;
class IPropertyHandle;

class FSmartTableSearchHighlightCustomization : public IPropertyTypeCustomization
{
public:
    static TSharedRef< IPropertyTypeCustomization > MakeInstance();

    virtual void CustomizeHeader( TSharedRef< IPropertyHandle > StructHandle, FDetailWidgetRow & HeaderRow, IPropertyTypeCustomizationUtils & Utils ) override;
    virtual void CustomizeChildren( TSharedRef< IPropertyHandle > StructHandle, IDetailChildrenBuilder & ChildBuilder, IPropertyTypeCustomizationUtils & Utils ) override;
};
