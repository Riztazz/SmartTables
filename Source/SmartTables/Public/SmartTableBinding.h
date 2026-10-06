// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#pragma once

#include "SmartTableSortKey.h"
#include "SmartTableTypes.h"
#include "UObject/WeakFieldPtr.h"

class UClass;
class UFunction;
struct FSmartTableColumn;

namespace SmartTable
{

    struct SMARTTABLES_API FBinding
    {
        TWeakFieldPtr< FProperty > Property;
        TWeakObjectPtr< UFunction > Function;

        bool IsValid() const
        {
            return Property.IsValid() || Function.IsValid();
        }
    };

    SMARTTABLES_API FBinding ResolveBinding( const UStruct * Struct, FName Name );

    SMARTTABLES_API TArray< FName > GetBindableNames( const UStruct * Struct );

    SMARTTABLES_API FString DescribeBindableNames( const UStruct * Struct );

    SMARTTABLES_API bool ShouldWarnForUnresolvedBinding( const FSmartTableColumn & Column, const FBinding & Value );

    SMARTTABLES_API bool ShouldWarnForUnresolvedSortBinding( const FSmartTableColumn & Column, const FBinding & Sort );

    SMARTTABLES_API FText ReadAsText( const FBinding & Binding, UObject * Item, const FSmartTableColumn & Column );

    SMARTTABLES_API FSmartTableSortKey ReadAsSortKey( const FBinding & Binding, UObject * Item );

    SMARTTABLES_API FText ReadStructAsText( const FBinding & Binding, const void * RowData, const FSmartTableColumn & Column );
    SMARTTABLES_API FSmartTableSortKey ReadStructAsSortKey( const FBinding & Binding, const void * RowData );

    enum class EBoundKind : uint8
    {
        Unsupported,
        Text,
        String,
        Name,
        Bool,
        Enum,
        Number,
        Object
    };

    SMARTTABLES_API EBoundKind Classify( const FProperty * Property );

    SMARTTABLES_API ESmartTableCellEditor EditorFor( const FBinding & Binding );

    SMARTTABLES_API bool WriteAsText( const FBinding & Binding, UObject * Item, const FText & Value );
    SMARTTABLES_API bool WriteStructAsText( const FBinding & Binding, void * RowData, const FText & Value );
}
