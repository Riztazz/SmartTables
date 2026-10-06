// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "SmartTableStyle.h"

#include "SmartTableColour.h"
#include "SmartTableConstants.h"
#include "SmartTableSettings.h"

#include "Brushes/SlateColorBrush.h"
#include "Brushes/SlateNoResource.h"
#include "Math/Color.h"
#include "Misc/CoreMisc.h"
#include "Styling/CoreStyle.h"
#include "Styling/DefaultStyleCache.h"
#include "Styling/SlateBrush.h"
#include "Styling/UMGCoreStyle.h"
#include "UObject/UObjectGlobals.h"

namespace
{

    FSlateBrush ColourBrush( const FLinearColor & Colour )
    {
        return FSlateColorBrush( Colour );
    }
}

USmartTableStyle::USmartTableStyle()
{
    BuildDefaultsFromSettings();
}

void USmartTableStyle::BuildDefaultsFromSettings()
{

    if ( IsRunningDedicatedServer() )
    {
        return;
    }

    const USmartTableSettings & Settings = *GetDefault< USmartTableSettings >();

    BackgroundBrush = ColourBrush( SmartTable::Srgb( 24, 26, 31 ) );

    RowStyle = UE::Slate::Private::FDefaultStyleCache::GetRuntime().GetTableRowStyle();
    RowStyle.SetEvenRowBackgroundBrush( FSlateNoResource() )
        .SetOddRowBackgroundBrush( ColourBrush( SmartTable::Srgb( 29, 32, 38 ) ) )
        .SetEvenRowBackgroundHoveredBrush( ColourBrush( Settings.RowHoveredColor ) )
        .SetOddRowBackgroundHoveredBrush( ColourBrush( Settings.RowHoveredColor ) )
        .SetActiveBrush( ColourBrush( Settings.SelectionColor ) )
        .SetActiveHoveredBrush( ColourBrush( Settings.SelectionHoveredColor ) )
        .SetInactiveBrush( ColourBrush( SmartTable::Srgb( 58, 66, 76 ) ) )
        .SetInactiveHoveredBrush( ColourBrush( SmartTable::Srgb( 68, 78, 90 ) ) )
        .SetTextColor( Settings.TextColor )
        .SetSelectedTextColor( FLinearColor::White );

    ScrollBarStyle = UE::Slate::Private::FDefaultStyleCache::GetRuntime().GetScrollBarStyle();
    ScrollBarStyle.SetNormalThumbImage( ColourBrush( SmartTable::Srgb( 72, 80, 92 ) ) )
        .SetHoveredThumbImage( ColourBrush( SmartTable::Srgb( 96, 106, 120 ) ) )
        .SetDraggedThumbImage( ColourBrush( SmartTable::Srgb( 120, 132, 148 ) ) )
        .SetVerticalBackgroundImage( ColourBrush( Settings.ScrollbarTrackColor ) )
        .SetHorizontalBackgroundImage( ColourBrush( Settings.ScrollbarTrackColor ) )
        .SetVerticalTopSlotImage( FSlateNoResource() )
        .SetVerticalBottomSlotImage( FSlateNoResource() )
        .SetHorizontalTopSlotImage( FSlateNoResource() )
        .SetHorizontalBottomSlotImage( FSlateNoResource() );

    const FLinearColor HeaderContentColour = SmartTable::Srgb( 139, 146, 156 );

    HeaderStyle = FUMGCoreStyle::Get().GetWidgetStyle< FHeaderRowStyle >( "TableView.Header" );

    HeaderStyle.SetBackgroundBrush( ColourBrush( Settings.HeaderSurfaceColor ) ).SetHorizontalSeparatorBrush( ColourBrush( Settings.SeparatorColor ) ).SetHorizontalSeparatorThickness( 1.0f );

    HeaderStyle.SetForegroundColor( FSlateColor( HeaderContentColour ) );

    HeaderStyle.ColumnStyle.SetNormalBrush( ColourBrush( Settings.HeaderSurfaceColor ) ).SetHoveredBrush( ColourBrush( Settings.HeaderHoveredColor ) );

    const FTableColumnHeaderStyle & CoreColumn = FCoreStyle::Get().GetWidgetStyle< FHeaderRowStyle >( "TableView.Header" ).ColumnStyle;

    HeaderStyle.ColumnStyle.SetSortPrimaryAscendingImage( CoreColumn.SortPrimaryAscendingImage ).SetSortPrimaryDescendingImage( CoreColumn.SortPrimaryDescendingImage ).SetSortSecondaryAscendingImage( CoreColumn.SortSecondaryAscendingImage ).SetSortSecondaryDescendingImage( CoreColumn.SortSecondaryDescendingImage );

    HeaderStyle.LastColumnStyle = HeaderStyle.ColumnStyle;

    HeaderStyle.ColumnSplitterStyle.SetHandleNormalBrush( ColourBrush( Settings.ColumnDividerColor ) ).SetHandleHighlightBrush( ColourBrush( SmartTable::Srgb( 110, 122, 140 ) ) );

    ColumnDividerBrush = ColourBrush( Settings.ColumnDividerColor );

    CellTextStyle = UE::Slate::Private::FDefaultStyleCache::GetRuntime().GetTextBlockStyle();
    CellTextStyle.SetFont( FUMGCoreStyle::GetDefaultFontStyle( "Regular", 11 ) ).SetColorAndOpacity( Settings.TextColor );

    HeaderTextStyle = CellTextStyle;
    HeaderTextStyle.SetFont( FUMGCoreStyle::GetDefaultFontStyle( "Bold", 9 ) ).SetColorAndOpacity( HeaderContentColour ).SetTransformPolicy( ETextTransformPolicy::ToUpper );

    EmptyTextStyle = CellTextStyle;
    EmptyTextStyle.SetColorAndOpacity( Settings.MutedTextColor );

    RowNumberTextStyle = CellTextStyle;
    RowNumberTextStyle.SetColorAndOpacity( Settings.MutedTextColor );

    MenuStyle.BackgroundBrush = ColourBrush( SmartTable::Srgb( 34, 37, 44 ) );

    MenuStyle.EntryStyle = FButtonStyle().SetNormal( FSlateNoResource() ).SetHovered( ColourBrush( Settings.SelectionColor ) ).SetPressed( ColourBrush( Settings.SelectionHoveredColor ) ).SetDisabled( FSlateNoResource() ).SetNormalPadding( FMargin( 0.0f ) ).SetPressedPadding( FMargin( 0.0f ) );

    MenuStyle.LabelTextStyle   = CellTextStyle;
    MenuStyle.HeadingTextStyle = HeaderTextStyle;

    MenuStyle.SeparatorBrush = ColourBrush( Settings.SeparatorColor );

    MenuStyle.CheckStyle = FCoreStyle::Get().GetWidgetStyle< FCheckBoxStyle >( "Menu.Check" );
    MenuStyle.CheckStyle.SetForegroundColor( FSlateColor( Settings.TextColor ) );

    BusyOverlayBrush = ColourBrush( FLinearColor( 0.0f, 0.0f, 0.0f, 0.5f ) );

    InsertMarkerBrush = ColourBrush( SmartTable::Srgb( 120, 180, 255 ) );

    RowInsertMarkerBrush           = ColourBrush( SmartTable::Srgb( 120, 180, 255 ) );
    RowInsertMarkerBrush.ImageSize = FVector2f( 1.0f, 2.0f );

    ColumnCaretBrush = ColourBrush( Settings.AccentColor );

    FocusBorderBrush.DrawAs          = ESlateBrushDrawType::RoundedBox;
    FocusBorderBrush.TintColor       = FSlateColor( FLinearColor::Transparent );
    FocusBorderBrush.OutlineSettings = FSlateBrushOutlineSettings( SmartTable::Metrics::OutlineWidth, FSlateColor( Settings.AccentColor ), 1.0f );

    DragSourceBrush.DrawAs    = ESlateBrushDrawType::Image;
    DragSourceBrush.TintColor = FSlateColor( FLinearColor( 0.0f, 0.0f, 0.0f, 0.55f ) );

    DragTargetBrush.DrawAs                       = ESlateBrushDrawType::RoundedBox;
    DragTargetBrush.TintColor                    = FSlateColor( FLinearColor( 0.0f, 0.0f, 0.0f, 0.0f ) );
    DragTargetBrush.OutlineSettings.RoundingType = ESlateBrushRoundingType::FixedRadius;
    DragTargetBrush.OutlineSettings.CornerRadii  = FVector4( 3.0, 3.0, 3.0, 3.0 );
    DragTargetBrush.OutlineSettings.Width        = SmartTable::Metrics::OutlineWidth;

    DragTargetBrush.OutlineSettings.Color = FSlateColor( Settings.AccentColor );
}
