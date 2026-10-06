// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "Fonts/FontMeasure.h"
#include "Framework/Application/SlateApplication.h"
#include "Input/Events.h"
#include "Input/Reply.h"
#include "Layout/Geometry.h"
#include "Logging/StructuredLog.h"
#include "Math/UnrealMathUtility.h"
#include "SmartTable.h"
#include "SmartTableCell.h"
#include "SmartTableColumnLayout.h"
#include "SmartTableConstants.h"
#include "SmartTableLog.h"
#include "SmartTableModel.h"
#include "SmartTableResizeGesture.h"
#include "SmartTableSettings.h"
#include "Table/SmartTableColumnView.h"
#include "View/SmartTableHeaderRow.h"
#include "View/SmartTableListView.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Views/SHeaderRow.h"

FReply USmartTable::HandleHeaderRowMouseDown( const FGeometry & Geometry, const FPointerEvent & MouseEvent )
{
    if ( !HeaderRow.IsValid() )
    {
        return FReply::Unhandled();
    }

    if ( MouseEvent.GetEffectingButton() == EKeys::RightMouseButton && HeaderMenuOptions.bEnabled )
    {

        PushMenuAtPoint( HeaderRow.ToSharedRef(), MakeTableMenu(), FVector2D( MouseEvent.GetScreenSpacePosition() ), MouseEvent.GetUserIndex() );

        return FReply::Handled();
    }

    if ( MouseEvent.GetEffectingButton() != EKeys::LeftMouseButton )
    {
        return FReply::Unhandled();
    }

    const float LocalX = Geometry.AbsoluteToLocal( MouseEvent.GetScreenSpacePosition() ).X;

    float ColumnLeft     = 0.0f;
    const FName ColumnId = FindColumnEdgeAt( LocalX, ColumnLeft );
    if ( ColumnId.IsNone() )
    {
        return FReply::Unhandled();
    }

    FSmartTablePointerResize Dragging;
    Dragging.ColumnId     = ColumnId;
    Dragging.PressLocalX  = LocalX;
    Dragging.StartWidth   = GetColumnWidth( ColumnId );
    Dragging.UserIndex    = MouseEvent.GetUserIndex();
    Dragging.PointerIndex = MouseEvent.GetPointerIndex();

    Resize.Kind.Set< FSmartTablePointerResize >( Dragging );

    return FReply::Handled().CaptureMouse( HeaderRow.ToSharedRef() );
}

FReply USmartTable::HandleHeaderRowMouseMove( const FGeometry & Geometry, const FPointerEvent & MouseEvent )
{

    const FSmartTablePointerResize * Dragging = Resize.AsPointer();
    if ( !Dragging )
    {
        return FReply::Unhandled();
    }

    const float LocalX = Geometry.AbsoluteToLocal( MouseEvent.GetScreenSpacePosition() ).X;

    SetColumnWidthWhileDragging( Dragging->ColumnId, Dragging->StartWidth + ( LocalX - Dragging->PressLocalX ) );

    return FReply::Handled();
}

bool USmartTable::IsRightDragScrolling() const
{
    return ListView.IsValid() && ListView->IsRightDragScrolling();
}

void USmartTable::BeginColumnResize( FName ColumnId )
{
    ColumnId = ColumnForIntent( ColumnId );

    const FSmartTableColumn * Column = FindColumn( ColumnId );
    if ( !Column )
    {
        WarnUnknownColumn( TEXT( "BeginColumnResize" ), ColumnId );
        return;
    }

    if ( !Column->bResizable || !ColumnView().IsShown( *Column ) || !bHeaderWidthsLive )
    {
        UE_LOGFMT( LogSmartTablesLayout, Verbose, "BeginColumnResize('{Column}') stopped: {Reason}.", ColumnId, !Column->bResizable ? TEXT( "the author marked this column not resizable" ) : ( !ColumnView().IsShown( *Column ) ? TEXT( "the column is hidden" ) : TEXT( "the table has not been laid out yet" ) ) );
        return;
    }

    if ( Resize.AsPointer() )
    {
        UE_LOGFMT( LogSmartTablesLayout, Verbose, "BeginColumnResize('{Column}') stopped: a pointer is already dragging '{Held}'.", ColumnId, Resize.GetColumnId() );
        return;
    }

    if ( Resize.IsActive() )
    {
        EndColumnResize();
    }

    FSmartTableIntentResize Holding;
    Holding.ColumnId = ColumnId;

    Resize.Kind.Set< FSmartTableIntentResize >( Holding );

    UE_LOGFMT( LogSmartTablesLayout, Verbose, "An intent starts the resize of '{Column}'.", ColumnId );
}

void USmartTable::UpdateColumnResize( float DeltaPixels )
{
    if ( !Resize.IsFromIntent() )
    {
        UE_LOGFMT( LogSmartTablesLayout, Verbose, "UpdateColumnResize({Delta}) has no resize to move. Call BeginColumnResize first.", DeltaPixels );
        return;
    }

    SetColumnWidthWhileDragging( Resize.GetColumnId(), GetColumnWidth( Resize.GetColumnId() ) + DeltaPixels );
}

