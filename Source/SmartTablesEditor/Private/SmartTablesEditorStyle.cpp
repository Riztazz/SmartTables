// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "SmartTablesEditorStyle.h"

#include "Interfaces/IPluginManager.h"
#include "Math/Vector2D.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleMacros.h"
#include "Styling/SlateStyleRegistry.h"

TSharedPtr< FSlateStyleSet > FSmartTablesEditorStyle::StyleSet;

namespace
{
    const FVector2D IconSize( 16.0f, 16.0f );
    const FVector2D ThumbnailSize( 64.0f, 64.0f );

    const TCHAR * IconedClasses[] = {
        TEXT( "SmartTable" ),
        TEXT( "SmartTableStyle" ),
        TEXT( "SmartTableCell" ),
        TEXT( "SmartTableModel" ),
    };
}

void FSmartTablesEditorStyle::Register()
{
    if ( StyleSet.IsValid() )
    {
        return;
    }

    const TSharedPtr< IPlugin > Plugin = IPluginManager::Get().FindPlugin( TEXT( "SmartTables" ) );
    checkf( Plugin.IsValid(), TEXT( "The editor module is loaded, so the plugin that owns it must be findable" ) );

    StyleSet = MakeShared< FSlateStyleSet >( TEXT( "SmartTablesEditorStyle" ) );

    StyleSet->SetContentRoot( Plugin->GetBaseDir() / TEXT( "Resources" ) );

    for ( const TCHAR * Name : IconedClasses )
    {
        StyleSet->Set( FName( *FString::Printf( TEXT( "ClassIcon.%s" ), Name ) ), new FSlateVectorImageBrush( StyleSet->RootToContentDir( Name, TEXT( ".svg" ) ), IconSize ) );

        StyleSet->Set( FName( *FString::Printf( TEXT( "ClassThumbnail.%s" ), Name ) ), new FSlateVectorImageBrush( StyleSet->RootToContentDir( Name, TEXT( ".svg" ) ), ThumbnailSize ) );
    }

    FSlateStyleRegistry::RegisterSlateStyle( *StyleSet );
}

void FSmartTablesEditorStyle::Unregister()
{
    if ( !StyleSet.IsValid() )
    {
        return;
    }

    FSlateStyleRegistry::UnRegisterSlateStyle( *StyleSet );
    StyleSet.Reset();
}
