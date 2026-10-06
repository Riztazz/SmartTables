// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#pragma once

#include "Blueprint/UserWidgetPool.h"
#include "Components/Widget.h"
#include "Containers/ArrayView.h"
#include "SmartTableArrivals.h"
#include "SmartTableCellDrag.h"
#include "SmartTableColumnLayout.h"
#include "SmartTableColumnSelection.h"
#include "SmartTableDeadSecondaries.h"
#include "SmartTableKeptRow.h"
#include "SmartTableOpenMenu.h"
#include "SmartTableResizeGesture.h"
#include "SmartTableScrollMove.h"
#include "SmartTableStyle.h"
#include "SmartTableTypes.h"
#include "Styling/SlateTypes.h"
#include "Templates/Function.h"
#include "Types/SlateEnums.h"
#include "Widgets/Views/SHeaderRow.h"
#include "SmartTable.generated.h"

class FWidgetPath;
class ITableRow;
class SHeaderRow;
class STableViewBase;
class UDataTable;
class USmartTableCell;
class USmartTableCellDragDropOp;
class USmartTableDataTableModel;
class USmartTableModel;
class USmartTableObjectModel;
class USmartTablePreviewModel;
class USmartTableRowWidget;
class SSmartTableRow;
template< typename ItemType >
class SListView;

class FMenuBuilder;
class USmartTableLayoutStore;

namespace SmartTable
{
    class FColumnView;
}

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnSmartTableSelectionChanged, const TArray< int32 > &, SelectedRows );

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams( FOnSmartTableRowActivated, UObject *, Item, int32, NaturalRow );
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnSmartTableSortChanged, const FSmartTableSortSpec &, SortSpec );
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnSmartTableLayoutChanged, const FSmartTableLayout &, Layout );

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnSmartTableColumnSelectionChanged, FName, ColumnId );

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams( FOnSmartTableCellValueChanged, UObject *, Item, int32, NaturalRow, FName, ColumnId, float, Value );

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams( FOnSmartTableCellDropped, USmartTableCellDragDropOp *, Payload, int32, TargetNaturalRow, FName, TargetColumnId );

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams( FOnSmartTableRowsDropped, const TArray< int32 > &, MovedNaturalRows, int32, InsertBeforeNaturalRow );

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams( FOnSmartTableRowMenuOpening, UObject *, Item, int32, NaturalRow );

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams( FOnSmartTableRowMenuEntry, FName, EntryId, int32, EntryIndex, UObject *, Item, int32, NaturalRow );

DECLARE_DELEGATE_TwoParams( FOnSmartTableExtendHeaderMenu, FName, FMenuBuilder & );

DECLARE_DELEGATE_TwoParams( FOnSmartTableExtendRowMenu, const FSmartTableKeptRow &, FMenuBuilder & );

UCLASS( meta = ( DisplayName = "Smart Table", ToolTip = "" ) )
class SMARTTABLES_API USmartTable : public UWidget
{
    GENERATED_BODY()

public:
    USmartTable( const FObjectInitializer & ObjectInitializer );

    struct SMARTTABLES_API FCellWalkScope
    {
        explicit FCellWalkScope( USmartTable * InTable );
        ~FCellWalkScope();

        FCellWalkScope( const FCellWalkScope & )             = delete;
        FCellWalkScope & operator=( const FCellWalkScope & ) = delete;

    private:

        TWeakObjectPtr< USmartTable > Table;

        bool bPrevious = false;
    };

    UFUNCTION( BlueprintCallable, Category = "Smart Table" )
    void SetItems( const TArray< UObject * > & InItems );

    UFUNCTION( BlueprintCallable, Category = "Smart Table" )
    void AddItem( UObject * Item );

    UFUNCTION( BlueprintCallable, Category = "Smart Table" )
    void RemoveItem( UObject * Item );

    UFUNCTION( BlueprintCallable, Category = "Smart Table" )
    void ClearItems();

    UFUNCTION( BlueprintCallable, Category = "Smart Table" )
    void SetDataTable( UDataTable * DataTable );

    UFUNCTION( BlueprintCallable, Category = "Smart Table" )
    void NotifyItemChanged( UObject * Item );

    UFUNCTION( BlueprintPure, Category = "Smart Table" )
    bool IsScrolledToEnd() const
    {
        return Scroll.bViewAtEnd;
    }

    UFUNCTION( BlueprintCallable, Category = "Smart Table" )
    void RefreshItems();

    TConstArrayView< TObjectPtr< UObject > > GetItems() const;

    UFUNCTION( BlueprintPure, Category = "Smart Table" )
    UObject * GetSelectedItem() const;

    UFUNCTION( BlueprintPure, Category = "Smart Table" )
    TArray< UObject * > GetSelectedItems() const;

    UFUNCTION( BlueprintPure, Category = "Smart Table" )
    TArray< int32 > GetSelectedRows() const;

    UFUNCTION( BlueprintPure, Category = "Smart Table" )
    bool IsRowSelected( int32 NaturalRow ) const;

    UFUNCTION( BlueprintCallable, Category = "Smart Table" )
    void SetRowSelected( int32 NaturalRow, bool bSelected = true );

    UFUNCTION( BlueprintCallable, Category = "Smart Table" )
    void SetRowsSelected( const TArray< int32 > & NaturalRows, bool bSelected = true );

    UFUNCTION( BlueprintCallable, Category = "Smart Table" )
    void SetSelectedItems( const TArray< UObject * > & Items, bool bSelected = true );

    UFUNCTION( BlueprintCallable, Category = "Smart Table" )
    void SetSelectedItem( UObject * Item, bool bSelected = true );

    UFUNCTION( BlueprintCallable, Category = "Smart Table" )
    void ClearSelection();

    UFUNCTION( BlueprintCallable, Category = "Smart Table" )
    void ScrollItemIntoView( UObject * Item );

