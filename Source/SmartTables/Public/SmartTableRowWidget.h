// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#pragma once

#include "Blueprint/UserWidget.h"
#include "SmartTableTypes.h"
#include "SmartTableRowWidget.generated.h"

class UNativeWidgetHost;
class USmartTable;
class UWidgetAnimation;

UCLASS( Abstract, Blueprintable, meta = ( ToolTip = "" ) )
class SMARTTABLES_API USmartTableRowWidget : public UUserWidget
{
    GENERATED_BODY()

public:

    static const FName ColumnsSlotName;

    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Smart Tables|Row", meta = ( ToolTip = "" ) )
    FName ArrivalAnimation = TEXT( "In" );

    UFUNCTION( BlueprintPure, Category = "Smart Tables|Row" )
    UWidgetAnimation * FindRowAnimation( FName AnimationName ) const;

    bool HostColumns( const TSharedRef< SWidget > & Columns );

    void AssignRow( USmartTable * InTable, int32 InNaturalRow, ESmartTableAssignReason InReason );

    void ArmArrival();

    bool PlayArrivalNow();

    void CancelArrival();

    UFUNCTION( BlueprintPure, Category = "Smart Tables|Row" )
    ESmartTableAssignReason GetAssignReason() const
    {
        return AssignReason;
    }

    void ReleaseRow();

    UFUNCTION( BlueprintPure, Category = "Smart Tables|Row" )
    USmartTable * GetTable() const
    {
        return Table.Get();
    }

    UFUNCTION( BlueprintPure, Category = "Smart Tables|Row" )
    int32 GetRowIndex() const
    {
        return NaturalRow;
    }

protected:
    virtual void NativeOnRowAssigned()
    {
    }

    virtual void NativeOnRowReleased()
    {
    }

    UFUNCTION( BlueprintImplementableEvent, Category = "Smart Tables|Row", meta = ( DisplayName = "On Row Assigned" ) )
    void OnRowAssigned( ESmartTableAssignReason Reason );

    UFUNCTION( BlueprintImplementableEvent, Category = "Smart Tables|Row", meta = ( DisplayName = "On Row Released" ) )
    void OnRowReleased();

private:

    FName ResolveColumnsSlot();

    TWeakObjectPtr< USmartTable > Table;

    UPROPERTY( Transient )
    TObjectPtr< UNativeWidgetHost > ColumnsHost;

    int32 NaturalRow = INDEX_NONE;

    ESmartTableAssignReason AssignReason = ESmartTableAssignReason::Scrolled;

    FName ColumnsSlot;
    bool bColumnsSlotResolved = false;

    bool bArrivalArmed = false;

    UPROPERTY( Transient )
    TObjectPtr< UWidgetAnimation > ResolvedArrival;

    FName ResolvedArrivalFor;

    UWidgetAnimation * ResolveArrival();
};
