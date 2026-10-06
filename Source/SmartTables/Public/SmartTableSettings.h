// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#pragma once

#include "Engine/DeveloperSettings.h"
#include "Math/Color.h"
#include "SmartTableColour.h"
#include "SmartTableSettings.generated.h"

UCLASS( Config = Game, DefaultConfig, meta = ( DisplayName = "Smart Tables", ToolTip = "" ) )
class SMARTTABLES_API USmartTableSettings : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    virtual FName GetContainerName() const override
    {
        return TEXT( "Project" );
    }

    virtual FName GetCategoryName() const override
    {
        return TEXT( "Plugins" );
    }

#if WITH_EDITOR

    virtual void PostEditChangeProperty( FPropertyChangedEvent & PropertyChangedEvent ) override;
#endif

    UPROPERTY( EditAnywhere, Config, Category = "Columns", meta = ( ToolTip = "", ClampMin = "2.0" ) )
    float ResizeGripWidth = 14.0f;

    UPROPERTY( EditAnywhere, Config, Category = "Columns", meta = ( ClampMin = "1.0" ) )
    float MinColumnWidth = 24.0f;

    UPROPERTY( EditAnywhere, Config, Category = "Columns", meta = ( ToolTip = "", ClampMin = "0.0" ) )
    float MaxAutoSizeColumnWidth = 600.0f;

    UPROPERTY( EditAnywhere, Config, Category = "Columns", meta = ( ToolTip = "", ClampMin = "1" ) )
    int32 MaxRowsMeasuredForAutoSize = 5000;

    UPROPERTY( EditAnywhere, Config, Category = "Palette" )
    FLinearColor HeaderSurfaceColor = SmartTable::Srgb( 38, 42, 50 );

    UPROPERTY( EditAnywhere, Config, Category = "Palette" )
    FLinearColor HeaderHoveredColor = SmartTable::Srgb( 50, 55, 65 );

    UPROPERTY( EditAnywhere, Config, Category = "Palette" )
    FLinearColor TextColor = SmartTable::Srgb( 200, 205, 212 );

    UPROPERTY( EditAnywhere, Config, Category = "Palette" )
    FLinearColor MutedTextColor = SmartTable::Srgb( 108, 115, 125 );

    UPROPERTY( EditAnywhere, Config, Category = "Palette" )
    FLinearColor RowHoveredColor = SmartTable::Srgb( 44, 48, 56 );

    UPROPERTY( EditAnywhere, Config, Category = "Palette", meta = ( ToolTip = "" ) )
    FLinearColor SelectionColor = SmartTable::Srgb( 44, 92, 130 );

    UPROPERTY( EditAnywhere, Config, Category = "Palette" )
    FLinearColor SelectionHoveredColor = SmartTable::Srgb( 54, 108, 150 );

    UPROPERTY( EditAnywhere, Config, Category = "Palette" )
    FLinearColor SeparatorColor = SmartTable::Srgb( 58, 64, 74 );

    UPROPERTY( EditAnywhere, Config, Category = "Palette" )
    FLinearColor ColumnDividerColor = SmartTable::Srgb( 52, 58, 68 );

    UPROPERTY( EditAnywhere, Config, Category = "Palette" )
    FLinearColor ScrollbarTrackColor = SmartTable::Srgb( 20, 22, 26 );

    UPROPERTY( EditAnywhere, Config, Category = "Palette", meta = ( ToolTip = "" ) )
    FLinearColor AccentColor = SmartTable::Srgb( 255, 186, 92 );
};
