// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "View/SmartTableRow.h"

#include "Algo/Transform.h"
#include "Input/DragAndDrop.h"
#include "Logging/StructuredLog.h"
#include "Misc/AssertionMacros.h"
#include "Rendering/DrawElements.h"
#include "SmartTable.h"
#include "SmartTableCell.h"
#include "SmartTableLog.h"
#include "SmartTableModel.h"
#include "SmartTableRowWidget.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SOverlay.h"

namespace
{

    class FSmartTableRowDragDropOp : public FDragDropOperation
    {
    public:
        DRAG_DROP_OPERATOR_TYPE( FSmartTableRowDragDropOp, FDragDropOperation )

        TArray< FSmartTableKeptRow > Rows;

        TWeakObjectPtr< USmartTable > SourceTable;
    };
}

void SSmartTableRow::Construct( const FArguments & InArgs, const TSharedRef< STableViewBase > & OwnerTable, USmartTable * InTable, int32 InNaturalRow )
{

    checkf( InTable, TEXT( "A Smart Table row was built with no owning table" ) );

    Table      = InTable;
    NaturalRow = InNaturalRow;

    if ( USmartTableModel * RowModel = InTable->GetModel() )
    {
        DrawnRowId = RowModel->GetRowId( NaturalRow );
    }

    FSuperRowType::FArguments RowArgs = FSuperRowType::FArguments().Style( &InTable->GetRowStyle() );

    if ( InTable->AllowsRowReorder() )
    {
        RowArgs.OnDragDetected( this, &SSmartTableRow::HandleDragDetected ).OnCanAcceptDrop( this, &SSmartTableRow::HandleCanAcceptDrop ).OnAcceptDrop( this, &SSmartTableRow::HandleAcceptDrop );
    }

    FSuperRowType::Construct( RowArgs, OwnerTable );

    ApplyRowColor();
}

void SSmartTableRow::ApplyRowColor()
{
    USmartTable * OwningTable = Table.Get();
    USmartTableModel * Model  = OwningTable ? OwningTable->GetModel() : nullptr;
    if ( !Model || NaturalRow == INDEX_NONE )
    {
        return;
    }

    const FLinearColor Colour = Model->GetRowColor( NaturalRow );

    SetBorderBackgroundColor( Colour.A > 0.0f ? FSlateColor( Colour ) : FSlateColor( FLinearColor::White ) );
}

SSmartTableRow::~SSmartTableRow()
{
    ReleaseCells();
    ReleaseRowWidget();
}

void SSmartTableRow::ConstructChildren( ETableViewMode::Type InOwnerTableMode, const TAttribute< FMargin > & InPadding, const TSharedRef< SWidget > & InContent )
{
    STableRow< int32 >::Content = InContent;

    TSharedRef< SWidget > RowContent = InContent;

    if ( USmartTable * OwningTable = Table.Get() )
    {

        if ( USmartTableRowWidget * Wrapper = OwningTable->AcquireRowWidget( NaturalRow, InContent ) )
        {
            RowWidget  = Wrapper;
            RowContent = Wrapper->TakeWidget();
        }
    }

    ChildSlot.Padding( InPadding )[ RowContent ];
}

void SSmartTableRow::ReassignRow( FName NewRowId, ESmartTableAssignReason Reason )
{
    DrawnRowId = NewRowId;

    RefreshCells( Reason );

    NotifyCellDragRoles();

    if ( USmartTableRowWidget * Wrapper = RowWidget.Get() )
    {
        Wrapper->AssignRow( Table.Get(), NaturalRow, Reason );
    }
}

void SSmartTableRow::ReleaseRowWidget()
{
    if ( USmartTableRowWidget * Wrapper = RowWidget.Get() )
    {
        if ( USmartTable * OwningTable = Table.Get() )
        {
            OwningTable->ReleaseRowWidget( Wrapper );
        }
        else
        {

            Wrapper->ReleaseRow();
        }
    }

    RowWidget.Reset();
}

