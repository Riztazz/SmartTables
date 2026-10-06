// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "SmartTableStyleDetails.h"

#include "DetailLayoutBuilder.h"
#include "PropertyHandle.h"

namespace SmartTablesEditor
{
    void HideLastColumnStyle( IDetailLayoutBuilder & DetailBuilder )
    {
        const TSharedPtr< IPropertyHandle > Header = DetailBuilder.GetProperty( TEXT( "HeaderStyle" ) );
        if ( !Header.IsValid() )
        {
            return;
        }

        if ( const TSharedPtr< IPropertyHandle > LastColumn = Header->GetChildHandle( TEXT( "LastColumnStyle" ) ) )
        {
            DetailBuilder.HideProperty( LastColumn );
        }
    }
}

TSharedRef< IDetailCustomization > FSmartTableStyleDetails::MakeInstance()
{
    return MakeShared< FSmartTableStyleDetails >();
}

void FSmartTableStyleDetails::CustomizeDetails( IDetailLayoutBuilder & DetailBuilder )
{
    SmartTablesEditor::HideLastColumnStyle( DetailBuilder );
}
