// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#pragma once

#include "SmartTableTypes.h"
#include "View/SmartTableRowIndex.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STableRow.h"

class USmartTable;
class USmartTableCell;
class USmartTableModel;
class USmartTableRowWidget;

class SSmartTableRow : public SMultiColumnTableRow< int32 >
{
public:
    SLATE_BEGIN_ARGS( SSmartTableRow )
    {
    }
    SLATE_END_ARGS()

    void Construct( const FArguments & InArgs, const TSharedRef< STableViewBase > & OwnerTable, USmartTable * InTable, int32 InNaturalRow );

    int32 GetNaturalRow() const
    {
        return NaturalRow;
    }

    virtual TSharedRef< SWidget > GenerateWidgetForColumn( const FName & ColumnId ) override;

    virtual FReply OnPreviewMouseButtonDown( const FGeometry & Geometry, const FPointerEvent & MouseEvent ) override;

    virtual FReply OnMouseButtonUp( const FGeometry & Geometry, const FPointerEvent & MouseEvent ) override;

    void RefreshCells( ESmartTableAssignReason Reason = ESmartTableAssignReason::Refreshed );

    void RefreshCell( FName ColumnId, ESmartTableAssignReason Reason = ESmartTableAssignReason::Refreshed );

    void ReleaseCells();

    FName GetDrawnRowId() const
    {
        return DrawnRowId;
    }

    void ReassignRow( FName NewRowId, ESmartTableAssignReason Reason );

    USmartTableRowWidget * GetRowWidget() const
    {
        return RowWidget.Get();
    }

    void NotifyCellDragRoles();

    virtual int32 OnPaintDropIndicator( EItemDropZone InItemDropZone, const FPaintArgs & Args, const FGeometry & AllottedGeometry, const FSlateRect & MyCullingRect, FSlateWindowElementList & OutDrawElements, int32 LayerId, const FWidgetStyle & InWidgetStyle, bool bParentEnabled ) const override;

    void ApplyRowColor();

    virtual ~SSmartTableRow() override;

protected:

    virtual void ConstructChildren( ETableViewMode::Type InOwnerTableMode, const TAttribute< FMargin > & InPadding, const TSharedRef< SWidget > & InContent ) override;

private:

    FReply HandleDragDetected( const FGeometry & Geometry, const FPointerEvent & MouseEvent );

    TOptional< EItemDropZone > HandleCanAcceptDrop( const FDragDropEvent & DragDropEvent, EItemDropZone Zone, int32 Row );

    FReply HandleAcceptDrop( const FDragDropEvent & DragDropEvent, EItemDropZone Zone, int32 Row );

    int32 GapForZone( EItemDropZone Zone );

    void MarkCellDragRole( USmartTableCell & Cell ) const;

    TWeakObjectPtr< USmartTable > Table;

    int32 NaturalRow = INDEX_NONE;

    TArray< TWeakObjectPtr< USmartTableCell > > Cells;

    void ReleaseRowWidget();

    TWeakObjectPtr< USmartTableRowWidget > RowWidget;

    FName DrawnRowId;
};
