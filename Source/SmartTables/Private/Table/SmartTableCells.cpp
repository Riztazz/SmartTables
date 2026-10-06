// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "Framework/Application/SlateApplication.h"
#include "Framework/Application/SlateUser.h"
#include "Layout/Geometry.h"
#include "Logging/StructuredLog.h"
#include "Misc/AssertionMacros.h"
#include "SmartTable.h"
#include "SmartTableArrivals.h"
#include "SmartTableCell.h"
#include "SmartTableCellDragDropOp.h"
#include "SmartTableLog.h"
#include "SmartTableModel.h"
#include "SmartTableRowWidget.h"
#include "View/SmartTableListView.h"
#include "View/SmartTableRow.h"

USmartTableCell * USmartTable::AcquireCell( FName ColumnId, int32 NaturalRow )
{
    const FSmartTableColumn * Column = FindColumn( ColumnId );
    if ( !Column )
    {

        UE_LOGFMT( LogSmartTablesLayout, VeryVerbose, "Row {Row} gets no cell. '{Column}' is no column of this table.", NaturalRow, ColumnId );
        return nullptr;
    }

    if ( !Model )
    {
        UE_LOGFMT( LogSmartTablesData, VeryVerbose, "'{Column}' row {Row} gets no cell. The table has no model yet.", ColumnId, NaturalRow );
        return nullptr;
    }

    if ( Column->CellClass && !Model->GetRowItem( NaturalRow ) && !ItemlessColumnsWarned.Contains( ColumnId ) )
    {

        ItemlessColumnsWarned.Add( ColumnId );

        if ( !IsDesignTime() && Column->CellClass->GetDefaultObject< USmartTableCell >()->HasItemSetter() )
        {
            UE_LOGFMT( LogSmartTablesData, Warning, "Column '{Column}' draws with {Class}, whose Item Setter takes the row's object, and {Model} has none for row {Row}. Those cells are handed nothing. Add Get Row Item to the model, or clear Item Setter on the cell.", ColumnId, Column->CellClass->GetName(), Model->GetClass()->GetName(), NaturalRow );
        }
        else
        {
            UE_LOGFMT( LogSmartTablesData, Verbose, "Column '{Column}' names a cell class and {Model} keeps no item for its rows, so GetItem() in those cells hands back null. Cells that read the model text still work. Add GetRowItem if your cell needs the object.", ColumnId, Model->GetClass()->GetName() );
        }
    }

    TSubclassOf< USmartTableCell > CellClass = Column->CellClass;
    if ( !CellClass )
    {
        CellClass = USmartTableTextCell::StaticClass();
    }

    USmartTableCell * Cell = CellPool.GetOrCreateInstance< USmartTableCell >( CellClass );
    if ( !Cell )
    {

        const TWeakObjectPtr< const UClass > * Warned = CellClassFailuresWarned.Find( ColumnId );
        if ( !Warned || Warned->Get() != CellClass.Get() )
        {
            CellClassFailuresWarned.Add( ColumnId, CellClass.Get() );

            UE_LOGFMT( LogSmartTablesData, Warning, "Column '{Column}' draws nothing. No cell of class {Class} would build. Point CellClass on the column at a USmartTableCell subclass that is not Abstract, or clear it to get the stock text cell.", ColumnId, CellClass->GetName() );
        }

        return nullptr;
    }

    ensureMsgf( Cell->GetColumnId().IsNone(), TEXT( "Pooled cell arrived still bound to column '%s' while being assigned to '%s'" ), *Cell->GetColumnId().ToString(), *ColumnId.ToString() );

    Cell->SetVisibility( bAllowCellDragDrop ? ESlateVisibility::Visible : ( bInteractiveCells ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::HitTestInvisible ) );

    Cell->AssignCell( this, Model, NaturalRow, ColumnId, ReasonForRow( NaturalRow ) );

    return Cell;
}

void USmartTable::ReleaseCell( USmartTableCell * Cell )
{

    checkf( Cell, TEXT( "USmartTable '%s' was asked to release a null cell" ), *GetName() );

    Cell->ReleaseCell();
    CellPool.Release( Cell );
}

bool USmartTable::IsFirstItemSetterWarningFor( const UClass & CellClass )
{
    bool bAlreadyWarned = false;
    ItemSetterClassesWarned.Add( &CellClass, &bAlreadyWarned );

    return !bAlreadyWarned;
}

