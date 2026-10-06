// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#pragma once

#include "Engine/DataAsset.h"
#include "Styling/SlateTypes.h"
#include "Widgets/Views/SHeaderRow.h"
#include "SmartTableStyle.generated.h"

USTRUCT( BlueprintType )
struct SMARTTABLES_API FSmartTableSearchHighlight
{
    GENERATED_BODY()

    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Smart Tables|Menu", meta = ( ToolTip = "" ) )
    bool bFill = true;

    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Smart Tables|Menu", meta = ( ToolTip = "" ) )
    bool bOutline = true;
};

USTRUCT( BlueprintType )
struct SMARTTABLES_API FSmartTableMenuStyle
{
    GENERATED_BODY()

    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Smart Tables|Menu" )
    FSlateBrush BackgroundBrush;

    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Smart Tables|Menu" )
    FButtonStyle EntryStyle;

    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Smart Tables|Menu" )
    FTextBlockStyle LabelTextStyle;

    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Smart Tables|Menu" )
    FSmartTableSearchHighlight SearchHighlight;

    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Smart Tables|Menu" )
    FTextBlockStyle HeadingTextStyle;

    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Smart Tables|Menu" )
    FSlateBrush SeparatorBrush;

    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Smart Tables|Menu" )
    FCheckBoxStyle CheckStyle;
};

UCLASS( BlueprintType, meta = ( ToolTip = "" ) )
class SMARTTABLES_API USmartTableStyle : public UDataAsset
{
    GENERATED_BODY()

public:
    USmartTableStyle();

    void BuildDefaultsFromSettings();

    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Smart Tables|Surfaces" )
    FSlateBrush BackgroundBrush;

    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Smart Tables|Surfaces" )
    FTableRowStyle RowStyle;

    UPROPERTY( EditAnywhere, Category = "Smart Tables|Surfaces", meta = ( ToolTip = "" ) )
    FHeaderRowStyle HeaderStyle;

    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Smart Tables|Drag and Drop", meta = ( ToolTip = "" ) )
    FSlateBrush DragSourceBrush;

    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Smart Tables|Drag and Drop" )
    FSlateBrush DragTargetBrush;

    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Smart Tables|Surfaces", meta = ( ToolTip = "" ) )
    FScrollBarStyle ScrollBarStyle;

    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Smart Tables|Text" )
    FTextBlockStyle HeaderTextStyle;

    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Smart Tables|Text" )
    FTextBlockStyle CellTextStyle;

    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Smart Tables|Text", meta = ( ToolTip = "" ) )
    FTextBlockStyle RowNumberTextStyle;

    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Smart Tables|Text" )
    FTextBlockStyle EmptyTextStyle;

    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Smart Tables|Details" )
    FSlateBrush InsertMarkerBrush;

    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Smart Tables|Details", meta = ( ToolTip = "" ) )
    FSlateBrush RowInsertMarkerBrush;

    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Smart Tables|Details" )
    FSlateBrush ColumnDividerBrush;

    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Smart Tables|Details", meta = ( ToolTip = "" ) )
    FSlateBrush ColumnCaretBrush;

    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Smart Tables|Details" )
    FSlateBrush FocusBorderBrush;

    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Smart Tables|Details" )
    FSlateBrush BusyOverlayBrush;

    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Smart Tables|Menu", meta = ( ToolTip = "" ) )
    FSmartTableMenuStyle MenuStyle;

    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Smart Tables|Metrics", meta = ( ToolTip = "", ClampMin = "0.0" ) )
    float HeaderHeight = 26.0f;
};
