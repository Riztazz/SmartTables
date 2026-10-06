// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "SmartTable.h"

#include "Engine/DataTable.h"
#include "Framework/Application/SlateApplication.h"
#include "Logging/StructuredLog.h"
#include "Misc/AssertionMacros.h"
#include "Misc/CoreMisc.h"
#include "SmartTableArrivals.h"
#include "SmartTableDataTableModel.h"
#include "SmartTableLog.h"
#include "SmartTableModel.h"
#include "SmartTableObjectModel.h"
#include "SmartTableScrollMove.h"
#include "SmartTableSettings.h"
#include "SmartTableStyle.h"
#include "Table/SmartTableColumnView.h"
#include "Table/SmartTableRowSpace.h"
#include "UObject/UObjectBaseUtility.h"
#include "View/SmartTableListView.h"
#include "View/SmartTableRow.h"

#define LOCTEXT_NAMESPACE "SmartTables"

const FName USmartTable::RowNumberColumnId( TEXT( "SmartTable_RowNumber" ) );

USmartTable::FCellWalkScope::FCellWalkScope( USmartTable * InTable )
    : Table( InTable )
{
    if ( USmartTable * Walking = Table.Get() )
    {
        bPrevious                 = Walking->bWalkingRowCells;
        Walking->bWalkingRowCells = true;
    }
}

USmartTable::FCellWalkScope::~FCellWalkScope()
{
    if ( USmartTable * Walking = Table.Get() )
    {

        Walking->bWalkingRowCells = bPrevious;
    }
}

USmartTable::USmartTable( const FObjectInitializer & ObjectInitializer )
    : Super( ObjectInitializer )
{
    bIsVariable = true;

    EmptyText           = LOCTEXT( "EmptyTable", "Nothing to show" );
    NoMatchesTextFormat = LOCTEXT( "NoMatches", "No rows match \"{0}\"" );

    const USmartTableSettings & Settings = *GetDefault< USmartTableSettings >();
    ResizeGripWidth                      = Settings.ResizeGripWidth;
    MinColumnWidth                       = Settings.MinColumnWidth;
    MaxAutoSizeColumnWidth               = Settings.MaxAutoSizeColumnWidth;

    if ( IsRunningDedicatedServer() )
    {
        return;
    }

    CopyStyleClassDefaults( *GetDefault< USmartTableStyle >() );
}

USmartTableObjectModel & USmartTable::GetOrCreateItemsModel()
{
    if ( !ItemsModel )
    {
        ItemsModel = NewObject< USmartTableObjectModel >( this );

        checkf( ItemsModel, TEXT( "USmartTable '%s' could not create its built-in items model" ), *GetName() );

        ItemsModel->SetColumns( Columns );

        UE_LOGFMT( LogSmartTablesData, Verbose, "An internal items model went up for '{Table}', over {Columns} column(s).", GetName(), Columns.Num() );
    }

    if ( Model != ItemsModel )
    {
        SetModel( ItemsModel );
    }

    return *ItemsModel;
}

void USmartTable::SetItems( const TArray< UObject * > & InItems )
{
    GetOrCreateItemsModel().SetItems( InItems );
}

void USmartTable::AddItem( UObject * Item )
{
    GetOrCreateItemsModel().AddItem( Item );
}

void USmartTable::RemoveItem( UObject * Item )
{
    if ( !ItemsModel || Model != ItemsModel )
    {
        UE_LOGFMT( LogSmartTablesData, Verbose, "RemoveItem({Item}) took nothing out. '{Table}' draws {Model}, and only rows from SetItems or AddItem come out this way.", GetNameSafe( Item ), GetName(), Model ? Model->GetClass()->GetName() : TEXT( "no model" ) );
        return;
    }

    ItemsModel->RemoveItem( Item );
}

void USmartTable::ClearItems()
{
    GetOrCreateItemsModel().ClearItems();
}

void USmartTable::SetDataTable( UDataTable * InDataTable )
{
    if ( !DataTableModel )
    {
        DataTableModel = NewObject< USmartTableDataTableModel >( this );

        checkf( DataTableModel, TEXT( "USmartTable '%s' could not create its built-in DataTable model" ), *GetName() );

        DataTableModel->SetColumns( Columns );
    }

    UE_LOGFMT( LogSmartTablesData, Verbose, "'{Table}' reads DataTable {Asset} from here on.", GetName(), GetNameSafe( InDataTable ) );

    SetModel( DataTableModel );
    DataTableModel->SetDataTable( InDataTable );
}

