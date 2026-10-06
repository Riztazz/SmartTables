// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "SmartTableSearchHighlightCustomization.h"

#include "Algo/AllOf.h"
#include "Containers/Array.h"
#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "IDetailPropertyRow.h"
#include "Misc/Attribute.h"
#include "PropertyHandle.h"
#include "SmartTable.h"
#include "Templates/Casts.h"

namespace
{
    TAttribute< bool > WhileATableHasAMenu( const TSharedRef< IPropertyHandle > & StructHandle )
    {
        return TAttribute< bool >::CreateLambda( [ StructHandle ]()
        {
            TArray< UObject * > Outers;
            StructHandle->GetOuterObjects( Outers );

            return Algo::AllOf( Outers, []( const UObject * Outer )
            {
                const USmartTable * Table = Cast< USmartTable >( Outer );
                return !Table || Table->HasRightClickMenu();
            } );
        } );
    }
}

TSharedRef< IPropertyTypeCustomization > FSmartTableSearchHighlightCustomization::MakeInstance()
{
    return MakeShared< FSmartTableSearchHighlightCustomization >();
}

void FSmartTableSearchHighlightCustomization::CustomizeHeader( TSharedRef< IPropertyHandle > StructHandle, FDetailWidgetRow & HeaderRow, IPropertyTypeCustomizationUtils & Utils )
{
    // clang-format off
    HeaderRow
        .EditCondition( WhileATableHasAMenu( StructHandle ), FOnBooleanValueChanged() )
        .NameContent()
        [
            StructHandle->CreatePropertyNameWidget()
        ];
    // clang-format on
}

void FSmartTableSearchHighlightCustomization::CustomizeChildren( TSharedRef< IPropertyHandle > StructHandle, IDetailChildrenBuilder & ChildBuilder, IPropertyTypeCustomizationUtils & Utils )
{
    const TAttribute< bool > Usable = WhileATableHasAMenu( StructHandle );

    uint32 ChildCount = 0;
    StructHandle->GetNumChildren( ChildCount );

    for ( uint32 ChildIndex = 0; ChildIndex < ChildCount; ++ChildIndex )
    {
        ChildBuilder.AddProperty( StructHandle->GetChildHandle( ChildIndex ).ToSharedRef() ).EditCondition( Usable, FOnBooleanValueChanged() );
    }
}
