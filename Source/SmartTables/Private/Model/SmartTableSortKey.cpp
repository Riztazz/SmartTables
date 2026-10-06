// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "SmartTableSortKey.h"

#include "SmartTableSorting.h"

FSmartTableSortKey FSmartTableSortKey::MakeEmpty()
{
    return FSmartTableSortKey();
}

FSmartTableSortKey FSmartTableSortKey::MakeNumber( double InNumber )
{
    FSmartTableSortKey Key;
    Key.Kind   = ESmartTableSortKeyKind::Numeric;
    Key.Number = InNumber;

    return Key;
}

FSmartTableSortKey FSmartTableSortKey::MakeBool( bool bInValue )
{
    return MakeNumber( bInValue ? 1.0 : 0.0 );
}

FSmartTableSortKey FSmartTableSortKey::MakeText( const FString & InText )
{
    FSmartTableSortKey Key;
    Key.Kind = ESmartTableSortKeyKind::String;
    Key.Text = InText;

    return Key;
}

int32 FSmartTableSortKey::Compare( const FSmartTableSortKey & Other ) const
{
    if ( Kind != Other.Kind )
    {

        return Kind < Other.Kind ? -1 : 1;
    }

    switch ( Kind )
    {
        case ESmartTableSortKeyKind::Numeric:
            return Number < Other.Number ? -1 : ( Number > Other.Number ? 1 : 0 );

        case ESmartTableSortKeyKind::String:
            return SmartTable::Sorting::CompareNatural( Text, Other.Text );

        default:
            return 0;
    }
}
