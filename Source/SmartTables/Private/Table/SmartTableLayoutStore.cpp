// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "SmartTableLayoutStore.h"

#include "CoreGlobals.h"
#include "Logging/StructuredLog.h"
#include "Misc/ConfigCacheIni.h"
#include "SmartTableLog.h"
#include "UObject/Class.h"

bool SmartTable::MigrateStoredLayout( FSmartTableLayout & Layout )
{
    if ( Layout.Version >= FSmartTableLayout::CurrentVersion )
    {
        return false;
    }

    if ( Layout.Version < 1 )
    {
        for ( FSmartTableColumnLayout & Column : Layout.Columns )
        {
            Column.Width      = 0.0f;
            Column.bUserWidth = false;
        }
    }

    Layout.Version = FSmartTableLayout::CurrentVersion;

    return true;
}

bool USmartTableLayoutStore::LoadLayout_Implementation( FName TableId, FSmartTableLayout & OutLayout ) const
{

    UE_LOGFMT( LogSmartTablesLayout, Verbose, "{Store} leaves LoadLayout empty, and nothing comes back for '{Table}'. USmartTableConfigLayoutStore already fills it in.", GetClass()->GetName(), TableId );

    return false;
}

void USmartTableLayoutStore::SaveLayout_Implementation( FName TableId, const FSmartTableLayout & Layout )
{
    UE_LOGFMT( LogSmartTablesLayout, Verbose, "{Store} leaves SaveLayout empty, and the layout for '{Table}' goes nowhere.", GetClass()->GetName(), TableId );
}

namespace
{
    FString ConfigKey( FName TableId )
    {
        return TableId.ToString();
    }
}

const FString & USmartTableConfigLayoutStore::ResolvedConfigFile() const
{
    return ConfigFileName.IsEmpty() ? GGameUserSettingsIni : ConfigFileName;
}

bool USmartTableConfigLayoutStore::LoadLayout_Implementation( FName TableId, FSmartTableLayout & OutLayout ) const
{
    FString Serialised;
    if ( !GConfig->GetString( *ConfigSection, *ConfigKey( TableId ), Serialised, ResolvedConfigFile() ) )
    {
        UE_LOGFMT( LogSmartTablesLayout, Verbose, "'{Table}' has nothing filed in [{Section}] of {File}.", TableId, ConfigSection, ResolvedConfigFile() );
        return false;
    }

    const bool bParsed = FSmartTableLayout::StaticStruct()->ImportText( *Serialised, &OutLayout, nullptr, PPF_None, nullptr, FSmartTableLayout::StaticStruct()->GetName() ) != nullptr;

    if ( !bParsed )
    {

        UE_LOGFMT( LogSmartTablesLayout, Warning, "The layout filed for '{Table}' would not read back, so it goes and the columns fall back to how they were authored. Use Reset Columns on the header menu of the table to clear the bad entry, found under [{Section}] in {File}.", TableId, ConfigSection, ResolvedConfigFile() );

        return false;
    }

    UE_LOGFMT( LogSmartTablesLayout, Verbose, "A layout came back for '{Table}': {Columns} column override(s).", TableId, OutLayout.Columns.Num() );

    return true;
}

void USmartTableConfigLayoutStore::SaveLayout_Implementation( FName TableId, const FSmartTableLayout & Layout )
{
    FString Serialised;
    FSmartTableLayout::StaticStruct()->ExportText( Serialised, &Layout, nullptr, nullptr, PPF_None, nullptr );

    UE_LOGFMT( LogSmartTablesLayout, Verbose, "The layout for '{Table}' goes into [{Section}] of {File}: {Columns} column override(s).", TableId, ConfigSection, ResolvedConfigFile(), Layout.Columns.Num() );

    GConfig->SetString( *ConfigSection, *ConfigKey( TableId ), *Serialised, ResolvedConfigFile() );

    ScheduleFlush();
}

void USmartTableConfigLayoutStore::ScheduleFlush()
{

    constexpr float FlushDelaySeconds = 1.0f;

    PendingFlushFiles.Add( ResolvedConfigFile() );

    if ( FlushHandle.IsValid() )
    {
        FTSTicker::GetCoreTicker().RemoveTicker( FlushHandle );
    }

    FlushHandle = FTSTicker::GetCoreTicker().AddTicker( TEXT( "SmartTableLayoutFlush" ), FlushDelaySeconds, [ Store = TWeakObjectPtr< USmartTableConfigLayoutStore >( this ) ]( float )
    {
        if ( USmartTableConfigLayoutStore * Alive = Store.Get() )
        {
            Alive->FlushNow();
        }

        return false;
    } );
}

void USmartTableConfigLayoutStore::FlushNow()
{

    FlushHandle.Reset();

    if ( GConfig )
    {
        for ( const FString & File : PendingFlushFiles )
        {
            GConfig->Flush( false, File );
        }
    }

    PendingFlushFiles.Reset();
}

void USmartTableConfigLayoutStore::BeginDestroy()
{
    if ( FlushHandle.IsValid() )
    {
        FTSTicker::GetCoreTicker().RemoveTicker( FlushHandle );

        FlushNow();
    }

    Super::BeginDestroy();
}
