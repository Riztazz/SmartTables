// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "SmartTableStyleNode.h"

#include "Customizations/SlateBrushCustomization.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Fonts/SlateFontInfo.h"
#include "IDetailChildrenBuilder.h"
#include "IDetailPropertyRow.h"
#include "IPropertyTypeCustomization.h"
#include "Misc/AssertionMacros.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "PropertyHandle.h"
#include "Styling/SlateBrush.h"
#include "UObject/UnrealType.h"

namespace
{
    class FSmartTableCustomizationUtils final : public IPropertyTypeCustomizationUtils
    {
    public:
        explicit FSmartTableCustomizationUtils( IDetailLayoutBuilder & Layout )
            : ThumbnailPool( Layout.GetThumbnailPool() )
            , PropertyUtilities( Layout.GetPropertyUtilities() )
        {
        }

        virtual TSharedPtr< FAssetThumbnailPool > GetThumbnailPool() const override
        {
            return ThumbnailPool;
        }

        virtual TSharedPtr< IPropertyUtilities > GetPropertyUtilities() const override
        {
            return PropertyUtilities;
        }

    private:
        TSharedPtr< FAssetThumbnailPool > ThumbnailPool;
        TSharedPtr< IPropertyUtilities > PropertyUtilities;
    };

    bool CarriesStyleWeight( const UStruct & Struct );

    bool PropertyCarriesStyleWeight( const FProperty & Property )
    {
        if ( const FStructProperty * StructProperty = CastField< FStructProperty >( &Property ) )
        {
            return StructProperty->Struct && CarriesStyleWeight( *StructProperty->Struct );
        }

        if ( const FArrayProperty * ArrayProperty = CastField< FArrayProperty >( &Property ) )
        {
            return ArrayProperty->Inner && PropertyCarriesStyleWeight( *ArrayProperty->Inner );
        }

        if ( const FSetProperty * SetProperty = CastField< FSetProperty >( &Property ) )
        {
            return SetProperty->ElementProp && PropertyCarriesStyleWeight( *SetProperty->ElementProp );
        }

        if ( const FMapProperty * MapProperty = CastField< FMapProperty >( &Property ) )
        {
            return ( MapProperty->KeyProp && PropertyCarriesStyleWeight( *MapProperty->KeyProp ) ) || ( MapProperty->ValueProp && PropertyCarriesStyleWeight( *MapProperty->ValueProp ) );
        }

        return false;
    }

    bool CarriesStyleWeight( const UStruct & Struct )
    {
        if ( &Struct == FSlateBrush::StaticStruct() || &Struct == FSlateFontInfo::StaticStruct() )
        {
            return true;
        }

        for ( TFieldIterator< FProperty > It( &Struct ); It; ++It )
        {
            if ( PropertyCarriesStyleWeight( **It ) )
            {
                return true;
            }
        }

        return false;
    }

    void AppendNestedFieldNames( const UStruct & Struct, int32 DepthLeft, FString & Out )
    {
        for ( TFieldIterator< FProperty > It( &Struct ); It; ++It )
        {
            Out.AppendChar( TEXT( ' ' ) );
            Out.Append( It->GetDisplayNameText().ToString() );

            const FStructProperty * StructProperty = CastField< FStructProperty >( *It );
            if ( DepthLeft > 1 && StructProperty && StructProperty->Struct )
            {
                AppendNestedFieldNames( *StructProperty->Struct, DepthLeft - 1, Out );
            }
        }
    }
}

FSmartTableStyleNode::FSmartTableStyleNode( TSharedRef< IPropertyHandle > InHandle, IDetailLayoutBuilder & InLayout )
    : Handle( InHandle )
    , DetailsView( InLayout.GetDetailsViewSharedPtr() )
{
    checkf( CastField< FStructProperty >( InHandle->GetProperty() ), TEXT( "FSmartTableStyleNode was built over a property that is not a struct - ShouldDefer is the gate and it refuses those" ) );
}

