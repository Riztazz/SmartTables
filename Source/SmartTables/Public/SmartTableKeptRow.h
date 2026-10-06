// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#pragma once

#include "HAL/Platform.h"
#include "Misc/CoreMiscDefines.h"
#include "Templates/Function.h"
#include "UObject/NameTypes.h"

struct SMARTTABLES_API FSmartTableKeptRow
{
    FName RowId;

    int32 NaturalRow = INDEX_NONE;

    int32 FindNow( int32 NumRows, TFunctionRef< FName( int32 ) > RowIdOf ) const;
};
