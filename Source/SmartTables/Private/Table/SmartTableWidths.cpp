// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "Fonts/FontMeasure.h"
#include "Framework/Application/SlateApplication.h"
#include "Logging/StructuredLog.h"
#include "Math/UnrealMathUtility.h"
#include "SmartTable.h"
#include "SmartTableColumnLayout.h"
#include "SmartTableConstants.h"
#include "SmartTableLog.h"
#include "SmartTableModel.h"
#include "Table/SmartTableColumnView.h"
#include "View/SmartTableListView.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Views/SHeaderRow.h"

void USmartTable::ReconcileColumnWidths()
{
    if ( !HeaderRow.IsValid() )
    {

        return;
    }

    const float Available = GetAvailableColumnWidth();
    if ( !bColumnWidthsDirty && FMath::IsNearlyEqual( Available, LastReconciledWidth, SmartTable::Metrics::WidthEpsilon ) )
    {
        return;
    }

    const float TotalWidth = Available - SmartTable::Metrics::ColumnEdgeMargin;
    if ( TotalWidth <= 0.0f )
    {

        return;
    }

    const TArray< SmartTable::ColumnLayout::FResolvedWidth > Resolved = SmartTable::ColumnLayout::ResolveWidths( ColumnView().ShownMetrics(), TotalWidth, MinColumnWidth, bStretchLastColumn );

    if ( Resolved.IsEmpty() )
    {
        return;
    }

    const TArray< FName > Moved = SmartTable::ColumnLayout::MovedColumns( ResolvedWidths, Resolved, SmartTable::Metrics::WidthEpsilon );

    ResolvedWidths.Reset();
    for ( const SmartTable::ColumnLayout::FResolvedWidth & Entry : Resolved )
    {
        ResolvedWidths.Add( Entry.ColumnId, Entry.Width );
    }

    LastReconciledWidth = Available;
    bColumnWidthsDirty  = false;

    UE_LOGFMT( LogSmartTablesLayout, Verbose, "{Count} column width(s) resolve for '{Table}' across {Width} px. {Moved} moved.", Resolved.Num(), GetName(), FMath::RoundToInt( TotalWidth ), Moved.Num() );

    const float Gutter = LeadingColumnOffset();

    if ( !bHeaderWidthsLive || !FMath::IsNearlyEqual( Gutter, HeaderGutterWidth, SmartTable::Metrics::WidthEpsilon ) )
    {
        bHeaderWidthsLive = true;
        HeaderGutterWidth = Gutter;

        RebuildHeader();

        return;
    }

    for ( const FName ColumnId : Moved )
    {
        RefreshVisibleRowCells( ColumnId );
    }
}

bool USmartTable::CanResizeColumn( FName ColumnId ) const
{
    if ( !bAllowColumnResize )
    {
        return false;
    }

    const FSmartTableColumn * Column = FindColumn( ColumnId );

    return Column && Column->bResizable && ColumnView().IsShown( *Column );
}

FName USmartTable::FindColumnEdgeAt( float LocalX, float & OutColumnLeft ) const
{
    if ( !bAllowColumnResize )
    {
        return NAME_None;
    }

    const SmartTable::ColumnLayout::FEdgeHit Hit = SmartTable::ColumnLayout::FindEdgeAt( ColumnView().ShownMetrics(), LocalX, ResizeGripWidth, LeadingColumnOffset() );

    if ( !Hit.ColumnId.IsNone() )
    {
        OutColumnLeft = Hit.Left;
    }

    return Hit.ColumnId;
}

void USmartTable::MeasureRowNumberWidth()
{
    MeasuredRowNumberWidth = 0.0f;

    const int32 NumRows = Model ? Model->GetNumPresentedRows() : 0;
    if ( !bShowRowNumbers || NumRows <= 0 || !FSlateApplication::IsInitialized() )
    {
        return;
    }

    const TSharedRef< FSlateFontMeasure > Measure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
    const FVector2D Size                          = Measure->Measure( FText::AsNumber( NumRows ).ToString(), RowNumberTextStyle.Font );

    const FVector2D Digit = Measure->Measure( TEXT( "0" ), RowNumberTextStyle.Font );

    MeasuredRowNumberWidth = static_cast< float >( Size.X + Digit.X ) + SmartTable::Metrics::ChromeCellPadding().GetTotalSpaceAlong< Orient_Horizontal >();
}

float USmartTable::GetRowNumberWidth() const
{

    return FMath::Max( RowNumberColumnWidth, MeasuredRowNumberWidth );
}

void USmartTable::SetStretchLastColumn( bool bInStretchLastColumn )
{
    bStretchLastColumn = bInStretchLastColumn;

    MarkColumnWidthsDirty();
    ReconcileColumnWidths();

    RefreshRowsForNewColumnWidths();
}

void USmartTable::RefreshRowsForNewColumnWidths()
{
    if ( !ListView.IsValid() )
    {
        return;
    }

    ListView->RequestListRefresh();
}

float USmartTable::GetAvailableColumnWidth() const
{
    const float Viewport = HorizontalScrollBox.IsValid() ? HorizontalScrollBox->GetTickSpaceGeometry().GetLocalSize().X : ( HeaderRow.IsValid() ? HeaderRow->GetTickSpaceGeometry().GetLocalSize().X : 0.0f );

    return Viewport - LeadingColumnOffset();
}

float USmartTable::GetColumnWidth( FName ColumnId ) const
{
    return ColumnView().CurrentWidth( ColumnId );
}

void USmartTable::SetColumnWidth( FName ColumnId, float Width )
{
    if ( !FindColumn( ColumnId ) )
    {
        WarnUnknownColumn( TEXT( "SetColumnWidth" ), ColumnId );
        return;
    }

    FSmartTableColumnLayout & Layout = LayoutFor( ColumnId );
    Layout.Width                     = FMath::Max( Width, MinColumnWidth );
    Layout.bUserWidth                = true;

    UE_LOGFMT( LogSmartTablesLayout, Verbose, "Column '{Column}' width now {Width} px (asked for {Wanted}).", ColumnId, FMath::RoundToInt( Layout.Width ), FMath::RoundToInt( Width ) );

    MarkColumnWidthsDirty();
    ReconcileColumnWidths();
    LayoutChangedByUser();
}

void USmartTable::ResetColumnWidth( FName ColumnId )
{
    if ( !FindColumn( ColumnId ) )
    {
        WarnUnknownColumn( TEXT( "ResetColumnWidth" ), ColumnId );
        return;
    }

    FSmartTableColumnLayout & Layout = LayoutFor( ColumnId );
    Layout.Width                     = 0.0f;
    Layout.bUserWidth                = false;

    UE_LOGFMT( LogSmartTablesLayout, Verbose, "Column '{Column}' width cleared. Back to its authored sizing.", ColumnId );

    MarkColumnWidthsDirty();
    ReconcileColumnWidths();
    LayoutChangedByUser();
}
