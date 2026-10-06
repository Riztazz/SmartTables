// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "SmartTableModel.h"

#include "Containers/Set.h"
#include "Logging/StructuredLog.h"
#include "Misc/ScopeExit.h"
#include "SmartTableLog.h"
#include "SmartTableSorting.h"

int32 USmartTableModel::GetNumRows_Implementation()
{
    return 0;
}

FText USmartTableModel::GetCellText_Implementation( int32 NaturalRow, FName ColumnId )
{
    return FText::GetEmpty();
}

ESmartTableCellEditor USmartTableModel::GetCellEditor_Implementation( int32 NaturalRow, FName ColumnId )
{
    return ESmartTableCellEditor::None;
}

bool USmartTableModel::SetCellText_Implementation( int32 NaturalRow, FName ColumnId, const FText & Value )
{
    return false;
}

FLinearColor USmartTableModel::GetCellColor_Implementation( int32 NaturalRow, FName ColumnId )
{
    return FLinearColor( 0.0f, 0.0f, 0.0f, 0.0f );
}

FLinearColor USmartTableModel::GetRowColor_Implementation( int32 NaturalRow )
{
    return FLinearColor( 0.0f, 0.0f, 0.0f, 0.0f );
}

FSmartTableSortKey USmartTableModel::GetCellSortKey_Implementation( int32 NaturalRow, FName ColumnId )
{
    const FText Text = GetCellText( NaturalRow, ColumnId );

    return Text.IsEmpty() ? FSmartTableSortKey::MakeEmpty() : FSmartTableSortKey::MakeText( Text.ToString() );
}

FName USmartTableModel::GetRowId_Implementation( int32 NaturalRow )
{

    return FName( TEXT( "Row" ), NaturalRow + 1 );
}

FSmartTableKeptRow USmartTableModel::KeepRow( int32 NaturalRow )
{
    return { NaturalRow != INDEX_NONE ? GetRowId( NaturalRow ) : NAME_None, NaturalRow };
}

int32 USmartTableModel::FindKeptRow( const FSmartTableKeptRow & Row )
{
    return Row.FindNow( GetNumRows(), [ this ]( int32 NaturalRow )
    {
        return GetRowId( NaturalRow );
    } );
}

UObject * USmartTableModel::GetRowItem_Implementation( int32 NaturalRow )
{
    return nullptr;
}

int32 USmartTableModel::NaturalRowOfItem_Implementation( UObject * Item )
{
    if ( !Item )
    {
        return INDEX_NONE;
    }

    const int32 NumRows = GetNumRows();
    for ( int32 NaturalRow = 0; NaturalRow < NumRows; ++NaturalRow )
    {
        if ( GetRowItem( NaturalRow ) == Item )
        {
            return NaturalRow;
        }
    }

    return INDEX_NONE;
}

bool USmartTableModel::SortRows_Implementation( const FSmartTableSortSpec & Spec )
{
    ApplySortSpecToPresentation( Spec );

    return true;
}

bool USmartTableModel::ApplyTextFilter_Implementation( const FText & FilterText, const TArray< FName > & AllColumnIds )
{
    ApplyFilterToPresentation( FilterText, AllColumnIds );

    return true;
}

bool USmartTableModel::MoveRows_Implementation( const TArray< int32 > & NaturalRows, int32 InsertBeforeNaturalRow )
{
    UE_LOGFMT( LogSmartTablesData, Verbose, "'{Model}' was told to move {Count} row(s) in front of row {Target} and leaves them where they sit. Override MoveRows to name where the rows live.", GetName(), NaturalRows.Num(), InsertBeforeNaturalRow );

    return false;
}

TArray< int32 > USmartTableModel::PlanRowMove( int32 NumRows, const TArray< int32 > & NaturalRows, int32 InsertBeforeNaturalRow )
{
    TArray< int32 > Order;

    if ( NumRows <= 0 || NaturalRows.IsEmpty() || InsertBeforeNaturalRow < 0 || InsertBeforeNaturalRow > NumRows )
    {
        return Order;
    }

    TSet< int32 > Moving;
    Moving.Reserve( NaturalRows.Num() );

    for ( const int32 Row : NaturalRows )
    {
        if ( Row < 0 || Row >= NumRows )
        {
            return Order;
        }

        bool bAlready = false;
        Moving.Add( Row, &bAlready );

        if ( bAlready )
        {
            return Order;
        }
    }

    if ( Moving.Num() == NumRows )
    {
        return Order;
    }

    TArray< int32 > Taken = NaturalRows;
    Taken.Sort();

    Order.Reserve( NumRows );

    for ( int32 Row = 0; Row < NumRows; ++Row )
    {
        if ( Row == InsertBeforeNaturalRow )
        {
            Order.Append( Taken );
        }

        if ( !Moving.Contains( Row ) )
        {
            Order.Add( Row );
        }
    }

    if ( InsertBeforeNaturalRow == NumRows )
    {
        Order.Append( Taken );
    }

    return Order;
}