USmartTableRowWidget * USmartTable::AcquireRowWidget( int32 NaturalRow, const TSharedRef< SWidget > & ColumnsContent )
{
    if ( !RowWidgetClass )
    {
        return nullptr;
    }

    USmartTableRowWidget * RowWidget = RowWidgetPool.GetOrCreateInstance< USmartTableRowWidget >( RowWidgetClass );
    if ( !RowWidget )
    {
        if ( RowWidgetSlotWarnedFor.Get() != RowWidgetClass.Get() )
        {
            RowWidgetSlotWarnedFor = RowWidgetClass.Get();

            UE_LOGFMT( LogSmartTables, Warning, "Rows draw unwrapped. No widget of class {Class} would build. Point RowWidgetClass at a USmartTableRowWidget subclass that is not Abstract, or clear it.", RowWidgetClass->GetName() );
        }

        return nullptr;
    }

    if ( !RowWidget->HostColumns( ColumnsContent ) )
    {

        if ( RowWidgetSlotWarnedFor.Get() != RowWidget->GetClass() )
        {
            RowWidgetSlotWarnedFor = RowWidget->GetClass();

            UE_LOGFMT( LogSmartTables, Warning, "Rows draw unwrapped. {Class} carries no named slot for the cells. Add a single NamedSlot to the Widget Blueprint, or name one '{Slot}' if it has several already.", RowWidget->GetClass()->GetName(), USmartTableRowWidget::ColumnsSlotName );
        }

        RowWidgetPool.Release( RowWidget );

        return nullptr;
    }

    RowWidget->AssignRow( this, NaturalRow, ReasonForRow( NaturalRow ) );

    ArmArrivalIfOwed( NaturalRow, RowWidget );

    return RowWidget;
}

void USmartTable::ReleaseRowWidget( USmartTableRowWidget * RowWidget )
{

    checkf( RowWidget, TEXT( "USmartTable '%s' was asked to release a null row widget" ), *GetName() );

    RowWidget->ReleaseRow();
    RowWidgetPool.Release( RowWidget );
}

bool USmartTable::CanPointerStartDrag( const FPointerEvent & PointerEvent )
{
    if ( !FSlateApplication::IsInitialized() )
    {
        return false;
    }

    const TSharedPtr< const FSlateUser > User = FSlateApplication::Get().GetUser( PointerEvent );

    return User.IsValid() && !User->IsVirtualUser();
}

ESmartTableAssignReason USmartTable::ReasonForRow( int32 NaturalRow ) const
{
    if ( NaturalRow == INDEX_NONE || Arrivals.IsEmpty() || !Model )
    {
        return ESmartTableAssignReason::Scrolled;
    }

    return Arrivals.IsNew( Model->GetRowId( NaturalRow ) ) ? ESmartTableAssignReason::Added : ESmartTableAssignReason::Scrolled;
}

void USmartTable::ArmArrivalIfOwed( int32 NaturalRow, USmartTableRowWidget * Wrapper )
{
    if ( !Wrapper || !Model )
    {
        return;
    }

    if ( Arrivals.OwesEntrance( Model->GetRowId( NaturalRow ) ) )
    {
        Wrapper->ArmArrival();
    }
}

void USmartTable::ReassignChangedRows()
{
    if ( !Model || !ListView.IsValid() )
    {
        return;
    }

    ForEachGeneratedRow( [ this ]( SSmartTableRow & Row )
    {
        const int32 NaturalRow = Row.GetNaturalRow();
        const FName NowDrawing = Model->GetRowId( NaturalRow );

        if ( NowDrawing == Row.GetDrawnRowId() )
        {
            return;
        }

        Row.ReassignRow( NowDrawing, ReasonForRow( NaturalRow ) );

        ArmArrivalIfOwed( NaturalRow, Row.GetRowWidget() );
    } );
}

void USmartTable::UpdateArrivals()
{
    if ( Arrivals.IsEmpty() || !ListView.IsValid() || !Model )
    {
        return;
    }

    Arrivals.Expire( FSlateApplication::Get().GetCurrentTime(), ArrivalWindowSeconds );

    const FGeometry & ListGeometry = ListView->GetTickSpaceGeometry();
    const float ListTop            = ListGeometry.GetAbsolutePosition().Y;
    const float ListBottom         = ListTop + ListGeometry.GetAbsoluteSize().Y;

    ForEachGeneratedRow( [ this, ListTop, ListBottom ]( SSmartTableRow & Row )
    {
        USmartTableRowWidget * Wrapper = Row.GetRowWidget();
        if ( !Wrapper )
        {
            return;
        }

        const FName RowId = Model->GetRowId( Row.GetNaturalRow() );

        if ( !Arrivals.OwesEntrance( RowId ) )
        {

            Wrapper->CancelArrival();

            return;
        }

        const FGeometry & RowGeometry = Wrapper->GetCachedGeometry();

        if ( !FSmartTableArrivals::IsRevealed( RowGeometry.GetAbsolutePosition().Y, RowGeometry.GetAbsoluteSize().Y, ListTop, ListBottom, RevealFraction ) )
        {
            return;
        }

        Arrivals.MarkPlayed( RowId, Wrapper->PlayArrivalNow() );
    } );
}

