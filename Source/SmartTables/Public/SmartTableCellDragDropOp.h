// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#pragma once

#include "Blueprint/DragDropOperation.h"
#include "SmartTableCellDragDropOp.generated.h"

class USmartTable;
class UWidget;

UCLASS( BlueprintType, meta = ( ToolTip = "" ) )
class SMARTTABLES_API USmartTableCellDragDropOp : public UDragDropOperation
{
    GENERATED_BODY()

public:

    UPROPERTY( BlueprintReadOnly, Category = "Smart Tables|Drag and Drop" )
    int32 SourceRow = INDEX_NONE;

    UPROPERTY( BlueprintReadOnly, Category = "Smart Tables|Drag and Drop" )
    FName SourceRowId;

    UPROPERTY( BlueprintReadOnly, Category = "Smart Tables|Drag and Drop" )
    FName SourceColumnId;

    UPROPERTY( BlueprintReadOnly, Category = "Smart Tables|Drag and Drop" )
    TObjectPtr< UObject > SourceItem;

    UPROPERTY( BlueprintReadOnly, Category = "Smart Tables|Drag and Drop" )
    TWeakObjectPtr< USmartTable > SourceTable;

    virtual void Drop_Implementation( const FPointerEvent & PointerEvent ) override;
    virtual void DragCancelled_Implementation( const FPointerEvent & PointerEvent ) override;

private:
    void EndDrag();

    static void ReleaseCellsIn( UWidget * Widget );
};
