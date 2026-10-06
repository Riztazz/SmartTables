// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#pragma once

#include "Layout/Margin.h"
#include "SmartTableRowSetDiff.h"
#include "Styling/SlateTypes.h"
#include "Templates/SubclassOf.h"
#include "UObject/ObjectMacros.h"
#include "SmartTableTypes.generated.h"

class USmartTableCell;

UENUM( BlueprintType )
enum class ESmartTableSortMode : uint8
{
    None,
    Ascending,
    Descending
};

UENUM( BlueprintType )
enum class ESmartTableColumnSizing : uint8
{

    Fill,

    Fixed
};

UENUM( BlueprintType )
enum class ESmartTableColumnSource : uint8
{

    None,

    ItemClass,

    DataTable,

    File
};

UENUM( BlueprintType )
enum class ESmartTableCellEditor : uint8
{
    None,
    Text,
    Number,
    Toggle
};

UENUM( BlueprintType )
enum class ESmartTableAssignReason : uint8
{

    Scrolled,

    Refreshed,

    Added,

    ValueChanged
};

UENUM( BlueprintType )
enum class ESmartTableSelectionMode : uint8
{

    None,

    Single,

    Multi
};

UENUM( BlueprintType )
enum class ESmartTableHeight : uint8
{

    Fill,

    FitRows
};

enum class ESmartTablePresentationScope : uint8
{

    Nothing,

    Cell,

    Row,

    Screen
};

struct SMARTTABLES_API FSmartTablePresentationChange
{

    bool bCellsStale = false;

    bool bOrderMoved = false;

    bool bRowSetMoved = false;

    FSmartTableRowSetDiff RowSetDiff;

    int32 NaturalRow = INDEX_NONE;

    FName ColumnId;

    bool IsAnything() const
    {
        return bCellsStale || bOrderMoved || bRowSetMoved;
    }

    ESmartTablePresentationScope Scope() const
    {
        if ( !IsAnything() )
        {
            return ESmartTablePresentationScope::Nothing;
        }

        if ( bOrderMoved || bRowSetMoved || NaturalRow == INDEX_NONE )
        {
            return ESmartTablePresentationScope::Screen;
        }

        return ColumnId.IsNone() ? ESmartTablePresentationScope::Row : ESmartTablePresentationScope::Cell;
    }

    void Add( const FSmartTablePresentationChange & Other )
    {
        if ( !Other.IsAnything() )
        {

            return;
        }

        const bool bWasAboutOneRow = !IsAnything() || ( NaturalRow != INDEX_NONE && Other.NaturalRow == NaturalRow );

        const bool bWasAboutOneCell = bWasAboutOneRow && ( !IsAnything() || ColumnId == Other.ColumnId );

        bCellsStale  = bCellsStale || Other.bCellsStale;
        bOrderMoved  = bOrderMoved || Other.bOrderMoved;
        bRowSetMoved = bRowSetMoved || Other.bRowSetMoved;

        RowSetDiff.Add( Other.RowSetDiff );

        NaturalRow = bWasAboutOneRow ? Other.NaturalRow : INDEX_NONE;
        ColumnId   = bWasAboutOneCell ? Other.ColumnId : NAME_None;
    }

    static FSmartTablePresentationChange OneRowsValues( int32 InNaturalRow )
    {
        FSmartTablePresentationChange Change;
        Change.bCellsStale = true;
        Change.NaturalRow  = InNaturalRow;

        return Change;
    }

    static FSmartTablePresentationChange OneCellsValue( int32 InNaturalRow, FName InColumnId )
    {
        FSmartTablePresentationChange Change;
        Change.bCellsStale = true;
        Change.NaturalRow  = InNaturalRow;
        Change.ColumnId    = InColumnId;

        return Change;
    }

    static FSmartTablePresentationChange EveryRowsValues()
    {
        FSmartTablePresentationChange Change;
        Change.bCellsStale = true;
        Change.bOrderMoved = true;

        return Change;
    }

    static FSmartTablePresentationChange Order()
    {
        FSmartTablePresentationChange Change;
        Change.bOrderMoved = true;

        return Change;
    }

    static FSmartTablePresentationChange RowSet( const FSmartTableRowSetDiff & Diff = FSmartTableRowSetDiff() )
    {
        FSmartTablePresentationChange Change;
        Change.bOrderMoved  = true;
        Change.bRowSetMoved = true;
        Change.RowSetDiff   = Diff;

        return Change;
    }

    static FSmartTablePresentationChange Everything()
    {
        FSmartTablePresentationChange Change = RowSet( FSmartTableRowSetDiff::Replaced() );
        Change.bCellsStale                   = true;

        return Change;
    }
};

USTRUCT( BlueprintType )
struct SMARTTABLES_API FSmartTableSortColumn
{
    GENERATED_BODY()

