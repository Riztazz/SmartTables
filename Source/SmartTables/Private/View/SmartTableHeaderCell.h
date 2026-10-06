// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#pragma once

#include "Input/DragAndDrop.h"
#include "SmartTableTypes.h"
#include "Styling/SlateBrush.h"
#include "Styling/SlateTypes.h"
#include "Widgets/SCompoundWidget.h"

class USmartTable;

class FSmartTableColumnDragDropOp : public FDragDropOperation
{
public:
    DRAG_DROP_OPERATOR_TYPE( FSmartTableColumnDragDropOp, FDragDropOperation )

    static TSharedRef< FSmartTableColumnDragDropOp > New( FName InColumnId, const FText & InLabel, const FSlateBrush & InBackground, const FTextBlockStyle & InLabelStyle );

    virtual TSharedPtr< SWidget > GetDefaultDecorator() const override;

    FName ColumnId;
    FText Label;

    FSlateBrush Background;
    FTextBlockStyle LabelStyle;
};

class SSmartTableHeaderCell : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS( SSmartTableHeaderCell )
    {
    }

    SLATE_ARGUMENT( FSmartTableColumn, Column )

    SLATE_ARGUMENT( FTextBlockStyle, FallbackTextStyle )

    SLATE_DEFAULT_SLOT( FArguments, Content )

    SLATE_END_ARGS()

    void Construct( const FArguments & InArgs, USmartTable * InTable, FName InColumnId );

    virtual FReply OnPreviewMouseButtonDown( const FGeometry & Geometry, const FPointerEvent & MouseEvent ) override;
    virtual FReply OnMouseButtonDown( const FGeometry & Geometry, const FPointerEvent & MouseEvent ) override;
    virtual FReply OnMouseButtonUp( const FGeometry & Geometry, const FPointerEvent & MouseEvent ) override;
    virtual FReply OnDragDetected( const FGeometry & Geometry, const FPointerEvent & MouseEvent ) override;
    virtual FCursorReply OnCursorQuery( const FGeometry & Geometry, const FPointerEvent & CursorEvent ) const override;

    virtual void OnDragLeave( const FDragDropEvent & DragDropEvent ) override;
    virtual FReply OnDragOver( const FGeometry & Geometry, const FDragDropEvent & DragDropEvent ) override;
    virtual FReply OnDrop( const FGeometry & Geometry, const FDragDropEvent & DragDropEvent ) override;

private:
    bool IsOverResizeEdge( const FGeometry & Geometry, const FPointerEvent & MouseEvent ) const;

    EVisibility GetSecondarySortVisibility() const;

    FLinearColor GetContentTint() const;

    EVisibility GetInsertBeforeVisibility() const;
    EVisibility GetInsertAfterVisibility() const;

    EVisibility GetCaretVisibility() const;

    const FSlateBrush * GetSortBrush() const;
    FSlateColor GetSortTint() const;
    EVisibility GetSortVisibility() const;
    bool DrawsSortArrow() const;

    FSmartTableColumn Column;

    FTextBlockStyle LabelStyle;

    TWeakObjectPtr< USmartTable > Table;

    FName ColumnId;

    bool bDragOver = false;

    bool bDropBefore = true;

    bool bPressWasOnResizeEdge = false;

    TOptional< FVector2D > RightPressScreenPosition;
};
