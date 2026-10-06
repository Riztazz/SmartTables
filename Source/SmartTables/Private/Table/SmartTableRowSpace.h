// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#pragma once

#include "Containers/Array.h"
#include "Containers/ArrayView.h"
#include "Containers/Set.h"
#include "Templates/Function.h"
#include "UObject/NameTypes.h"

namespace SmartTable::RowSpace
{

    SMARTTABLES_API int32 StepCaret( int32 Caret, int64 Delta, int32 NumPresented );

    SMARTTABLES_API int32 StepCaretByPage( int32 Caret, int32 PageRows, int32 PageDelta, int32 NumPresented );

    SMARTTABLES_API int32 GapForBlockMove( TConstArrayView< int32 > PresentedRows, int32 Delta, int32 NumPresented );

    SMARTTABLES_API int32 NaturalGapFor( int32 PresentedGap, int32 NumRows, TFunctionRef< int32( int32 ) > PresentedToNatural );

    SMARTTABLES_API TSet< FName > IdsOf( TConstArrayView< int32 > NaturalRows, TFunctionRef< FName( int32 ) > RowIdOf );

    struct FFoundRows
    {

        TArray< int32 > Rows;

        int32 Caret = INDEX_NONE;
    };

    SMARTTABLES_API FFoundRows FindByIds( TConstArrayView< int32 > Rows, const TSet< FName > & Ids, FName CaretId, TFunctionRef< FName( int32 ) > RowIdOf );

    SMARTTABLES_API FFoundRows FindByIdsInEveryRow( int32 NumRows, const TSet< FName > & Ids, FName CaretId, TFunctionRef< FName( int32 ) > RowIdOf );

    SMARTTABLES_API int32 FindIdNear( TConstArrayView< int32 > Rows, FName Id, int32 Around, TFunctionRef< FName( int32 ) > RowIdOf );

    SMARTTABLES_API int32 FindIdNearInEveryRow( int32 NumRows, FName Id, int32 Around, TFunctionRef< FName( int32 ) > RowIdOf );
}
