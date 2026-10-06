// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#pragma once

#include "Containers/Ticker.h"
#include "SmartTableTypes.h"
#include "UObject/Object.h"
#include "SmartTableLayoutStore.generated.h"

UCLASS( Abstract, Blueprintable, BlueprintType, EditInlineNew, meta = ( ToolTip = "" ) )
class SMARTTABLES_API USmartTableLayoutStore : public UObject
{
    GENERATED_BODY()

public:

    UFUNCTION( BlueprintNativeEvent, Category = "Smart Tables|Layout" )
    bool LoadLayout( FName TableId, FSmartTableLayout & OutLayout ) const;

    UFUNCTION( BlueprintNativeEvent, Category = "Smart Tables|Layout" )
    void SaveLayout( FName TableId, const FSmartTableLayout & Layout );
};

UCLASS( BlueprintType, EditInlineNew, meta = ( ToolTip = "" ) )
class SMARTTABLES_API USmartTableConfigLayoutStore : public USmartTableLayoutStore
{
    GENERATED_BODY()

public:

    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Smart Tables" )
    FString ConfigSection = TEXT( "SmartTables.Layouts" );

    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Smart Tables", meta = ( ToolTip = "" ) )
    FString ConfigFileName;

    virtual void BeginDestroy() override;

protected:
    virtual bool LoadLayout_Implementation( FName TableId, FSmartTableLayout & OutLayout ) const override;
    virtual void SaveLayout_Implementation( FName TableId, const FSmartTableLayout & Layout ) override;

private:

    const FString & ResolvedConfigFile() const;

    void ScheduleFlush();

    void FlushNow();

    FTSTicker::FDelegateHandle FlushHandle;

    TSet< FString > PendingFlushFiles;
};

namespace SmartTable
{

    SMARTTABLES_API bool MigrateStoredLayout( FSmartTableLayout & Layout );
}