void USmartTable::EndColumnResize()
{
    if ( !Resize.IsFromIntent() )
    {
        return;
    }

    FinishResize();
}

bool USmartTable::EndPointerResize( int32 UserIndex, uint32 PointerIndex )
{
    if ( !Resize.IsDrivenBy( UserIndex, PointerIndex ) )
    {
        return false;
    }

    FinishResize();

    return true;
}

float USmartTable::WidthAtResizeStart( FName ColumnId )
{
    if ( const float * Snapshot = Resize.StartWidths.Find( ColumnId ) )
    {
        return *Snapshot;
    }

    const float Width = GetColumnWidth( ColumnId );

    Resize.StartWidths.Add( ColumnId, Width );

    return Width;
}

float USmartTable::TakeWidthFromColumnsRightOf( FName DraggedColumnId, float DesiredWidth )
{
    const TArray< FName > Shown = ColumnView().ShownOrder();
    const int32 DraggedAt       = Shown.IndexOfByKey( DraggedColumnId );
    if ( DraggedAt == INDEX_NONE || !HeaderRow.IsValid() )
    {
        return DesiredWidth;
    }

    TArray< float > StartRightWidths;
    StartRightWidths.Reserve( Shown.Num() - DraggedAt - 1 );
    for ( int32 Index = DraggedAt + 1; Index < Shown.Num(); ++Index )
    {
        StartRightWidths.Add( WidthAtResizeStart( Shown[ Index ] ) );
    }

    const SmartTable::ColumnLayout::FGrantedDrag Drag = SmartTable::ColumnLayout::ResolveGrowDrag( WidthAtResizeStart( DraggedColumnId ), DesiredWidth, StartRightWidths, MinColumnWidth );

    if ( !Drag.bNegotiated )
    {
        return Drag.Granted;
    }

    TArray< float > Drawn;
    Drawn.Reserve( Drag.RightWidths.Num() );
    for ( int32 Index = 0; Index < Drag.RightWidths.Num(); ++Index )
    {
        Drawn.Add( GetColumnWidth( Shown[ DraggedAt + 1 + Index ] ) );
    }

    for ( const int32 Index : SmartTable::ColumnLayout::WidthsToWrite( Drag.RightWidths, Drawn, SmartTable::Metrics::WidthEpsilon ) )
    {
        FSmartTableColumnLayout & Neighbour = LayoutFor( Shown[ DraggedAt + 1 + Index ] );
        Neighbour.Width                     = Drag.RightWidths[ Index ];

        Neighbour.bUserWidth = true;
    }

    return Drag.Granted;
}

void USmartTable::SetColumnWidthWhileDragging( FName ColumnId, float Width )
{

    const float Granted = bAllowHorizontalScroll ? FMath::Max( Width, MinColumnWidth ) : TakeWidthFromColumnsRightOf( ColumnId, Width );

    FSmartTableColumnLayout & Layout = LayoutFor( ColumnId );
    Layout.Width                     = Granted;
    Layout.bUserWidth                = true;

    bLayoutSavePending = true;

    UE_LOGFMT( LogSmartTablesInput, VeryVerbose, "'{Column}' pulls to {Width} px (asked for {Wanted}).", ColumnId, FMath::RoundToInt( Granted ), FMath::RoundToInt( Width ) );

    MarkColumnWidthsDirty();
    ReconcileColumnWidths();

    if ( ListView.IsValid() )
    {
        ListView->RequestListRefresh();
    }
}

void USmartTable::HandleColumnWidthChanged( float NewWidth, FName ColumnId )
{

    const FSmartTableColumn * Column = FindColumn( ColumnId );
    if ( !Column )
    {

        UE_LOGFMT( LogSmartTablesInput, VeryVerbose, "New width for '{Column}' dropped. There is no column under that id any more.", ColumnId );
        return;
    }

    if ( !CanResizeColumn( ColumnId ) )
    {
        UE_LOGFMT( LogSmartTablesInput, VeryVerbose, "New width for '{Column}' dropped. Resizing is off for it.", ColumnId );
        return;
    }

    SetColumnWidthWhileDragging( ColumnId, NewWidth );
}

