// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#pragma once

#include "Containers/ArrayView.h"
#include "UObject/NameTypes.h"

struct SMARTTABLES_API FSmartTableColumnSelection
{

    FName Selected;

    int32 LastIndex = INDEX_NONE;

    bool Select( FName ColumnId, TArrayView< const FName > ShownIds );

    bool Move( int32 Delta, TArrayView< const FName > ShownIds );

    FName Resolve( FName ColumnId, TArrayView< const FName > ShownIds );

    bool Revalidate( TArrayView< const FName > ShownIds );
};
