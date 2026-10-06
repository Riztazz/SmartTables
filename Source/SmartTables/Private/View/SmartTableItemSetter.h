// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#pragma once

#include "Containers/UnrealString.h"
#include "Templates/ValueOrError.h"
#include "UObject/NameTypes.h"

class UClass;
class UFunction;
class UObject;

namespace SmartTable
{

    bool CanBeItemSetter( const UFunction & Function );

    TValueOrError< UFunction *, FString > ResolveItemSetter( const UClass & Class, FName Name );

    const UClass & ItemSetterInputClass( const UFunction & Setter );

    void CallItemSetter( UObject & Target, UFunction & Setter, UObject * Item );
}
