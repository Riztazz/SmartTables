// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "SmartTable.h"
#include "SmartTableDetails.h"
#include "SmartTableSearchHighlightCustomization.h"
#include "SmartTableStyle.h"
#include "SmartTableStyleDetails.h"
#include "SmartTablesEditorStyle.h"

class FSmartTablesEditorModule : public IModuleInterface
{
public:
    virtual void StartupModule() override
    {
        FSmartTablesEditorStyle::Register();

        FPropertyEditorModule & PropertyEditor = FModuleManager::LoadModuleChecked< FPropertyEditorModule >( TEXT( "PropertyEditor" ) );

        PropertyEditor.RegisterCustomClassLayout( USmartTable::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic( &FSmartTableDetails::MakeInstance ) );
        PropertyEditor.RegisterCustomClassLayout( USmartTableStyle::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic( &FSmartTableStyleDetails::MakeInstance ) );
        PropertyEditor.RegisterCustomPropertyTypeLayout( FSmartTableSearchHighlight::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic( &FSmartTableSearchHighlightCustomization::MakeInstance ) );
    }

    virtual void ShutdownModule() override
    {
        if ( FPropertyEditorModule * PropertyEditor = FModuleManager::GetModulePtr< FPropertyEditorModule >( TEXT( "PropertyEditor" ) ) )
        {
            PropertyEditor->UnregisterCustomClassLayout( USmartTable::StaticClass()->GetFName() );
            PropertyEditor->UnregisterCustomClassLayout( USmartTableStyle::StaticClass()->GetFName() );
            PropertyEditor->UnregisterCustomPropertyTypeLayout( FSmartTableSearchHighlight::StaticStruct()->GetFName() );
        }

        FSmartTablesEditorStyle::Unregister();
    }
};

IMPLEMENT_MODULE( FSmartTablesEditorModule, SmartTablesEditor )