void USmartTable::NotifyItemChanged( UObject * Item )
{
    if ( !ItemsModel )
    {
        UE_LOGFMT( LogSmartTablesData, Verbose, "NotifyItemChanged({Item}) has nothing to tell. This table has no items model and never took items.", GetNameSafe( Item ) );
        return;
    }

    const int32 NaturalRow = ItemsModel->IndexOfItem( Item );
    if ( NaturalRow == INDEX_NONE )
    {
        UE_LOGFMT( LogSmartTablesData, Verbose, "NotifyItemChanged({Item}) has nothing to tell. This table holds no such item.", GetNameSafe( Item ) );
        return;
    }

    ItemsModel->NotifyRowChanged( NaturalRow );
}

void USmartTable::RefreshItems()
{
    if ( !Model )
    {
        UE_LOGFMT( LogSmartTablesData, Verbose, "RefreshItems has nothing to read. No model sits on the table yet." );
        return;
    }

    Model->NotifyRowsChanged();
}

void USmartTable::SetModel( USmartTableModel * InModel )
{
    if ( Model == InModel )
    {
        return;
    }

    UE_LOGFMT( LogSmartTablesData, Verbose, "Table '{Table}' swaps model {From} for {To}.", GetName(), GetNameSafe( Model.Get() ), GetNameSafe( InModel ) );

    if ( Model )
    {
        Model->OnPresentationChanged().RemoveAll( this );
        Model->OnBusyChanged().RemoveAll( this );
    }

    Model = InModel;
    ItemlessColumnsWarned.Reset();
    CellClassFailuresWarned.Reset();

    Arrivals.Reset();
    FocusedRow = INDEX_NONE;
    Scroll.Reset();

    if ( ListView.IsValid() )
    {
        ListView->ClearSelection();
        ListView->ScrollToTop();
    }

    if ( Model )
    {
        Model->OnPresentationChanged().AddUObject( this, &USmartTable::HandlePresentationChanged );
        Model->OnBusyChanged().AddUObject( this, &USmartTable::HandleModelBusyChanged );

        Model->RebuildPresentation( FSmartTablePresentationChange::Everything() );
    }

    SetBusy( Model && Model->IsBusy() );

    if ( !Model )
    {
        RebuildRowIndices();
    }

    SetSortSpec( ActiveLayout.SortSpec );

    RebuildHeader();
}

UObject * USmartTable::ItemForRow( int32 NaturalRow ) const
{
    return Model && NaturalRow != INDEX_NONE ? Model->GetRowItem( NaturalRow ) : nullptr;
}

FName USmartTable::RowIdAt( int32 NaturalRow ) const
{
    return Model && NaturalRow != INDEX_NONE ? Model->GetRowId( NaturalRow ) : NAME_None;
}

void USmartTable::ForEachGeneratedRow( TFunctionRef< void( SSmartTableRow & ) > Visit )
{

    for ( int32 Index = GeneratedRows.Num() - 1; Index >= 0; --Index )
    {
        const TSharedPtr< SSmartTableRow > Row = GeneratedRows[ Index ].Pin();
        if ( !Row.IsValid() )
        {
            GeneratedRows.RemoveAtSwap( Index );
            continue;
        }

        Visit( *Row );
    }
}

int32 USmartTable::NaturalGapForPresentedGap( int32 PresentedGap ) const
{
    if ( !Model )
    {
        return 0;
    }

    return SmartTable::RowSpace::NaturalGapFor( PresentedGap, Model->GetNumRows(), [ this ]( int32 PresentedRow )
    {
        return Model->PresentedToNaturalRow( PresentedRow );
    } );
}

bool USmartTable::MoveRowsTo( const TArray< int32 > & NaturalRows, int32 InsertBeforeNaturalRow )
{
    if ( !Model )
    {
        return false;
    }

    const auto RowIdOf = [ this ]( int32 NaturalRow )
    {
        return Model->GetRowId( NaturalRow );
    };

    const TSet< FName > SelectedIds = SmartTable::RowSpace::IdsOf( GetSelectedRows(), RowIdOf );

    const FName CaretId = FocusedRow != INDEX_NONE ? RowIdOf( FocusedRow ) : NAME_None;

    if ( !Model->MoveRows( NaturalRows, InsertBeforeNaturalRow ) )
    {
        return false;
    }

    UE_LOGFMT( LogSmartTablesData, Verbose, "'{Table}' put {Count} row(s) in front of row {Target}.", GetName(), NaturalRows.Num(), InsertBeforeNaturalRow );

    {
        TGuardValue< bool > Hold( bHoldSelectionBroadcast, true );

        if ( ( !SelectedIds.IsEmpty() || !CaretId.IsNone() ) && ListView.IsValid() )
        {
            const SmartTable::RowSpace::FFoundRows Found = SmartTable::RowSpace::FindByIdsInEveryRow( Model->GetNumRows(), SelectedIds, CaretId, RowIdOf );

            if ( !SelectedIds.IsEmpty() )
            {
                ListView->Private_ClearSelection();
                ApplySelection( Found.Rows, true );
            }

            FocusedRow = Found.Caret;

            if ( Found.Caret != INDEX_NONE && ListView->IsItemSelected( Found.Caret ) )
            {
                ListView->Private_SetItemSelection( Found.Caret, true,  true );
            }
        }

        if ( !GetSortSpec().Columns.IsEmpty() )
        {
            SetSortSpec( FSmartTableSortSpec() );
        }

        Model->NotifyNumRowsChanged();
    }

    if ( bSelectionBroadcastOwed )
    {
        bSelectionBroadcastOwed = false;
        HandleSelectionChanged( INDEX_NONE, ESelectInfo::Direct );
    }

    OnRowsDropped.Broadcast( NaturalRows, InsertBeforeNaturalRow );

    return true;
}

