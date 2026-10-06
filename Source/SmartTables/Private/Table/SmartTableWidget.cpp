// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "Framework/Application/SlateApplication.h"
#include "HAL/PlatformTime.h"
#include "Logging/StructuredLog.h"
#include "Math/UnrealMathUtility.h"
#include "SmartTable.h"
#include "SmartTableHelpers.h"
#include "SmartTableLog.h"
#include "SmartTableModel.h"
#include "Table/SmartTableHeight.h"
#include "Types/SlateStructs.h"
#include "Types/WidgetActiveTimerDelegate.h"
#include "View/SmartTableHeaderRow.h"
#include "View/SmartTableListView.h"
#include "View/SmartTableRoot.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Images/SThrobber.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBar.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/SWindow.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/SHeaderRow.h"
#include "Widgets/Views/SListView.h"

TSharedRef< SWidget > USmartTable::RebuildWidget()
{

    ApplyStyleAsset();

    CellPool.SetWorld( GetWorld() );
    RowWidgetPool.SetWorld( GetWorld() );

    const TSharedRef< SSmartTableHeaderRow > OwnHeaderRow = SNew( SSmartTableHeaderRow ).Style( &HeaderStyle );
    OwnHeaderRow->Table                                   = this;

    HeaderRow = OwnHeaderRow;

    HeaderRow->SetOnMouseButtonDown( FPointerEventHandler::CreateUObject( this, &USmartTable::HandleHeaderRowMouseDown ) );
    HeaderRow->SetOnMouseMove( FPointerEventHandler::CreateUObject( this, &USmartTable::HandleHeaderRowMouseMove ) );

    BackgroundStyle.SetBackgroundBrush( BackgroundBrush );

    using FRowDelegates = TSlateDelegates< int32 >;

    VerticalScrollBar = SNew( SScrollBar ).Style( &ScrollBarStyle ).Orientation( Orient_Vertical );

    // clang-format off
    ListView = SNew( SSmartTableListView )
        .ListItemsSource( &RowIndices )
        .SelectionMode( SmartTable::ToSlateSelectionMode( SelectionMode ) )
        .ListViewStyle( &BackgroundStyle )
        .ScrollBarStyle( &ScrollBarStyle )
        .ExternalScrollbar( VerticalScrollBar )

        .ConsumeMouseWheel( EConsumeMouseWheel::Always )
        .OnGenerateRow( FRowDelegates::FOnGenerateRow::CreateUObject( this, &USmartTable::HandleGenerateRow ) )
        .OnSelectionChanged( FRowDelegates::FOnSelectionChanged::CreateUObject( this, &USmartTable::HandleSelectionChanged ) )
        .OnMouseButtonDoubleClick( FRowDelegates::FOnMouseButtonClick::CreateUObject( this, &USmartTable::HandleRowActivated ) )

        .OnKeyDownHandler( FOnKeyDown::CreateUObject( this, &USmartTable::HandleListKeyDown ) )
        .HeaderRow( HeaderRow );
    // clang-format on

    ListView->SetIsRightClickScrollingEnabled( bAllowRightClickDragScroll );

    ListView->Table = this;

    HeaderRow->SetVisibility( bShowHeader ? EVisibility::Visible : EVisibility::Collapsed );

    HeaderRow->SetOnGetMaxRowSizeForColumn( FOnGetMaxRowSizeForColumn::CreateSP( ListView.ToSharedRef(), &SListView< int32 >::GetMaxRowSizeForColumn ) );

    ApplyDesignTimePreview();

    ResolvedWidths.Reset();
    LastReconciledWidth = -1.0f;
    bColumnWidthsDirty  = true;
    bHeaderWidthsLive   = false;
    HeaderGutterWidth   = 0.0f;

    Scroll.Reset();

    LoadStoredLayout();

    UE_LOGFMT( LogSmartTablesData, Verbose, "Table '{Table}' builds: {Columns} column(s), model {Model}, {DesignTime}.", GetName(), Columns.Num(), GetNameSafe( Model.Get() ), IsDesignTime() ? TEXT( "design time" ) : TEXT( "runtime" ) );

    RebuildHeader();
    RebuildRowIndices();

    SetSortSpec( ActiveLayout.SortSpec );

    if ( HeightMode == ESmartTableHeight::FitRows && RowHeight <= 0.0f )
    {

        UE_LOGFMT( LogSmartTablesLayout, Warning, "HeightMode is FitRows and RowHeight is 0, so there is no row height to count up. Set a RowHeight, or use Fill instead." );
    }

    // clang-format off
    TSharedRef< SOverlay > Root = SNew( SOverlay )
        + SOverlay::Slot()
            [
                SNew( SHorizontalBox )
                + SHorizontalBox::Slot()
                    .FillWidth( 1.0f )
                    [
                        SAssignNew( HorizontalScrollBox, SScrollBox )
                            .Orientation( Orient_Horizontal )
                            .ScrollBarStyle( &ScrollBarStyle )
                            .ScrollBarVisibility( bAllowHorizontalScroll ? EVisibility::Visible : EVisibility::Collapsed )
                        + SScrollBox::Slot()
                            .FillSize( 1.0f )
                            [
                                SNew( SBox )
                                    .HeightOverride_UObject( this, &USmartTable::GetFitRowsHeight )
                                    [
                                        ListView.ToSharedRef()
                                    ]
                            ]
                    ]
                + SHorizontalBox::Slot()
                    .AutoWidth()
                    [
                        VerticalScrollBar.ToSharedRef()
                    ]
            ]

        + SOverlay::Slot()
            .HAlign( HAlign_Center )
            .VAlign( VAlign_Center )
            [
                SAssignNew( EmptyStateText, STextBlock )
                    .TextStyle( &EmptyTextStyle )
                    .Text_UObject( this, &USmartTable::GetEmptyStateText )
                    .Visibility_UObject( this, &USmartTable::GetEmptyStateVisibility )
            ]

        + SOverlay::Slot()
            [
                SNew( SImage )
                    .Image( &FocusBorderBrush )
                    .Visibility_UObject( this, &USmartTable::GetFocusBorderVisibility )
            ]

        + SOverlay::Slot()
            [
                SNew( SBorder )
                    .BorderImage( &BusyOverlayBrush )
                    .HAlign( HAlign_Center )
                    .VAlign( VAlign_Center )
                    .Visibility_UObject( this, &USmartTable::GetBusyOverlayVisibility )
                    [
                        SNew( SCircularThrobber )
                    ]
            ];
    // clang-format on

    Root->RegisterActiveTimer( 0.0f, FWidgetActiveTimerDelegate::CreateUObject( this, &USmartTable::TickTable ) );

    return SNew( SSmartTableRoot, this, ListView )[ Root ];
}

