// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#pragma once

#include "Framework/Views/TableViewTypeTraits.h"

template<>
struct TIsValidListItem< int32 >
{
    enum
    {
        Value = true
    };
};

template<>
struct TListTypeTraits< int32 >
{
public:
    typedef int32 NullableType;

    using MapKeyFuncs       = TDefaultMapHashableKeyFuncs< int32, TSharedRef< ITableRow >, false >;
    using MapKeyFuncsSparse = TDefaultMapHashableKeyFuncs< int32, FSparseItemInfo, false >;
    using SetKeyFuncs       = DefaultKeyFuncs< int32 >;

    template< typename U >
    static void AddReferencedObjects( FReferenceCollector & Collector, TArray< int32 > & ItemsWithGeneratedWidgets, TSet< int32 > & SelectedItems, TMap< const U *, int32 > & WidgetToItemMap )
    {
    }

    static bool IsPtrValid( const int32 & InValue )
    {
        return InValue != INDEX_NONE;
    }

    static void ResetPtr( int32 & InValue )
    {
        InValue = INDEX_NONE;
    }

    static int32 MakeNullPtr()
    {
        return INDEX_NONE;
    }

    static int32 NullableItemTypeConvertToItemType( const int32 & InValue )
    {
        return InValue;
    }

    static FString DebugDump( int32 InValue )
    {
        return FString::FromInt( InValue );
    }

    class SerializerType
    {
    };
};
