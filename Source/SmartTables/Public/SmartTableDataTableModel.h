// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#pragma once

#include "Containers/Set.h"
#include "SmartTableBinding.h"
#include "SmartTableModel.h"
#include "SmartTableDataTableModel.generated.h"

class UDataTable;

UCLASS( BlueprintType, meta = ( ToolTip = "" ) )
class SMARTTABLES_API USmartTableDataTableRow : public UObject
{
    GENERATED_BODY()

public:

    UPROPERTY( BlueprintReadOnly, Category = "Smart Tables" )
    TObjectPtr< UDataTable > DataTable;

    UPROPERTY( BlueprintReadOnly, Category = "Smart Tables" )
    FName RowName;
};

UCLASS( BlueprintType, meta = ( ToolTip = "" ) )
class SMARTTABLES_API USmartTableDataTableModel : public USmartTableModel
{
    GENERATED_BODY()

public:

    UFUNCTION( BlueprintCallable, Category = "Smart Tables|Model" )
    void SetDataTable( UDataTable * InDataTable );

    UFUNCTION( BlueprintPure, Category = "Smart Tables|Model" )
    UDataTable * GetDataTable() const
    {
        return DataTable;
    }

    void SetColumns( const TArray< FSmartTableColumn > & InColumns );

protected:
    virtual int32 GetNumRows_Implementation() override;
    virtual FText GetCellText_Implementation( int32 NaturalRow, FName ColumnId ) override;
    virtual FSmartTableSortKey GetCellSortKey_Implementation( int32 NaturalRow, FName ColumnId ) override;
    virtual FName GetRowId_Implementation( int32 NaturalRow ) override;
    virtual UObject * GetRowItem_Implementation( int32 NaturalRow ) override;
    virtual int32 NaturalRowOfItem_Implementation( UObject * Item ) override;

private:

    void ResolveBindings();

    virtual void BeginDestroy() override;

    virtual ESmartTableCellEditor GetCellEditor_Implementation( int32 NaturalRow, FName ColumnId ) override;
    virtual bool SetCellText_Implementation( int32 NaturalRow, FName ColumnId, const FText & Value ) override;

    void HandleDataTableChanged();

    FDelegateHandle DataTableChangedHandle;

    int32 IndexOfColumn( FName ColumnId ) const;

    TMap< FName, int32 > ColumnOrderById;

    static FString UnresolvedSignature( const struct FSmartTableColumn & Column, const UScriptStruct & RowStruct );

    uint8 * RowData( int32 NaturalRow ) const;

    UPROPERTY( Transient )
    TObjectPtr< UDataTable > DataTable;

    UPROPERTY( Transient )
    TArray< FSmartTableColumn > Columns;

    UPROPERTY( Transient )
    TMap< FName, TObjectPtr< USmartTableDataTableRow > > RowProxies;

    TArray< FName > RowNames;

    TArray< SmartTable::FBinding > ValueBindings;
    TArray< SmartTable::FBinding > SortBindings;

    TMap< FName, FString > UnresolvedWarnings;

    FString NoRowStructWarnedFor;
};