void USmartTable::ReleaseSlateResources( bool bReleaseChildren )
{
    Super::ReleaseSlateResources( bReleaseChildren );

    CellPool.ReleaseAllSlateResources();
    RowWidgetPool.ReleaseAllSlateResources();

    MenuHost.Dismiss();

    ListView.Reset();
    HeaderRow.Reset();
    EmptyStateText.Reset();
    HorizontalScrollBox.Reset();
    VerticalScrollBar.Reset();
    GeneratedRows.Reset();

    Resize.Reset();
}

void USmartTable::SynchronizeProperties()
{
    Super::SynchronizeProperties();

    if ( ListView.IsValid() )
    {
        ListView->SetSelectionMode( SmartTable::ToSlateSelectionMode( SelectionMode ) );

        BackgroundStyle.SetBackgroundBrush( BackgroundBrush );
        ListView->SetStyle( &BackgroundStyle );

        ListView->SetIsRightClickScrollingEnabled( bAllowRightClickDragScroll );
    }

    if ( HeaderRow.IsValid() )
    {
        HeaderRow->SetVisibility( bShowHeader ? EVisibility::Visible : EVisibility::Collapsed );
    }

    if ( HorizontalScrollBox.IsValid() )
    {
        HorizontalScrollBox->SetScrollBarVisibility( bAllowHorizontalScroll ? EVisibility::Visible : EVisibility::Collapsed );
    }

    if ( StyleAsset != AppliedStyleAsset )
    {
        AppliedStyleAsset = StyleAsset;

        ApplyStyleAsset();
        RefreshAppliedStyle();
    }

    MeasureRowNumberWidth();

    ApplyDesignTimePreview();
    RebuildHeader();
}