void USmartTableModel::ApplySortSpecToPresentation( const FSmartTableSortSpec & Spec )
{
    ActiveSortSpec = Spec;
    RebuildPresentation();
}

void USmartTableModel::ApplyFilterToPresentation( const FText & FilterText, const TArray< FName > & AllColumnIds )
{
    ActiveFilterText = FilterText;
    FilterColumnIds  = AllColumnIds;
    RebuildPresentation();
}

TArray< int32 > USmartTableModel::CollectFilteredRows()
{
    const int32 NumRows = GetNumRows();

    TArray< int32 > Rows;
    Rows.Reserve( NumRows );

    const FString Needle  = ActiveFilterText.ToString();
    const bool bFiltering = !Needle.IsEmpty();

    for ( int32 NaturalRow = 0; NaturalRow < NumRows; ++NaturalRow )
    {
        if ( bFiltering )
        {
            const bool bMatches = FilterColumnIds.ContainsByPredicate( [ this, NaturalRow, &Needle ]( FName ColumnId )
            {
                return GetCellText( NaturalRow, ColumnId ).ToString().Contains( Needle );
            } );

            if ( !bMatches )
            {
                continue;
            }
        }

        Rows.Add( NaturalRow );
    }

    return Rows;
}

void USmartTableModel::ExtractSortKeys( TArray< TArray< FSmartTableSortKey > > & OutLevels, TArray< ESmartTableSortMode > & OutModes )
{
    const int32 NumRows = GetNumRows();

    OutLevels.Reset( ActiveSortSpec.Columns.Num() );
    OutModes.Reset( ActiveSortSpec.Columns.Num() );

    for ( const FSmartTableSortColumn & Column : ActiveSortSpec.Columns )
    {
        TArray< FSmartTableSortKey > & Keys = OutLevels.AddDefaulted_GetRef();
        Keys.Reserve( NumRows );
        for ( int32 NaturalRow = 0; NaturalRow < NumRows; ++NaturalRow )
        {
            Keys.Add( GetCellSortKey( NaturalRow, Column.ColumnId ) );
        }

        OutModes.Add( Column.Mode );
    }
}

void USmartTableModel::RebuildPresentation( const FSmartTablePresentationChange & WhenDone )
{
    PendingChange.Add( WhenDone );

    TArray< int32 > Rows = CollectFilteredRows();

    UE_LOGFMT( LogSmartTablesSort, Verbose, "{Model} lays out its presentation: {Presented} of {Total} row(s) clear the filter, {Levels} sort level(s).", GetClass()->GetName(), Rows.Num(), GetNumRows(), ActiveSortSpec.Columns.Num() );

    if ( ActiveSortSpec.IsEmpty() )
    {
        ApplySortedRows( ++SortToken, MoveTemp( Rows ),  false );
        return;
    }

    TArray< TArray< FSmartTableSortKey > > KeyLevels;
    TArray< ESmartTableSortMode > Modes;
    ExtractSortKeys( KeyLevels, Modes );

    const int32 Token = ++SortToken;

    if ( !WorkDispatcher )
    {
        UE_LOGFMT( LogSmartTablesSort, Verbose, "Sort {Token}: {Rows} row(s) sort inline on the game thread. No dispatcher is set.", Token, Rows.Num() );

        SmartTable::Sorting::SortIndices( Rows, KeyLevels, Modes );
        ApplySortedRows( Token, MoveTemp( Rows ),  false );
        return;
    }

    ++SortsInFlight;
    LastDispatchedToken = Token;
    SetBusy( true );

    UE_LOGFMT( LogSmartTablesSort, Verbose, "Sort {Token}: {Rows} rows go out across {Levels} level(s), with {InFlight} still in flight.", Token, Rows.Num(), KeyLevels.Num(), SortsInFlight );

    TWeakObjectPtr< USmartTableModel > Weak( this );

    WorkDispatcher( [ Weak, Token, Rows = MoveTemp( Rows ), KeyLevels = MoveTemp( KeyLevels ), Modes = MoveTemp( Modes ) ]() mutable
    {
        UE_LOGFMT( LogSmartTablesSort, Verbose, "Sort {Token}: the worker picks it up.", Token );

        SmartTable::Sorting::SortIndices( Rows, KeyLevels, Modes );

        UE_LOGFMT( LogSmartTablesSort, Verbose, "Sort {Token}: the worker is done and hands back {Rows} rows.", Token, Rows.Num() );

        SmartTable::Dispatchers::RunOnGameThread( [ Weak, Token, Rows = MoveTemp( Rows ) ]() mutable
        {
            USmartTableModel * Model = Weak.Get();

            UE_LOGFMT( LogSmartTablesSort, Verbose, "Sort {Token}: the game thread has the result, model {State}.", Token, Model ? TEXT( "alive" ) : TEXT( "COLLECTED" ) );

            if ( Model )
            {
                Model->ApplySortedRows( Token, MoveTemp( Rows ),  true );
            }
        } );
    } );
}

