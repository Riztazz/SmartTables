// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "SmartTableBinding.h"

#include "Internationalization/Culture.h"
#include "Internationalization/FastDecimalFormat.h"
#include "Internationalization/Internationalization.h"
#include "Logging/StructuredLog.h"
#include "Misc/AssertionMacros.h"
#include "SmartTableCell.h"
#include "SmartTableLog.h"
#include "SmartTableTypes.h"
#include "UObject/Class.h"
#include "UObject/StructOnScope.h"
#include "UObject/TextProperty.h"
#include "UObject/UnrealType.h"

SmartTable::EBoundKind SmartTable::Classify( const FProperty * Property )
{
    if ( CastField< FTextProperty >( Property ) )
    {
        return EBoundKind::Text;
    }

    if ( CastField< FStrProperty >( Property ) )
    {
        return EBoundKind::String;
    }

    if ( CastField< FNameProperty >( Property ) )
    {
        return EBoundKind::Name;
    }

    if ( CastField< FBoolProperty >( Property ) )
    {
        return EBoundKind::Bool;
    }

    if ( CastField< FEnumProperty >( Property ) )
    {
        return EBoundKind::Enum;
    }

    if ( const FNumericProperty * AsNumeric = CastField< FNumericProperty >( Property ) )
    {
        return AsNumeric->GetIntPropertyEnum() ? EBoundKind::Enum : EBoundKind::Number;
    }

    if ( CastField< FObjectPropertyBase >( Property ) )
    {
        return EBoundKind::Object;
    }

    return EBoundKind::Unsupported;
}

namespace
{

    bool IsBindableFunction( const UFunction * Function )
    {
        if ( !Function || !Function->HasAnyFunctionFlags( FUNC_BlueprintPure ) )
        {
            return false;
        }

        const FProperty * ReturnProperty = Function->GetReturnProperty();
        if ( !ReturnProperty )
        {
            return false;
        }

        return Function->NumParms == 1;
    }

    FText NumberToText( double Value, const FSmartTableColumn & Column )
    {
        FNumberFormattingOptions Options;
        Options.MaximumFractionalDigits = Column.MaxFractionalDigits;

        return FText::AsNumber( Value, &Options );
    }

    const UEnum * EnumBehind( const FProperty * Property, const void * Container, int64 & OutValue )
    {
        if ( const FEnumProperty * AsEnum = CastField< FEnumProperty >( Property ) )
        {
            OutValue = AsEnum->GetUnderlyingProperty()->GetSignedIntPropertyValue( AsEnum->ContainerPtrToValuePtr< void >( Container ) );

            return AsEnum->GetEnum();
        }

        const FNumericProperty * AsNumeric = CastFieldChecked< const FNumericProperty >( Property );
        OutValue                           = AsNumeric->GetSignedIntPropertyValue( AsNumeric->ContainerPtrToValuePtr< void >( Container ) );

        return AsNumeric->GetIntPropertyEnum();
    }

    bool PropertyAsText( const FProperty * Property, const void * Container, const FSmartTableColumn & Column, FText & OutText )
    {
        switch ( SmartTable::Classify( Property ) )
        {
            case SmartTable::EBoundKind::Text:
                OutText = CastFieldChecked< const FTextProperty >( Property )->GetPropertyValue_InContainer( Container );
                return true;

            case SmartTable::EBoundKind::String:
                OutText = FText::FromString( CastFieldChecked< const FStrProperty >( Property )->GetPropertyValue_InContainer( Container ) );
                return true;

            case SmartTable::EBoundKind::Name:
                OutText = FText::FromName( CastFieldChecked< const FNameProperty >( Property )->GetPropertyValue_InContainer( Container ) );
                return true;

            case SmartTable::EBoundKind::Bool:
                OutText = CastFieldChecked< const FBoolProperty >( Property )->GetPropertyValue_InContainer( Container ) ? NSLOCTEXT( "SmartTables", "True", "true" ) : NSLOCTEXT( "SmartTables", "False", "false" );
                return true;

            case SmartTable::EBoundKind::Enum:
            {

                int64 Value          = 0;
                const UEnum * Values = EnumBehind( Property, Container, Value );
                OutText              = Values->GetDisplayNameTextByValue( Value );
                return true;
            }

            case SmartTable::EBoundKind::Number:
            {
                const FNumericProperty * AsNumeric = CastFieldChecked< const FNumericProperty >( Property );
                const void * Value                 = AsNumeric->ContainerPtrToValuePtr< void >( Container );
                OutText                            = AsNumeric->IsFloatingPoint() ? NumberToText( AsNumeric->GetFloatingPointPropertyValue( Value ), Column ) : FText::AsNumber( AsNumeric->GetSignedIntPropertyValue( Value ) );
                return true;
            }

            case SmartTable::EBoundKind::Object:
            {
                const UObject * Value = CastFieldChecked< const FObjectPropertyBase >( Property )->GetObjectPropertyValue_InContainer( Container );
                OutText               = Value ? FText::FromString( Value->GetName() ) : FText::GetEmpty();
                return true;
            }

            default:
                return false;
        }
    }

