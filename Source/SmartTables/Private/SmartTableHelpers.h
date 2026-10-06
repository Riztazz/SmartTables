// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

#pragma once

#include "Containers/Array.h"
#include "Framework/Views/ITypedTableView.h"
#include "SmartTableTypes.h"
#include "UObject/NameTypes.h"

namespace SmartTable
{

    inline ESelectionMode::Type ToSlateSelectionMode( ESmartTableSelectionMode Mode )
    {
        switch ( Mode )
        {
            case ESmartTableSelectionMode::Single:
                return ESelectionMode::Single;

            case ESmartTableSelectionMode::Multi:
                return ESelectionMode::Multi;

            default:
                return ESelectionMode::None;
        }
    }

    inline bool IsAsciiDigit( TCHAR Character )
    {
        return Character >= TEXT( '0' ) && Character <= TEXT( '9' );
    }

    template< typename ElementType >
    ElementType * FindByColumnId( TArray< ElementType > & In, FName ColumnId )
    {
        return In.FindByPredicate( [ ColumnId ]( const ElementType & Candidate )
        {
            return Candidate.ColumnId == ColumnId;
        } );
    }

    template< typename ElementType >
    const ElementType * FindByColumnId( const TArray< ElementType > & In, FName ColumnId )
    {
        return In.FindByPredicate( [ ColumnId ]( const ElementType & Candidate )
        {
            return Candidate.ColumnId == ColumnId;
        } );
    }
}