void USmartTable::UpdateResizeAutoScroll()
{

    if ( Resize.IsFromIntent() )
    {
        return;
    }

    if ( !Resize.IsActive() || !bAllowHorizontalScroll || !HorizontalScrollBox.IsValid() || !HeaderRow.IsValid() )
    {
        return;
    }

    const FGeometry & Viewport = HorizontalScrollBox->GetTickSpaceGeometry();
    const float ViewportWidth  = Viewport.GetLocalSize().X;
    if ( ViewportWidth <= 0.0f )
    {
        return;
    }

    const FVector2D CursorInViewport = Viewport.AbsoluteToLocal( FSlateApplication::Get().GetCursorPos() );

    const float EdgeZone = 48.0f;
    const float Step     = 24.0f;

    const float Delta = SmartTable::ColumnLayout::AutoScrollStep( CursorInViewport.X, ViewportWidth, EdgeZone, Step );
    if ( FMath::IsNearlyZero( Delta ) )
    {
        return;
    }

    HorizontalScrollBox->SetScrollOffset( FMath::Max( HorizontalScrollBox->GetScrollOffset() + Delta, 0.0f ) );
}

void USmartTable::ScrollColumnsBy( float DeltaPixels )
{
    if ( !HorizontalScrollBox.IsValid() || !bAllowHorizontalScroll )
    {
        UE_LOGFMT( LogSmartTablesInput, Verbose, "ScrollColumnsBy({Delta}) went nowhere: {Reason}.", DeltaPixels, HorizontalScrollBox.IsValid() ? TEXT( "this table does not scroll horizontally" ) : TEXT( "the widget is not built yet" ) );
        return;
    }

    HorizontalScrollBox->SetScrollOffset( FMath::Max( HorizontalScrollBox->GetScrollOffset() + DeltaPixels, 0.0f ) );
}

void USmartTable::SettleGripWidths()
{

    if ( Resize.IsActive() )
    {
        return;
    }

    FinishResize();
}

void USmartTable::FinishResize()
{

    if ( Resize.IsAtRest() && !bLayoutSavePending )
    {
        return;
    }

    if ( Resize.IsActive() )
    {
        UE_LOGFMT( LogSmartTablesLayout, Verbose, "Column '{Column}' stops resizing at {Width} px.", Resize.GetColumnId(), FMath::RoundToInt( GetColumnWidth( Resize.GetColumnId() ) ) );
    }

    Resize.Reset();

    if ( bLayoutSavePending )
    {
        bLayoutSavePending = false;
        LayoutChangedByUser();
    }
}

void USmartTable::SizeColumnToContent( FName ColumnId )
{
    if ( !ListView.IsValid() || !HeaderRow.IsValid() )
    {
        UE_LOGFMT( LogSmartTablesLayout, Verbose, "SizeColumnToContent('{Column}') measured nothing. The widget is not up yet.", ColumnId );
        return;
    }

    const FSmartTableColumn * Column = FindColumn( ColumnId );
    if ( !Column )
    {
        WarnUnknownColumn( TEXT( "SizeColumnToContent" ), ColumnId );
        return;
    }

    const TSharedRef< FSlateFontMeasure > Measure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
    const FText Label                             = Column->GetLabel();

    float Content = Measure->Measure( Label.ToString(), HeaderTextStyle.Font ).X + SmartTable::Metrics::SortGlyphAllowance;

    if ( Column->CellClass )
    {

        Content = FMath::Max( Content, static_cast< float >( ListView->GetMaxRowSizeForColumn( ColumnId, Orient_Horizontal ).X ) );
    }

    if ( Model )
    {

        const int32 NumRows  = Model->GetNumPresentedRows();
        const int32 Measured = FMath::Min( NumRows, GetDefault< USmartTableSettings >()->MaxRowsMeasuredForAutoSize );

        for ( int32 PresentedRow = 0; PresentedRow < Measured; ++PresentedRow )
        {
            const int32 NaturalRow = Model->PresentedToNaturalRow( PresentedRow );
            const FString Text     = Model->GetCellText( NaturalRow, ColumnId ).ToString();

            Content = FMath::Max( Content, static_cast< float >( Measure->Measure( Text, CellTextStyle.Font ).X ) );
        }

        if ( Measured < NumRows )
        {

            UE_LOGFMT( LogSmartTablesLayout, Verbose, "Column '{Column}' sized to the first {Measured} of {Total} rows. Anything longer further down stays clipped.", ColumnId, Measured, NumRows );
        }
    }

    float Width = Content + Column->CellPadding.GetTotalSpaceAlong< Orient_Horizontal >();

    if ( MaxAutoSizeColumnWidth > 0.0f && Width > MaxAutoSizeColumnWidth )
    {

        UE_LOGFMT( LogSmartTablesLayout, Verbose, "Column '{Column}' wants {Wanted} px to fit its longest value, capped at {Cap}. Raise MaxAutoSizeColumnWidth on the table, or in Project Settings, Plugins, Smart Tables.", ColumnId, FMath::RoundToInt( Width ), FMath::RoundToInt( MaxAutoSizeColumnWidth ) );

        Width = MaxAutoSizeColumnWidth;
    }

    UE_LOGFMT( LogSmartTablesLayout, Verbose, "Column '{Column}' fits its content: {Width} px.", ColumnId, FMath::RoundToInt( Width ) );

    SetColumnWidth( ColumnId, Width );
}
