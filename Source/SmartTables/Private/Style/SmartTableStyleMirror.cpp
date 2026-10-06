// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#include "Style/SmartTableStyleMirror.h"

#include "SmartTable.h"
#include "SmartTableStyle.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"

namespace SmartTable::StyleMirror
{
    const TArray< FPair > & Pairs()
    {

        static const UClass * ReadFromStyle = nullptr;
        static const UClass * ReadFromTable = nullptr;
        static TArray< FPair > Mirrored;

        const UClass * StyleClass = USmartTableStyle::StaticClass();
        const UClass * TableClass = USmartTable::StaticClass();

        if ( ReadFromStyle == StyleClass && ReadFromTable == TableClass )
        {
            return Mirrored;
        }

        ReadFromStyle = StyleClass;
        ReadFromTable = TableClass;

        Mirrored.Reset();

        for ( FProperty * OnStyle = StyleClass->PropertyLink; OnStyle; OnStyle = OnStyle->PropertyLinkNext )
        {
            FProperty * OnTable = TableClass->FindPropertyByName( OnStyle->GetFName() );

            if ( OnTable && OnTable->SameType( OnStyle ) && OnTable->ArrayDim == OnStyle->ArrayDim )
            {
                Mirrored.Add( { OnStyle, OnTable } );
            }
        }

        return Mirrored;
    }

    void StyleToTable( const USmartTableStyle & From, USmartTable & To )
    {
        for ( const FPair & Pair : Pairs() )
        {

            Pair.OnTable->CopyCompleteValue( Pair.OnTable->ContainerPtrToValuePtr< void >( &To ), Pair.OnStyle->ContainerPtrToValuePtr< const void >( &From ) );
        }
    }

    void TableToStyle( const USmartTable & From, USmartTableStyle & To )
    {
        for ( const FPair & Pair : Pairs() )
        {
            Pair.OnStyle->CopyCompleteValue( Pair.OnStyle->ContainerPtrToValuePtr< void >( &To ), Pair.OnTable->ContainerPtrToValuePtr< const void >( &From ) );
        }
    }
}