void USmartTable::MoveRowsFromDrop( const TArray< FSmartTableKeptRow > & Rows, int32 InsertBeforeNaturalRow )
{
    if ( !Model )
    {
        return;
    }

    TArray< int32 > NaturalRows;
    NaturalRows.Reserve( Rows.Num() );

    for ( const FSmartTableKeptRow & Row : Rows )
    {
        const int32 NaturalNow = Model->FindKeptRow( Row );
        if ( NaturalNow != INDEX_NONE )
        {
            NaturalRows.Add( NaturalNow );
        }
    }

    if ( NaturalRows.Num() < Rows.Num() )
    {
        UE_LOGFMT( LogSmartTablesInput, Verbose, "'{Table}': {Left} of the {Picked} row(s) the drag picked up left while it was out. {Moving} move.", GetName(), Rows.Num() - NaturalRows.Num(), Rows.Num(), NaturalRows.Num() );
    }

    if ( NaturalRows.IsEmpty() )
    {
        return;
    }

    const FSmartTableSortSpec Before = GetSortSpec();

    if ( MoveRowsTo( NaturalRows, InsertBeforeNaturalRow ) && !( GetSortSpecRef() == Before ) )
    {
        LayoutChangedByUser();
    }
}

bool USmartTable::MoveSelectedRows( int32 Delta )
{
    if ( !Model || Delta == 0 )
    {
        return false;
    }

    const TArray< int32 > Selected = GetSelectedRows();
    if ( Selected.IsEmpty() )
    {
        return false;
    }

    TArray< int32 > Presented;
    Presented.Reserve( Selected.Num() );

    for ( const int32 NaturalRow : Selected )
    {
        const int32 PresentedRow = Model->NaturalToPresentedRow( NaturalRow );

        if ( PresentedRow == INDEX_NONE )
        {
            UE_LOGFMT( LogSmartTablesInput, Verbose, "'{Table}' moved nothing. Row {Row} is selected and the filter hides it.", GetName(), NaturalRow );
            return false;
        }

        Presented.Add( PresentedRow );
    }

    const int32 PresentedGap = SmartTable::RowSpace::GapForBlockMove( Presented, Delta, Model->GetNumPresentedRows() );

    return PresentedGap != INDEX_NONE && MoveRowsTo( Selected, NaturalGapForPresentedGap( PresentedGap ) );
}

void USmartTable::RebuildRowIndices()
{

    const int32 NumPresented = Model ? Model->GetNumPresentedRows() : 0;

    ensureMsgf( !Model || NumPresented <= Model->GetNumRows(), TEXT( "Model %s presents %d rows but only has %d" ), *GetNameSafe( Model.Get() ), NumPresented, Model ? Model->GetNumRows() : 0 );

    const auto RowIdOf = [ this ]( int32 NaturalRow )
    {
        return Model->GetRowId( NaturalRow );
    };

    TSet< FName > SelectedIds;
    FName FocusedId;
    if ( Model && ListView.IsValid() )
    {
        SelectedIds = SmartTable::RowSpace::IdsOf( ListView->GetSelectedItems(), RowIdOf );

        if ( FocusedRow != INDEX_NONE )
        {
            FocusedId = RowIdOf( FocusedRow );
        }
    }

    FocusedRow = INDEX_NONE;

    MeasureRowNumberWidth();

    RowIndices = Model ? TArray< int32 >( Model->GetPresentedRows() ) : TArray< int32 >();

    UE_LOGFMT( LogSmartTablesInput, Verbose, "Row index for '{Table}' up fresh: {Presented} presented row(s), {Selected} selection(s) to restore.", GetName(), NumPresented, SelectedIds.Num() );

    RetargetScrollAfterStructuralChange();

    if ( !ListView.IsValid() )
    {
        return;
    }

    if ( !SelectedIds.IsEmpty() || !FocusedId.IsNone() )
    {

        const SmartTable::RowSpace::FFoundRows Found = SmartTable::RowSpace::FindByIds( RowIndices, SelectedIds, FocusedId, RowIdOf );

        FocusedRow = Found.Caret;

        if ( !SelectedIds.IsEmpty() )
        {
            if ( Found.Rows.IsEmpty() )
            {
                ListView->ClearSelection();
            }
            else
            {
                ListView->Private_ClearSelection();
                ApplySelection( Found.Rows, true );
            }
        }
    }

    ListView->RequestListRefresh();
}

