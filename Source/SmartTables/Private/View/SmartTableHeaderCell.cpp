// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "View/SmartTableHeaderCell.h"

#include "SmartTableConstants.h"

#include "Framework/Application/SlateApplication.h"
#include "Logging/StructuredLog.h"
#include "Misc/AssertionMacros.h"
#include "SmartTable.h"
#include "SmartTableLog.h"
#include "Styling/SlateBrush.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

TSharedRef< FSmartTableColumnDragDropOp > FSmartTableColumnDragDropOp::New( FName InColumnId, const FText & InLabel, const FSlateBrush & InBackground, const FTextBlockStyle & InLabelStyle )
{
    TSharedRef< FSmartTableColumnDragDropOp > Operation = MakeShared< FSmartTableColumnDragDropOp >();
    Operation->ColumnId                                 = InColumnId;
    Operation->Label                                    = InLabel;
    Operation->Background                               = InBackground;
    Operation->LabelStyle                               = InLabelStyle;
    Operation->Construct();

    return Operation;
}

TSharedPtr< SWidget > FSmartTableColumnDragDropOp::GetDefaultDecorator() const
{
    // clang-format off
    return SNew( SBorder )
        .BorderImage( &Background )
        .Padding( SmartTable::Metrics::ChromeCellPadding() )
        [
            SNew( STextBlock )
                .Text( Label )
                .TextStyle( &LabelStyle )
        ];
    // clang-format on
}

void SSmartTableHeaderCell::Construct( const FArguments & InArgs, USmartTable * InTable, FName InColumnId )
{

    checkf( InTable, TEXT( "A Smart Table header cell for '%s' was built with no owning table" ), *InColumnId.ToString() );

    Table      = InTable;
    ColumnId   = InColumnId;
    Column     = InArgs._Column;
    LabelStyle = Column.bOverrideHeaderTextStyle ? Column.HeaderTextStyle : InArgs._FallbackTextStyle;

    const FSlateBrush * Marker = &InTable->GetInsertMarkerBrush();

    SetColorAndOpacity( TAttribute< FLinearColor >::CreateSP( this, &SSmartTableHeaderCell::GetContentTint ) );

    // clang-format off
    ChildSlot
    [
        SNew( SOverlay )
        + SOverlay::Slot()
            [
                SNew( SHorizontalBox )
                + SHorizontalBox::Slot()
                    .FillWidth( 1.0f )
                    [
                        InArgs._Content.Widget != SNullWidget::NullWidget ? InArgs._Content.Widget :
                        StaticCastSharedRef< SWidget >( SNew( SBox )
                            .HeightOverride( InTable->GetHeaderHeight() )
                            .Padding( Column.CellPadding )
                            .HAlign( Column.HeaderHAlign )
                            .VAlign( VAlign_Center )
                            [
                                SNew( SHorizontalBox )

                                + SHorizontalBox::Slot()
                                    .AutoWidth()
                                    .VAlign( VAlign_Center )
                                    .Padding( 0.0f, 0.0f, Column.bShowHeaderIcon ? Column.HeaderIconSpacing : 0.0f, 0.0f )
                                    [
                                        SNew( SImage )
                                            .Image( &Column.HeaderIcon )
                                            .Visibility( Column.bShowHeaderIcon ? EVisibility::Visible : EVisibility::Collapsed )
                                    ]

                                + SHorizontalBox::Slot()
                                    .FillWidth( 1.0f )
                                    .VAlign( VAlign_Center )
                                    [
                                        SNew( STextBlock )
                                            .TextStyle( &LabelStyle )
                                            .Text( Column.GetLabel() )
                                            .OverflowPolicy( ETextOverflowPolicy::Ellipsis )
                                    ]
                            ] )
                    ]

                + SHorizontalBox::Slot()
                    .AutoWidth()
                    .VAlign( VAlign_Center )
                    .Padding( FMargin( 4.0f, 0.0f, 0.0f, 0.0f ) )
                    [
                        SNew( SImage )
                            .Image( this, &SSmartTableHeaderCell::GetSortBrush )
                            .Visibility( this, &SSmartTableHeaderCell::GetSortVisibility )
                            .ColorAndOpacity( this, &SSmartTableHeaderCell::GetSortTint )
                    ]
                + SHorizontalBox::Slot()
                    .AutoWidth()
                    .VAlign( VAlign_Center )
                    .Padding( FMargin( 2.0f, 0.0f, 0.0f, 0.0f ) )
                    [
                        SNew( SImage )
                            .Image( this, &SSmartTableHeaderCell::GetSortBrush )
                            .Visibility( this, &SSmartTableHeaderCell::GetSecondarySortVisibility )
                            .ColorAndOpacity( this, &SSmartTableHeaderCell::GetSortTint )
                    ]
            ]

        + SOverlay::Slot()
            .HAlign( HAlign_Left )
            [
                SNew( SBox )
                    .WidthOverride( SmartTable::Metrics::MarkerThickness )
                    .Visibility( this, &SSmartTableHeaderCell::GetInsertBeforeVisibility )
                    [
                        SNew( SImage )
                            .Image( Marker )
                    ]
            ]
        + SOverlay::Slot()
            .HAlign( HAlign_Right )
            [
                SNew( SBox )
                    .WidthOverride( SmartTable::Metrics::MarkerThickness )
                    .Visibility( this, &SSmartTableHeaderCell::GetInsertAfterVisibility )
                    [
                        SNew( SImage )
                            .Image( Marker )
                    ]
            ]

        + SOverlay::Slot()
            .VAlign( VAlign_Bottom )
            [
                SNew( SBox )
                    .HeightOverride( SmartTable::Metrics::MarkerThickness )
                    .Visibility( this, &SSmartTableHeaderCell::GetCaretVisibility )
                    [
                        SNew( SImage )
                            .Image( &InTable->GetColumnCaretBrush() )
                    ]
            ]
        + SOverlay::Slot()
            .HAlign( HAlign_Right )
            [
                SNew( SBox )
                    .WidthOverride( 1.0f )
                    .Visibility( EVisibility::HitTestInvisible )
                    [
                        SNew( SImage )
                            .Image( &InTable->GetColumnDividerBrush() )
                    ]
            ]
    ];
    // clang-format on
}