    bool PropertyAsSortKey( const FProperty * Property, const void * Container, FSmartTableSortKey & OutKey )
    {
        switch ( SmartTable::Classify( Property ) )
        {
            case SmartTable::EBoundKind::Bool:
                OutKey = FSmartTableSortKey::MakeBool( CastFieldChecked< const FBoolProperty >( Property )->GetPropertyValue_InContainer( Container ) );
                return true;

            case SmartTable::EBoundKind::Enum:
            {

                int64 Value = 0;
                EnumBehind( Property, Container, Value );
                OutKey = FSmartTableSortKey::MakeNumber( static_cast< double >( Value ) );
                return true;
            }

            case SmartTable::EBoundKind::Number:
            {
                const FNumericProperty * AsNumeric = CastFieldChecked< const FNumericProperty >( Property );
                const void * Value                 = AsNumeric->ContainerPtrToValuePtr< void >( Container );
                OutKey                             = AsNumeric->IsFloatingPoint() ? FSmartTableSortKey::MakeNumber( AsNumeric->GetFloatingPointPropertyValue( Value ) ) : FSmartTableSortKey::MakeNumber( static_cast< double >( AsNumeric->GetSignedIntPropertyValue( Value ) ) );
                return true;
            }

            case SmartTable::EBoundKind::Text:
                OutKey = FSmartTableSortKey::MakeText( CastFieldChecked< const FTextProperty >( Property )->GetPropertyValue_InContainer( Container ).ToString() );
                return true;

            case SmartTable::EBoundKind::String:
                OutKey = FSmartTableSortKey::MakeText( CastFieldChecked< const FStrProperty >( Property )->GetPropertyValue_InContainer( Container ) );
                return true;

            case SmartTable::EBoundKind::Name:
                OutKey = FSmartTableSortKey::MakeText( CastFieldChecked< const FNameProperty >( Property )->GetPropertyValue_InContainer( Container ).ToString() );
                return true;

            case SmartTable::EBoundKind::Object:
            {
                const UObject * Value = CastFieldChecked< const FObjectPropertyBase >( Property )->GetObjectPropertyValue_InContainer( Container );
                OutKey                = Value ? FSmartTableSortKey::MakeText( Value->GetName() ) : FSmartTableSortKey::MakeEmpty();
                return true;
            }

            default:
                return false;
        }
    }

    struct FFunctionResult
    {
        explicit FFunctionResult( UFunction * InFunction )
            : Function( InFunction )
            , Parameters( InFunction )
        {
        }

        void Invoke( UObject * Item )
        {

            checkf( Item, TEXT( "Bound function '%s' has no object to run on" ), *Function->GetName() );

            Item->ProcessEvent( Function, Parameters.GetStructMemory() );
        }

        uint8 * Buffer()
        {
            return Parameters.GetStructMemory();
        }

        UFunction * Function = nullptr;

        FStructOnScope Parameters;
    };
}

SmartTable::FBinding SmartTable::ResolveBinding( const UStruct * Struct, FName Name )
{
    FBinding Binding;
    if ( !Struct || Name.IsNone() )
    {

        UE_LOGFMT( LogSmartTablesData, VeryVerbose, "Nothing to bind: {Reason}.", Struct ? TEXT( "the binding name is None" ) : TEXT( "there is no class or struct to resolve against" ) );
        return Binding;
    }

    if ( FProperty * Direct = Struct->FindPropertyByName( Name ) )
    {
        Binding.Property = Direct;
        return Binding;
    }

    const FString Wanted = Name.ToString();
    for ( TFieldIterator< FProperty > It( Struct ); It; ++It )
    {
        if ( It->GetAuthoredName().Equals( Wanted, ESearchCase::IgnoreCase ) )
        {
            Binding.Property = *It;
            return Binding;
        }
    }

    if ( const UClass * Class = Cast< UClass >( Struct ) )
    {
        if ( UFunction * Function = Class->FindFunctionByName( Name ) )
        {
            if ( IsBindableFunction( Function ) )
            {
                Binding.Function = Function;
            }
        }
    }

    return Binding;
}

