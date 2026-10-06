// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#pragma once

#include "Containers/Array.h"
#include "Containers/Map.h"
#include "SmartTableColumnLayout.h"
#include "SmartTableTypes.h"
#include "UObject/NameTypes.h"

namespace SmartTable
{

    class SMARTTABLES_API FColumnView
    {
    public:
        FColumnView( const TArray< FSmartTableColumn > & InColumns, const FSmartTableLayout & InLayout, const TMap< FName, float > & InResolvedWidths );

        FColumnView( const FColumnView & )             = delete;
        FColumnView & operator=( const FColumnView & ) = delete;

        const FSmartTableColumn * Find( FName ColumnId ) const;

        const FSmartTableColumnLayout * FindOverride( FName ColumnId ) const;

        bool IsShown( const FSmartTableColumn & Column ) const;

        bool IsShown( FName ColumnId ) const;

        TArray< FName > AuthoredIds() const;

        TArray< FName > Order() const;

        TArray< FName > ShownOrder() const;

        float CurrentWidth( FName ColumnId ) const;

        TArray< ColumnLayout::FColumn > Metrics() const;

        TArray< ColumnLayout::FColumn > ShownMetrics() const;

    private:
        const TArray< FSmartTableColumn > & Columns;
        const FSmartTableLayout & Layout;
        const TMap< FName, float > & ResolvedWidths;
    };
}
