// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#pragma once

#include "Containers/Array.h"
#include "Containers/ArrayView.h"
#include "Containers/Set.h"
#include "Internationalization/Text.h"
#include "SmartTableTypes.h"
#include "Templates/SubclassOf.h"
#include "UObject/NameTypes.h"
#include "UObject/SoftObjectPath.h"
#include "UObject/SoftObjectPtr.h"

class UDataTable;
class UStruct;

namespace SmartTable
{

    SMARTTABLES_API const UStruct * ResolveColumnSchema( ESmartTableColumnSource Source, const TSubclassOf< UObject > & ItemClass, const TSoftObjectPtr< UDataTable > & Table, FText & OutSchemaName, FText & OutProblem );

    SMARTTABLES_API TArray< FSmartTableColumn > ColumnsFromStruct( const UStruct * Struct );

    SMARTTABLES_API TArray< FSmartTableColumn > ColumnsFromCsv( const FString & Csv );

    SMARTTABLES_API TArray< FSmartTableColumn > ColumnsFromJson( const FString & Json );

    SMARTTABLES_API int32 AppendMissingColumns( TArray< FSmartTableColumn > & Columns, TArrayView< const FSmartTableColumn > Candidates );

    SMARTTABLES_API TSet< FName > DuplicatedColumnIds( TArrayView< const FSmartTableColumn > Columns );

#if WITH_EDITOR

    SMARTTABLES_API FText DescribeSourcesLost( ESmartTableColumnSource NewSource, const TSubclassOf< UObject > & ItemClass, const TSoftObjectPtr< UDataTable > & SourceTable, const FFilePath & SourceFile );

    SMARTTABLES_API FString ResolveSourceFile( const FFilePath & SourceFile );

    struct FUnresolvedBinding
    {
        FName ColumnId;
        FName Binding;
    };

    SMARTTABLES_API TArray< FUnresolvedBinding > FindUnresolvedBindings( const UStruct * Schema, TArrayView< const FSmartTableColumn > Columns );
#endif
}
