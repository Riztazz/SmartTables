// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#pragma once

#include "Containers/UnrealString.h"
#include "Internationalization/Text.h"
#include "Layout/Margin.h"
#include "UObject/NameTypes.h"

namespace SmartTable::Metrics
{

    constexpr float ColumnEdgeMargin = 2.0f;

    constexpr float SortGlyphAllowance = 18.0f;

    constexpr float WidthEpsilon = 0.5f;

    constexpr float MarkerThickness = 2.0f;

    constexpr float OutlineWidth = 2.0f;

    constexpr float NumericColumnWidth = 110.0f;

    constexpr float ToggleColumnWidth = 80.0f;

    inline FMargin ChromeCellPadding()
    {
        return FMargin( 8.0f, 2.0f );
    }
}

namespace SmartTable::Text
{

    inline FText UnresolvedBinding( FName BindingName )
    {
        return FText::FromString( FString::Printf( TEXT( "<%s?>" ), *BindingName.ToString() ) );
    }
}
