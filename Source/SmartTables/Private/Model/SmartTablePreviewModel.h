// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#pragma once

#include "SmartTableBinding.h"
#include "SmartTableModel.h"
#include "SmartTablePreviewModel.generated.h"

UCLASS()
class USmartTablePreviewModel : public USmartTableModel
{
    GENERATED_BODY()

public:

    void SetPreview( const TArray< FSmartTableColumn > & InColumns, const UStruct * InSchema, const class UDataTable * InRowSource, int32 InNumRows );

protected:
    virtual int32 GetNumRows_Implementation() override;
    virtual FText GetCellText_Implementation( int32 NaturalRow, FName ColumnId ) override;

private:

    UPROPERTY( Transient )
    TArray< FSmartTableColumn > Columns;

    UPROPERTY( Transient )
    TObjectPtr< const UStruct > Schema;

    UPROPERTY( Transient )
    TObjectPtr< const class UDataTable > RowSource;

    TArray< FName > RowNames;

    const UStruct * EffectiveSchema() const;

    int32 NumRows = 0;
};