    UPROPERTY( EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Smart Tables" )
    FName ColumnId;

    UPROPERTY( EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Smart Tables" )
    ESmartTableSortMode Mode = ESmartTableSortMode::Ascending;

    bool operator==( const FSmartTableSortColumn & Other ) const
    {
        return ColumnId == Other.ColumnId && Mode == Other.Mode;
    }
};

USTRUCT( BlueprintType )
struct SMARTTABLES_API FSmartTableSortSpec
{
    GENERATED_BODY()

    UPROPERTY( EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Smart Tables" )
    TArray< FSmartTableSortColumn > Columns;

    bool IsEmpty() const
    {
        return Columns.IsEmpty();
    }

    ESmartTableSortMode GetModeFor( FName ColumnId ) const;

    int32 GetPriorityFor( FName ColumnId ) const;

    bool operator==( const FSmartTableSortSpec & Other ) const
    {
        return Columns == Other.Columns;
    }
};

UENUM( BlueprintType )
enum class ESmartTableCellDragRole : uint8
{

    None,

    Source,

    Target
};

USTRUCT( BlueprintType )
struct SMARTTABLES_API FSmartTableColumn
{
    GENERATED_BODY()

    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Smart Tables" )
    FName ColumnId;

    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Smart Tables", meta = ( ToolTip = "" ) )
    FText Header;

    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Smart Tables", meta = ( ToolTip = "", GetOptions = "GetBindableNames" ) )
    FName BindingName;

    UPROPERTY( EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Smart Tables", meta = ( ToolTip = "" ) )
    FName SortBindingName;

    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Smart Tables" )
    ESmartTableColumnSizing Sizing = ESmartTableColumnSizing::Fill;

    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Smart Tables", meta = ( ClampMin = "0.0" ) )
    float Width = 1.0f;

    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Smart Tables", meta = ( ToolTip = "" ) )
    TEnumAsByte< EHorizontalAlignment > HAlign = HAlign_Left;

    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Smart Tables" )
    FMargin CellPadding = FMargin( 8.0f, 2.0f );

    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Smart Tables", meta = ( ToolTip = "", ClampMin = "0", ClampMax = "8" ) )
    int32 MaxFractionalDigits = 2;

    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Smart Tables" )
    bool bSortable = true;

    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Smart Tables" )
    bool bResizable = true;

    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Smart Tables" )
    bool bHideable = true;

    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Smart Tables" )
    bool bHiddenByDefault = false;

    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Smart Tables" )
    TSubclassOf< USmartTableCell > CellClass;

    UPROPERTY( EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Smart Tables", meta = ( ToolTip = "" ) )
    TSubclassOf< USmartTableCell > DragVisualClass;

    UPROPERTY( EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Smart Tables|Header", meta = ( ToolTip = "" ) )
    bool bShowHeaderIcon = false;

    UPROPERTY( EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Smart Tables|Header", meta = ( ToolTip = "", EditCondition = "bShowHeaderIcon" ) )
    FSlateBrush HeaderIcon;

    UPROPERTY( EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Smart Tables|Header", meta = ( EditCondition = "bShowHeaderIcon", ClampMin = "0.0" ) )
    float HeaderIconSpacing = 6.0f;

    UPROPERTY( EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Smart Tables|Header", meta = ( ToolTip = "" ) )
    bool bOverrideHeaderTextStyle = false;

    UPROPERTY( EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Smart Tables|Header", meta = ( EditCondition = "bOverrideHeaderTextStyle" ) )
    FTextBlockStyle HeaderTextStyle;

    UPROPERTY( EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Smart Tables|Header" )
    TEnumAsByte< EHorizontalAlignment > HeaderHAlign = HAlign_Left;

    FText GetLabel() const;

    FName GetSortBinding() const;

    FName GetValueBinding() const;
};

USTRUCT( BlueprintType )
struct SMARTTABLES_API FSmartTableHeaderMenuOptions
{
    GENERATED_BODY()

    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Smart Tables", meta = ( ToolTip = "" ) )
    bool bEnabled = true;

    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Smart Tables", meta = ( ToolTip = "" ) )
    bool bOfferSort = true;

    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Smart Tables" )
    bool bOfferSizeToContent = true;

    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Smart Tables", meta = ( ToolTip = "" ) )
    bool bOfferMove = true;

    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Smart Tables" )
    bool bOfferVisibility = true;

    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Smart Tables" )
    bool bOfferResetLayout = true;
};

USTRUCT( BlueprintType )
struct SMARTTABLES_API FSmartTableMenuEntry
{
    GENERATED_BODY()

    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Smart Tables", meta = ( ToolTip = "" ) )
    FName Id;

    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Smart Tables" )
    FText Label;

    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Smart Tables" )
    FText ToolTip;

    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Smart Tables", meta = ( ToolTip = "" ) )
    bool bEnabled = true;

    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Smart Tables" )
    bool bVisible = true;

    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Smart Tables" )
    bool bSeparatorAbove = false;
};

USTRUCT( BlueprintType )
struct SMARTTABLES_API FSmartTableRowMenuOptions
{
    GENERATED_BODY()

    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Smart Tables", meta = ( ToolTip = "" ) )
    bool bEnabled = false;

    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Smart Tables" )
    FText Heading;

    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Smart Tables", meta = ( ToolTip = "" ) )
    bool bSelectRowFirst = false;
};

USTRUCT( BlueprintType )
struct SMARTTABLES_API FSmartTableColumnLayout
{
    GENERATED_BODY()

    UPROPERTY( BlueprintReadWrite, SaveGame, Category = "Smart Tables" )
    FName ColumnId;

    UPROPERTY( BlueprintReadWrite, SaveGame, Category = "Smart Tables" )
    float Width = 0.0f;

    UPROPERTY( BlueprintReadWrite, SaveGame, Category = "Smart Tables" )
    bool bUserWidth = false;

    UPROPERTY( BlueprintReadWrite, SaveGame, Category = "Smart Tables" )
    bool bHidden = false;
};

USTRUCT( BlueprintType )
struct SMARTTABLES_API FSmartTableLayout
{
    GENERATED_BODY()

    static constexpr int32 CurrentVersion = 2;

    UPROPERTY( BlueprintReadWrite, SaveGame, Category = "Smart Tables" )
    int32 Version = 0;

    UPROPERTY( BlueprintReadWrite, SaveGame, Category = "Smart Tables" )
    TArray< FName > Order;

    UPROPERTY( BlueprintReadWrite, SaveGame, Category = "Smart Tables" )
    TArray< FSmartTableColumnLayout > Columns;

    UPROPERTY( BlueprintReadWrite, SaveGame, Category = "Smart Tables" )
    FSmartTableSortSpec SortSpec;
};
