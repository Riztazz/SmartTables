// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "Logging/StructuredLog.h"
#include "Misc/Attribute.h"
#include "SmartTable.h"
#include "SmartTableColumnLayout.h"
#include "SmartTableColumnSchema.h"
#include "SmartTableDataTableModel.h"
#include "SmartTableLog.h"
#include "SmartTableObjectModel.h"
#include "Table/SmartTableColumnView.h"
#include "View/SmartTableHeaderColumns.h"
#include "View/SmartTableListView.h"
#include "Widgets/Views/SHeaderRow.h"

SmartTable::FColumnView USmartTable::ColumnView() const
{
    return SmartTable::FColumnView( Columns, ActiveLayout, ResolvedWidths );
}

const FSmartTableColumn * USmartTable::FindColumn( FName ColumnId ) const
{
    return ColumnView().Find( ColumnId );
}

const TArray< FSmartTableColumn > & USmartTable::GetColumns() const
{
    return Columns;
}

void USmartTable::SetColumns( const TArray< FSmartTableColumn > & InColumns )
{
    WarnAboutDuplicateColumnIds( InColumns );

    Columns = InColumns;
    ItemlessColumnsWarned.Reset();
    CellClassFailuresWarned.Reset();

    DeadSecondaries.MarkStale();

    UE_LOGFMT( LogSmartTablesLayout, Verbose, "'{Table}' declares {Count} column(s).", GetName(), Columns.Num() );

    if ( ItemsModel )
    {
        ItemsModel->SetColumns( Columns );
    }

    if ( DataTableModel )
    {
        DataTableModel->SetColumns( Columns );
    }

    RebuildHeader();
}

void USmartTable::WarnAboutDuplicateColumnIds( const TArray< FSmartTableColumn > & InColumns )
{
    TSet< FName > Duplicated = SmartTable::DuplicatedColumnIds( InColumns );

    for ( const FName ColumnId : Duplicated )
    {
        if ( DuplicateColumnIdsWarned.Contains( ColumnId ) )
        {
            continue;
        }

        UE_LOGFMT( LogSmartTablesLayout, Warning, "Table '{Table}' carries more than one column with the ColumnId '{Column}'. Every lookup takes the FIRST, so the later one is invisible and never draws, sorts or hides. Give it its own ColumnId, or drop it.", GetName(), ColumnId );
    }

    DuplicateColumnIdsWarned = MoveTemp( Duplicated );
}

FText USmartTable::GetColumnLabel( FName ColumnId ) const
{
    const FSmartTableColumn * Column = FindColumn( ColumnId );
    if ( !Column )
    {
        return FText::GetEmpty();
    }

    return Column->GetLabel();
}

TArray< FName > USmartTable::GetColumnOrder() const
{
    return ColumnView().Order();
}

void USmartTable::SetColumnOrder( const TArray< FName > & ColumnIds )
{

    ActiveLayout.Order = SmartTable::ColumnLayout::MergeOrder( ColumnView().AuthoredIds(), ColumnIds );

    UE_LOGFMT( LogSmartTablesLayout, Verbose, "Column order changed. {Named} of {Total} column(s) named, the rest stay as authored.", ColumnIds.Num(), Columns.Num() );

    RebuildHeader();
    LayoutChangedByUser();
}

void USmartTable::MoveColumn( FName ColumnId, int32 Delta )
{
    TArray< FName > Order = GetColumnOrder();

    const int32 From = Order.IndexOfByKey( ColumnId );
    if ( From == INDEX_NONE )
    {
        WarnUnknownColumn( TEXT( "MoveColumn" ), ColumnId );
        return;
    }

    const SmartTable::FColumnView View = ColumnView();

    const int32 To = SmartTable::ColumnLayout::LandingIndex( Order, From, Delta, [ &View ]( FName Neighbour )
    {
        return View.IsShown( Neighbour );
    } );

    if ( To == INDEX_NONE )
    {

        UE_LOGFMT( LogSmartTablesLayout, Verbose, "Column '{Column}' already sits at position {Position} of {Count} and cannot move {Delta}.", ColumnId, From, Order.Num(), Delta );
        return;
    }

    Order.RemoveAt( From );
    Order.Insert( ColumnId, To );

    SetColumnOrder( Order );
}

void USmartTable::MoveColumnNextTo( FName Moved, FName Target, bool bAfter )
{
    if ( Moved == Target )
    {

        UE_LOGFMT( LogSmartTablesLayout, Verbose, "MoveColumnNextTo('{Column}') has nowhere to go. A column cannot move next to itself.", Moved );
        return;
    }

    TArray< FName > Order = GetColumnOrder();
    if ( Order.Remove( Moved ) == 0 )
    {
        WarnUnknownColumn( TEXT( "MoveColumnNextTo" ), Moved );
        return;
    }

    const int32 TargetIndex = Order.IndexOfByKey( Target );
    if ( TargetIndex == INDEX_NONE )
    {
        WarnUnknownColumn( TEXT( "MoveColumnNextTo" ), Target );
        return;
    }

    UE_LOGFMT( LogSmartTablesLayout, Verbose, "'{Column}' sits {Side} '{Target}' now.", Moved, bAfter ? TEXT( "after" ) : TEXT( "before" ), Target );

    Order.Insert( Moved, bAfter ? TargetIndex + 1 : TargetIndex );
    SetColumnOrder( Order );
}