const FSlateBrush * SSmartTableHeaderCell::GetSortBrush() const
{
    const USmartTable * OwningTable = Table.Get();

    return OwningTable ? OwningTable->GetSortBrush( ColumnId ) : nullptr;
}

FSlateColor SSmartTableHeaderCell::GetSortTint() const
{
    const USmartTable * OwningTable = Table.Get();

    return OwningTable ? OwningTable->GetSortTint() : FSlateColor::UseForeground();
}

bool SSmartTableHeaderCell::DrawsSortArrow() const
{
    const FSlateBrush * Brush = GetSortBrush();

    return Brush && Brush->DrawAs != ESlateBrushDrawType::NoDrawType;
}

EVisibility SSmartTableHeaderCell::GetSortVisibility() const
{
    return DrawsSortArrow() ? EVisibility::HitTestInvisible : EVisibility::Collapsed;
}

FLinearColor SSmartTableHeaderCell::GetContentTint() const
{

    if ( !FSlateApplication::Get().GetModifierKeys().IsShiftDown() )
    {
        return FLinearColor::White;
    }

    const USmartTable * OwningTable = Table.Get();
    if ( !OwningTable )
    {
        return FLinearColor::White;
    }

    return OwningTable->IsDeadSecondaryColumn( ColumnId ) ? FLinearColor( 1.0f, 1.0f, 1.0f, 0.35f ) : FLinearColor::White;
}

EVisibility SSmartTableHeaderCell::GetSecondarySortVisibility() const
{
    const USmartTable * OwningTable = Table.Get();

    return OwningTable && OwningTable->IsSecondarySortColumn( ColumnId ) && DrawsSortArrow() ? EVisibility::HitTestInvisible : EVisibility::Collapsed;
}