bool FSmartTableStyleNode::ShouldDefer( const FProperty & Property )
{
    const FStructProperty * StructProperty = CastField< FStructProperty >( &Property );
    if ( !StructProperty || !StructProperty->Struct )
    {
        return false;
    }

    if ( Property.HasMetaData( TEXT( "EditConditionHides" ) ) )
    {
        return false;
    }

    if ( StructProperty->Struct == FSlateBrush::StaticStruct() )
    {
        return true;
    }

    FPropertyEditorModule & PropertyEditor = FModuleManager::LoadModuleChecked< FPropertyEditorModule >( TEXT( "PropertyEditor" ) );
    if ( PropertyEditor.IsCustomizedStruct( StructProperty->Struct, FCustomPropertyTypeLayoutMap() ) )
    {
        return false;
    }

    return CarriesStyleWeight( *StructProperty->Struct );
}

void FSmartTableStyleNode::AddChild( IDetailChildrenBuilder & ChildrenBuilder, TSharedRef< IPropertyHandle > ChildHandle )
{
    const FProperty * Property = ChildHandle->GetProperty();

    if ( Property && ShouldDefer( *Property ) )
    {
        ChildrenBuilder.AddCustomBuilder( MakeShared< FSmartTableStyleNode >( ChildHandle, ChildrenBuilder.GetParentCategory().GetParentLayout() ) );
    }
    else
    {
        ChildrenBuilder.AddProperty( ChildHandle );
    }
}

void FSmartTableStyleNode::AddChildren( IDetailChildrenBuilder & ChildrenBuilder, const TSharedRef< IPropertyHandle > & Parent )
{
    uint32 NumChildren = 0;
    Parent->GetNumChildren( NumChildren );

    for ( uint32 Child = 0; Child < NumChildren; ++Child )
    {
        const TSharedPtr< IPropertyHandle > ChildHandle = Parent->GetChildHandle( Child );
        if ( ChildHandle.IsValid() && !ChildHandle->IsCustomized() )
        {
            AddChild( ChildrenBuilder, ChildHandle.ToSharedRef() );
        }
    }
}

FName FSmartTableStyleNode::GetName() const
{
    return FName( *( TEXT( "SmartTableStyle_" ) + Handle->GetProperty()->GetName() ) );
}

TSharedPtr< IPropertyHandle > FSmartTableStyleNode::GetPropertyHandle() const
{
    return Handle;
}

FText FSmartTableStyleNode::FilterText( const TSharedRef< IPropertyHandle > & Handle )
{
    FString FilterNames = Handle->GetPropertyDisplayName().ToString();

    const FStructProperty * StructProperty = CastField< FStructProperty >( Handle->GetProperty() );
    if ( StructProperty && StructProperty->Struct )
    {
        AppendNestedFieldNames( *StructProperty->Struct, 3, FilterNames );
    }

    return FText::FromString( FilterNames );
}

void FSmartTableStyleNode::GenerateHeaderRowContent( FDetailWidgetRow & NodeRow )
{
    const TSharedRef< IPropertyHandle > LocalHandle = Handle;

    NodeRow.FilterString( FilterText( Handle ) )
        .PropertyHandleList( { Handle } )
        .OverrideResetToDefault( FResetToDefaultOverride::Create( FIsResetToDefaultVisible::CreateLambda(
                                                                      []( TSharedPtr< IPropertyHandle > InHandle )
    {
        return InHandle.IsValid() && InHandle->DiffersFromDefault();
    } ),
            FResetToDefaultHandler::CreateLambda(
                []( TSharedPtr< IPropertyHandle > InHandle )
    {
        if ( InHandle.IsValid() )
        {
            InHandle->ResetToDefault();
        }
    } ) ) )
        .IsEnabled( TAttribute< bool >::CreateLambda(
            [ LocalHandle ]()
    {
        return LocalHandle->IsEditable();
    } ) )
        .NameContent()[ Handle->CreatePropertyNameWidget() ]
        .ValueContent()[

            Handle->CreatePropertyValueWidgetWithCustomization( DetailsView.Pin().Get() ) ];
}

void FSmartTableStyleNode::GenerateRealChildren( IDetailChildrenBuilder & ChildrenBuilder )
{
    const FStructProperty * StructProperty = CastField< FStructProperty >( Handle->GetProperty() );

    if ( StructProperty->Struct == FSlateBrush::StaticStruct() )
    {
        FSmartTableCustomizationUtils Utils( ChildrenBuilder.GetParentCategory().GetParentLayout() );
        BrushCustomization = FSlateBrushStructCustomization::MakeInstance( false );
        BrushCustomization->CustomizeChildren( Handle, ChildrenBuilder, Utils );
        return;
    }

    AddChildren( ChildrenBuilder, Handle );
}