    UFUNCTION( BlueprintCallable, Category = "Smart Table" )
    void ScrollRowIntoView( int32 NaturalRow );

    UFUNCTION( BlueprintCallable, Category = "Smart Table" )
    void ScrollToEnd();

    UFUNCTION( BlueprintCallable, Category = "Smart Table|Input" )
    void FocusTable();

    UFUNCTION( BlueprintPure, Category = "Smart Table|Input" )
    int32 GetFocusedRow() const
    {
        return FocusedRow;
    }

    UFUNCTION( BlueprintCallable, Category = "Smart Table|Input" )
    void MoveSelection( int32 Delta );

    UFUNCTION( BlueprintCallable, Category = "Smart Table|Input" )
    void MoveSelectionByPage( int32 PageDelta );

    UFUNCTION( BlueprintCallable, Category = "Smart Table|Input" )
    void SelectFirstRow();

    UFUNCTION( BlueprintCallable, Category = "Smart Table|Input" )
    void SelectLastRow();

    UFUNCTION( BlueprintCallable, Category = "Smart Table|Input" )
    void ActivateSelectedRow();

    UFUNCTION( BlueprintCallable, Category = "Smart Table|Input" )
    void CycleColumnSort( FName ColumnId, bool bAsSecondary = false );

    UFUNCTION( BlueprintCallable, Category = "Smart Table|Input" )
    void OpenHeaderMenu( FName ColumnId );

    UFUNCTION( BlueprintCallable, Category = "Smart Table|Input" )
    void CloseMenu();

    int32 GetMenuDrivingUser() const
    {
        return MenuHost.GetDrivingUser();
    }

    UFUNCTION( BlueprintCallable, Category = "Smart Table|Input" )
    void OpenTableMenu();

    UFUNCTION( BlueprintCallable, Category = "Smart Table|Input" )
    void BeginColumnResize( FName ColumnId );

    bool EndPointerResize( int32 UserIndex, uint32 PointerIndex );

    UFUNCTION( BlueprintCallable, Category = "Smart Table|Columns" )
    void SelectColumn( FName ColumnId );

    UFUNCTION( BlueprintCallable, Category = "Smart Table|Columns" )
    void MoveColumnSelection( int32 Delta );

    UFUNCTION( BlueprintPure, Category = "Smart Table|Columns" )
    FName GetSelectedColumn() const
    {
        return ColumnSelection.Selected;
    }

    UPROPERTY( BlueprintAssignable, Category = "Smart Table|Columns" )
    FOnSmartTableColumnSelectionChanged OnColumnSelectionChanged;

    void SelectColumnAt( float LocalX );

    UFUNCTION( BlueprintCallable, Category = "Smart Table|Input" )
    void UpdateColumnResize( float DeltaPixels );

    UFUNCTION( BlueprintCallable, Category = "Smart Table|Input" )
    void EndColumnResize();

    UFUNCTION( BlueprintCallable, Category = "Smart Table|Columns" )
    void SetColumns( const TArray< FSmartTableColumn > & InColumns );

    UFUNCTION( BlueprintPure, Category = "Smart Table|Columns" )
    const TArray< FSmartTableColumn > & GetColumns() const;

    UFUNCTION( BlueprintCallable, Category = "Smart Table|Columns" )
    void SetColumnVisible( FName ColumnId, bool bVisible );

    UFUNCTION( BlueprintPure, Category = "Smart Table|Columns" )
    bool IsColumnVisible( FName ColumnId ) const;

    UFUNCTION( BlueprintCallable, Category = "Smart Table|Sorting" )
    void SortByColumn( FName ColumnId, ESmartTableSortMode SortMode );

    UFUNCTION( BlueprintCallable, Category = "Smart Table|Sorting" )
    void SortBySecondaryColumn( FName ColumnId, ESmartTableSortMode SortMode );

    UFUNCTION( BlueprintCallable, Category = "Smart Table|Sorting" )
    void SetSortSpec( const FSmartTableSortSpec & Spec );

    UFUNCTION( BlueprintPure, Category = "Smart Table|Sorting" )
    FSmartTableSortSpec GetSortSpec() const;

    const FSmartTableSortSpec & GetSortSpecRef() const;

    UFUNCTION( BlueprintCallable, Category = "Smart Table|Filtering" )
    void SetFilterText( const FText & InFilterText );

    UFUNCTION( BlueprintPure, Category = "Smart Table|Filtering" )
    FText GetFilterText() const;

    UFUNCTION( BlueprintCallable, Category = "Smart Table|Model" )
    void SetModel( USmartTableModel * InModel );

    UFUNCTION( BlueprintPure, Category = "Smart Table|Model" )
    USmartTableModel * GetModel() const
    {
        return Model;
    }

    UPROPERTY( BlueprintAssignable, Category = "Smart Table|Events" )
    FOnSmartTableSelectionChanged OnSelectionChanged;

    UPROPERTY( BlueprintAssignable, Category = "Smart Table|Events" )
    FOnSmartTableRowActivated OnRowActivated;

    UPROPERTY( BlueprintAssignable, Category = "Smart Table|Events" )
    FOnSmartTableSortChanged OnSortChanged;

    UPROPERTY( BlueprintAssignable, Category = "Smart Table|Events" )
    FOnSmartTableLayoutChanged OnLayoutChanged;

    UPROPERTY( BlueprintAssignable, Category = "Smart Table|Events" )
    FOnSmartTableCellValueChanged OnCellValueChanged;

    UPROPERTY( BlueprintAssignable, Category = "Smart Table|Drag and Drop" )
    FOnSmartTableCellDropped OnCellDropped;

    UPROPERTY( BlueprintAssignable, Category = "Smart Table|Drag and Drop" )
    FOnSmartTableRowsDropped OnRowsDropped;

