// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "Framework/Application/SlateApplication.h"
#include "Logging/StructuredLog.h"
#include "Math/UnrealMathUtility.h"
#include "SmartTable.h"
#include "SmartTableColumnLayout.h"
#include "SmartTableHelpers.h"
#include "SmartTableLog.h"
#include "SmartTableModel.h"
#include "Table/SmartTableColumnView.h"
#include "Table/SmartTableRowSpace.h"
#include "View/SmartTableListView.h"

void USmartTable::SetSelectionMode( ESmartTableSelectionMode InSelectionMode )
{
    SelectionMode = InSelectionMode;

    if ( !ListView.IsValid() )
    {
        return;
    }

    if ( SelectionMode == ESmartTableSelectionMode::None )
    {
        ListView->ClearSelection();

        FocusedRow = INDEX_NONE;
    }

    ListView->SetSelectionMode( SmartTable::ToSlateSelectionMode( SelectionMode ) );
}

void USmartTable::SelectColumn( FName ColumnId )
{
    const TArray< FName > Shown = ColumnView().ShownOrder();

    if ( !ColumnId.IsNone() && !Shown.Contains( ColumnId ) )
    {
        UE_LOGFMT( LogSmartTablesInput, Warning, "SelectColumn('{Column}') found nothing. '{Table}' shows no column with that name. Check the id against the Columns array, and check that the column is not hidden.", ColumnId, GetName() );
        return;
    }

    ColumnSelectionChanged( ColumnSelection.Select( ColumnId, Shown ) );
}

EVisibility USmartTable::GetFocusBorderVisibility() const
{
    if ( !bShowFocusBorder )
    {
        return EVisibility::Collapsed;
    }

    return IsTableFocused() ? EVisibility::HitTestInvisible : EVisibility::Collapsed;
}

bool USmartTable::IsTableFocused() const
{

    const TSharedPtr< SWidget > Root = GetCachedWidget();

    return Root.IsValid() && Root->HasAnyUserFocusOrFocusedDescendants();
}

void USmartTable::ApplySelection( TConstArrayView< int32 > NaturalRows, bool bSelected )
{
    if ( !ListView.IsValid() )
    {
        return;
    }

    if ( bSelected && SelectionMode == ESmartTableSelectionMode::None )
    {
        UE_LOGFMT( LogSmartTablesInput, Verbose, "'{Table}' selected nothing. Its Selection Mode is None.", GetName() );

        return;
    }

    if ( bSelected && SelectionMode == ESmartTableSelectionMode::Single && NaturalRows.Num() > 1 )
    {
        UE_LOGFMT( LogSmartTablesInput, Verbose, "'{Table}' was asked to select {Asked} rows and took the first. Its Selection Mode is Single, so it holds one at a time.", GetName(), NaturalRows.Num() );

        NaturalRows = NaturalRows.Left( 1 );
    }

    ListView->SetItemSelection( NaturalRows, bSelected, ESelectInfo::Direct );
}

void USmartTable::SetRowsSelected( const TArray< int32 > & NaturalRows, bool bSelected )
{
    TArray< int32 > Present;
    Present.Reserve( NaturalRows.Num() );

    for ( const int32 Row : NaturalRows )
    {
        if ( IsRowDrawn( Row ) )
        {
            Present.Add( Row );
        }
    }

    if ( Present.Num() != NaturalRows.Num() )
    {
        UE_LOGFMT( LogSmartTablesInput, Verbose, "SetRowsSelected on '{Table}' took {Taken} of {Asked} rows. The rest are not drawn, so a filter is hiding them.", GetName(), Present.Num(), NaturalRows.Num() );
    }

    ApplySelection( Present, bSelected );
}

void USmartTable::SetSelectedItems( const TArray< UObject * > & Items, bool bSelected )
{
    USmartTableModel * RowModel = GetModel();
    if ( !RowModel )
    {
        UE_LOGFMT( LogSmartTablesInput, Verbose, "SetSelectedItems on '{Table}' selected nothing. This table has no model.", GetName() );

        return;
    }

    TArray< int32 > Rows;
    Rows.Reserve( Items.Num() );

    for ( UObject * Item : Items )
    {
        const int32 Row = RowModel->NaturalRowOfItem( Item );
        if ( IsRowDrawn( Row ) )
        {
            Rows.Add( Row );
        }
    }

    ApplySelection( Rows, bSelected );
}