EActiveTimerReturnType USmartTable::TickTable( double CurrentTime, float DeltaTime )
{

    SampleViewAtEnd();

    UpdateArrivals();

    if ( bHeaderRebuildPending && !bWalkingRowCells )
    {
        bHeaderRebuildPending = false;
        RebuildHeader();
    }

    if ( HeaderRow.IsValid() && FSlateApplication::Get().GetModifierKeys().IsShiftDown() )
    {
        RefreshDeadSecondaryColumns();
    }

    ReconcileColumnWidths();

    UpdateResizeAutoScroll();

    SettleGripWidths();

    StartAtEndIfOwed();
    StepScrollAnimation( DeltaTime );
    WarnIfSizedByContent();

    return EActiveTimerReturnType::Continue;
}

void USmartTable::WarnIfSizedByContent()
{
    if ( bWarnedSizedByContent || !ListView.IsValid() || RowHeight <= 0.0f )
    {
        return;
    }

    constexpr int32 SizeCheckPeriodInFrames = 30;
    if ( ++FramesSinceSizeCheck < SizeCheckPeriodInFrames )
    {
        return;
    }

    FramesSinceSizeCheck = 0;

    const float Allotted  = ListView->GetTickSpaceGeometry().GetLocalSize().Y;
    const int32 Generated = ListView->GetNumGeneratedChildren();
    if ( Allotted <= 0.0f || Generated <= 0 )
    {
        return;
    }

    const TSharedPtr< SWindow > Window = FSlateApplication::Get().FindWidgetWindow( ListView.ToSharedRef() );
    if ( !Window.IsValid() )
    {
        return;
    }

    const float WindowHeight = Window->GetClientSizeInScreen().Y;
    if ( !SmartTable::Height::LooksSizedByContent( Allotted, Generated, WindowHeight, RowHeight ) )
    {
        return;
    }

    bWarnedSizedByContent = true;

    if ( HeightMode == ESmartTableHeight::FitRows )
    {
        UE_LOGFMT( LogSmartTablesLayout, Warning, "'{Table}' built {Built} rows because HeightMode is FitRows with no MaxVisibleRows, so it asked for the height of every row it holds. Set MaxVisibleRows, or use HeightMode Fill inside a parent that gives it a height.", GetName(), Generated );
        return;
    }

    const int32 OnScreen = SmartTable::Height::RowsWindowFits( WindowHeight, RowHeight );

    UE_LOGFMT( LogSmartTablesLayout, Warning, "'{Table}' built {Built} rows where about {Fits} fit on screen: its parent is sizing to content, so the list was asked how tall it wants to be and gave back all of them. Place it in a Vertical Box slot set to Fill, a Size Box with a height, or anything that gives it a height instead of asking for one.", GetName(), Generated, OnScreen );
}

FOptionalSize USmartTable::GetFitRowsHeight() const
{

    if ( HeightMode != ESmartTableHeight::FitRows || RowHeight <= 0.0f )
    {
        return FOptionalSize();
    }

    const float DrawnHeaderHeight = bShowHeader && HeaderRow.IsValid() ? FMath::Max( HeaderRow->GetDesiredSize().Y, HeaderHeight ) : 0.0f;

    return FOptionalSize( SmartTable::Height::FitRows( Model ? Model->GetNumPresentedRows() : 0, MaxVisibleRows, RowHeight, DrawnHeaderHeight ) );
}

FText USmartTable::GetEmptyStateText() const
{
    return MakeEmptyStateText( Model ? Model->GetNumRows() : 0, GetFilterText(), EmptyText, NoMatchesTextFormat );
}

EVisibility USmartTable::GetEmptyStateVisibility() const
{

    return !Model || Model->GetNumPresentedRows() == 0 ? EVisibility::HitTestInvisible : EVisibility::Collapsed;
}

FText USmartTable::MakeEmptyStateText( int32 NumRows, const FText & FilterText, const FText & InEmptyText, const FText & InNoMatchesFormat )
{

    const bool bFilteredOut = NumRows > 0 && !FilterText.IsEmpty();

    return bFilteredOut ? FText::Format( InNoMatchesFormat, FilterText ) : InEmptyText;
}

void USmartTable::SetBusy( bool bInBusy )
{
    if ( bBusy == bInBusy )
    {
        return;
    }

    bBusy            = bInBusy;
    BusyStartSeconds = FPlatformTime::Seconds();

    UE_LOGFMT( LogSmartTables, Verbose, "Table '{Table}' is {State}. The overlay shows after {Delay} s of it.", GetName(), bBusy ? TEXT( "busy" ) : TEXT( "idle" ), BusyOverlayDelaySeconds );
}

