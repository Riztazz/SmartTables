// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "Logging/StructuredLog.h"
#include "Misc/ScopeExit.h"
#include "SmartTable.h"
#include "SmartTableLog.h"
#include "SmartTableModel.h"
#include "SmartTableSorting.h"
#include "Table/SmartTableColumnView.h"

void USmartTable::HandleHeaderGesture( FName ColumnId, bool bShiftDown )
{

    if ( !bAllowUserSorting )
    {
        UE_LOGFMT( LogSmartTablesSort, Verbose, "A click on the '{Column}' header sorts nothing. This table never sorts on a click.", ColumnId );
        return;
    }

    CycleColumnSort( ColumnId, bShiftDown && bAllowSecondarySort );
}

void USmartTable::CycleColumnSort( FName ColumnId, bool bAsSecondary )
{
    ColumnId = ColumnForIntent( ColumnId );

    const FSmartTableColumn * Column = FindColumn( ColumnId );
    if ( !Column || !Column->bSortable )
    {

        UE_LOGFMT( LogSmartTablesSort, Verbose, "CycleColumnSort('{Column}') sorts nothing: {Reason}.", ColumnId, Column ? TEXT( "the column is not sortable" ) : TEXT( "no such column" ) );
        return;
    }

    const ESmartTableSortMode Next = SmartTable::Sorting::NextSortMode( GetSortSpecRef().GetModeFor( ColumnId ), bAllowSortNone );

    ON_SCOPE_EXIT
    {
        LayoutChangedByUser();
    };

    if ( bAsSecondary )
    {
        SortBySecondaryColumn( ColumnId, Next );
        return;
    }

    SortByColumn( ColumnId, Next );
}

void USmartTable::SortFromMenu( FName ColumnId, ESmartTableSortMode SortMode, bool bSecondary )
{
    if ( bSecondary )
    {
        SortBySecondaryColumn( ColumnId, SortMode );
    }
    else
    {
        SortByColumn( ColumnId, SortMode );
    }

    LayoutChangedByUser();
}

void USmartTable::SortByColumn( FName ColumnId, ESmartTableSortMode SortMode )
{
    SetSortSpec( SmartTable::Sorting::SpecWithPrimary( ColumnId, SortMode ) );
}

void USmartTable::SortBySecondaryColumn( FName ColumnId, ESmartTableSortMode SortMode )
{

    RecomputeDeadSecondaryColumns();

    if ( IsDeadSecondaryColumn( ColumnId ) )
    {
        UE_LOGFMT( LogSmartTablesSort, Verbose, "Second sort level on '{Column}' turned down. The primary sort left it no tied rows to order.", ColumnId );
        return;
    }

    SetSortSpec( SmartTable::Sorting::SpecWithSecondary( GetSortSpec(), ColumnId, SortMode ) );
}

void USmartTable::SetSortSpec( const FSmartTableSortSpec & Spec )
{
    if ( !Model )
    {

        UE_LOGFMT( LogSmartTablesSort, Verbose, "A sort on '{Table}' has nothing to sort. No model yet.", GetName() );
        return;
    }

    UE_LOGFMT( LogSmartTablesSort, Verbose, "'{Table}' asked for a sort: {Levels} level(s), primary '{Primary}'.", GetName(), Spec.Columns.Num(), Spec.Columns.IsEmpty() ? NAME_None : Spec.Columns[ 0 ].ColumnId );

    const FSmartTableSortSpec Previous = Model->GetActiveSortSpec();

    if ( Spec.IsEmpty() && Previous.IsEmpty() )
    {

        ActiveLayout.SortSpec = Previous;

        return;
    }

    if ( !Model->SortRows( Spec ) )
    {
        if ( Spec.IsEmpty() )
        {

            UE_LOGFMT( LogSmartTablesSort, Verbose, "Model {Model} does not sort, so '{Table}' stays in the order the model gives.", Model->GetClass()->GetName(), GetName() );
            return;
        }

        UE_LOGFMT( LogSmartTablesSort, Warning, "Model {Model} turned down a sort of column '{Column}'. The header arrow comes back and the rows stay. Clear bSortable on that column, or set SortRows on the model to handle it.", Model->GetClass()->GetName(), Spec.Columns[ 0 ].ColumnId );
        return;
    }

    ActiveLayout.SortSpec = Model->GetActiveSortSpec();

    if ( !( Previous == Model->GetActiveSortSpec() ) )
    {
        OnSortChanged.Broadcast( Model->GetActiveSortSpec() );
    }
}