void USmartTable::SelectColumnAt( float LocalX )
{

    const FName Hit = SmartTable::ColumnLayout::ColumnAt( ColumnView().ShownMetrics(), LocalX, LeadingColumnOffset() );

    if ( !Hit.IsNone() )
    {
        ColumnSelectionChanged( ColumnSelection.Select( Hit, ColumnView().ShownOrder() ) );
    }
}

void USmartTable::MoveColumnSelection( int32 Delta )
{
    ColumnSelectionChanged( ColumnSelection.Move( Delta, ColumnView().ShownOrder() ) );
}

void USmartTable::ColumnSelectionChanged( bool bChanged )
{
    if ( !bChanged )
    {
        return;
    }

    UE_LOGFMT( LogSmartTablesInput, Verbose, "Column selection on '{Table}' moved to '{Column}'.", GetName(), ColumnSelection.Selected );

    OnColumnSelectionChanged.Broadcast( ColumnSelection.Selected );
}

void USmartTable::FocusTable()
{
    if ( !ListView.IsValid() )
    {
        UE_LOGFMT( LogSmartTablesInput, Verbose, "FocusTable took no focus. '{Table}' is not up yet.", GetName() );
        return;
    }

    FSlateApplication::Get().SetKeyboardFocus( ListView, EFocusCause::SetDirectly );
}

int32 USmartTable::PresentedIndexOfCaret() const
{
    return PresentedRowOf( FocusedRow );
}

void USmartTable::MoveCaretTo( int32 PresentedRow )
{

    checkf( RowIndices.IsValidIndex( PresentedRow ), TEXT( "USmartTable '%s' moved its caret to presented row %d of %d" ), *GetName(), PresentedRow, RowIndices.Num() );

    const int32 TargetRow = RowIndices[ PresentedRow ];

    FocusedRow = TargetRow;

    ListView->SetSelection( TargetRow, ESelectInfo::OnNavigation );
    ListView->RequestScrollIntoView( TargetRow );
}

bool USmartTable::CanMoveCaret( const TCHAR * Caller ) const
{
    if ( !ListView.IsValid() || RowIndices.IsEmpty() || SelectionMode == ESmartTableSelectionMode::None )
    {

        UE_LOGFMT( LogSmartTablesInput, Verbose, "{Caller} changed nothing: {Reason}.", Caller, !ListView.IsValid() ? TEXT( "the widget is not built yet" ) : ( RowIndices.IsEmpty() ? TEXT( "the table has no rows" ) : TEXT( "this table's selection mode is None" ) ) );
        return false;
    }

    return true;
}

void USmartTable::MoveSelection( int32 Delta )
{
    if ( !CanMoveCaret( TEXT( "MoveSelection" ) ) || Delta == 0 )
    {
        return;
    }

    MoveCaretTo( SmartTable::RowSpace::StepCaret( PresentedIndexOfCaret(), Delta, RowIndices.Num() ) );
}

void USmartTable::MoveSelectionByPage( int32 PageDelta )
{
    if ( !CanMoveCaret( TEXT( "MoveSelectionByPage" ) ) || PageDelta == 0 )
    {
        return;
    }

    const int32 Page = FMath::Max( 1, ListView->GetNumGeneratedChildren() );

    MoveCaretTo( SmartTable::RowSpace::StepCaretByPage( PresentedIndexOfCaret(), Page, PageDelta, RowIndices.Num() ) );
}

void USmartTable::SelectFirstRow()
{
    if ( CanMoveCaret( TEXT( "SelectFirstRow" ) ) )
    {
        MoveCaretTo( 0 );
    }
}

void USmartTable::SelectLastRow()
{
    if ( CanMoveCaret( TEXT( "SelectLastRow" ) ) )
    {
        MoveCaretTo( RowIndices.Num() - 1 );
    }
}