    UFUNCTION( BlueprintCallable, Category = "Smart Table" )
    void SetRowHeight( float InRowHeight );

    UFUNCTION( BlueprintCallable, Category = "Smart Table|Row Numbers" )
    void SetShowRowNumbers( bool bInShowRowNumbers );

    UFUNCTION( BlueprintCallable, Category = "Smart Table" )
    void SetShowHeader( bool bInShowHeader );

    UFUNCTION( BlueprintCallable, Category = "Smart Table" )
    void SetInteractiveCells( bool bInInteractiveCells );

    UFUNCTION( BlueprintPure, Category = "Smart Table" )
    bool AreCellsInteractive() const
    {
        return bInteractiveCells;
    }

    UFUNCTION( BlueprintCallable, Category = "Smart Table|Drag and Drop" )
    void SetAllowCellDragDrop( bool bInAllowCellDragDrop );

    UFUNCTION( BlueprintPure, Category = "Smart Table|Drag and Drop" )
    static bool CanPointerStartDrag( const FPointerEvent & PointerEvent );

    UFUNCTION( BlueprintPure, Category = "Smart Table|Drag and Drop" )
    bool AllowsCellDragDrop() const
    {
        return bAllowCellDragDrop;
    }

    UFUNCTION( BlueprintPure, Category = "Smart Table|Drag and Drop" )
    ESmartTableCellDragRole GetCellDragRole( int32 NaturalRow, FName ColumnId ) const;

    UFUNCTION( BlueprintPure, Category = "Smart Table|Drag and Drop" )
    bool IsCellDragActive() const
    {
        return CellDrag.IsActive();
    }

    UFUNCTION( BlueprintCallable, Category = "Smart Table|Drag and Drop" )
    bool MoveRowsTo( const TArray< int32 > & NaturalRows, int32 InsertBeforeNaturalRow );

    UFUNCTION( BlueprintCallable, Category = "Smart Table|Drag and Drop" )
    bool MoveSelectedRows( int32 Delta );

    UFUNCTION( BlueprintPure, Category = "Smart Table|Drag and Drop" )
    bool AllowsRowReorder() const
    {
        return bAllowRowReorder;
    }

    UFUNCTION( BlueprintCallable, Category = "Smart Table|Drag and Drop" )
    void SetAllowRowReorder( bool bInAllowRowReorder );

    UFUNCTION( BlueprintPure, Category = "Smart Table|Drag and Drop" )
    int32 NaturalGapForPresentedGap( int32 PresentedGap ) const;

    const FSlateBrush & GetRowInsertMarkerBrush() const
    {
        return RowInsertMarkerBrush;
    }

    const FSlateBrush * GetCellDragBrush( ESmartTableCellDragRole Role ) const;

    ESmartTableCellDragRole GetCellDragRoleOfRowId( FName RowId, FName ColumnId ) const
    {
        return CellDrag.RoleOf( RowId, ColumnId );
    }

    void BeginCellDrag( int32 NaturalRow, FName ColumnId );
    void SetCellDragTarget( int32 NaturalRow, FName ColumnId );
    void ClearCellDragTarget( int32 NaturalRow, FName ColumnId );

    void EndCellDrag();

    UFUNCTION( BlueprintCallable, Category = "Smart Table" )
    void SetHeightMode( ESmartTableHeight InHeightMode, int32 InMaxVisibleRows = 0 );

    UFUNCTION( BlueprintCallable, Category = "Smart Table" )
    void SetSelectionMode( ESmartTableSelectionMode InSelectionMode );

    UFUNCTION( BlueprintPure, Category = "Smart Table" )
    ESmartTableSelectionMode GetSelectionMode() const
    {
        return SelectionMode;
    }

    UFUNCTION( BlueprintCallable, Category = "Smart Table|Columns" )
    void SetStretchLastColumn( bool bInStretchLastColumn );

    UFUNCTION( BlueprintCallable, Category = "Smart Table|Sorting" )
    void SetAllowDeadSecondarySort( bool bInAllowDeadSecondarySort );

    UFUNCTION( BlueprintCallable, Category = "Smart Table|Columns" )
    void SetAllowHorizontalScroll( bool bInAllowHorizontalScroll );

    UFUNCTION( BlueprintCallable, Category = "Smart Table|Input" )
    void SetAllowRightClickDragScroll( bool bInAllowRightClickDragScroll );

#if WITH_EDITOR

    static FName ColumnsPropertyName()
    {
        return GET_MEMBER_NAME_CHECKED( USmartTable, Columns );
    }

    static FName ColumnSourcePropertyName()
    {
        return GET_MEMBER_NAME_CHECKED( USmartTable, ColumnSource );
    }

    static FName ExpectedItemClassPropertyName()
    {
        return GET_MEMBER_NAME_CHECKED( USmartTable, ExpectedItemClass );
    }

    static FName SourceTablePropertyName()
    {
        return GET_MEMBER_NAME_CHECKED( USmartTable, SourceTable );
    }

    static FName SourceFilePropertyName()
    {
        return GET_MEMBER_NAME_CHECKED( USmartTable, SourceFile );
    }

    TArray< FSmartTableColumn > MergedColumnsFromSource() const;

    FText DescribeSourcesLostBySwitchingTo( ESmartTableColumnSource NewSource ) const;
#endif

    UFUNCTION( BlueprintCallable, Category = "Smart Table|Style" )
    void SetStyleAsset( USmartTableStyle * InStyleAsset );

    UFUNCTION( BlueprintPure, Category = "Smart Table|Style" )
    USmartTableStyle * GetStyleAsset() const
    {
        return StyleAsset;
    }

    UFUNCTION( BlueprintPure, Category = "Smart Table|Layout" )
    FSmartTableLayout GetLayout() const;

