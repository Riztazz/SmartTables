// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#pragma once

#include "Containers/Array.h"
#include "Containers/ArrayView.h"
#include "Containers/Map.h"
#include "Misc/Optional.h"
#include "SmartTableTypes.h"
#include "Templates/Function.h"
#include "UObject/NameTypes.h"

namespace SmartTable::ColumnLayout
{

    struct FColumn
    {

        FName ColumnId;

        ESmartTableColumnSizing Sizing = ESmartTableColumnSizing::Fill;

        float AuthoredWidth = 1.0f;

        float CurrentWidth = 0.0f;

        bool bResizable = true;

        TOptional< float > UserWidth;
    };

    struct FResolvedWidth
    {
        FName ColumnId;

        float Width = 0.0f;
    };

    SMARTTABLES_API TArray< FResolvedWidth > ResolveWidths( TArrayView< const FColumn > Columns, float TotalWidth, float MinWidth, bool bStretchLast );

    struct FEdgeHit
    {

        FName ColumnId;

        float Left = 0.0f;
    };

    SMARTTABLES_API FEdgeHit FindEdgeAt( TArrayView< const FColumn > ShownColumns, float LocalX, float GripWidth, float LeadingOffset );

    struct FGrantedDrag
    {

        float Granted = 0.0f;

        TArray< float > RightWidths;

        bool bNegotiated = false;
    };

    SMARTTABLES_API FGrantedDrag ResolveGrowDrag( float StartDraggedWidth, float DesiredWidth, TArrayView< const float > StartRightWidths, float MinWidth );

    SMARTTABLES_API TArray< int32 > WidthsToWrite( TArrayView< const float > Targets, TArrayView< const float > Drawn, float Epsilon );

    SMARTTABLES_API float StretchLast( float BaseWidth, float TotalWidth, float OthersTotal );

    enum class EHeaderWidthMode : uint8
    {
        Manual,
        Fixed,
        Fill
    };

    SMARTTABLES_API EHeaderWidthMode HeaderWidthModeFor( const FSmartTableColumn & Column, bool bWidthsLive );

    SMARTTABLES_API TArray< FName > MovedColumns( const TMap< FName, float > & Before, TArrayView< const FResolvedWidth > After, float Epsilon );

    SMARTTABLES_API float AutoScrollStep( float CursorX, float ViewportWidth, float EdgeZone, float Step );

    SMARTTABLES_API TArray< FName > MergeOrder( TArrayView< const FName > AuthoredIds, TArrayView< const FName > RequestedIds );

    SMARTTABLES_API int32 LandingIndex( TArrayView< const FName > Order, int32 From, int32 Delta, TFunctionRef< bool( FName ) > IsShown );

    SMARTTABLES_API FName ColumnAt( TArrayView< const FColumn > ShownColumns, float LocalX, float LeadingOffset );
}