void USmartTable::ActivateSelectedRow()
{
    if ( !IsRowDrawn( FocusedRow ) )
    {
        UE_LOGFMT( LogSmartTablesInput, Verbose, "ActivateSelectedRow fired nothing: {Reason}.", FocusedRow == INDEX_NONE ? TEXT( "no row has the caret" ) : TEXT( "the caret's row is not currently drawn" ) );
        return;
    }

    HandleRowActivated( FocusedRow );
}

FReply USmartTable::HandleListKeyDown( const FGeometry & Geometry, const FKeyEvent & KeyEvent )
{
    if ( KeyEvent.GetKey() != EKeys::Enter || !IsRowDrawn( FocusedRow ) )
    {
        return FReply::Unhandled();
    }

    HandleRowActivated( FocusedRow );

    return FReply::Handled();
}

void USmartTable::HandleSelectionChanged( int32 NaturalRow, ESelectInfo::Type SelectInfo )
{
    if ( bHoldSelectionBroadcast )
    {
        bSelectionBroadcastOwed = true;
        return;
    }

    if ( ListView.IsValid() )
    {
        for ( int32 SelectedRow : ListView->GetSelectedItems() )
        {
            if ( ListView->Private_HasSelectorFocus( SelectedRow ) )
            {
                FocusedRow = SelectedRow;
                break;
            }
        }
    }

    OnSelectionChanged.Broadcast( GetSelectedRows() );
}

void USmartTable::HandleRowActivated( int32 NaturalRow )
{
    OnRowActivated.Broadcast( ItemForRow( NaturalRow ), NaturalRow );
}

TArray< int32 > USmartTable::GetSelectedRows() const
{
    return ListView.IsValid() ? ListView->GetSelectedItems() : TArray< int32 >();
}

bool USmartTable::IsRowSelected( int32 NaturalRow ) const
{
    return ListView.IsValid() && ListView->IsItemSelected( NaturalRow );
}

void USmartTable::SetRowSelected( int32 NaturalRow, bool bSelected )
{
    if ( !ListView.IsValid() || !IsRowDrawn( NaturalRow ) )
    {
        UE_LOGFMT( LogSmartTablesInput, Verbose, "SetRowSelected({Row}) selected nothing: {Reason}.", NaturalRow, ListView.IsValid() ? TEXT( "that row is not currently drawn - a filter may be hiding it" ) : TEXT( "the widget is not built yet" ) );
        return;
    }

    ListView->SetItemSelection( NaturalRow, bSelected );
}

UObject * USmartTable::GetSelectedItem() const
{
    if ( !ListView.IsValid() )
    {
        return nullptr;
    }

    const TArray< int32 > Selected = ListView->GetSelectedItems();

    return Selected.IsEmpty() ? nullptr : ItemForRow( Selected[ 0 ] );
}

TArray< UObject * > USmartTable::GetSelectedItems() const
{
    TArray< UObject * > Items;
    if ( !ListView.IsValid() )
    {
        return Items;
    }

    for ( int32 NaturalRow : ListView->GetSelectedItems() )
    {
        if ( UObject * Item = ItemForRow( NaturalRow ) )
        {
            Items.Add( Item );
        }
    }

    return Items;
}

void USmartTable::SetSelectedItem( UObject * Item, bool bSelected )
{
    if ( !ListView.IsValid() || !Model )
    {
        UE_LOGFMT( LogSmartTablesInput, Verbose, "SetSelectedItem({Item}) selected nothing. The table still has no {Missing}.", GetNameSafe( Item ), ListView.IsValid() ? TEXT( "model" ) : TEXT( "built widget" ) );
        return;
    }

    const int32 NaturalRow = Model->NaturalRowOfItem( Item );

    if ( !IsRowDrawn( NaturalRow ) )
    {
        UE_LOGFMT( LogSmartTablesInput, Verbose, "SetSelectedItem({Item}) selected nothing. No drawn row holds that item.", GetNameSafe( Item ) );
        return;
    }

    ListView->SetItemSelection( NaturalRow, bSelected );
}

void USmartTable::ClearSelection()
{
    if ( ListView.IsValid() )
    {
        ListView->ClearSelection();
    }
}