FSmartTableSortSpec USmartTable::GetSortSpec() const
{
    return GetSortSpecRef();
}

const FSmartTableSortSpec & USmartTable::GetSortSpecRef() const
{

    static const FSmartTableSortSpec Unsorted;

    return Model ? Model->ActiveSortSpecRef() : Unsorted;
}

bool USmartTable::IsSecondarySortColumn( FName ColumnId ) const
{
    return GetSortSpecRef().GetPriorityFor( ColumnId ) == 1;
}

bool USmartTable::CanSortBySecondaryColumn( FName ColumnId ) const
{

    return bAllowSecondarySort && SmartTable::Sorting::HasLevelOtherThan( GetSortSpecRef(), ColumnId ) && !IsDeadSecondaryColumn( ColumnId );
}

void USmartTable::SetAllowDeadSecondarySort( bool bInAllowDeadSecondarySort )
{
    bAllowDeadSecondarySort = bInAllowDeadSecondarySort;

    DeadSecondaries.MarkStale();
}

bool USmartTable::IsDeadSecondaryColumn( FName ColumnId ) const
{
    if ( bAllowDeadSecondarySort )
    {
        return false;
    }

    return DeadSecondaries.Contains( ColumnId );
}

void USmartTable::RefreshDeadSecondaryColumns()
{

    if ( bAllowDeadSecondarySort || !DeadSecondaries.IsStale() )
    {
        return;
    }

    RecomputeDeadSecondaryColumns();
}

void USmartTable::RecomputeDeadSecondaryColumns()
{
    if ( bAllowDeadSecondarySort )
    {
        return;
    }

    if ( !Model )
    {
        DeadSecondaries.Clear();
        return;
    }

    USmartTableModel & SortedModel = *Model;

    DeadSecondaries.Recompute( Model->ActiveSortSpecRef(), Model->GetNumPresentedRows(), SmartTable::Sorting::SortableColumnIds( Columns ), [ &SortedModel ]( int32 PresentedRow, FName ColumnId )
    {
        return SortedModel.GetCellSortKey( SortedModel.PresentedToNaturalRow( PresentedRow ), ColumnId );
    } );
}

void USmartTable::SetFilterText( const FText & InFilterText )
{
    if ( !Model )
    {
        UE_LOGFMT( LogSmartTablesData, Verbose, "A filter on '{Table}' has nothing to filter. No model yet.", GetName() );
        return;
    }

    const TArray< FName > AllColumnIds = ColumnView().AuthoredIds();

    UE_LOGFMT( LogSmartTablesData, Verbose, "Table '{Table}' filters by \"{Filter}\" over all {Columns} column(s).", GetName(), InFilterText.ToString(), AllColumnIds.Num() );

    if ( !Model->ApplyTextFilter( InFilterText, AllColumnIds ) )
    {
        UE_LOGFMT( LogSmartTablesData, Warning, "Model {Model} turned down the filter \"{Filter}\" and the rows stand. Set ApplyTextFilter on the model to handle it, or do not offer a filter box over this data.", Model->GetClass()->GetName(), InFilterText.ToString() );
        return;
    }

    UE_LOGFMT( LogSmartTablesData, Verbose, "Filter on: {Shown} of {Total} row(s) match.", Model->GetNumPresentedRows(), Model->GetNumRows() );
}

FText USmartTable::GetFilterText() const
{
    return Model ? Model->GetActiveFilterText() : FText::GetEmpty();
}
