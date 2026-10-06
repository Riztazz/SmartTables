// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "View/SmartTableItemSetter.h"

#include "HAL/UnrealMemory.h"
#include "Misc/AssertionMacros.h"
#include "UObject/Class.h"
#include "UObject/Object.h"
#include "UObject/UnrealType.h"

namespace SmartTable
{
    namespace
    {

        FString WhyNotItemSetter( const UFunction & Function )
        {
            if ( !Function.HasAllFunctionFlags( FUNC_Public | FUNC_BlueprintCallable ) || Function.HasAnyFunctionFlags( FUNC_Static | FUNC_Delegate ) )
            {
                return TEXT( "is not a public function a graph can call" );
            }

            if ( Function.GetReturnProperty() )
            {
                return TEXT( "hands back a value" );
            }

            if ( Function.NumParms != 1 )
            {
                return FString::Printf( TEXT( "takes %d inputs and not one" ), Function.NumParms );
            }

            const FProperty * Input = *TFieldIterator< FProperty >( &Function );
            if ( Input->HasAnyPropertyFlags( CPF_OutParm ) )
            {
                return TEXT( "takes its input by reference" );
            }

            if ( !Input->IsA< FObjectProperty >() )
            {
                return FString::Printf( TEXT( "takes %s and not an object" ), *Input->GetCPPType() );
            }

            return FString();
        }

        const FObjectProperty & ItemSetterInput( const UFunction & Setter )
        {
            const FObjectProperty * Input = CastField< FObjectProperty >( *TFieldIterator< FProperty >( &Setter ) );
            checkf( Input, TEXT( "%s takes no object, so it never came out of ResolveItemSetter" ), *Setter.GetName() );

            return *Input;
        }
    }

    bool CanBeItemSetter( const UFunction & Function )
    {
        return WhyNotItemSetter( Function ).IsEmpty();
    }

    TValueOrError< UFunction *, FString > ResolveItemSetter( const UClass & Class, FName Name )
    {
        UFunction * Function = Class.FindFunctionByName( Name );
        if ( !Function )
        {
            return MakeError( FString( TEXT( "is no function of that class" ) ) );
        }

        FString WhyNot = WhyNotItemSetter( *Function );
        if ( !WhyNot.IsEmpty() )
        {
            return MakeError( MoveTemp( WhyNot ) );
        }

        return MakeValue( Function );
    }

    const UClass & ItemSetterInputClass( const UFunction & Setter )
    {
        const UClass * Taken = ItemSetterInput( Setter ).PropertyClass;
        checkf( Taken, TEXT( "%s takes an object of no class" ), *Setter.GetName() );

        return *Taken;
    }

    void CallItemSetter( UObject & Target, UFunction & Setter, UObject * Item )
    {

        checkf( !Item || Item->IsA( &ItemSetterInputClass( Setter ) ), TEXT( "%s was handed a %s, which it does not take" ), *Setter.GetName(), *Item->GetClass()->GetName() );

        uint8 * Parms = static_cast< uint8 * >( FMemory_Alloca_Aligned( Setter.ParmsSize, Setter.GetMinAlignment() ) );
        FMemory::Memzero( Parms, Setter.ParmsSize );

        ItemSetterInput( Setter ).SetObjectPropertyValue_InContainer( Parms, Item );

        Target.ProcessEvent( &Setter, Parms );
    }
}