void USmartTable::HandlePresentationChanged( const FSmartTablePresentationChange & Change )
{
    const ESmartTablePresentationScope Scope = Change.Scope();

    if ( Scope == ESmartTablePresentationScope::Screen )
    {
        DeadSecondaries.MarkStale();
    }

    if ( bApplyingPresentation )
    {

        UE_LOGFMT( LogSmartTablesData, VeryVerbose, "Nested presentation change on '{Table}' dropped, with {Arrived} arrived and {Left} left row(s) in it.", GetName(), Change.RowSetDiff.Arrived.Num(), Change.RowSetDiff.Left.Num() );
        return;
    }

    UE_LOGFMT( LogSmartTablesData, VeryVerbose, "Presentation on '{Table}' now differs: cells={Cells} order={Order} rowSet={RowSet} row={Row} column={Column}.", GetName(), Change.bCellsStale, Change.bOrderMoved, Change.bRowSetMoved, Change.NaturalRow, Change.ColumnId );

    TGuardValue< bool > Guard( bApplyingPresentation, true );

    if ( Change.bOrderMoved || Change.bRowSetMoved )
    {
        const FSmartTableHeldRow Held = Change.bRowSetMoved ? TopRowOnScreen() : FSmartTableHeldRow();
        const int32 PresentedBefore   = RowIndices.Num();

        const bool bWasAtEnd = Scroll.bViewAtEnd || ( !RowIndices.IsEmpty() && Scroll.IsHeadingTo( RowIndices.Last() ) );

        if ( Change.bRowSetMoved && FSlateApplication::IsInitialized() )
        {
            Arrivals.Apply( Change.RowSetDiff, FSlateApplication::Get().GetCurrentTime() );
        }

        RebuildRowIndices();

        ReassignChangedRows();

        if ( Change.bRowSetMoved )
        {
            ScrollAfterRowSetChange( Change.RowSetDiff, Held, PresentedBefore, bWasAtEnd );
        }

        RememberTopRow();
    }

    if ( Scope == ESmartTablePresentationScope::Row || Scope == ESmartTablePresentationScope::Cell )
    {

        if ( ListView.IsValid() )
        {
            if ( TSharedPtr< ITableRow > TableRow = ListView->WidgetFromItem( Change.NaturalRow ) )
            {
                SSmartTableRow & Row = *StaticCastSharedPtr< SSmartTableRow >( TableRow );

                if ( Scope == ESmartTablePresentationScope::Cell )
                {
                    Row.RefreshCell( Change.ColumnId, ESmartTableAssignReason::ValueChanged );
                }
                else
                {
                    Row.RefreshCells( ESmartTableAssignReason::ValueChanged );
                }
            }
        }

        return;
    }

    if ( Change.bCellsStale )
    {

        ForEachGeneratedRow( []( SSmartTableRow & Row )
        {
            Row.RefreshCells();
        } );
    }
}

TSharedRef< ITableRow > USmartTable::HandleGenerateRow( int32 NaturalRow, const TSharedRef< STableViewBase > & OwnerTable )
{
    TSharedRef< SSmartTableRow > NewRow = SNew( SSmartTableRow, OwnerTable, this, NaturalRow );

    GeneratedRows.Add( NewRow );

    return NewRow;
}

FName USmartTable::ColumnForIntent( FName ColumnId )
{
    const FName Before = ColumnSelection.Selected;
    const FName Target = ColumnSelection.Resolve( ColumnId, ColumnView().ShownOrder() );

    ColumnSelectionChanged( ColumnSelection.Selected != Before );

    return Target;
}

bool USmartTable::IsRowDrawn( int32 NaturalRow ) const
{

    return Model && Model->NaturalToPresentedRow( NaturalRow ) != INDEX_NONE;
}

int32 USmartTable::PresentedRowOf( int32 NaturalRow ) const
{
    return Model && NaturalRow != INDEX_NONE ? Model->NaturalToPresentedRow( NaturalRow ) : INDEX_NONE;
}

TConstArrayView< TObjectPtr< UObject > > USmartTable::GetItems() const
{

    if ( const USmartTableObjectModel * ObjectModel = Cast< USmartTableObjectModel >( Model ) )
    {
        return ObjectModel->GetItems();
    }

    return {};
}

#undef LOCTEXT_NAMESPACE
