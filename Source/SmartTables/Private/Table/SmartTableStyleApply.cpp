// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "Logging/StructuredLog.h"
#include "SmartTable.h"
#include "SmartTableLog.h"
#include "SmartTableStyle.h"
#include "Style/SmartTableStyleMirror.h"
#include "Styling/SlateBrush.h"
#include "View/SmartTableListView.h"
#include "View/SmartTableRow.h"
#include "Widgets/Text/STextBlock.h"

void USmartTable::CopyStyleClassDefaults( const USmartTableStyle & Defaults )
{
    SmartTable::StyleMirror::StyleToTable( Defaults, *this );
    MirrorLastColumnStyle();
}

void USmartTable::MirrorLastColumnStyle()
{

    HeaderStyle.LastColumnStyle = HeaderStyle.ColumnStyle;
}

const FSlateBrush * USmartTable::GetSortBrush( FName ColumnId ) const
{
    const FSmartTableSortSpec & Spec = GetSortSpecRef();
    const ESmartTableSortMode Mode   = Spec.GetModeFor( ColumnId );
    if ( Mode == ESmartTableSortMode::None )
    {
        return nullptr;
    }

    const FTableColumnHeaderStyle & ColumnStyle = HeaderStyle.ColumnStyle;

    return Mode == ESmartTableSortMode::Ascending ? &ColumnStyle.SortPrimaryAscendingImage : &ColumnStyle.SortPrimaryDescendingImage;
}

FSlateColor USmartTable::GetSortTint() const
{
    return HeaderTextStyle.ColorAndOpacity;
}

const FSlateBrush * USmartTable::GetCellDragBrush( ESmartTableCellDragRole Role ) const
{
    if ( !bMarkDragCells )
    {
        return nullptr;
    }

    switch ( Role )
    {
        case ESmartTableCellDragRole::Source:
            return &DragSourceBrush;

        case ESmartTableCellDragRole::Target:
            return &DragTargetBrush;

        default:
            return nullptr;
    }
}

void USmartTable::ApplyStyleAsset()
{
    if ( !AuthoredStyle )
    {

        AuthoredStyle = NewObject< USmartTableStyle >( this, NAME_None, RF_Transient );

        SmartTable::StyleMirror::TableToStyle( *this, *AuthoredStyle );
    }

    SmartTable::StyleMirror::StyleToTable( StyleAsset ? *StyleAsset : *AuthoredStyle, *this );

    if ( !StyleAsset )
    {

        AuthoredStyle = nullptr;
    }

    MirrorLastColumnStyle();

    MenuHost.DropStyle();
}

void USmartTable::SetStyleAsset( USmartTableStyle * InStyleAsset )
{
    StyleAsset        = InStyleAsset;
    AppliedStyleAsset = InStyleAsset;

    UE_LOGFMT( LogSmartTables, Verbose, "Table '{Table}' takes the style {Style} from here on (None means back to the authored look).", GetName(), GetNameSafe( InStyleAsset ) );

    ApplyStyleAsset();
    RefreshAppliedStyle();
}

#if WITH_EDITOR
void USmartTable::ReapplyAfterPaletteChange( const TSet< const UObject * > & Refreshed )
{

    if ( !Refreshed.Contains( this ) && !( StyleAsset && Refreshed.Contains( StyleAsset ) ) )
    {
        return;
    }

    if ( !ListView.IsValid() )
    {
        return;
    }

    ApplyStyleAsset();
    RefreshAppliedStyle();
}
#endif

void USmartTable::RefreshAppliedStyle()
{

    MeasureRowNumberWidth();

    BackgroundStyle.SetBackgroundBrush( BackgroundBrush );
    if ( ListView.IsValid() )
    {
        ListView->SetStyle( &BackgroundStyle );
    }

    RebuildHeader();

    ForEachGeneratedRow( []( SSmartTableRow & Row )
    {
        Row.RefreshCells();
    } );

    if ( EmptyStateText.IsValid() )
    {
        EmptyStateText->SetTextStyle( &EmptyTextStyle,  true );
    }

    RefreshRowsForNewColumnWidths();
}
