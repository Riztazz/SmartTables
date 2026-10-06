// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "SmartTableSettings.h"

#include "Style/SmartTableStyleRefresh.h"

#if WITH_EDITOR

void USmartTableSettings::PostEditChangeProperty( FPropertyChangedEvent & PropertyChangedEvent )
{
    Super::PostEditChangeProperty( PropertyChangedEvent );

    const FProperty * Edited = PropertyChangedEvent.MemberProperty;

    if ( Edited && Edited->GetMetaData( TEXT( "Category" ) ) != TEXT( "Palette" ) )
    {
        return;
    }

    SmartTable::StyleRefresh::FromSettings();
}

#endif