    UFUNCTION( BlueprintCallable, Category = "Smart Table|Layout" )
    void ApplyLayout( const FSmartTableLayout & Layout );

    UFUNCTION( BlueprintCallable, Category = "Smart Table|Layout" )
    void ResetLayout();

    UFUNCTION( BlueprintCallable, Category = "Smart Table|Layout" )
    void SetTableId( FName InTableId );

    UFUNCTION( BlueprintCallable, Category = "Smart Table|Layout" )
    void SetLayoutStore( USmartTableLayoutStore * InLayoutStore );

    UFUNCTION( BlueprintCallable, Category = "Smart Table|Layout" )
    void SetColumnWidth( FName ColumnId, float Width );

    UFUNCTION( BlueprintCallable, Category = "Smart Table|Layout" )
    void ResetColumnWidth( FName ColumnId );

    UFUNCTION( BlueprintPure, Category = "Smart Table|Layout" )
    float GetColumnWidth( FName ColumnId ) const;

    float GetResizeGripWidth() const
    {
        return ResizeGripWidth;
    }

    bool CanResizeColumn( FName ColumnId ) const;

    bool IsResizingColumn() const
    {
        return Resize.IsActive();
    }

    UFUNCTION( BlueprintCallable, Category = "Smart Table|Layout" )
    void SizeColumnToContent( FName ColumnId );

    UFUNCTION( BlueprintCallable, Category = "Smart Table|Layout" )
    void SetColumnOrder( const TArray< FName > & ColumnIds );

    UFUNCTION( BlueprintPure, Category = "Smart Table|Layout" )
    TArray< FName > GetColumnOrder() const;

    UFUNCTION( BlueprintCallable, Category = "Smart Table|Layout" )
    void MoveColumn( FName ColumnId, int32 Delta );

    UFUNCTION( BlueprintCallable, Category = "Smart Table|Layout" )
    void MoveColumnNextTo( FName Moved, FName Target, bool bAfter );

    UFUNCTION( BlueprintPure, Category = "Smart Table|Layout" )
    bool IsHeaderMenuEnabled() const
    {
        return HeaderMenuOptions.bEnabled;
    }

    UFUNCTION( BlueprintPure, Category = "Smart Table|Layout" )
    bool CanReorderColumns() const
    {
        return bAllowColumnReorder;
    }

    UFUNCTION( BlueprintPure, Category = "Smart Table|Columns" )
    FText GetColumnLabel( FName ColumnId ) const;

    UFUNCTION( BlueprintCallable, Category = "Smart Table|Busy" )
    void SetBusy( bool bInBusy );

    UFUNCTION( BlueprintPure, Category = "Smart Table|Busy" )
    bool IsBusy() const
    {
        return bBusy;
    }

    UFUNCTION( BlueprintCallable, Category = "Smart Table|Columns" )
    void ScrollColumnsBy( float DeltaPixels );

    UFUNCTION( BlueprintCallable, Category = "Smart Table|Rows" )
    void OpenRowMenu( int32 NaturalRow );

    void OpenRowMenuFromPointer( int32 NaturalRow, const FVector2D & AbsolutePosition, int32 DrivingUserIndex );

    UFUNCTION( BlueprintCallable, Category = "Smart Table|Rows" )
    void SetRowMenuEntries( const TArray< FSmartTableMenuEntry > & Entries );

    UFUNCTION( BlueprintPure, Category = "Smart Table|Rows" )
    bool IsRowMenuEnabled() const
    {
        return RowMenuOptions.bEnabled;
    }

    bool HasRightClickMenu() const
    {
        return ( bShowHeader && IsHeaderMenuEnabled() ) || IsRowMenuEnabled();
    }

    bool IsRightDragScrolling() const;

    bool AllowsHorizontalScroll() const
    {
        return bAllowHorizontalScroll;
    }

    UPROPERTY( BlueprintAssignable, Category = "Smart Table|Events" )
    FOnSmartTableRowMenuOpening OnRowMenuOpening;

    UPROPERTY( BlueprintAssignable, Category = "Smart Table|Events" )
    FOnSmartTableRowMenuEntry OnRowMenuEntryChosen;

    FOnSmartTableExtendHeaderMenu OnExtendHeaderMenu;

    FOnSmartTableExtendRowMenu OnExtendRowMenu;

    USmartTableCell * AcquireCell( FName ColumnId, int32 NaturalRow );

    void ReleaseCell( USmartTableCell * Cell );

    bool IsFirstItemSetterWarningFor( const UClass & CellClass );

    USmartTableRowWidget * AcquireRowWidget( int32 NaturalRow, const TSharedRef< SWidget > & ColumnsContent );

    void ReleaseRowWidget( USmartTableRowWidget * RowWidget );

    const FSmartTableColumn * FindColumn( FName ColumnId ) const;

    static const FName RowNumberColumnId;

    UFUNCTION( BlueprintCallable, Category = "Smart Table" )
    void NotifyCellValueChanged( UObject * Item, int32 NaturalRow, FName ColumnId, float Value );

    void NotifyCellDropped( USmartTableCellDragDropOp * Payload, int32 TargetNaturalRow, FName TargetColumnId );

    void CopyStyleClassDefaults( const USmartTableStyle & Defaults );

#if WITH_EDITOR

    void ReapplyAfterPaletteChange( const TSet< const UObject * > & Refreshed );
#endif

    TSharedRef< SWidget > MakeRowNumberWidget( TSharedRef< SSmartTableRow > Row ) const;

    void HandleHeaderGesture( FName ColumnId, bool bShiftDown );

    void MoveRowsFromDrop( const TArray< FSmartTableKeptRow > & Rows, int32 InsertBeforeNaturalRow );

    TSharedRef< SWidget > MakeHeaderMenu( FName ColumnId );

    void PushMenuOnPath( TSharedRef< SWidget > Anchor, const FWidgetPath & AnchorPath, TSharedRef< SWidget > Menu, FVector2D AbsolutePosition, int32 DrivingUserIndex );

