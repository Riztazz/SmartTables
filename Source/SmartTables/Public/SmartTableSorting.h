// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#pragma once

#include "Containers/Array.h"
#include "Containers/ArrayView.h"
#include "Containers/Set.h"
#include "SmartTableSortKey.h"
#include "SmartTableTypes.h"
#include "Templates/Function.h"
#include "UObject/NameTypes.h"

namespace SmartTable::Sorting
{

    SMARTTABLES_API int32 CompareNatural( const FString & A, const FString & B );

    SMARTTABLES_API void SortIndices( TArray< int32 > & InOutIndices, TArrayView< const TArray< FSmartTableSortKey > > KeyLevels, TArrayView< const ESmartTableSortMode > Modes );

    SMARTTABLES_API ESmartTableSortMode NextSortMode( ESmartTableSortMode Current, bool bAllowNone );

    SMARTTABLES_API FSmartTableSortSpec SpecWithPrimary( FName ColumnId, ESmartTableSortMode Mode );

    SMARTTABLES_API FSmartTableSortSpec SpecWithSecondary( const FSmartTableSortSpec & Current, FName ColumnId, ESmartTableSortMode Mode );

    SMARTTABLES_API bool HasLevelOtherThan( const FSmartTableSortSpec & Spec, FName ColumnId );

    SMARTTABLES_API TSet< FName > FindColumnsThatCannotBreakTies( int32 NumPresentedRows, FName PrimaryColumn, TArrayView< const FName > Candidates, TFunctionRef< FSmartTableSortKey( int32 PresentedRow, FName ColumnId ) > ReadKey );

    SMARTTABLES_API TArray< FName > SortableColumnIds( TArrayView< const FSmartTableColumn > Columns );
}