void USmartTable::WarnUnknownColumn( const TCHAR * Caller, FName ColumnId ) const
{

    UE_LOGFMT( LogSmartTablesLayout, Warning, "{Caller} found no column called '{Column}' on this table. Use the ColumnId from its Columns array.", Caller, ColumnId );
}

bool USmartTable::IsColumnVisible( FName ColumnId ) const
{
    return ColumnView().IsShown( ColumnId );
}

void USmartTable::SetColumnVisible( FName ColumnId, bool bVisible )
{
    if ( !FindColumn( ColumnId ) )
    {
        WarnUnknownColumn( TEXT( "SetColumnVisible" ), ColumnId );
        return;
    }

    if ( IsColumnVisible( ColumnId ) == bVisible )
    {
        return;
    }

    UE_LOGFMT( LogSmartTablesLayout, Verbose, "Column '{Column}' now {State}.", ColumnId, bVisible ? TEXT( "shown" ) : TEXT( "hidden" ) );

    LayoutFor( ColumnId ).bHidden = !bVisible;

    RebuildHeader();
    LayoutChangedByUser();
}

void USmartTable::ToggleColumnVisible( FName ColumnId )
{
    SetColumnVisible( ColumnId, !IsColumnVisible( ColumnId ) );
}

void USmartTable::RebuildHeader()
{
    if ( !HeaderRow.IsValid() )
    {

        UE_LOGFMT( LogSmartTablesLayout, Verbose, "'{Table}' holds off its header rebuild. The widget is not up yet.", GetName() );
        return;
    }

    if ( bWalkingRowCells )
    {
        bHeaderRebuildPending = true;

        UE_LOGFMT( LogSmartTablesLayout, Verbose, "The header rebuild for '{Table}' waits on a row still filling its own cells. It goes on the next tick.", GetName() );
        return;
    }

    HeaderRow->ClearColumns();

    if ( bShowRowNumbers )
    {

        HeaderRow->AddColumn( SmartTable::HeaderColumns::RowNumber( *this, GetRowNumberWidth() ) );
    }

    const TArray< FName > ShownOrder = ColumnView().ShownOrder();

    if ( ShownOrder.IsEmpty() )
    {
        UE_LOGFMT( LogSmartTablesLayout, Verbose, "Table '{Table}' hides all {Count} of its columns. The placeholder header draws, so the menu can bring one back.", GetName(), Columns.Num() );

        HeaderRow->AddColumn( SmartTable::HeaderColumns::Placeholder( *this ) );

        return;
    }

    for ( const FName OrderedId : ShownOrder )
    {
        const FSmartTableColumn * Found = FindColumn( OrderedId );
        if ( !Found )
        {
            continue;
        }

        const FSmartTableColumn & Column                      = *Found;
        const SmartTable::ColumnLayout::EHeaderWidthMode Mode = SmartTable::ColumnLayout::HeaderWidthModeFor( Column, bHeaderWidthsLive );

        TAttribute< float > ManualWidth;
        FOnWidthChanged OnWidthChanged;

        if ( Mode == SmartTable::ColumnLayout::EHeaderWidthMode::Manual )
        {
            ManualWidth = TAttribute< float >::Create( TAttribute< float >::FGetter::CreateUObject( this, &USmartTable::GetColumnWidth, Column.ColumnId ) );

            OnWidthChanged = FOnWidthChanged::CreateUObject( this, &USmartTable::HandleColumnWidthChanged, Column.ColumnId );
        }

        HeaderRow->AddColumn( SmartTable::HeaderColumns::ForColumn( *this, Column, Mode, ManualWidth, OnWidthChanged ) );
    }

    UE_LOGFMT( LogSmartTablesLayout, Verbose, "Header up for '{Table}': {Shown} of {Total} column(s) drawn, row numbers {Numbers}.", GetName(), ShownOrder.Num(), Columns.Num(), bShowRowNumbers ? TEXT( "on" ) : TEXT( "off" ) );

    MarkColumnWidthsDirty();

    HeaderLayoutChanged();
}

void USmartTable::HeaderLayoutChanged()
{

    if ( ListView.IsValid() )
    {
        ListView->RequestListRefresh();
    }
}

TSharedRef< SWidget > USmartTable::MakeRowNumberWidget( TSharedRef< SSmartTableRow > Row ) const
{
    return SmartTable::HeaderColumns::RowNumberCell( Row, RowNumberTextStyle );
}
