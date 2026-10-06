// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#pragma once

#include "Containers/UnrealString.h"
#include "UObject/ObjectMacros.h"
#include "SmartTableSortKey.generated.h"

UENUM( BlueprintType )
enum class ESmartTableSortKeyKind : uint8
{

    Empty,

    Numeric,

    String
};

USTRUCT( BlueprintType )
struct SMARTTABLES_API FSmartTableSortKey
{
    GENERATED_BODY()

    UPROPERTY( BlueprintReadWrite, Category = "Smart Tables" )
    ESmartTableSortKeyKind Kind = ESmartTableSortKeyKind::Empty;

    UPROPERTY( BlueprintReadWrite, Category = "Smart Tables" )
    double Number = 0.0;

    UPROPERTY( BlueprintReadWrite, Category = "Smart Tables" )
    FString Text;

    static FSmartTableSortKey MakeEmpty();
    static FSmartTableSortKey MakeNumber( double InNumber );

    static FSmartTableSortKey MakeBool( bool bInValue );

    static FSmartTableSortKey MakeText( const FString & InText );

    bool IsEmpty() const
    {
        return Kind == ESmartTableSortKeyKind::Empty;
    }

    int32 Compare( const FSmartTableSortKey & Other ) const;
};