void USmartTable::HandleModelBusyChanged( bool bInBusy )
{
    SetBusy( bInBusy );
}

EVisibility USmartTable::GetBusyOverlayVisibility() const
{
    if ( !bBusy || FPlatformTime::Seconds() - BusyStartSeconds < BusyOverlayDelaySeconds )
    {
        return EVisibility::Collapsed;
    }

    return bBlockInputWhileBusy ? EVisibility::Visible : EVisibility::HitTestInvisible;
}

void USmartTable::SetRowHeight( float InRowHeight )
{
    RowHeight = InRowHeight;

    bWarnedSizedByContent = false;

    if ( ListView.IsValid() )
    {
        ListView->RebuildList();
    }
}

void USmartTable::SetShowRowNumbers( bool bInShowRowNumbers )
{
    bShowRowNumbers = bInShowRowNumbers;

    MeasureRowNumberWidth();

    RebuildHeader();

    if ( ListView.IsValid() )
    {
        ListView->RebuildList();
    }
}

void USmartTable::SetShowHeader( bool bInShowHeader )
{
    if ( bShowHeader == bInShowHeader )
    {
        return;
    }

    bShowHeader = bInShowHeader;

    if ( HeaderRow.IsValid() )
    {
        HeaderRow->SetVisibility( bShowHeader ? EVisibility::Visible : EVisibility::Collapsed );
    }

    InvalidateLayoutAndVolatility();
}

void USmartTable::SetHeightMode( ESmartTableHeight InHeightMode, int32 InMaxVisibleRows )
{
    if ( HeightMode == InHeightMode && MaxVisibleRows == InMaxVisibleRows )
    {
        return;
    }

    HeightMode     = InHeightMode;
    MaxVisibleRows = FMath::Max( InMaxVisibleRows, 0 );

    bWarnedSizedByContent = false;

    UE_LOGFMT( LogSmartTablesLayout, Verbose, "'{Table}' sets its height by {Mode} from here on, with a cap of {Max} row(s).", GetName(), HeightMode == ESmartTableHeight::FitRows ? TEXT( "its rows" ) : TEXT( "its parent" ), MaxVisibleRows );

    InvalidateLayoutAndVolatility();
}

void USmartTable::SetInteractiveCells( bool bInInteractiveCells )
{
    bInteractiveCells = bInInteractiveCells;

    if ( ListView.IsValid() )
    {
        ListView->RebuildList();
    }
}

void USmartTable::SetAllowCellDragDrop( bool bInAllowCellDragDrop )
{
    if ( bAllowCellDragDrop == bInAllowCellDragDrop )
    {
        return;
    }

    bAllowCellDragDrop = bInAllowCellDragDrop;

    UE_LOGFMT( LogSmartTablesInput, Verbose, "'{Table}' {State} drag and drop on cells.", GetName(), bAllowCellDragDrop ? TEXT( "allows" ) : TEXT( "refuses" ) );

    if ( ListView.IsValid() )
    {
        ListView->RebuildList();
    }
}

void USmartTable::SetAllowRowReorder( bool bInAllowRowReorder )
{
    if ( bAllowRowReorder == bInAllowRowReorder )
    {
        return;
    }

    bAllowRowReorder = bInAllowRowReorder;

    UE_LOGFMT( LogSmartTablesInput, Verbose, "'{Table}' {State} row reorder.", GetName(), bAllowRowReorder ? TEXT( "allows" ) : TEXT( "refuses" ) );

    if ( ListView.IsValid() )
    {
        ListView->RebuildList();
    }
}

void USmartTable::SetAllowHorizontalScroll( bool bInAllowHorizontalScroll )
{
    bAllowHorizontalScroll = bInAllowHorizontalScroll;

    if ( HorizontalScrollBox.IsValid() )
    {
        HorizontalScrollBox->SetScrollBarVisibility( bAllowHorizontalScroll ? EVisibility::Visible : EVisibility::Collapsed );

        if ( !bAllowHorizontalScroll )
        {
            HorizontalScrollBox->SetScrollOffset( 0.0f );
        }
    }

    RefreshRowsForNewColumnWidths();
}

void USmartTable::SetAllowRightClickDragScroll( bool bInAllowRightClickDragScroll )
{
    bAllowRightClickDragScroll = bInAllowRightClickDragScroll;

    if ( ListView.IsValid() )
    {
        ListView->SetIsRightClickScrollingEnabled( bAllowRightClickDragScroll );
    }
}
