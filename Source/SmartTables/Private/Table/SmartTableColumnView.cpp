// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "Table/SmartTableColumnView.h"

#include "Algo/Transform.h"
#include "Misc/Optional.h"
#include "SmartTableHelpers.h"

SmartTable::FColumnView::FColumnView( const TArray< FSmartTableColumn > & InColumns, const FSmartTableLayout & InLayout, const TMap< FName, float > & InResolvedWidths )
    : Columns( InColumns )
    , Layout( InLayout )
    , ResolvedWidths( InResolvedWidths )
{
}

const FSmartTableColumn * SmartTable::FColumnView::Find( FName ColumnId ) const
{
    return FindByColumnId( Columns, ColumnId );
}

const FSmartTableColumnLayout * SmartTable::FColumnView::FindOverride( FName ColumnId ) const
{
    return FindByColumnId( Layout.Columns, ColumnId );
}

bool SmartTable::FColumnView::IsShown( const FSmartTableColumn & Column ) const
{
    const FSmartTableColumnLayout * Override = FindOverride( Column.ColumnId );

    return Override ? !Override->bHidden : !Column.bHiddenByDefault;
}

bool SmartTable::FColumnView::IsShown( FName ColumnId ) const
{
    const FSmartTableColumn * Column = Find( ColumnId );

    return Column && IsShown( *Column );
}

TArray< FName > SmartTable::FColumnView::AuthoredIds() const
{
    TArray< FName > Ids;
    Ids.Reserve( Columns.Num() );

    Algo::Transform( Columns, Ids, &FSmartTableColumn::ColumnId );

    return Ids;
}

TArray< FName > SmartTable::FColumnView::Order() const
{
    const TArray< FName > Authored = AuthoredIds();

    return Layout.Order.IsEmpty() ? Authored : ColumnLayout::MergeOrder( Authored, Layout.Order );
}

TArray< FName > SmartTable::FColumnView::ShownOrder() const
{
    TArray< FName > Shown = Order();

    Shown.RemoveAll( [ this ]( FName ColumnId )
    {
        return !IsShown( ColumnId );
    } );

    return Shown;
}

float SmartTable::FColumnView::CurrentWidth( FName ColumnId ) const
{
    if ( const float * Resolved = ResolvedWidths.Find( ColumnId ) )
    {
        return *Resolved;
    }

    const FSmartTableColumnLayout * Override = FindOverride( ColumnId );
    if ( Override && Override->bUserWidth && Override->Width > 0.0f )
    {
        return Override->Width;
    }

    const FSmartTableColumn * Column = Find( ColumnId );
    if ( !Column )
    {
        return 0.0f;
    }

    return Column->Sizing == ESmartTableColumnSizing::Fixed ? Column->Width : 0.0f;
}

TArray< SmartTable::ColumnLayout::FColumn > SmartTable::FColumnView::Metrics() const
{
    TArray< ColumnLayout::FColumn > All;
    All.Reserve( Columns.Num() );

    for ( const FSmartTableColumn & Column : Columns )
    {
        const FSmartTableColumnLayout * Override = FindOverride( Column.ColumnId );

        ColumnLayout::FColumn & Metric = All.AddDefaulted_GetRef();
        Metric.ColumnId                = Column.ColumnId;
        Metric.Sizing                  = Column.Sizing;
        Metric.AuthoredWidth           = Column.Width;
        Metric.CurrentWidth            = CurrentWidth( Column.ColumnId );
        Metric.bResizable              = Column.bResizable;

        Metric.UserWidth = Override && Override->bUserWidth ? TOptional< float >( Override->Width ) : TOptional< float >();
    }

    return All;
}

TArray< SmartTable::ColumnLayout::FColumn > SmartTable::FColumnView::ShownMetrics() const
{
    const TArray< ColumnLayout::FColumn > All = Metrics();

    TArray< ColumnLayout::FColumn > Shown;
    Shown.Reserve( All.Num() );

    for ( const FName ColumnId : ShownOrder() )
    {
        if ( const ColumnLayout::FColumn * Metric = FindByColumnId( All, ColumnId ) )
        {
            Shown.Add( *Metric );
        }
    }

    return Shown;
}