FReply SSmartTableRow::OnMouseButtonUp( const FGeometry & Geometry, const FPointerEvent & MouseEvent )
{
    USmartTable * OwningTable = Table.Get();

    const bool bScrolling = OwningTable && OwningTable->IsRightDragScrolling();

    if ( MouseEvent.GetEffectingButton() == EKeys::RightMouseButton && OwningTable && OwningTable->IsRowMenuEnabled() && !bScrolling )
    {

        OwningTable->OpenRowMenuFromPointer( NaturalRow, FVector2D( MouseEvent.GetScreenSpacePosition() ), MouseEvent.GetUserIndex() );

        return FReply::Handled();
    }

    if ( MouseEvent.GetEffectingButton() == EKeys::RightMouseButton && bScrolling )
    {
        return FReply::Handled();
    }

    return SMultiColumnTableRow< int32 >::OnMouseButtonUp( Geometry, MouseEvent );
}

FReply SSmartTableRow::HandleDragDetected( const FGeometry & Geometry, const FPointerEvent & MouseEvent )
{
    if ( !USmartTable::CanPointerStartDrag( MouseEvent ) )
    {
        UE_LOGFMT( LogSmartTablesInput, Verbose, "Row {Row} will not drag: the pointer holds no cursor.", NaturalRow );
        return FReply::Unhandled();
    }

    USmartTable * OwningTable = Table.Get();
    USmartTableModel * Model  = OwningTable ? OwningTable->GetModel() : nullptr;
    if ( !Model )
    {
        return FReply::Unhandled();
    }

    TArray< int32 > Picked = OwningTable->GetSelectedRows();

    if ( !Picked.Contains( NaturalRow ) )
    {
        Picked = { NaturalRow };
    }

    TSharedRef< FSmartTableRowDragDropOp > Operation = MakeShared< FSmartTableRowDragDropOp >();
    Operation->SourceTable                           = OwningTable;

    Algo::Transform( Picked, Operation->Rows, [ Model ]( int32 Row )
    {
        return Model->KeepRow( Row );
    } );

    UE_LOGFMT( LogSmartTablesInput, Verbose, "A row drag starts on row {Row} of '{Table}', holding {Count} row(s).", NaturalRow, OwningTable->GetName(), Operation->Rows.Num() );

    return FReply::Handled().BeginDragDrop( Operation );
}

int32 SSmartTableRow::GapForZone( EItemDropZone Zone )
{
    USmartTable * OwningTable = Table.Get();
    if ( !OwningTable )
    {
        return INDEX_NONE;
    }

    const int32 PresentedRow = GetIndexInList();
    if ( PresentedRow == INDEX_NONE )
    {
        return INDEX_NONE;
    }

    const int32 PresentedGap = Zone == EItemDropZone::AboveItem ? PresentedRow : PresentedRow + 1;

    return OwningTable->NaturalGapForPresentedGap( PresentedGap );
}

TOptional< EItemDropZone > SSmartTableRow::HandleCanAcceptDrop( const FDragDropEvent & DragDropEvent, EItemDropZone Zone, int32 Row )
{
    const TSharedPtr< FSmartTableRowDragDropOp > Operation = DragDropEvent.GetOperationAs< FSmartTableRowDragDropOp >();

    USmartTable * OwningTable = Table.Get();

    if ( !Operation.IsValid() || !OwningTable || Operation->SourceTable.Get() != OwningTable )
    {
        return TOptional< EItemDropZone >();
    }

    const EItemDropZone Snapped = Zone == EItemDropZone::BelowItem ? EItemDropZone::BelowItem : EItemDropZone::AboveItem;

    return GapForZone( Snapped ) == INDEX_NONE ? TOptional< EItemDropZone >() : TOptional< EItemDropZone >( Snapped );
}

FReply SSmartTableRow::HandleAcceptDrop( const FDragDropEvent & DragDropEvent, EItemDropZone Zone, int32 Row )
{
    const TSharedPtr< FSmartTableRowDragDropOp > Operation = DragDropEvent.GetOperationAs< FSmartTableRowDragDropOp >();

    USmartTable * OwningTable = Table.Get();
    if ( !Operation.IsValid() || !OwningTable )
    {
        return FReply::Unhandled();
    }

    const int32 Gap = GapForZone( Zone );
    if ( Gap == INDEX_NONE )
    {
        return FReply::Unhandled();
    }

    OwningTable->MoveRowsFromDrop( Operation->Rows, Gap );

    return FReply::Handled();
}

