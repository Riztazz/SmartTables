// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#pragma once

#include "SmartTableBinding.h"
#include "SmartTableModel.h"
#include "UObject/ObjectKey.h"
#include "SmartTableObjectModel.generated.h"

UCLASS( BlueprintType, meta = ( ToolTip = "" ) )
class SMARTTABLES_API USmartTableObjectModel : public USmartTableModel
{
    GENERATED_BODY()

public:

    void SetItems( const TArray< UObject * > & InItems );

    void AddItem( UObject * Item );

    void RemoveItem( UObject * Item );

    void ClearItems();

    const TArray< TObjectPtr< UObject > > & GetItems() const
    {
        return Items;
    }

    int32 IndexOfItem( const UObject * Item ) const;

    static FName RowIdOf( const UObject * Item );

    void SetColumns( const TArray< FSmartTableColumn > & InColumns );

protected:
    virtual int32 GetNumRows_Implementation() override;
    virtual FText GetCellText_Implementation( int32 NaturalRow, FName ColumnId ) override;
    virtual FSmartTableSortKey GetCellSortKey_Implementation( int32 NaturalRow, FName ColumnId ) override;
    virtual FName GetRowId_Implementation( int32 NaturalRow ) override;
    virtual UObject * GetRowItem_Implementation( int32 NaturalRow ) override;
    virtual int32 NaturalRowOfItem_Implementation( UObject * Item ) override;
    virtual bool MoveRows_Implementation( const TArray< int32 > & NaturalRows, int32 InsertBeforeNaturalRow ) override;

private:

    struct FColumnBinding
    {

        int32 Column = INDEX_NONE;

        SmartTable::FBinding Value;
        SmartTable::FBinding Sort;
    };

    void ResolveBindingsFor( UClass * Class );

    TArray< FName > RowIds() const;

    virtual ESmartTableCellEditor GetCellEditor_Implementation( int32 NaturalRow, FName ColumnId ) override;
    virtual bool SetCellText_Implementation( int32 NaturalRow, FName ColumnId, const FText & Value ) override;

    const FColumnBinding * FindBinding( const UObject * Item, FName ColumnId );

    UPROPERTY( Transient )
    TArray< TObjectPtr< UObject > > Items;

    UPROPERTY( Transient )
    TArray< FSmartTableColumn > Columns;

    TMap< TWeakObjectPtr< UClass >, TArray< FColumnBinding > > BindingsByClass;

    TMap< FName, int32 > ColumnOrderById;

    TMap< TObjectKey< UObject >, int32 > RowByItem;

    void RebuildItemRows();
};