TArray< FName > SmartTable::GetBindableNames( const UStruct * Struct )
{
    TArray< FName > Names;
    if ( !Struct )
    {
        return Names;
    }

    for ( TFieldIterator< FProperty > It( Struct ); It; ++It )
    {
        Names.Add( FName( *It->GetAuthoredName() ) );
    }

    if ( const UClass * Class = Cast< UClass >( Struct ) )
    {
        for ( TFieldIterator< UFunction > It( Class ); It; ++It )
        {
            if ( IsBindableFunction( *It ) )
            {
                Names.Add( It->GetFName() );
            }
        }
    }

    return Names;
}

FText SmartTable::ReadAsText( const FBinding & Binding, UObject * Item, const FSmartTableColumn & Column )
{
    if ( !Item )
    {
        UE_LOGFMT( LogSmartTablesData, VeryVerbose, "Column '{Column}' draws nothing. There is no item under it to read.", Column.ColumnId );
        return FText::GetEmpty();
    }

    if ( const FProperty * Property = Binding.Property.Get() )
    {
        FText Text;

        return PropertyAsText( Property, Item, Column, Text ) ? Text : NSLOCTEXT( "SmartTables", "UnsupportedType", "<unsupported>" );
    }

    UFunction * Function = Binding.Function.Get();
    if ( !Function )
    {

        UE_LOGFMT( LogSmartTablesData, VeryVerbose, "Column '{Column}' binds '{Binding}'. No live property and no live function answers to that name.", Column.ColumnId, Column.GetValueBinding() );
        return FText::GetEmpty();
    }

    FFunctionResult Result( Function );
    Result.Invoke( Item );

    const FProperty * ReturnProperty = Function->GetReturnProperty();

    if ( const FStructProperty * AsStruct = CastField< FStructProperty >( ReturnProperty ) )
    {
        if ( AsStruct->Struct == FSmartTableSortKey::StaticStruct() )
        {
            const FSmartTableSortKey * Key = ReturnProperty->ContainerPtrToValuePtr< FSmartTableSortKey >( Result.Buffer() );
            return Key->Kind == ESmartTableSortKeyKind::String ? FText::FromString( Key->Text ) : ( Key->IsEmpty() ? FText::GetEmpty() : NumberToText( Key->Number, Column ) );
        }
    }

    FText Text;
    return PropertyAsText( ReturnProperty, Result.Buffer(), Column, Text ) ? Text : NSLOCTEXT( "SmartTables", "UnsupportedType", "<unsupported>" );
}

FText SmartTable::ReadStructAsText( const FBinding & Binding, const void * RowData, const FSmartTableColumn & Column )
{
    const FProperty * Property = Binding.Property.Get();
    if ( !RowData || !Property )
    {
        UE_LOGFMT( LogSmartTablesData, VeryVerbose, "Column '{Column}' draws nothing: {Reason}.", Column.ColumnId, RowData ? TEXT( "its binding is not a field on the row struct" ) : TEXT( "there is no row data to read" ) );
        return FText::GetEmpty();
    }

    FText Text;

    return PropertyAsText( Property, RowData, Column, Text ) ? Text : NSLOCTEXT( "SmartTables", "UnsupportedType", "<unsupported>" );
}

FSmartTableSortKey SmartTable::ReadStructAsSortKey( const FBinding & Binding, const void * RowData )
{
    const FProperty * Property = Binding.Property.Get();
    if ( !RowData || !Property )
    {
        return FSmartTableSortKey::MakeEmpty();
    }

    FSmartTableSortKey Key;

    return PropertyAsSortKey( Property, RowData, Key ) ? Key : FSmartTableSortKey::MakeEmpty();
}

FSmartTableSortKey SmartTable::ReadAsSortKey( const FBinding & Binding, UObject * Item )
{
    if ( !Item )
    {
        return FSmartTableSortKey::MakeEmpty();
    }

    if ( const FProperty * Property = Binding.Property.Get() )
    {
        FSmartTableSortKey Key;

        return PropertyAsSortKey( Property, Item, Key ) ? Key : FSmartTableSortKey::MakeEmpty();
    }

    UFunction * Function = Binding.Function.Get();
    if ( !Function )
    {
        return FSmartTableSortKey::MakeEmpty();
    }

    FFunctionResult Result( Function );
    Result.Invoke( Item );

    const FProperty * ReturnProperty = Function->GetReturnProperty();

    if ( const FStructProperty * AsStruct = CastField< FStructProperty >( ReturnProperty ) )
    {
        if ( AsStruct->Struct == FSmartTableSortKey::StaticStruct() )
        {
            return *ReturnProperty->ContainerPtrToValuePtr< FSmartTableSortKey >( Result.Buffer() );
        }
    }

    FSmartTableSortKey Key;
    return PropertyAsSortKey( ReturnProperty, Result.Buffer(), Key ) ? Key : FSmartTableSortKey::MakeEmpty();
}