int32 SSmartTableRow::OnPaintDropIndicator( EItemDropZone InItemDropZone, const FPaintArgs & Args, const FGeometry & AllottedGeometry, const FSlateRect & MyCullingRect, FSlateWindowElementList & OutDrawElements, int32 LayerId, const FWidgetStyle & InWidgetStyle, bool bParentEnabled ) const
{
    USmartTable * OwningTable = Table.Get();
    if ( !OwningTable )
    {
        return LayerId;
    }

    const FSlateBrush & Marker = OwningTable->GetRowInsertMarkerBrush();

    const FVector2f RowSize = AllottedGeometry.GetLocalSize();
    const float Thickness   = FMath::Max( 1.0f, Marker.ImageSize.Y );

    const float Top = InItemDropZone == EItemDropZone::BelowItem ? RowSize.Y - Thickness * 0.5f : -Thickness * 0.5f;

    FSlateDrawElement::MakeBox( OutDrawElements, LayerId++, AllottedGeometry.ToPaintGeometry( FVector2f( RowSize.X, Thickness ), FSlateLayoutTransform( FVector2f( 0.0f, Top ) ) ), &Marker, ESlateDrawEffect::None, Marker.GetTint( InWidgetStyle ) * InWidgetStyle.GetColorAndOpacityTint() );

    return LayerId;
}

FReply SSmartTableRow::OnPreviewMouseButtonDown( const FGeometry & Geometry, const FPointerEvent & MouseEvent )
{
    if ( USmartTable * OwningTable = Table.Get() )
    {

        OwningTable->SelectColumnAt( Geometry.AbsoluteToLocal( MouseEvent.GetScreenSpacePosition() ).X );
    }

    return FReply::Unhandled();
}

TSharedRef< SWidget > SSmartTableRow::GenerateWidgetForColumn( const FName & ColumnId )
{
    USmartTable * OwningTable = Table.Get();
    if ( !OwningTable || NaturalRow == INDEX_NONE )
    {

        UE_LOGFMT( LogSmartTablesData, VeryVerbose, "Row {Row} draws nothing for '{Column}': {Reason}.", NaturalRow, ColumnId, OwningTable ? TEXT( "the row has no natural row" ) : TEXT( "its table is gone" ) );
        return SNullWidget::NullWidget;
    }

    if ( ColumnId == USmartTable::RowNumberColumnId )
    {

        return OwningTable->MakeRowNumberWidget( SharedThis( this ) );
    }

    for ( int32 Index = Cells.Num() - 1; Index >= 0; --Index )
    {
        USmartTableCell * Existing = Cells[ Index ].Get();
        if ( !Existing )
        {
            Cells.RemoveAtSwap( Index );
            continue;
        }

        if ( Existing->GetColumnId() == ColumnId )
        {

            OwningTable->ReleaseCell( Existing );
            Cells.RemoveAtSwap( Index );
        }
    }

    USmartTableCell * Cell = OwningTable->AcquireCell( ColumnId, NaturalRow );
    if ( !Cell )
    {

        return SNullWidget::NullWidget;
    }

    Cells.Add( Cell );

    {

        const USmartTable::FCellWalkScope Walk( OwningTable );

        MarkCellDragRole( *Cell );
    }

    const FSmartTableColumn * Column = OwningTable->FindColumn( ColumnId );

    const TWeakObjectPtr< USmartTable > WeakTable = OwningTable;
    const TWeakPtr< SSmartTableRow > WeakRow      = SharedThis( this );
    const FName CapturedColumn                    = ColumnId;

    auto MarkingBrush = [ WeakTable, WeakRow, CapturedColumn ]() -> const FSlateBrush *
    {
        const USmartTable * MarkTable = WeakTable.Get();

        if ( !MarkTable || !MarkTable->IsCellDragActive() )
        {
            return nullptr;
        }

        const TSharedPtr< SSmartTableRow > Row = WeakRow.Pin();

        return Row.IsValid() ? MarkTable->GetCellDragBrush( MarkTable->GetCellDragRoleOfRowId( Row->GetDrawnRowId(), CapturedColumn ) ) : nullptr;
    };

    // clang-format off
    return SNew( SBox )
        .Padding( Column ? Column->CellPadding : FMargin( 0.0f ) )
        .MinDesiredHeight( OwningTable->GetRowHeight() )
        .VAlign( VAlign_Center )
        [
            SNew( SOverlay )
            + SOverlay::Slot()
                [
                    Cell->TakeWidget()
                ]

            + SOverlay::Slot()
                [
                    SNew( SImage )
                        .Image_Lambda( MarkingBrush )
                        .Visibility_Lambda( [ MarkingBrush ]()
                            {
                                return MarkingBrush() ? EVisibility::HitTestInvisible : EVisibility::Collapsed;
                            } )
                ]
        ];
    // clang-format on
}