bool SSmartTableHeaderCell::IsOverResizeEdge( const FGeometry & Geometry, const FPointerEvent & MouseEvent ) const
{
    const USmartTable * OwningTable = Table.Get();

    if ( !OwningTable || !OwningTable->CanResizeColumn( ColumnId ) )
    {
        return false;
    }

    const float LocalX = Geometry.AbsoluteToLocal( MouseEvent.GetScreenSpacePosition() ).X;

    return LocalX >= Geometry.GetLocalSize().X - OwningTable->GetResizeGripWidth();
}

FCursorReply SSmartTableHeaderCell::OnCursorQuery( const FGeometry & Geometry, const FPointerEvent & CursorEvent ) const
{

    return IsOverResizeEdge( Geometry, CursorEvent ) ? FCursorReply::Cursor( EMouseCursor::ResizeLeftRight ) : FCursorReply::Unhandled();
}

EVisibility SSmartTableHeaderCell::GetCaretVisibility() const
{
    const USmartTable * Owner = Table.Get();
    if ( !Owner || !Owner->IsColumnCaretShown() || ColumnId.IsNone() || Owner->GetSelectedColumn() != ColumnId )
    {
        return EVisibility::Collapsed;
    }

    if ( Owner->DoesColumnCaretNeedFocus() && !Owner->IsTableFocused() )
    {
        return EVisibility::Collapsed;
    }

    return EVisibility::HitTestInvisible;
}

FReply SSmartTableHeaderCell::OnPreviewMouseButtonDown( const FGeometry &, const FPointerEvent & )
{
    if ( USmartTable * Owner = Table.Get() )
    {
        Owner->SelectColumn( ColumnId );
    }

    return FReply::Unhandled();
}

EVisibility SSmartTableHeaderCell::GetInsertBeforeVisibility() const
{
    return bDragOver && bDropBefore ? EVisibility::HitTestInvisible : EVisibility::Collapsed;
}

EVisibility SSmartTableHeaderCell::GetInsertAfterVisibility() const
{
    return bDragOver && !bDropBefore ? EVisibility::HitTestInvisible : EVisibility::Collapsed;
}

FReply SSmartTableHeaderCell::OnMouseButtonDown( const FGeometry & Geometry, const FPointerEvent & MouseEvent )
{

    if ( MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && IsOverResizeEdge( Geometry, MouseEvent ) )
    {

        bPressWasOnResizeEdge = true;

        return FReply::Unhandled();
    }

    bPressWasOnResizeEdge = false;

    if ( MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton )
    {

        return FReply::Handled().DetectDrag( SharedThis( this ), EKeys::LeftMouseButton );
    }

    if ( MouseEvent.GetEffectingButton() == EKeys::RightMouseButton )
    {

        const USmartTable * OwningTable = Table.Get();
        if ( OwningTable && OwningTable->IsHeaderMenuEnabled() )
        {

            RightPressScreenPosition = MouseEvent.GetScreenSpacePosition();

            return FReply::Handled().CaptureMouse( SharedThis( this ) );
        }
    }

    return FReply::Unhandled();
}

