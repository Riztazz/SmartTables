// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "View/SmartTableHeaderColumns.h"

#include "Internationalization/Internationalization.h"
#include "SmartTable.h"
#include "SmartTableConstants.h"
#include "View/SmartTableHeaderCell.h"
#include "View/SmartTableRow.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "SmartTables"

namespace
{

    const FName NoColumnsPlaceholderId( TEXT( "SmartTable_NoColumns" ) );
}

SHeaderRow::FColumn::FArguments SmartTable::HeaderColumns::RowNumber( USmartTable & Table, float Width )
{
    // clang-format off
    return SHeaderRow::Column( USmartTable::RowNumberColumnId )
        .DefaultLabel( LOCTEXT( "RowNumberHeader", "#" ) )
        .FixedWidth( Width )
        .HAlignHeader( HAlign_Fill )
        .HAlignCell( HAlign_Right )
        .HeaderContentPadding( FMargin( 0.0f ) )
        .HeaderContent()
        [
            SNew( SSmartTableHeaderCell, &Table, USmartTable::RowNumberColumnId )
            [
                SNew( SBox )
                    .HeightOverride( Table.GetHeaderHeight() )
                    .Padding( SmartTable::Metrics::ChromeCellPadding() )
                    .HAlign( HAlign_Right )
                    .VAlign( VAlign_Center )
                    [
                        SNew( STextBlock )
                            .TextStyle( &Table.GetHeaderTextStyle() )
                            .Text( LOCTEXT( "RowNumberHeader", "#" ) )
                    ]
            ]
        ];
    // clang-format on
}

SHeaderRow::FColumn::FArguments SmartTable::HeaderColumns::Placeholder( USmartTable & Table )
{
    // clang-format off
    return SHeaderRow::Column( NoColumnsPlaceholderId )
        .DefaultLabel( FText::GetEmpty() )
        .FillWidth( 1.0f )
        .HAlignHeader( HAlign_Fill )
        .HeaderContentPadding( FMargin( 0.0f ) )
        .HeaderContent()
        [
            SNew( SSmartTableHeaderCell, &Table, NoColumnsPlaceholderId )
            [
                SNew( SBox )
                    .HeightOverride( Table.GetHeaderHeight() )
                    .Padding( SmartTable::Metrics::ChromeCellPadding() )
                    .VAlign( VAlign_Center )
                    [
                        SNew( STextBlock )
                            .TextStyle( &Table.GetEmptyTextStyle() )
                            .Text( LOCTEXT( "NoColumnsShown", "All columns hidden - right-click to bring one back" ) )
                    ]
            ]
        ];
    // clang-format on
}

SHeaderRow::FColumn::FArguments SmartTable::HeaderColumns::ForColumn( USmartTable & Table, const FSmartTableColumn & Column, ColumnLayout::EHeaderWidthMode Mode, const TAttribute< float > & ManualWidth, const FOnWidthChanged & OnWidthChanged )
{
    // clang-format off
    SHeaderRow::FColumn::FArguments Args = SHeaderRow::Column( Column.ColumnId )
        .DefaultLabel( Column.GetLabel() )
        .HAlignCell( Column.HAlign )

        .HAlignHeader( HAlign_Fill )

        .HeaderContentPadding( FMargin( 0.0f ) )

        .HeaderComboVisibility( EHeaderComboVisibility::Never );
    // clang-format on

    Args.HeaderContent()[ SNew( SSmartTableHeaderCell, &Table, Column.ColumnId ).Column( Column ).FallbackTextStyle( Table.GetHeaderTextStyle() ) ];

    switch ( Mode )
    {
        case ColumnLayout::EHeaderWidthMode::Manual:
            Args.ManualWidth( ManualWidth );
            Args.OnWidthChanged( OnWidthChanged );
            break;

        case ColumnLayout::EHeaderWidthMode::Fixed:
            Args.FixedWidth( Column.Width );
            break;

        case ColumnLayout::EHeaderWidthMode::Fill:
            Args.FillWidth( Column.Width );
            break;
    }

    return Args;
}

TSharedRef< SWidget > SmartTable::HeaderColumns::RowNumberCell( const TSharedRef< SSmartTableRow > & Row, const FTextBlockStyle & Style )
{

    TWeakPtr< SSmartTableRow > WeakRow = Row;

    // clang-format off
    return SNew( SBox )
        .Padding( SmartTable::Metrics::ChromeCellPadding() )
        .HAlign( HAlign_Right )
        .VAlign( VAlign_Center )
        [
            SNew( STextBlock )
                .TextStyle( &Style )
                .Text_Lambda( [ WeakRow, LastIndex = int32( INDEX_NONE ), LastText = FText::GetEmpty() ]() mutable
                    {
                        const TSharedPtr< SSmartTableRow > Pinned = WeakRow.Pin();
                        if ( !Pinned.IsValid() )
                        {
                            return FText::GetEmpty();
                        }

                        const int32 Index = Pinned->GetIndexInList();
                        if ( Index != LastIndex )
                        {
                            LastIndex = Index;
                            LastText  = FText::AsNumber( Index + 1 );
                        }

                        return LastText;
                    } )
        ];
    // clang-format on
}

#undef LOCTEXT_NAMESPACE