    void PushMenuAtPoint( TSharedRef< SWidget > Anchor, TSharedRef< SWidget > Menu, FVector2D AbsolutePosition, int32 DrivingUserIndex );

    const FHeaderRowStyle & GetHeaderStyle() const
    {
        return HeaderStyle;
    }

    const FTextBlockStyle & GetHeaderTextStyle() const
    {
        return HeaderTextStyle;
    }

    const FSlateBrush & GetInsertMarkerBrush() const
    {
        return InsertMarkerBrush;
    }

    const FSlateBrush & GetColumnDividerBrush() const
    {
        return ColumnDividerBrush;
    }

    const FSlateBrush & GetColumnCaretBrush() const
    {
        return ColumnCaretBrush;
    }

    UFUNCTION( BlueprintPure, Category = "Smart Table|Input" )
    bool IsTableFocused() const;

    bool DoesColumnCaretNeedFocus() const
    {
        return bColumnCaretNeedsFocus;
    }

    bool IsColumnCaretShown() const
    {
        return bShowColumnCaret;
    }

    bool IsFocusOnPointerInteractionEnabled() const
    {
        return bFocusOnPointerInteraction;
    }

    const FSlateBrush * GetSortBrush( FName ColumnId ) const;

    FSlateColor GetSortTint() const;

    bool IsSecondarySortColumn( FName ColumnId ) const;

    UFUNCTION( BlueprintCallable, Category = "Smart Table|Sorting" )
    void RefreshDeadSecondaryColumns();

    UFUNCTION( BlueprintPure, Category = "Smart Table|Sorting" )
    bool IsDeadSecondaryColumn( FName ColumnId ) const;

    const FTableRowStyle & GetRowStyle() const
    {
        return RowStyle;
    }

    const FTextBlockStyle & GetCellTextStyle() const
    {
        return CellTextStyle;
    }

    const FTextBlockStyle & GetEmptyTextStyle() const
    {
        return EmptyTextStyle;
    }

    float GetHeaderHeight() const
    {
        return HeaderHeight;
    }

    float GetRowHeight() const
    {
        return RowHeight;
    }

    static FText MakeEmptyStateText( int32 NumRows, const FText & FilterText, const FText & InEmptyText, const FText & InNoMatchesFormat );

protected:
    virtual TSharedRef< SWidget > RebuildWidget() override;
    virtual void ReleaseSlateResources( bool bReleaseChildren ) override;
    virtual void SynchronizeProperties() override;

#if WITH_EDITOR
    virtual const FText GetPaletteCategory() override;
    virtual void PostEditChangeProperty( FPropertyChangedEvent & PropertyChangedEvent ) override;

    virtual void ValidateCompiledDefaults( class IWidgetCompilerLog & CompileLog ) const override;
#endif

    UFUNCTION()
    TArray< FName > GetBindableNames() const;

    const UStruct * ResolveColumnSchema( FText & OutSchemaName, FText & OutProblem ) const;

    void ChooseRowMenuEntry( FName EntryId, int32 EntryIndex, FSmartTableKeptRow Row );

    UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Smart Table", meta = ( ToolTip = "", TitleProperty = "ColumnId" ) )
    TArray< FSmartTableColumn > Columns;

    UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Smart Table", meta = ( ToolTip = "" ) )
    ESmartTableColumnSource ColumnSource = ESmartTableColumnSource::None;

    UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Smart Table", meta = ( ToolTip = "", EditCondition = "ColumnSource == ESmartTableColumnSource::ItemClass", EditConditionHides ) )
    TSubclassOf< UObject > ExpectedItemClass;

    UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Smart Table", meta = ( ToolTip = "", EditCondition = "ColumnSource == ESmartTableColumnSource::DataTable", EditConditionHides ) )
    TSoftObjectPtr< class UDataTable > SourceTable;

    UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Smart Table", meta = ( ToolTip = "", EditCondition = "ColumnSource == ESmartTableColumnSource::File", EditConditionHides, FilePathFilter = "Data files (*.csv; *.json)|*.csv;*.json", RelativeToGameDir ) )
    FFilePath SourceFile;

    UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Smart Table" )
    ESmartTableSelectionMode SelectionMode = ESmartTableSelectionMode::Single;

    UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Smart Table", meta = ( ToolTip = "" ) )
    bool bInteractiveCells = false;

    UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Smart Table|Drag and Drop", meta = ( ToolTip = "" ) )
    bool bAllowCellDragDrop = false;

    UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Smart Table|Drag and Drop", meta = ( ToolTip = "" ) )
    bool bAllowRowReorder = false;

    UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Smart Table|Drag and Drop", meta = ( ToolTip = "" ) )
    bool bMarkDragCells = true;

    UPROPERTY( EditAnywhere, AdvancedDisplay, Category = "Smart Table|Drag and Drop", meta = ( ToolTip = "", EditCondition = "StyleAsset == nullptr" ) )
    FSlateBrush DragSourceBrush;

    UPROPERTY( EditAnywhere, AdvancedDisplay, Category = "Smart Table|Drag and Drop", meta = ( ToolTip = "", EditCondition = "StyleAsset == nullptr" ) )
    FSlateBrush DragTargetBrush;

    UPROPERTY( EditAnywhere, AdvancedDisplay, Category = "Smart Table|Drag and Drop", meta = ( EditCondition = "StyleAsset == nullptr" ) )
    FSlateBrush RowInsertMarkerBrush;

    UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Smart Table", meta = ( ToolTip = "", ClampMin = "0.0" ) )
    float RowHeight = 24.0f;

    UPROPERTY( EditAnywhere, AdvancedDisplay, Category = "Smart Table", meta = ( ToolTip = "", ClampMin = "0.0" ) )
    float ArrivalWindowSeconds = 2.0f;