void USmartTableModel::ApplySortedRows( int32 Token, TArray< int32 > && Rows, bool bWasDispatched )
{
    if ( bWasDispatched )
    {
        --SortsInFlight;
    }

    ON_SCOPE_EXIT
    {
        if ( SortsInFlight == 0 )
        {
            SetBusy( false );
        }
    };

    if ( Token != SortToken )
    {
        UE_LOGFMT( LogSmartTablesSort, Verbose, "Sort {Token}: {Current} came after it, so it goes ({InFlight} still in flight).", Token, SortToken, SortsInFlight );
        return;
    }

    UE_LOGFMT( LogSmartTablesSort, Verbose, "Sort {Token}: landed ({InFlight} still in flight).", Token, SortsInFlight );

    PresentedToNatural = MoveTemp( Rows );

    NaturalToPresented.Reset();
    NaturalToPresented.SetNumUninitialized( GetNumRows() );
    for ( int32 & Where : NaturalToPresented )
    {
        Where = INDEX_NONE;
    }

    for ( int32 Presented = 0; Presented < PresentedToNatural.Num(); ++Presented )
    {
        const int32 Natural = PresentedToNatural[ Presented ];
        if ( NaturalToPresented.IsValidIndex( Natural ) )
        {
            NaturalToPresented[ Natural ] = Presented;
        }
    }

    LastDispatchedToken = INDEX_NONE;

    const FSmartTablePresentationChange Landed = MoveTemp( PendingChange );
    PendingChange                              = FSmartTablePresentationChange::Order();

    BroadcastPresentationChanged( Landed );
}

void USmartTableModel::SetBusy( bool bInBusy )
{
    if ( bBusy == bInBusy )
    {
        return;
    }

    bBusy = bInBusy;
    BusyChanged.Broadcast( bBusy );
}

int32 USmartTableModel::PresentedToNaturalRow( int32 PresentedRow ) const
{
    return PresentedToNatural.IsValidIndex( PresentedRow ) ? PresentedToNatural[ PresentedRow ] : INDEX_NONE;
}

int32 USmartTableModel::NaturalToPresentedRow( int32 NaturalRow ) const
{
    return NaturalToPresented.IsValidIndex( NaturalRow ) ? NaturalToPresented[ NaturalRow ] : INDEX_NONE;
}

void USmartTableModel::NotifyRowChanged( int32 NaturalRow )
{
    UE_LOGFMT( LogSmartTablesData, VeryVerbose, "{Model}: row {Row} reads from the top.", GetClass()->GetName(), NaturalRow );

    NotifyValuesChanged( FSmartTablePresentationChange::OneRowsValues( NaturalRow ) );
}

void USmartTableModel::NotifyCellChanged( int32 NaturalRow, FName ColumnId )
{

    NotifyValuesChanged( ColumnId.IsNone() ? FSmartTablePresentationChange::OneRowsValues( NaturalRow ) : FSmartTablePresentationChange::OneCellsValue( NaturalRow, ColumnId ) );
}

void USmartTableModel::NotifyValuesChanged( const FSmartTablePresentationChange & Change )
{
    if ( LastDispatchedToken == SortToken )
    {

        PendingChange.Add( Change );

        UE_LOGFMT( LogSmartTablesData, VeryVerbose, "{Model}: the tick for row {Row} column '{Column}' waits on the rebuild in flight.", GetClass()->GetName(), Change.NaturalRow, Change.ColumnId );
        return;
    }

    BroadcastPresentationChanged( Change );
}

void USmartTableModel::NotifyRowsChanged()
{
    UE_LOGFMT( LogSmartTablesData, Verbose, "{Model}: every row reads from the top, and the active order goes back on.", GetClass()->GetName() );

    RebuildPresentation( FSmartTablePresentationChange::EveryRowsValues() );
}

void USmartTableModel::NotifyRowsAdded( const TArray< int32 > & NaturalRows )
{
    FSmartTableRowSetDiff Diff;
    Diff.Arrived.Reserve( NaturalRows.Num() );

    for ( const int32 NaturalRow : NaturalRows )
    {
        Diff.Arrived.Add( KeepRow( NaturalRow ) );
    }

    NotifyRowSetChanged( Diff );
}

void USmartTableModel::NotifyRowsRemoved( const TArray< FName > & RowIds )
{
    FSmartTableRowSetDiff Diff;
    Diff.Left.Append( RowIds );

    NotifyRowSetChanged( Diff );
}

void USmartTableModel::NotifyNumRowsChanged()
{
    NotifyRowSetChanged( FSmartTableRowSetDiff() );
}

void USmartTableModel::NotifyRowSetChanged( const FSmartTableRowSetDiff & Diff )
{
    UE_LOGFMT( LogSmartTablesData, Verbose, "{Model}: the row set changed, {Arrived} row(s) in, {Left} out, replaced {Replaced}. Selection follows the row ids back.", GetClass()->GetName(), Diff.Arrived.Num(), Diff.Left.Num(), Diff.bReplaced );

    RebuildPresentation( FSmartTablePresentationChange::RowSet( Diff ) );
}

void USmartTableModel::BroadcastPresentationChanged( const FSmartTablePresentationChange & Change )
{
    PresentationChanged.Broadcast( Change );
}