FReply SSmartTableHeaderCell::OnMouseButtonUp( const FGeometry & Geometry, const FPointerEvent & MouseEvent )
{
    USmartTable * OwningTable = Table.Get();
    if ( !OwningTable )
    {
        return FReply::Unhandled();
    }

    if ( MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton )
    {

        const bool bEndedAResize = bPressWasOnResizeEdge || OwningTable->IsResizingColumn();

        bPressWasOnResizeEdge = false;

        if ( bEndedAResize )
        {
            return FReply::Handled();
        }

        OwningTable->HandleHeaderGesture( ColumnId, MouseEvent.IsShiftDown() );

        return FReply::Handled();
    }

    if ( MouseEvent.GetEffectingButton() == EKeys::RightMouseButton && OwningTable->IsHeaderMenuEnabled() )
    {

        const TOptional< FVector2D > Pressed = RightPressScreenPosition;
        RightPressScreenPosition.Reset();

        if ( Pressed.IsSet() && FVector2D::Distance( *Pressed, FVector2D( MouseEvent.GetScreenSpacePosition() ) ) >= FSlateApplication::Get().GetDragTriggerDistance() )
        {
            UE_LOGFMT( LogSmartTablesInput, Verbose, "A right drag from header '{Column}' opened no menu. The pointer moved too far to count as a click.", ColumnId );

            return FReply::Handled().ReleaseMouseCapture();
        }

        const FWidgetPath * EventPath = MouseEvent.GetEventPath();

        OwningTable->PushMenuOnPath( SharedThis( this ), EventPath ? *EventPath : FWidgetPath(), OwningTable->MakeHeaderMenu( ColumnId ), MouseEvent.GetScreenSpacePosition(), MouseEvent.GetUserIndex() );

        return FReply::Handled().ReleaseMouseCapture();
    }

    return FReply::Unhandled();
}

FReply SSmartTableHeaderCell::OnDragDetected( const FGeometry & Geometry, const FPointerEvent & MouseEvent )
{
    if ( !USmartTable::CanPointerStartDrag( MouseEvent ) )
    {
        UE_LOGFMT( LogSmartTablesLayout, Verbose, "Header '{Column}' will not drag: {Reason}.", ColumnId, TEXT( "the pointer holds no cursor" ) );
        return FReply::Unhandled();
    }

    USmartTable * OwningTable = Table.Get();

    if ( !OwningTable || !OwningTable->CanReorderColumns() || !OwningTable->FindColumn( ColumnId ) )
    {
        UE_LOGFMT( LogSmartTablesLayout, Verbose, "Header '{Column}' will not drag: {Reason}.", ColumnId, !OwningTable ? TEXT( "its table is gone" ) : ( !OwningTable->CanReorderColumns() ? TEXT( "bAllowColumnReorder is off on the table" ) : TEXT( "it is the no-columns placeholder" ) ) );
        return FReply::Unhandled();
    }

    return FReply::Handled().BeginDragDrop( FSmartTableColumnDragDropOp::New( ColumnId, OwningTable->GetColumnLabel( ColumnId ), OwningTable->GetHeaderStyle().BackgroundBrush, OwningTable->GetHeaderTextStyle() ) );
}

void SSmartTableHeaderCell::OnDragLeave( const FDragDropEvent & DragDropEvent )
{
    bDragOver = false;
}

FReply SSmartTableHeaderCell::OnDragOver( const FGeometry & Geometry, const FDragDropEvent & DragDropEvent )
{
    const TSharedPtr< FSmartTableColumnDragDropOp > Operation = DragDropEvent.GetOperationAs< FSmartTableColumnDragDropOp >();
    if ( !Operation.IsValid() || Operation->ColumnId == ColumnId )
    {
        return FReply::Unhandled();
    }

    const float LocalX = Geometry.AbsoluteToLocal( DragDropEvent.GetScreenSpacePosition() ).X;

    bDragOver   = true;
    bDropBefore = LocalX < Geometry.GetLocalSize().X * 0.5f;

    return FReply::Handled();
}

FReply SSmartTableHeaderCell::OnDrop( const FGeometry & Geometry, const FDragDropEvent & DragDropEvent )
{
    bDragOver = false;

    const TSharedPtr< FSmartTableColumnDragDropOp > Operation = DragDropEvent.GetOperationAs< FSmartTableColumnDragDropOp >();
    USmartTable * OwningTable                                 = Table.Get();
    if ( !Operation.IsValid() || !OwningTable )
    {

        UE_LOGFMT( LogSmartTablesLayout, Verbose, "A drop on header '{Column}' went nowhere: {Reason}.", ColumnId, Operation.IsValid() ? TEXT( "its table is gone" ) : TEXT( "the payload is not a Smart Table column" ) );
        return FReply::Unhandled();
    }

    OwningTable->MoveColumnNextTo( Operation->ColumnId, ColumnId, !bDropBefore );

    return FReply::Handled();
}