    UPROPERTY( EditAnywhere, Category = "Smart Table", meta = ( ToolTip = "" ) )
    bool bStickToEnd = true;

    UPROPERTY( EditAnywhere, Category = "Smart Table", meta = ( ToolTip = "" ) )
    bool bStartAtEnd = false;

    UPROPERTY( EditAnywhere, AdvancedDisplay, Category = "Smart Table", meta = ( ToolTip = "", ClampMin = "0.05", ClampMax = "1.0" ) )
    float RevealFraction = 0.75f;

    UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Smart Table", meta = ( ToolTip = "" ) )
    TSubclassOf< USmartTableRowWidget > RowWidgetClass;

    UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Smart Table", meta = ( ToolTip = "" ) )
    ESmartTableHeight HeightMode = ESmartTableHeight::Fill;

    UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Smart Table", meta = ( ToolTip = "", ClampMin = "0", EditCondition = "HeightMode == ESmartTableHeight::FitRows" ) )
    int32 MaxVisibleRows = 0;

    UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Smart Table", meta = ( ToolTip = "" ) )
    bool bShowHeader = true;

    UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Smart Table|Row Numbers", meta = ( ToolTip = "" ) )
    bool bShowRowNumbers = false;

    UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Smart Table|Columns", meta = ( ToolTip = "" ) )
    bool bStretchLastColumn = false;

    UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Smart Table|Columns", meta = ( ToolTip = "" ) )
    bool bAllowHorizontalScroll = false;

    UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Smart Table|Row Numbers", meta = ( ClampMin = "8.0" ) )
    float RowNumberColumnWidth = 48.0f;

    UPROPERTY( EditAnywhere, AdvancedDisplay, Category = "Smart Table|Row Numbers", meta = ( ToolTip = "", EditCondition = "StyleAsset == nullptr" ) )
    FTextBlockStyle RowNumberTextStyle;

    UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Smart Table|Preview", meta = ( ClampMin = "0", ClampMax = "24" ) )
    int32 PreviewRowCount = 6;

    UPROPERTY( EditAnywhere, Category = "Smart Table|Style", meta = ( ToolTip = "" ) )
    TObjectPtr< USmartTableStyle > StyleAsset;

    UPROPERTY( EditAnywhere, Category = "Smart Table|Style", meta = ( ToolTip = "", EditCondition = "StyleAsset == nullptr" ) )
    FSlateBrush BackgroundBrush;

    UPROPERTY( EditAnywhere, Category = "Smart Table|Style", meta = ( EditCondition = "StyleAsset == nullptr" ) )
    FTableRowStyle RowStyle;

    UPROPERTY( EditAnywhere, Category = "Smart Table|Style", meta = ( EditCondition = "StyleAsset == nullptr" ) )
    FHeaderRowStyle HeaderStyle;

    UPROPERTY( EditAnywhere, Category = "Smart Table|Style", meta = ( EditCondition = "StyleAsset == nullptr" ) )
    FScrollBarStyle ScrollBarStyle;

    UPROPERTY( EditAnywhere, Category = "Smart Table|Style", meta = ( ToolTip = "", EditCondition = "StyleAsset == nullptr" ) )
    FTextBlockStyle HeaderTextStyle;

    UPROPERTY( EditAnywhere, Category = "Smart Table|Style", meta = ( ToolTip = "", EditCondition = "StyleAsset == nullptr" ) )
    FTextBlockStyle CellTextStyle;

    UPROPERTY( EditAnywhere, Category = "Smart Table|Style", meta = ( ToolTip = "", EditCondition = "StyleAsset == nullptr", ClampMin = "0.0" ) )
    float HeaderHeight = 0.0f;

    UPROPERTY( EditAnywhere, Category = "Smart Table|Empty State" )
    FText EmptyText;

    UPROPERTY( EditAnywhere, Category = "Smart Table|Empty State", meta = ( ToolTip = "" ) )
    FText NoMatchesTextFormat;

    UPROPERTY( EditAnywhere, AdvancedDisplay, Category = "Smart Table|Empty State", meta = ( EditCondition = "StyleAsset == nullptr" ) )
    FTextBlockStyle EmptyTextStyle;

    UPROPERTY( EditAnywhere, Instanced, BlueprintReadOnly, Category = "Smart Table|Layout", meta = ( ToolTip = "" ) )
    TObjectPtr< USmartTableLayoutStore > LayoutStore;

    UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Smart Table|Layout", meta = ( ToolTip = "" ) )
    FName TableId;

    UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Smart Table|Sorting", meta = ( ToolTip = "" ) )
    bool bAllowUserSorting = true;

    UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Smart Table|Sorting", meta = ( ToolTip = "" ) )
    bool bAllowSecondarySort = true;

    UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Smart Table|Sorting" )
    bool bAllowSortNone = true;

    UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Smart Table|Sorting", meta = ( ToolTip = "" ) )
    bool bAllowDeadSecondarySort = false;

    UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Smart Table|Columns", meta = ( ToolTip = "" ) )
    bool bAllowColumnReorder = true;

    UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Smart Table|Columns", meta = ( ToolTip = "" ) )
    bool bAllowColumnResize = true;

    UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Smart Table|Columns" )
    FSmartTableHeaderMenuOptions HeaderMenuOptions;

    UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Smart Table|Rows", meta = ( ToolTip = "" ) )
    bool bScrollToAddedRow = true;

    UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Smart Table|Rows", meta = ( ToolTip = "", ClampMin = "0.0" ) )
    float ScrollToRowSeconds = 0.25f;

    UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Smart Table|Rows", meta = ( ToolTip = "" ) )
    FSmartTableRowMenuOptions RowMenuOptions;

    UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Smart Table|Rows", meta = ( ToolTip = "", TitleProperty = "Id" ) )
    TArray< FSmartTableMenuEntry > RowMenuEntries;

    UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Smart Table|Columns", meta = ( ToolTip = "", ClampMin = "2.0" ) )
    float ResizeGripWidth = 0.0f;

    UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Smart Table|Columns", meta = ( ToolTip = "", ClampMin = "1.0" ) )
    float MinColumnWidth = 0.0f;

    UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Smart Table|Columns", meta = ( ClampMin = "0.0" ) )
    float MaxAutoSizeColumnWidth = 0.0f;

    UPROPERTY( EditAnywhere, AdvancedDisplay, Category = "Smart Table|Style", meta = ( EditCondition = "StyleAsset == nullptr" ) )
    FSlateBrush InsertMarkerBrush;

    UPROPERTY( EditAnywhere, Category = "Smart Table|Style", meta = ( EditCondition = "StyleAsset == nullptr" ) )
    FSlateBrush ColumnDividerBrush;

    UPROPERTY( EditAnywhere, AdvancedDisplay, Category = "Smart Table|Style", meta = ( EditCondition = "StyleAsset == nullptr" ) )
    FSlateBrush ColumnCaretBrush;

    UPROPERTY( EditAnywhere, AdvancedDisplay, Category = "Smart Table|Style", meta = ( EditCondition = "StyleAsset == nullptr" ) )
    FSlateBrush FocusBorderBrush;

    UPROPERTY( EditAnywhere, Category = "Smart Table|Input", meta = ( ToolTip = "" ) )
    bool bFocusOnPointerInteraction = true;

    UPROPERTY( EditAnywhere, Category = "Smart Table|Input", meta = ( ToolTip = "" ) )
    bool bAllowRightClickDragScroll = true;

    UPROPERTY( EditAnywhere, Category = "Smart Table|Style", meta = ( ToolTip = "" ) )
    bool bShowFocusBorder = true;

    UPROPERTY( EditAnywhere, Category = "Smart Table|Style", meta = ( ToolTip = "" ) )
    bool bShowColumnCaret = true;

    UPROPERTY( EditAnywhere, Category = "Smart Table|Style", meta = ( ToolTip = "" ) )
    bool bColumnCaretNeedsFocus = false;

    UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Smart Table|Busy", meta = ( ToolTip = "", ClampMin = "0.0" ) )
    float BusyOverlayDelaySeconds = 0.15f;

    UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Smart Table|Busy" )
    bool bBlockInputWhileBusy = true;

    UPROPERTY( EditAnywhere, AdvancedDisplay, Category = "Smart Table|Busy", meta = ( EditCondition = "StyleAsset == nullptr" ) )
    FSlateBrush BusyOverlayBrush;

    UPROPERTY( EditAnywhere, Category = "Smart Table|Style", meta = ( ToolTip = "", EditCondition = "StyleAsset == nullptr" ) )
    FSmartTableMenuStyle MenuStyle;

private:
    TSharedRef< ITableRow > HandleGenerateRow( int32 NaturalRow, const TSharedRef< STableViewBase > & OwnerTable );
    void HandleSelectionChanged( int32 NaturalRow, ESelectInfo::Type SelectInfo );
    void HandleRowActivated( int32 NaturalRow );

    void RebuildHeader();

    FText GetEmptyStateText() const;
    EVisibility GetEmptyStateVisibility() const;

    FOptionalSize GetFitRowsHeight() const;

    EVisibility GetBusyOverlayVisibility() const;
    void HandleModelBusyChanged( bool bInBusy );

    FMenuBuilder MakeMenuBuilder();
    TSharedRef< SWidget > HostMenu( FMenuBuilder & Menu );

    void AppendTableWideEntries( FMenuBuilder & Menu );

    TSharedRef< SWidget > MakeTableMenu();

    void HeaderLayoutChanged();

    void RefreshVisibleRowCells( FName ColumnId );

    void NotifyVisibleCellsOfDragRoles();

    FSmartTableCellDrag CellDrag;

    EActiveTimerReturnType TickTable( double CurrentTime, float DeltaTime );

    float GetRowNumberWidth() const;

    float LeadingColumnOffset() const
    {
        return bShowRowNumbers ? GetRowNumberWidth() : 0.0f;
    }

    float GetAvailableColumnWidth() const;

    void ReconcileColumnWidths();

    void MarkColumnWidthsDirty()
    {
        bColumnWidthsDirty = true;
    }

    void RefreshRowsForNewColumnWidths();

    float LastReconciledWidth = -1.0f;

    TMap< FName, float > ResolvedWidths;

    void HandleColumnWidthChanged( float NewWidth, FName ColumnId );

    void SetColumnWidthWhileDragging( FName ColumnId, float Width );

    void SettleGripWidths();

    void FinishResize();

    void UpdateResizeAutoScroll();

    float TakeWidthFromColumnsRightOf( FName DraggedColumnId, float DesiredWidth );

    float WidthAtResizeStart( FName ColumnId );

    FReply HandleHeaderRowMouseDown( const FGeometry & Geometry, const FPointerEvent & MouseEvent );
    FReply HandleHeaderRowMouseMove( const FGeometry & Geometry, const FPointerEvent & MouseEvent );

    FName FindColumnEdgeAt( float LocalX, float & OutColumnLeft ) const;

    TSharedRef< SWidget > MakeColumnVisibilityMenu();

    void ToggleColumnVisible( FName ColumnId );

    SmartTable::FColumnView ColumnView() const;

    FSmartTableColumnLayout & LayoutFor( FName ColumnId );

    void WarnUnknownColumn( const TCHAR * Caller, FName ColumnId ) const;

    void PushMenuAt( TSharedRef< SWidget > Anchor, TSharedRef< SWidget > Menu, bool bBelow );

    void ApplySelection( TConstArrayView< int32 > NaturalRows, bool bSelected );