FString SmartTable::DescribeBindableNames( const UStruct * Struct )
{
    return FString::JoinBy( GetBindableNames( Struct ), TEXT( ", " ), []( FName Name )
    {
        return Name.ToString();
    } );
}

bool SmartTable::ShouldWarnForUnresolvedBinding( const FSmartTableColumn & Column, const FBinding & Value )
{
    if ( Value.IsValid() )
    {
        return false;
    }

    const bool bWidgetOnlyColumn = Column.CellClass && Column.BindingName.IsNone();

    return !bWidgetOnlyColumn;
}

bool SmartTable::ShouldWarnForUnresolvedSortBinding( const FSmartTableColumn & Column, const FBinding & Sort )
{
    return !Column.SortBindingName.IsNone() && !Sort.IsValid();
}

ESmartTableCellEditor SmartTable::EditorFor( const FBinding & Binding )
{
    const FProperty * Property = Binding.Property.Get();
    if ( !Property )
    {
        return ESmartTableCellEditor::None;
    }

    switch ( Classify( Property ) )
    {
        case EBoundKind::Bool:
            return ESmartTableCellEditor::Toggle;

        case EBoundKind::Number:
            return ESmartTableCellEditor::Number;

        case EBoundKind::Text:
        case EBoundKind::String:
        case EBoundKind::Name:
            return ESmartTableCellEditor::Text;

        default:
            return ESmartTableCellEditor::None;
    }
}

namespace
{
    bool WriteProperty( const FProperty * Property, void * Container, const FText & Value )
    {
        switch ( SmartTable::Classify( Property ) )
        {
            case SmartTable::EBoundKind::String:
                CastFieldChecked< const FStrProperty >( Property )->SetPropertyValue_InContainer( Container, Value.ToString() );
                return true;

            case SmartTable::EBoundKind::Name:
                CastFieldChecked< const FNameProperty >( Property )->SetPropertyValue_InContainer( Container, FName( *Value.ToString() ) );
                return true;

            case SmartTable::EBoundKind::Text:
                CastFieldChecked< const FTextProperty >( Property )->SetPropertyValue_InContainer( Container, Value );
                return true;

            case SmartTable::EBoundKind::Bool:

                CastFieldChecked< const FBoolProperty >( Property )->SetPropertyValue_InContainer( Container, Value.ToString().ToBool() );
                return true;

            case SmartTable::EBoundKind::Number:
            {
                const FNumericProperty * AsNumeric = CastFieldChecked< const FNumericProperty >( Property );

                const FDecimalNumberFormattingRules & Rules = FInternationalization::Get().GetCurrentCulture()->GetDecimalNumberFormattingRules();

                const FString Text = Value.ToString();

                double Number = 0.0;
                if ( !FastDecimalFormat::StringToNumber( *Text, Rules, FNumberParsingOptions::DefaultWithGrouping(), Number ) )
                {
                    return false;
                }

                void * Address = AsNumeric->ContainerPtrToValuePtr< void >( Container );
                if ( AsNumeric->IsFloatingPoint() )
                {
                    AsNumeric->SetFloatingPointPropertyValue( Address, Number );

                    return true;
                }

                if ( Text.Contains( FString::Chr( Rules.DecimalSeparatorCharacter ) ) )
                {
                    return false;
                }

                AsNumeric->SetIntPropertyValue( Address, static_cast< int64 >( Number ) );

                return true;
            }

            default:
                return false;
        }
    }
}

bool SmartTable::WriteAsText( const FBinding & Binding, UObject * Item, const FText & Value )
{

    if ( !Item || EditorFor( Binding ) == ESmartTableCellEditor::None )
    {
        return false;
    }

    return WriteProperty( Binding.Property.Get(), Item, Value );
}

bool SmartTable::WriteStructAsText( const FBinding & Binding, void * RowData, const FText & Value )
{
    if ( !RowData || EditorFor( Binding ) == ESmartTableCellEditor::None )
    {
        return false;
    }

    return WriteProperty( Binding.Property.Get(), RowData, Value );
}