void SSmartTableRow::NotifyCellDragRoles()
{

    const USmartTable::FCellWalkScope Walk( Table.Get() );

    const TArray< TWeakObjectPtr< USmartTableCell >, TInlineAllocator< 16 > > Marked( Cells );

    for ( const TWeakObjectPtr< USmartTableCell > & Weak : Marked )
    {
        if ( USmartTableCell * Cell = Weak.Get() )
        {
            MarkCellDragRole( *Cell );
        }
    }
}

void SSmartTableRow::MarkCellDragRole( USmartTableCell & Cell ) const
{
    if ( const USmartTable * OwningTable = Table.Get() )
    {
        Cell.SetCellDragRole( OwningTable->GetCellDragRoleOfRowId( DrawnRowId, Cell.GetColumnId() ) );
    }
}

void SSmartTableRow::RefreshCell( FName ColumnId, ESmartTableAssignReason Reason )
{
    if ( NaturalRow == INDEX_NONE )
    {
        UE_LOGFMT( LogSmartTablesData, VeryVerbose, "A row with no natural row behind it cannot redraw '{Column}'.", ColumnId );
        return;
    }

    const USmartTable::FCellWalkScope Walk( Table.Get() );

    for ( const TWeakObjectPtr< USmartTableCell > & Weak : Cells )
    {
        USmartTableCell * Cell = Weak.Get();
        if ( Cell && Cell->GetColumnId() == ColumnId )
        {
            Cell->AssignCell( Cell->GetTable(), Cell->GetModel(), NaturalRow, ColumnId, Reason );
            break;
        }
    }

    ApplyRowColor();
}

void SSmartTableRow::RefreshCells( ESmartTableAssignReason Reason )
{
    if ( NaturalRow == INDEX_NONE )
    {
        UE_LOGFMT( LogSmartTablesData, VeryVerbose, "A row with no natural row behind it has nothing to read." );
        return;
    }

    const USmartTable::FCellWalkScope Walk( Table.Get() );

    const TArray< TWeakObjectPtr< USmartTableCell >, TInlineAllocator< 16 > > Assigned( Cells );

    for ( const TWeakObjectPtr< USmartTableCell > & Weak : Assigned )
    {
        if ( USmartTableCell * Cell = Weak.Get() )
        {

            Cell->AssignCell( Cell->GetTable(), Cell->GetModel(), NaturalRow, Cell->GetColumnId(), Reason );
        }
    }

    ApplyRowColor();
}

void SSmartTableRow::ReleaseCells()
{
    USmartTable * OwningTable = Table.Get();

    const USmartTable::FCellWalkScope Walk( OwningTable );

    const TArray< TWeakObjectPtr< USmartTableCell > > Held = MoveTemp( Cells );

    for ( const TWeakObjectPtr< USmartTableCell > & Weak : Held )
    {
        if ( USmartTableCell * Cell = Weak.Get() )
        {
            if ( OwningTable )
            {
                OwningTable->ReleaseCell( Cell );
            }
            else
            {

                Cell->ReleaseCell();
            }
        }
    }
}