    int32 PresentedIndexOfCaret() const;

    void MoveCaretTo( int32 PresentedRow );

    bool IsRowDrawn( int32 NaturalRow ) const;

    int32 PresentedRowOf( int32 NaturalRow ) const;

    bool CanMoveCaret( const TCHAR * Caller ) const;

    FReply HandleListKeyDown( const FGeometry & Geometry, const FKeyEvent & KeyEvent );

    void LayoutChangedByUser();

    void ApplyDesignTimePreview();

    void RebuildRowIndices();

    void HandlePresentationChanged( const FSmartTablePresentationChange & Change );

    bool CanSortBySecondaryColumn( FName ColumnId ) const;

    void RecomputeDeadSecondaryColumns();

    void SortFromMenu( FName ColumnId, ESmartTableSortMode SortMode, bool bSecondary );

    bool LoadStoredLayout();

    void RestoreStoredLayout();

    USmartTableObjectModel & GetOrCreateItemsModel();

    UObject * ItemForRow( int32 NaturalRow ) const;

    FName RowIdAt( int32 NaturalRow ) const;

    UPROPERTY( Transient )
    TObjectPtr< USmartTableModel > Model;

    UPROPERTY( Transient )
    TObjectPtr< USmartTableObjectModel > ItemsModel;

    UPROPERTY( Transient )
    TObjectPtr< USmartTableDataTableModel > DataTableModel;

    UPROPERTY( Transient )
    TObjectPtr< USmartTablePreviewModel > PreviewModel;

    UPROPERTY( Transient )
    FTableViewStyle BackgroundStyle;

    UPROPERTY( Transient )
    FSmartTableLayout ActiveLayout;

    TSharedPtr< class STextBlock > EmptyStateText;

    UPROPERTY( Transient )
    FUserWidgetPool CellPool;

    UPROPERTY( Transient )
    FUserWidgetPool RowWidgetPool;

    TWeakObjectPtr< const UClass > RowWidgetSlotWarnedFor;

    ESmartTableAssignReason ReasonForRow( int32 NaturalRow ) const;

    FSmartTableArrivals Arrivals;

    void UpdateArrivals();

    void SampleViewAtEnd();

    void ReassignChangedRows();

    void ArmArrivalIfOwed( int32 NaturalRow, USmartTableRowWidget * Wrapper );

    FSmartTableOpenMenu MenuHost;

    bool bWarnedSizedByContent = false;

    TSharedRef< class SWidget > MakeRowMenu( const FSmartTableKeptRow & Row );

    void OpenRowMenuInternal( int32 NaturalRow, const TOptional< FVector2D > & AbsolutePosition, int32 DrivingUserIndex );

    void BeginScrollToRow( int32 PresentedRow );

    void StepScrollAnimation( float DeltaTime );

    void StartAtEndIfOwed();

    void RetargetScrollAfterStructuralChange();

    FSmartTableHeldRow TopRowOnScreen();

    void RememberTopRow();

    void ScrollAfterRowSetChange( const FSmartTableRowSetDiff & Diff, const FSmartTableHeldRow & Held, int32 PresentedBefore, bool bWasAtEnd );

    void HoldRowAtTop( const FSmartTableHeldRow & Held, const FSmartTableRowSetDiff & Diff );

    int32 LastDrawnArrival( const FSmartTableRowSetDiff & Diff ) const;

    void PlaceScrollOnRow( int32 NaturalRow );

    FSmartTableScrollMove Scroll;

    void WarnIfSizedByContent();

    void WarnAboutDuplicateColumnIds( const TArray< FSmartTableColumn > & InColumns );

    float MeasuredRowNumberWidth = 0.0f;

    void MirrorLastColumnStyle();

    void ApplyStyleAsset();

    void RefreshAppliedStyle();

    UPROPERTY( Transient )
    TObjectPtr< USmartTableStyle > AppliedStyleAsset;

    UPROPERTY( Transient )
    TObjectPtr< USmartTableStyle > AuthoredStyle;

    void MeasureRowNumberWidth();

    int32 FramesSinceSizeCheck = 0;

    bool bWarnedEmptyRowMenu = false;

    EVisibility GetFocusBorderVisibility() const;

    TArray< int32 > RowIndices;

    TArray< TWeakPtr< SSmartTableRow > > GeneratedRows;

    void ForEachGeneratedRow( TFunctionRef< void( SSmartTableRow & ) > Visit );

    TSharedPtr< class SSmartTableListView > ListView;

    TSharedPtr< class SScrollBar > VerticalScrollBar;

    TSharedPtr< class SScrollBox > HorizontalScrollBox;

    TSet< FName > ItemlessColumnsWarned;

    TMap< FName, TWeakObjectPtr< const UClass > > CellClassFailuresWarned;

    TSet< TWeakObjectPtr< const UClass > > ItemSetterClassesWarned;

    bool bMissingTableIdWarned = false;

    TSharedPtr< SHeaderRow > HeaderRow;

    bool bApplyingPresentation = false;

    bool bHoldSelectionBroadcast = false;

    bool bSelectionBroadcastOwed = false;

    bool bWalkingRowCells = false;

    bool bHeaderRebuildPending = false;

    FSmartTableDeadSecondaries DeadSecondaries;

    bool bBusy = false;

    bool bColumnWidthsDirty = true;

    bool bHeaderWidthsLive = false;

    float HeaderGutterWidth = 0.0f;

    bool bLayoutSavePending = false;

    int32 FocusedRow = INDEX_NONE;

    FSmartTableColumnSelection ColumnSelection;

    FName ColumnForIntent( FName ColumnId );

    void ColumnSelectionChanged( bool bChanged );

    TSet< FName > DuplicateColumnIdsWarned;

    FSmartTableResizeGesture Resize;

    double BusyStartSeconds = 0.0;
};