ESmartTableCellDragRole USmartTable::GetCellDragRole( int32 NaturalRow, FName ColumnId ) const
{

    return CellDrag.IsActive() ? CellDrag.RoleOf( RowIdAt( NaturalRow ), ColumnId ) : ESmartTableCellDragRole::None;
}

void USmartTable::BeginCellDrag( int32 NaturalRow, FName ColumnId )
{
    CellDrag.Begin( RowIdAt( NaturalRow ), ColumnId );

    UE_LOGFMT( LogSmartTablesInput, VeryVerbose, "A drag on '{Table}' starts at row {Row}, column '{Column}'.", GetName(), NaturalRow, ColumnId );

    NotifyVisibleCellsOfDragRoles();
}

void USmartTable::SetCellDragTarget( int32 NaturalRow, FName ColumnId )
{
    if ( CellDrag.SetTarget( RowIdAt( NaturalRow ), ColumnId ) )
    {
        NotifyVisibleCellsOfDragRoles();
    }
}

void USmartTable::ClearCellDragTarget( int32 NaturalRow, FName ColumnId )
{
    if ( CellDrag.ClearTarget( RowIdAt( NaturalRow ), ColumnId ) )
    {
        NotifyVisibleCellsOfDragRoles();
    }
}

void USmartTable::EndCellDrag()
{
    if ( !CellDrag.End() )
    {
        return;
    }

    UE_LOGFMT( LogSmartTablesInput, VeryVerbose, "The drag on '{Table}' is over and marks nothing now.", GetName() );

    NotifyVisibleCellsOfDragRoles();
}

void USmartTable::NotifyVisibleCellsOfDragRoles()
{

    ForEachGeneratedRow( []( SSmartTableRow & Row )
    {
        Row.NotifyCellDragRoles();
    } );
}

void USmartTable::NotifyCellDropped( USmartTableCellDragDropOp * Payload, int32 TargetNaturalRow, FName TargetColumnId )
{

    checkf( Payload, TEXT( "USmartTable '%s' was told about a drop with no payload" ), *GetName() );

    const USmartTable * Source     = Payload->SourceTable.Get();
    USmartTableModel * SourceModel = Source ? Source->GetModel() : nullptr;

    if ( SourceModel && !Payload->SourceRowId.IsNone() )
    {
        const int32 SourceNow = SourceModel->FindKeptRow( { Payload->SourceRowId, Payload->SourceRow } );
        if ( SourceNow == INDEX_NONE )
        {
            UE_LOGFMT( LogSmartTablesInput, Verbose, "Table '{Table}': a drop landed on row {To} column '{ToColumn}' after row '{From}', where the drag began, left. Nothing is reported.", GetName(), TargetNaturalRow, TargetColumnId, Payload->SourceRowId );
            return;
        }

        Payload->SourceRow = SourceNow;
    }
    else
    {
        UE_LOGFMT( LogSmartTablesInput, Verbose, "Table '{Table}': the drop names {Missing}, so row {From} goes out as the drag began.", GetName(), SourceModel ? TEXT( "no source row id" ) : TEXT( "no source table with a model" ), Payload->SourceRow );
    }

    UE_LOGFMT( LogSmartTablesInput, Verbose, "Table '{Table}': row {From} column '{FromColumn}' landed on row {To} column '{ToColumn}'. Nothing moved. That is up to the listener.", GetName(), Payload->SourceRow, Payload->SourceColumnId, TargetNaturalRow, TargetColumnId );

    OnCellDropped.Broadcast( Payload, TargetNaturalRow, TargetColumnId );
}

void USmartTable::NotifyCellValueChanged( UObject * Item, int32 NaturalRow, FName ColumnId, float Value )
{

    OnCellValueChanged.Broadcast( Item, NaturalRow, ColumnId, Value );
}

void USmartTable::RefreshVisibleRowCells( FName ColumnId )
{

    ForEachGeneratedRow( [ ColumnId ]( SSmartTableRow & Row )
    {
        Row.RefreshCell( ColumnId );
    } );
}
