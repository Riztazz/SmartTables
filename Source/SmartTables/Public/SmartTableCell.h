// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#pragma once

#include "Blueprint/UserWidget.h"
#include "SmartTableTypes.h"
#include "SmartTableCell.generated.h"

class UFunction;
class USmartTable;
class USmartTableModel;
class UWidgetAnimation;

UCLASS( Abstract, meta = ( ToolTip = "" ) )
class SMARTTABLES_API USmartTableCell : public UUserWidget
{
    GENERATED_BODY()

public:
    void AssignCell( USmartTable * InTable, USmartTableModel * InModel, int32 InNaturalRow, FName InColumnId, ESmartTableAssignReason InReason );

    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Smart Tables|Cell", meta = ( ToolTip = "" ) )
    FName ValueChangedAnimation = TEXT( "Pulse" );

    UFUNCTION( BlueprintPure, Category = "Smart Tables|Cell" )
    UWidgetAnimation * FindCellAnimation( FName AnimationName ) const;

    UFUNCTION( BlueprintPure, Category = "Smart Tables|Cell" )
    ESmartTableAssignReason GetAssignReason() const
    {
        return AssignReason;
    }

    void ReleaseCell();

    UFUNCTION( BlueprintPure, Category = "Smart Tables|Cell" )
    USmartTable * GetTable() const
    {
        return Table.Get();
    }

    UFUNCTION( BlueprintPure, Category = "Smart Tables|Cell" )
    USmartTableModel * GetModel() const
    {
        return Model;
    }

    UFUNCTION( BlueprintPure, Category = "Smart Tables|Cell" )
    int32 GetRowIndex() const
    {
        return NaturalRow;
    }

    UFUNCTION( BlueprintPure, Category = "Smart Tables|Cell" )
    FName GetColumnId() const
    {
        return ColumnId;
    }

    UFUNCTION( BlueprintPure, Category = "Smart Tables|Cell" )
    UObject * GetItem() const;

    bool HasItemSetter() const
    {
        return !ItemSetter.IsNone();
    }

    UFUNCTION( BlueprintNativeEvent, Category = "Smart Tables|Drag and Drop" )
    class USmartTableCellDragDropOp * MakeCellDragPayload();

    UFUNCTION( BlueprintNativeEvent, Category = "Smart Tables|Drag and Drop" )
    UWidget * MakeCellDragVisual();

    UFUNCTION( BlueprintPure, Category = "Smart Tables|Drag and Drop" )
    ESmartTableCellDragRole GetCellDragRole() const
    {
        return DragRole;
    }

    void SetCellDragRole( ESmartTableCellDragRole NewRole );

    UFUNCTION( BlueprintNativeEvent, Category = "Smart Tables|Drag and Drop", meta = ( DisplayName = "On Cell Drag Role Changed" ) )
    void OnCellDragRoleChanged( ESmartTableCellDragRole NewRole );

protected:

    UPROPERTY( EditDefaultsOnly, Category = "Smart Tables|Cell", meta = ( GetOptions = "GetItemSetterOptions", ToolTip = "" ) )
    FName ItemSetter;

    UFUNCTION()
    TArray< FName > GetItemSetterOptions() const;

    virtual void NativeOnCellAssigned()
    {
    }

    virtual void NativeOnCellReleased()
    {
    }

    virtual FReply NativeOnMouseButtonDown( const FGeometry & Geometry, const FPointerEvent & MouseEvent ) override;

    virtual void NativeOnDragDetected( const FGeometry & Geometry, const FPointerEvent & MouseEvent, UDragDropOperation *& OutOperation ) override;

    virtual bool NativeOnDragOver( const FGeometry & Geometry, const FDragDropEvent & DragDropEvent, UDragDropOperation * InOperation ) override;

    virtual void NativeOnDragLeave( const FDragDropEvent & DragDropEvent, UDragDropOperation * InOperation ) override;

    virtual bool NativeOnDrop( const FGeometry & Geometry, const FDragDropEvent & DragDropEvent, UDragDropOperation * InOperation ) override;

    UFUNCTION( BlueprintImplementableEvent, Category = "Smart Tables|Cell", meta = ( DisplayName = "On Cell Assigned" ) )
    void OnCellAssigned( ESmartTableAssignReason Reason );

    UFUNCTION( BlueprintImplementableEvent, Category = "Smart Tables|Cell", meta = ( DisplayName = "On Cell Released" ) )
    void OnCellReleased();

    UFUNCTION( BlueprintImplementableEvent, Category = "Smart Tables|Cell", meta = ( DisplayName = "On Item Set" ) )
    void OnItemSet( UObject * Item );

private:

    void PlayValueChanged( ESmartTableAssignReason Reason );

    TWeakObjectPtr< USmartTable > Table;

    UPROPERTY( Transient )
    TObjectPtr< USmartTableModel > Model;

    int32 NaturalRow = INDEX_NONE;
    FName ColumnId;

    ESmartTableCellDragRole DragRole = ESmartTableCellDragRole::None;

    ESmartTableAssignReason AssignReason = ESmartTableAssignReason::Scrolled;

    TOptional< bool > bWantsItem;

    void HandOverItem( UObject * Item );

    UFunction * FindItemSetter();

    bool IsFirstItemSetterWarning();

    TOptional< TWeakObjectPtr< UFunction > > ResolvedItemSetter;

    UPROPERTY( Transient )
    TObjectPtr< UObject > HandedItem;

    UPROPERTY( Transient )
    TObjectPtr< UWidgetAnimation > ResolvedFlash;

    FName ResolvedFlashFor;

    UWidgetAnimation * ResolveFlash();
};

UCLASS( meta = ( ToolTip = "" ) )
class SMARTTABLES_API USmartTableTextCell : public USmartTableCell
{
    GENERATED_BODY()

protected:
    virtual void NativeOnCellAssigned() override;
    virtual TSharedRef< SWidget > RebuildWidget() override;

private:
    TSharedPtr< class STextBlock > TextBlock;
};

UCLASS( meta = ( ToolTip = "" ) )
class SMARTTABLES_API USmartTableEditableCell : public USmartTableCell
{
    GENERATED_BODY()

protected:
    virtual void NativeOnCellAssigned() override;
    virtual void NativeOnCellReleased() override;
    virtual TSharedRef< SWidget > RebuildWidget() override;

private:

    void ShowModelValue();

    void ShowNothing();

    bool IsEditOpen() const;

    void EndOpenEdit( const TCHAR * Why );

    void CommitText( const FText & Value, ETextCommit::Type Cause );

    bool IsCellChecked( const FText & Drawn ) const;

    void CommitToggle( ECheckBoxState State );

    TSharedPtr< class SWidgetSwitcher > Switcher;
    TSharedPtr< class SEditableTextBox > TextBox;
    TSharedPtr< class SCheckBox > CheckBox;
    TSharedPtr< class STextBlock > ReadOnlyText;

    ESmartTableCellEditor Editor = ESmartTableCellEditor::None;
};
